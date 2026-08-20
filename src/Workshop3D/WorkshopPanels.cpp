#include "WorkshopPanels.hpp"
#include "UI/UITooltipHelper.hpp"
#include "UI/WorkshopTooltips.hpp"
#include "Simulation/SimulationEngine.hpp"

using SZM::UITooltip::Hover;
using SZM::UITooltip::HelpMarker;
using SZM::UITooltip::SectionHint;
namespace WT = SZM::WorkshopTooltips;
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <imgui.h>
#include <limits>
#include <sstream>

namespace {

using SZM::Geometry::Vector3;

void EnsureWorkshopReady() {
    auto& workshop = SZM::Workshop3D::WorkshopManager::GetInstance();
    if (!workshop.IsInitialized()) {
        workshop.Init();
    }
}

std::string ToLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

float DensityFromMaterial(const std::string& materialName) {
    const std::string material = ToLowerCopy(materialName);
    if (material.find("steel") != std::string::npos) return 7850.0f;
    if (material.find("aluminum") != std::string::npos || material.find("aluminium") != std::string::npos) return 2700.0f;
    if (material.find("titanium") != std::string::npos) return 4500.0f;
    if (material.find("copper") != std::string::npos) return 8960.0f;
    return 1600.0f;
}

std::string BuildCncGCodePreview(
    const std::string& partName,
    const char* operation,
    double lengthMm,
    double widthMm,
    double depthMm,
    double toolDiameterMm,
    double stepdownMm,
    double stepoverMm,
    double feedRateMmMin,
    double spindleRpm,
    int& outPasses,
    float& outCutLengthMm
) {
    const double safeZ = 15.0;
    const double tool = std::max(0.5, toolDiameterMm);
    const double stepdown = std::max(0.1, stepdownMm);
    const double stepover = std::max(0.1, stepoverMm);
    const double feed = std::max(1.0, feedRateMmMin);
    const double xMin = tool * 0.5;
    const double yMin = tool * 0.5;
    const double xMax = std::max(xMin, lengthMm - tool * 0.5);
    const double yMax = std::max(yMin, widthMm - tool * 0.5);

    outPasses = std::max(1, static_cast<int>(std::ceil(std::max(0.1, depthMm) / stepdown)));
    double cutLength = 0.0;
    Vector3 last(xMin, yMin, safeZ);

    std::ostringstream gcode;
    gcode << std::fixed << std::setprecision(3);
    gcode << "(Program for: " << partName << ")\n";
    gcode << "(Operation: " << operation << ")\n";
    gcode << "(Tool: " << tool << " mm flat end mill)\n";
    gcode << "G21 (Metric units)\n";
    gcode << "G90 (Absolute positioning)\n";
    gcode << "G17 (XY plane)\n";
    gcode << "G54 (Work offset)\n";
    gcode << "S" << static_cast<int>(std::round(spindleRpm)) << " M03 (Spindle on clockwise)\n";
    gcode << "G00 Z" << safeZ << " (safe height)\n";

    const auto addCut = [&](double x, double y, double z, const char* comment) {
        const Vector3 next(x, y, z);
        cutLength += (next - last).Magnitude();
        last = next;
        gcode << "G01 X" << x << " Y" << y << " Z" << z << " F" << feed
              << " (" << comment << ")\n";
    };

    for (int pass = 1; pass <= outPasses; ++pass) {
        const double z = -std::min(static_cast<double>(pass) * stepdown, depthMm);
        gcode << "(Pass " << pass << "/" << outPasses << " Z" << z << ")\n";
        gcode << "G00 X" << xMin << " Y" << yMin << " (rapid to start)\n";
        gcode << "G01 Z" << z << " F" << std::max(1.0, feed * 0.25) << " (plunge)\n";
        last = Vector3(xMin, yMin, z);

        if (std::string(operation) == "profile") {
            addCut(xMax, yMin, z, "profile +X");
            addCut(xMax, yMax, z, "profile +Y");
            addCut(xMin, yMax, z, "profile -X");
            addCut(xMin, yMin, z, "profile -Y");
        } else {
            double y = yMin;
            int row = 0;
            while (y <= yMax + 1e-6) {
                addCut((row % 2 == 0) ? xMax : xMin, y, z, "raster cut");
                const double nextY = y + stepover;
                if (nextY <= yMax + 1e-6) {
                    addCut((row % 2 == 0) ? xMax : xMin, nextY, z, "stepover");
                }
                y = nextY;
                ++row;
            }
        }
    }

    gcode << "G00 Z" << safeZ << " (retract)\n";
    gcode << "M05 (Spindle stop)\n";
    gcode << "M30 (End program)\n";

    outCutLengthMm = static_cast<float>(cutLength);
    return gcode.str();
}

void CreatePrimitiveBox(SZM::Workshop3D::Part& part, const Vector3& halfExtents) {
    part.GetVertices() = {
        {-halfExtents.x, -halfExtents.y, -halfExtents.z},
        { halfExtents.x, -halfExtents.y, -halfExtents.z},
        { halfExtents.x,  halfExtents.y, -halfExtents.z},
        {-halfExtents.x,  halfExtents.y, -halfExtents.z},
        {-halfExtents.x, -halfExtents.y,  halfExtents.z},
        { halfExtents.x, -halfExtents.y,  halfExtents.z},
        { halfExtents.x,  halfExtents.y,  halfExtents.z},
        {-halfExtents.x,  halfExtents.y,  halfExtents.z}
    };

    part.GetIndices() = {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7,
        3, 2, 6, 3, 6, 7,
        4, 5, 1, 4, 1, 0
    };

    part.SetBounds(
        Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z),
        Vector3(halfExtents.x, halfExtents.y, halfExtents.z)
    );
}

ImU32 PackedColorToImU32(uint32_t color) {
    const int a = static_cast<int>((color >> 24U) & 0xFFU);
    const int r = static_cast<int>((color >> 16U) & 0xFFU);
    const int g = static_cast<int>((color >> 8U) & 0xFFU);
    const int b = static_cast<int>(color & 0xFFU);
    return IM_COL32(r, g, b, a);
}

Vector3 RotatePoint(Vector3 point, const Vector3& rotationDegrees) {
    const double rx = rotationDegrees.x * 3.14159265358979323846 / 180.0;
    const double ry = rotationDegrees.y * 3.14159265358979323846 / 180.0;
    const double rz = rotationDegrees.z * 3.14159265358979323846 / 180.0;

    const double cx = std::cos(rx);
    const double sx = std::sin(rx);
    const double cy = std::cos(ry);
    const double sy = std::sin(ry);
    const double cz = std::cos(rz);
    const double sz = std::sin(rz);

    Vector3 rotated = point;
    rotated = Vector3(
        rotated.x,
        rotated.y * cx - rotated.z * sx,
        rotated.y * sx + rotated.z * cx
    );
    rotated = Vector3(
        rotated.x * cy + rotated.z * sy,
        rotated.y,
        -rotated.x * sy + rotated.z * cy
    );
    rotated = Vector3(
        rotated.x * cz - rotated.y * sz,
        rotated.x * sz + rotated.y * cz,
        rotated.z
    );
    return rotated;
}

Vector3 ToViewSpace(
    const Vector3& point,
    const Vector3& cameraTarget,
    float yaw,
    float pitch
) {
    const Vector3 shifted = point - cameraTarget;
    const double cy = std::cos(yaw);
    const double sy = std::sin(yaw);
    const double cp = std::cos(pitch);
    const double sp = std::sin(pitch);

    Vector3 rotated(
        shifted.x * cy + shifted.z * sy,
        shifted.y,
        -shifted.x * sy + shifted.z * cy
    );
    rotated = Vector3(
        rotated.x,
        rotated.y * cp - rotated.z * sp,
        rotated.y * sp + rotated.z * cp
    );

    return rotated;
}

ImVec2 ProjectPoint(
    const Vector3& point,
    const ImVec2& center,
    float yaw,
    float pitch,
    float zoom,
    const Vector3& cameraTarget
) {
    const Vector3 rotated = ToViewSpace(point, cameraTarget, yaw, pitch);

    const double depth = rotated.z + 6.0;
    const double scale = zoom / std::max(1.0, depth);
    return ImVec2(
        center.x + static_cast<float>(rotated.x * scale),
        center.y - static_cast<float>(rotated.y * scale)
    );
}

float ProjectDepth(
    const Vector3& point,
    float yaw,
    float pitch,
    const Vector3& cameraTarget
) {
    return static_cast<float>(ToViewSpace(point, cameraTarget, yaw, pitch).z + 6.0);
}

struct ProjectedInstanceData {
    uint32_t instanceId = 0U;
    std::shared_ptr<SZM::Workshop3D::AssemblyInstance> instance;
    Vector3 centerWorld;
    std::array<Vector3, 8> cornersWorld{};
    std::array<ImVec2, 8> cornersScreen{};
    ImVec2 centerScreen = ImVec2(0.0f, 0.0f);
    ImVec2 minScreen = ImVec2(0.0f, 0.0f);
    ImVec2 maxScreen = ImVec2(0.0f, 0.0f);
    float depth = 0.0f;
};

ProjectedInstanceData ProjectInstance(
    uint32_t instanceId,
    const std::shared_ptr<SZM::Workshop3D::AssemblyInstance>& instance,
    const ImVec2& center,
    float yaw,
    float pitch,
    float zoom,
    const Vector3& cameraTarget
) {
    ProjectedInstanceData data;
    data.instanceId = instanceId;
    data.instance = instance;
    data.centerWorld = instance->GetPosition();
    data.centerScreen = ProjectPoint(data.centerWorld, center, yaw, pitch, zoom, cameraTarget);
    data.depth = ProjectDepth(data.centerWorld, yaw, pitch, cameraTarget);

    const Vector3 minBounds = instance->GetScaledBBoxMin();
    const Vector3 maxBounds = instance->GetScaledBBoxMax();
    const Vector3 rotation = instance->GetRotation();
    const Vector3 position = instance->GetPosition();

    data.cornersWorld = {
        Vector3(minBounds.x, minBounds.y, minBounds.z),
        Vector3(maxBounds.x, minBounds.y, minBounds.z),
        Vector3(maxBounds.x, maxBounds.y, minBounds.z),
        Vector3(minBounds.x, maxBounds.y, minBounds.z),
        Vector3(minBounds.x, minBounds.y, maxBounds.z),
        Vector3(maxBounds.x, minBounds.y, maxBounds.z),
        Vector3(maxBounds.x, maxBounds.y, maxBounds.z),
        Vector3(minBounds.x, maxBounds.y, maxBounds.z)
    };

    data.minScreen = ImVec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    data.maxScreen = ImVec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    for (size_t i = 0; i < data.cornersWorld.size(); ++i) {
        data.cornersWorld[i] = RotatePoint(data.cornersWorld[i], rotation) + position;
        data.cornersScreen[i] = ProjectPoint(data.cornersWorld[i], center, yaw, pitch, zoom, cameraTarget);
        data.minScreen.x = std::min(data.minScreen.x, data.cornersScreen[i].x);
        data.minScreen.y = std::min(data.minScreen.y, data.cornersScreen[i].y);
        data.maxScreen.x = std::max(data.maxScreen.x, data.cornersScreen[i].x);
        data.maxScreen.y = std::max(data.maxScreen.y, data.cornersScreen[i].y);
    }

    return data;
}

bool IsPointInsideProjectedBounds(const ImVec2& point, const ProjectedInstanceData& data, float padding = 8.0f) {
    return point.x >= data.minScreen.x - padding &&
           point.x <= data.maxScreen.x + padding &&
           point.y >= data.minScreen.y - padding &&
           point.y <= data.maxScreen.y + padding;
}

bool SolveScreenDelta(
    const ImVec2& basisA,
    const ImVec2& basisB,
    const ImVec2& delta,
    double& coeffA,
    double& coeffB
) {
    const double det = static_cast<double>(basisA.x) * basisB.y - static_cast<double>(basisA.y) * basisB.x;
    if (std::abs(det) < 1e-6) {
        coeffA = 0.0;
        coeffB = 0.0;
        return false;
    }

    coeffA = (static_cast<double>(delta.x) * basisB.y - static_cast<double>(delta.y) * basisB.x) / det;
    coeffB = (static_cast<double>(basisA.x) * delta.y - static_cast<double>(basisA.y) * delta.x) / det;
    return true;
}

Vector3 DragDeltaOnPlane(
    const Vector3& origin,
    const ImVec2& screenDelta,
    const ImVec2& center,
    float yaw,
    float pitch,
    float zoom,
    const Vector3& cameraTarget,
    const Vector3& basisA,
    const Vector3& basisB
) {
    const ImVec2 projectedOrigin = ProjectPoint(origin, center, yaw, pitch, zoom, cameraTarget);
    const ImVec2 projectedA = ProjectPoint(origin + basisA, center, yaw, pitch, zoom, cameraTarget);
    const ImVec2 projectedB = ProjectPoint(origin + basisB, center, yaw, pitch, zoom, cameraTarget);

    const ImVec2 screenBasisA(projectedA.x - projectedOrigin.x, projectedA.y - projectedOrigin.y);
    const ImVec2 screenBasisB(projectedB.x - projectedOrigin.x, projectedB.y - projectedOrigin.y);

    double amountA = 0.0;
    double amountB = 0.0;
    if (!SolveScreenDelta(screenBasisA, screenBasisB, screenDelta, amountA, amountB)) {
        return Vector3(0.0, 0.0, 0.0);
    }

    return basisA * amountA + basisB * amountB;
}

double VerticalDragDelta(
    const Vector3& origin,
    const ImVec2& screenDelta,
    const ImVec2& center,
    float yaw,
    float pitch,
    float zoom,
    const Vector3& cameraTarget
) {
    const ImVec2 projectedOrigin = ProjectPoint(origin, center, yaw, pitch, zoom, cameraTarget);
    const ImVec2 projectedUp = ProjectPoint(origin + Vector3(0.0, 1.0, 0.0), center, yaw, pitch, zoom, cameraTarget);
    const ImVec2 screenBasis(projectedUp.x - projectedOrigin.x, projectedUp.y - projectedOrigin.y);
    const double denom = static_cast<double>(screenBasis.x) * screenBasis.x +
                         static_cast<double>(screenBasis.y) * screenBasis.y;
    if (denom < 1e-6) {
        return 0.0;
    }

    return (static_cast<double>(screenDelta.x) * screenBasis.x +
            static_cast<double>(screenDelta.y) * screenBasis.y) / denom;
}

ImU32 ColorForInstance(
    uint32_t instanceId,
    const SZM::Workshop3D::AssemblyInstance& instance,
    const SZM::Workshop3D::WorkshopManager& workshop
) {
    const uint32_t componentId = workshop.GetSimulationComponentForInstance(instanceId);
    if (componentId != 0U) {
        if (auto* component = SZM::SimulationEngine::GetInstance().GetComponent(componentId)) {
            switch (workshop.GetVisualizationMode()) {
                case SZM::Graphics::Heatmap::Mode::Stress:
                    return IM_COL32(
                        static_cast<int>(255.0f * component->stressRatio),
                        static_cast<int>(220.0f * (1.0f - component->stressRatio)),
                        static_cast<int>(255.0f * (1.0f - component->stressRatio)),
                        255
                    );
                case SZM::Graphics::Heatmap::Mode::Temperature:
                    return IM_COL32(
                        static_cast<int>(255.0f * component->tempRatio),
                        80,
                        static_cast<int>(255.0f * (1.0f - component->tempRatio)),
                        255
                    );
                case SZM::Graphics::Heatmap::Mode::DangerIndicator:
                    return component->isDangerous ? IM_COL32(230, 70, 70, 255) : IM_COL32(70, 200, 110, 255);
                case SZM::Graphics::Heatmap::Mode::None:
                default:
                    break;
            }
        }
    }

    return PackedColorToImU32(instance.GetPart()->GetProperties().color);
}

void DrawPartArtifactList(const SZM::Workshop3D::Part& part) {
    const auto& artifacts = part.GetArtifactPaths();
    if (artifacts.empty()) {
        ImGui::TextDisabled("No artifacts indexed.");
        return;
    }

    for (const auto& artifact : artifacts) {
        ImGui::BulletText("%s", artifact.c_str());
    }
}

} // namespace

namespace SZM::Workshop3D {

    PartLibraryPanel::PartLibraryPanel() {
        EnsureWorkshopReady();
    }

    void PartLibraryPanel::Draw() {
        EnsureWorkshopReady();
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        DrawToolbar();
        ImGui::Separator();
        DrawPartList();
        ImGui::Separator();
        DrawSelectedPartCard();

        ImGui::End();
    }

    void PartLibraryPanel::DrawToolbar() {
        auto& workshop = WorkshopManager::GetInstance();

        SectionHint("Parts from codes/ FEA examples and external CAD. Hover (?) and controls for help.");
        ImGui::InputTextWithHint("Search", "Filter by name, category, or material", m_SearchFilter, sizeof(m_SearchFilter));
        Hover(WT::LibrarySearch);
        ImGui::TextDisabled("code 8: %s", workshop.GetCode8Directory().empty() ? "not found" : workshop.GetCode8Directory().c_str());
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
            ImGui::SetTooltip("%s", WT::LibraryCode8Path);
        }

        if (ImGui::Button("Reload code 8", ImVec2(-1, 0)) && !workshop.GetCode8Directory().empty()) {
            workshop.ImportCode8Examples(workshop.GetCode8Directory());
        }
        Hover(WT::LibraryReloadCode8);

        ImGui::InputTextWithHint("Import Path", "OBJ / STL / GEO / STEP file path", m_ImportPath, sizeof(m_ImportPath));
        Hover(WT::LibraryImportPath);
        if (ImGui::Button("Import External Part", ImVec2(-1, 0))) {
            workshop.ImportPart(m_ImportPath);
        }
        Hover(WT::LibraryImportButton);
    }

    void PartLibraryPanel::DrawPartList() {
        auto& workshop = WorkshopManager::GetInstance();
        std::vector<std::shared_ptr<Part>> parts;
        parts.reserve(workshop.GetPartLibrary().GetAllParts().size());
        for (const auto& [id, part] : workshop.GetPartLibrary().GetAllParts()) {
            (void)id;
            parts.push_back(part);
        }

        std::sort(parts.begin(), parts.end(), [](const auto& lhs, const auto& rhs) {
            return lhs->GetProperties().name < rhs->GetProperties().name;
        });

        const std::string filter = ToLowerCopy(m_SearchFilter);

        ImGui::Text("Indexed Parts: %zu", parts.size());
        ImGui::BeginChild("WorkshopPartList", ImVec2(0, 220), true);
        for (const auto& part : parts) {
            const auto& props = part->GetProperties();
            const std::string haystack = ToLowerCopy(props.name + " " + props.category + " " + props.material + " " + props.description);
            if (!filter.empty() && haystack.find(filter) == std::string::npos) {
                continue;
            }

            const bool selected = workshop.GetSelectedPartId() == part->GetID();
            if (ImGui::Selectable((props.name + "##part").c_str(), selected)) {
                workshop.SetSelectedPart(part->GetID());
            }
            ImGui::SameLine();
            ImGui::TextDisabled("[%s]", props.category.c_str());
        }
        ImGui::EndChild();
    }

    void PartLibraryPanel::DrawSelectedPartCard() {
        auto& workshop = WorkshopManager::GetInstance();
        auto part = workshop.GetSelectedPart();
        if (!part) {
            ImGui::TextDisabled("Select a part to inspect it here.");
            return;
        }

        const auto& props = part->GetProperties();
        ImGui::Text("%s", props.name.c_str());
        ImGui::TextDisabled("%s | %s", props.category.c_str(), props.material.c_str());
        ImGui::Separator();
        ImGui::TextWrapped("%s", props.description.empty() ? "No description available." : props.description.c_str());
        ImGui::Spacing();
        ImGui::BulletText("Source: %s", props.sourcePath.c_str());
        ImGui::BulletText("Preview: %s", props.previewPath.empty() ? "none" : props.previewPath.c_str());
        ImGui::BulletText("Reference: %s", props.referencePath.empty() ? "none" : props.referencePath.c_str());
        ImGui::BulletText("Bounds: %.2f x %.2f x %.2f m",
                          part->GetBBoxMax().x - part->GetBBoxMin().x,
                          part->GetBBoxMax().y - part->GetBBoxMin().y,
                          part->GetBBoxMax().z - part->GetBBoxMin().z);
        ImGui::BulletText("Mass estimate: %.2f kg", props.mass);

        if (ImGui::Button("Auto Place In Assembly", ImVec2(-1, 0))) {
            workshop.AddPartToActiveAssembly(part->GetID(), workshop.SuggestPlacement(*part));
        }
        Hover(WT::LibraryAutoPlace);

        if (ImGui::CollapsingHeader("Artifacts", ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawPartArtifactList(*part);
        }
    }

    PartEditorPanel::PartEditorPanel() {
        EnsureWorkshopReady();
    }

    void PartEditorPanel::Draw() {
        EnsureWorkshopReady();
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        DrawPrimitiveCreator();
        ImGui::Separator();
        DrawSelectedPartDetails();

        ImGui::End();
    }

    void PartEditorPanel::DrawPrimitiveCreator() {
        auto& workshop = WorkshopManager::GetInstance();

        SectionHint("Parametric blocks for rapid machine layout before detailed CAD import.");
        ImGui::Text("Create Workshop Primitive");
        HelpMarker(WT::EditorCreateBlock);
        ImGui::InputText("Part Name", m_PartName, sizeof(m_PartName));
        Hover(WT::EditorPartName);
        ImGui::InputText("Material", m_MaterialName, sizeof(m_MaterialName));
        Hover(WT::EditorMaterial);

        if (ImGui::CollapsingHeader("Material Synthesis (Plan 25)")) {
            const char* recipes[] = {
                "Al-Mg-Si CNC Alloy",
                "Ti-C Lightweight Composite",
                "Fe-Cr-Ni Heat Resistant Alloy"
            };
            ImGui::Combo("Recipe", &m_SynthesisPreset, recipes, IM_ARRAYSIZE(recipes));

            const char* synthName = "Synth Al-Mg-Si CNC Alloy";
            const char* composition = "Al 86% / Mg 8% / Si 6%";
            m_SynthDensity = 2730.0f;
            m_SynthYieldStrengthMPa = 355.0f;
            m_SynthThermalConductivity = 155.0f;

            if (m_SynthesisPreset == 1) {
                synthName = "Synth Ti-C Lightweight Composite";
                composition = "Ti 78% / C 18% / Al 4%";
                m_SynthDensity = 4110.0f;
                m_SynthYieldStrengthMPa = 720.0f;
                m_SynthThermalConductivity = 38.0f;
            } else if (m_SynthesisPreset == 2) {
                synthName = "Synth Fe-Cr-Ni Heat Alloy";
                composition = "Fe 62% / Cr 22% / Ni 16%";
                m_SynthDensity = 7900.0f;
                m_SynthYieldStrengthMPa = 540.0f;
                m_SynthThermalConductivity = 24.0f;
            }

            ImGui::TextDisabled("%s", composition);
            ImGui::BulletText("Density: %.0f kg/m^3", m_SynthDensity);
            ImGui::BulletText("Yield: %.0f MPa", m_SynthYieldStrengthMPa);
            ImGui::BulletText("Thermal k: %.1f W/(m*K)", m_SynthThermalConductivity);
            ImGui::Checkbox("Use Synthesized Material For New Block", &m_UseSynthesizedMaterial);

            if (ImGui::Button("Synthesize & Apply Material", ImVec2(-1, 0))) {
                std::snprintf(m_MaterialName, sizeof(m_MaterialName), "%s", synthName);
                m_UseSynthesizedMaterial = true;
            }
        }

        ImGui::InputTextMultiline("Description", m_Description, sizeof(m_Description), ImVec2(-1, 70));
        Hover(WT::EditorDescription);
        ImGui::DragFloat3("Size (m)", m_Size, 0.01f, 0.05f, 5.0f, "%.2f");
        Hover(WT::EditorSize);
        ImGui::Checkbox("Add To Assembly After Create", &m_AddToAssemblyAfterCreate);
        Hover(WT::EditorAddToAssembly);

        if (ImGui::Button("Create Parametric Block", ImVec2(-1, 0))) {
            auto part = workshop.GetPartLibrary().CreatePart(m_PartName);
            auto props = part->GetProperties();
            props.name = m_PartName;
            props.material = m_MaterialName;
            props.description = m_Description;
            props.category = "Custom";
            props.analysisType = "Workshop";
            props.sourcePath = "Generated in app";
            props.density = m_UseSynthesizedMaterial ? m_SynthDensity : DensityFromMaterial(m_MaterialName);
            props.color = 0xFFD49B54U;
            part->SetProperties(props);

            CreatePrimitiveBox(*part, Vector3(m_Size[0] * 0.5, m_Size[1] * 0.5, m_Size[2] * 0.5));
            part->CalculateMass(props.density);

            workshop.SetSelectedPart(part->GetID());

            if (m_AddToAssemblyAfterCreate) {
                workshop.AddPartToActiveAssembly(part->GetID(), workshop.SuggestPlacement(*part));
            }
        }
        Hover(WT::EditorCreateBlock);
    }

    void PartEditorPanel::DrawSelectedPartDetails() {
        auto& workshop = WorkshopManager::GetInstance();
        auto part = workshop.GetSelectedPart();
        if (!part) {
            ImGui::TextDisabled("Select a library item or create a new primitive to inspect it.");
            return;
        }

        const auto& props = part->GetProperties();
        ImGui::Text("Selected Part");
        ImGui::BulletText("Name: %s", props.name.c_str());
        ImGui::BulletText("Category: %s", props.category.c_str());
        ImGui::BulletText("Material: %s", props.material.c_str());
        ImGui::BulletText("Density: %.0f kg/m^3", props.density);
        ImGui::BulletText("Mass: %.2f kg", props.mass);
        ImGui::BulletText("Radius: %.2f m", part->GetBBoxRadius());
        ImGui::TextWrapped("%s", props.description.empty() ? "No notes available." : props.description.c_str());
    }

    AssemblyPanel::AssemblyPanel() {
        EnsureWorkshopReady();
    }

    void AssemblyPanel::Draw() {
        EnsureWorkshopReady();
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        DrawAssemblySelector();
        ImGui::Separator();
        DrawPlacementTools();
        ImGui::Separator();
        DrawInstanceList();
        ImGui::Separator();
        DrawSelectedInstanceEditor();

        ImGui::End();
    }

    void AssemblyPanel::DrawAssemblySelector() {
        auto& workshop = WorkshopManager::GetInstance();
        auto activeAssembly = workshop.GetActiveAssembly();

        ImGui::InputText("Assembly Name", m_AssemblyName, sizeof(m_AssemblyName));
        Hover(WT::AssemblyName);
        if (ImGui::Button("Create Assembly", ImVec2(-1, 0))) {
            const UUID id = workshop.CreateAssembly(m_AssemblyName);
            workshop.GetAssemblyManager().SetActiveAssembly(id);
        }
        Hover(WT::AssemblyCreate);

        if (activeAssembly) {
            ImGui::Text("Active: %s", activeAssembly->GetName().c_str());
            ImGui::TextDisabled("Instances: %zu", activeAssembly->GetInstanceCount());
        }
    }

    void AssemblyPanel::DrawPlacementTools() {
        auto& workshop = WorkshopManager::GetInstance();
        const auto& partsMap = workshop.GetPartLibrary().GetAllParts();
        if (workshop.GetSelectedPartId().IsNull() && !partsMap.empty()) {
            workshop.SetSelectedPart(partsMap.begin()->first);
        }
        auto part = workshop.GetSelectedPart();

        ImGui::Text("Placement");
        if (!partsMap.empty()) {
            const char* preview = part ? part->GetProperties().name.c_str() : "Select a part";
            if (ImGui::BeginCombo("Part", preview)) {
                for (const auto& [partId, candidate] : partsMap) {
                    const bool selected = part && part->GetID() == partId;
                    if (ImGui::Selectable(candidate->GetProperties().name.c_str(), selected)) {
                        workshop.SetSelectedPart(partId);
                        part = workshop.GetSelectedPart();
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            Hover(WT::AssemblyPartCombo);
        }

        ImGui::Checkbox("Auto Placement", &m_AutoPlacement);
        Hover(WT::AssemblyAutoPlacement);
        ImGui::DragFloat3("Position", m_Placement, 0.05f, -10.0f, 10.0f, "%.2f");
        Hover(WT::AssemblyPosition);
        ImGui::InputText("Instance Name", m_InstanceName, sizeof(m_InstanceName));
        Hover(WT::AssemblyInstanceName);

        if (!part) {
            ImGui::TextDisabled("Pick or create a part first.");
            return;
        }

        if (ImGui::Button("Add Selected Part", ImVec2(-1, 0))) {
            const std::string instanceName = m_InstanceName[0] == '\0' ? std::string() : std::string(m_InstanceName);
            const Vector3 placement = m_AutoPlacement
                ? workshop.SuggestPlacement(*part)
                : Vector3(m_Placement[0], m_Placement[1], m_Placement[2]);
            workshop.AddPartToActiveAssembly(
                part->GetID(),
                placement,
                instanceName
            );
            m_Placement[0] = static_cast<float>(placement.x);
            m_Placement[1] = static_cast<float>(placement.y);
            m_Placement[2] = static_cast<float>(placement.z);
        }
        Hover(WT::AssemblyAddPart);
    }

    void AssemblyPanel::DrawInstanceList() {
        auto& workshop = WorkshopManager::GetInstance();
        auto assembly = workshop.GetActiveAssembly();
        if (!assembly) {
            ImGui::TextDisabled("No active assembly.");
            return;
        }

        ImGui::BeginChild("AssemblyInstances", ImVec2(0, 200), true);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
            ImGui::SetTooltip("%s", WT::AssemblyInstanceList);
        }
        for (const auto& [instanceId, instance] : assembly->GetAllInstances()) {
            const bool selected = workshop.GetSelectedInstanceId() == instanceId;
            if (ImGui::Selectable((instance->GetInstanceName() + "##instance").c_str(), selected)) {
                workshop.SetSelectedInstance(instanceId);
                const auto& position = instance->GetPosition();
                const auto& rotation = instance->GetRotation();
                const auto& scale = instance->GetScale();
                m_Placement[0] = static_cast<float>(position.x);
                m_Placement[1] = static_cast<float>(position.y);
                m_Placement[2] = static_cast<float>(position.z);
                m_Rotation[0] = static_cast<float>(rotation.x);
                m_Rotation[1] = static_cast<float>(rotation.y);
                m_Rotation[2] = static_cast<float>(rotation.z);
                m_Scale[0] = static_cast<float>(scale.x);
                m_Scale[1] = static_cast<float>(scale.y);
                m_Scale[2] = static_cast<float>(scale.z);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("%s", instance->GetPart()->GetProperties().category.c_str());
        }
        ImGui::EndChild();
    }

    void AssemblyPanel::DrawSelectedInstanceEditor() {
        auto& workshop = WorkshopManager::GetInstance();
        auto assembly = workshop.GetActiveAssembly();
        auto instance = workshop.GetSelectedInstance();
        if (!assembly || !instance) {
            ImGui::TextDisabled("Select an instance to edit its transform.");
            return;
        }

        ImGui::Text("Selected: %s", instance->GetInstanceName().c_str());
        const auto& position = instance->GetPosition();
        const auto& rotation = instance->GetRotation();
        const auto& scale = instance->GetScale();
        m_Placement[0] = static_cast<float>(position.x);
        m_Placement[1] = static_cast<float>(position.y);
        m_Placement[2] = static_cast<float>(position.z);
        m_Rotation[0] = static_cast<float>(rotation.x);
        m_Rotation[1] = static_cast<float>(rotation.y);
        m_Rotation[2] = static_cast<float>(rotation.z);
        m_Scale[0] = static_cast<float>(scale.x);
        m_Scale[1] = static_cast<float>(scale.y);
        m_Scale[2] = static_cast<float>(scale.z);

        const bool positionChanged = ImGui::DragFloat3(
            "Instance Position", m_Placement, 0.05f, -10.0f, 10.0f, "%.2f");
        Hover(WT::AssemblyInstancePosition);
        const bool rotationChanged = ImGui::DragFloat3(
            "Instance Rotation", m_Rotation, 1.0f, -180.0f, 180.0f, "%.0f");
        Hover(WT::AssemblyInstanceRotation);
        if (positionChanged || rotationChanged) {
            instance->SetTransform(
                Vector3(m_Placement[0], m_Placement[1], m_Placement[2]),
                Vector3(m_Rotation[0], m_Rotation[1], m_Rotation[2])
            );
        }

        if (ImGui::DragFloat3("Instance Scale", m_Scale, 0.01f, 0.05f, 10.0f, "%.2f")) {
            instance->SetScale(Vector3(m_Scale[0], m_Scale[1], m_Scale[2]));
        }
        Hover(WT::AssemblyInstanceScale);

        const Vector3 scaledSize = instance->GetScaledSize();
        ImGui::TextDisabled("Scaled size: %.2f x %.2f x %.2f m",
                            scaledSize.x, scaledSize.y, scaledSize.z);
        ImGui::TextDisabled("Scaled mass estimate: %.2f kg", instance->GetScaledMass());

        if (ImGui::Button(instance->IsVisible() ? "Hide Instance" : "Show Instance", ImVec2(-1, 0))) {
            instance->SetVisible(!instance->IsVisible());
        }
        Hover(WT::AssemblyHideInstance);

        if (ImGui::Button("Remove Instance", ImVec2(-1, 0))) {
            assembly->RemoveInstance(workshop.GetSelectedInstanceId());
            workshop.SetSelectedInstance(0U);
        }
        Hover(WT::AssemblyRemoveInstance);
    }

    WorkshopViewportPanel::WorkshopViewportPanel() {
        EnsureWorkshopReady();
    }

    void WorkshopViewportPanel::Draw() {
        EnsureWorkshopReady();
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        HelpMarker(WT::ViewportCanvas);
        ImGui::SliderFloat("Yaw", &m_CameraYaw, -3.14f, 3.14f, "%.2f");
        Hover(WT::ViewportYaw);
        ImGui::SliderFloat("Pitch", &m_CameraPitch, -1.2f, 1.2f, "%.2f");
        Hover(WT::ViewportPitch);
        ImGui::SliderFloat("Zoom", &m_Zoom, 60.0f, 460.0f, "%.0f");
        Hover(WT::ViewportZoom);
        ImGui::Checkbox("Grid", &m_ShowGrid);
        Hover(WT::ViewportGrid);
        ImGui::SameLine();
        ImGui::Checkbox("Labels", &m_ShowLabels);
        Hover(WT::ViewportLabels);
        ImGui::SameLine();
        ImGui::Checkbox("Auto Rotate", &m_AutoRotate);
        Hover(WT::ViewportAutoRotate);
        ImGui::SameLine();
        ImGui::Checkbox("Help", &m_ShowControlsHint);
        Hover(WT::ViewportHelp);

        if (ImGui::Button("Frame Selection")) {
            if (auto instance = WorkshopManager::GetInstance().GetSelectedInstance()) {
                m_CameraTarget = instance->GetPosition();
                m_Zoom = std::clamp(
                    180.0f + static_cast<float>(instance->GetScaledRadius()) * 120.0f,
                    80.0f,
                    460.0f
                );
            }
        }
        Hover(WT::ViewportFrameSelection);
        ImGui::SameLine();
        if (ImGui::Button("Reset Camera")) {
            m_CameraYaw = 0.75f;
            m_CameraPitch = 0.45f;
            m_Zoom = 220.0f;
            m_CameraTarget = Vector3(0.0, 0.0, 0.0);
        }
        Hover(WT::ViewportResetCamera);

        DrawViewport();
        ImGui::End();
    }

    void WorkshopViewportPanel::DrawViewport() {
        auto& workshop = WorkshopManager::GetInstance();
        auto assembly = workshop.GetActiveAssembly();
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const ImVec2 requestedCanvasSize(std::max(avail.x, 80.0f), std::max(avail.y, 120.0f));
        ImGui::InvisibleButton("WorkshopCanvas", requestedCanvasSize);
        const ImVec2 canvasPos = ImGui::GetItemRectMin();
        const ImVec2 canvasMax = ImGui::GetItemRectMax();
        const ImVec2 canvasSize(canvasMax.x - canvasPos.x, canvasMax.y - canvasPos.y);
        const bool canvasHovered = ImGui::IsItemHovered();
        const bool canvasActive = ImGui::IsItemActive();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(canvasPos, canvasMax, IM_COL32(20, 24, 30, 255), 6.0f);
        drawList->AddRect(canvasPos, canvasMax, IM_COL32(60, 68, 78, 255), 6.0f);

        if (m_AutoRotate && m_InteractionMode == InteractionMode::None) {
            m_CameraYaw += ImGui::GetIO().DeltaTime * 0.45f;
        }

        const ImVec2 center(canvasPos.x + canvasSize.x * 0.5f, canvasPos.y + canvasSize.y * 0.56f);
        const ImVec2 mousePos = ImGui::GetMousePos();
        const Vector3 planeBasisX(1.0, 0.0, 0.0);
        const Vector3 planeBasisZ(0.0, 0.0, 1.0);

        auto buildProjectedInstances = [&]() {
            std::vector<ProjectedInstanceData> projectedInstances;
            if (!assembly) {
                return projectedInstances;
            }

            projectedInstances.reserve(assembly->GetAllInstances().size());
            for (const auto& [instanceId, instance] : assembly->GetAllInstances()) {
                if (!instance->IsVisible()) {
                    continue;
                }

                projectedInstances.push_back(ProjectInstance(
                    instanceId,
                    instance,
                    center,
                    m_CameraYaw,
                    m_CameraPitch,
                    m_Zoom,
                    m_CameraTarget
                ));
            }
            return projectedInstances;
        };

        auto projectedInstances = buildProjectedInstances();

        m_HoveredInstanceId = 0U;
        m_HoveredHandleIndex = -1;
        if (canvasHovered && !projectedInstances.empty()) {
            float bestDepth = std::numeric_limits<float>::max();
            float bestDistance = std::numeric_limits<float>::max();
            for (const auto& data : projectedInstances) {
                if (!IsPointInsideProjectedBounds(mousePos, data)) {
                    continue;
                }

                const float dx = mousePos.x - data.centerScreen.x;
                const float dy = mousePos.y - data.centerScreen.y;
                const float distance = std::sqrt(dx * dx + dy * dy);
                if (data.depth < bestDepth || (std::abs(data.depth - bestDepth) < 1e-4f && distance < bestDistance)) {
                    bestDepth = data.depth;
                    bestDistance = distance;
                    m_HoveredInstanceId = data.instanceId;
                }
            }

            if (const uint32_t selectedId = workshop.GetSelectedInstanceId(); selectedId != 0U) {
                for (const auto& data : projectedInstances) {
                    if (data.instanceId != selectedId) {
                        continue;
                    }

                    float bestHandleDistance = 14.0f;
                    for (int i = 0; i < static_cast<int>(data.cornersScreen.size()); ++i) {
                        const float dx = mousePos.x - data.cornersScreen[static_cast<size_t>(i)].x;
                        const float dy = mousePos.y - data.cornersScreen[static_cast<size_t>(i)].y;
                        const float distance = std::sqrt(dx * dx + dy * dy);
                        if (distance < bestHandleDistance) {
                            bestHandleDistance = distance;
                            m_HoveredHandleIndex = i;
                        }
                    }
                    break;
                }
            }
        }

        if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_LastMousePos = mousePos;

            if (m_HoveredHandleIndex >= 0) {
                const auto selectedId = workshop.GetSelectedInstanceId();
                if (auto instance = workshop.GetSelectedInstance()) {
                    for (const auto& data : projectedInstances) {
                        if (data.instanceId != selectedId) {
                            continue;
                        }

                        workshop.SetSelectedInstance(selectedId);
                        m_InteractionMode = InteractionMode::ResizeInstance;
                        m_InteractionInstanceId = selectedId;
                        m_DragStartScale = instance->GetScale();
                        const ImVec2 handlePos = data.cornersScreen[static_cast<size_t>(m_HoveredHandleIndex)];
                        const float dx = handlePos.x - data.centerScreen.x;
                        const float dy = handlePos.y - data.centerScreen.y;
                        m_DragStartHandleDistance = std::max(8.0f, std::sqrt(dx * dx + dy * dy));
                        break;
                    }
                }
            } else if (m_HoveredInstanceId != 0U) {
                workshop.SetSelectedInstance(m_HoveredInstanceId);
                if (auto instance = workshop.GetSelectedInstance()) {
                    m_InteractionMode = InteractionMode::DragInstance;
                    m_InteractionInstanceId = m_HoveredInstanceId;
                    m_DragStartPosition = instance->GetPosition();
                }
            } else {
                workshop.SetSelectedInstance(0U);
                m_InteractionMode = InteractionMode::None;
                m_InteractionInstanceId = 0U;
            }
        }

        if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            m_InteractionMode = InteractionMode::OrbitCamera;
            m_LastMousePos = mousePos;
        }

        if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
            m_InteractionMode = InteractionMode::PanCamera;
            m_LastMousePos = mousePos;
        }

        if (canvasHovered) {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                m_Zoom = std::clamp(m_Zoom + wheel * 18.0f, 60.0f, 460.0f);
            }
        }

        const ImVec2 mouseDelta(mousePos.x - m_LastMousePos.x, mousePos.y - m_LastMousePos.y);
        switch (m_InteractionMode) {
            case InteractionMode::OrbitCamera:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    m_CameraYaw += mouseDelta.x * 0.01f;
                    m_CameraPitch = std::clamp(m_CameraPitch - mouseDelta.y * 0.01f, -1.2f, 1.2f);
                    m_LastMousePos = mousePos;
                } else {
                    m_InteractionMode = InteractionMode::None;
                }
                break;

            case InteractionMode::PanCamera:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                    const Vector3 panDelta = DragDeltaOnPlane(
                        m_CameraTarget,
                        mouseDelta,
                        center,
                        m_CameraYaw,
                        m_CameraPitch,
                        m_Zoom,
                        m_CameraTarget,
                        planeBasisX,
                        planeBasisZ
                    );
                    m_CameraTarget = m_CameraTarget - panDelta;
                    m_LastMousePos = mousePos;
                } else {
                    m_InteractionMode = InteractionMode::None;
                }
                break;

            case InteractionMode::DragInstance:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    if (auto instance = assembly ? assembly->GetInstance(m_InteractionInstanceId) : nullptr) {
                        const auto& rotation = instance->GetRotation();
                        Vector3 position = instance->GetPosition();
                        if (ImGui::GetIO().KeyShift) {
                            position.y += VerticalDragDelta(
                                position,
                                mouseDelta,
                                center,
                                m_CameraYaw,
                                m_CameraPitch,
                                m_Zoom,
                                m_CameraTarget
                            );
                        } else {
                            position = position + DragDeltaOnPlane(
                                position,
                                mouseDelta,
                                center,
                                m_CameraYaw,
                                m_CameraPitch,
                                m_Zoom,
                                m_CameraTarget,
                                planeBasisX,
                                planeBasisZ
                            );
                        }
                        instance->SetTransform(position, rotation);
                    }
                    m_LastMousePos = mousePos;
                } else {
                    m_InteractionMode = InteractionMode::None;
                    m_InteractionInstanceId = 0U;
                }
                break;

            case InteractionMode::ResizeInstance:
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    if (auto instance = assembly ? assembly->GetInstance(m_InteractionInstanceId) : nullptr) {
                        projectedInstances = buildProjectedInstances();
                        for (const auto& data : projectedInstances) {
                            if (data.instanceId != m_InteractionInstanceId) {
                                continue;
                            }

                            const float dx = mousePos.x - data.centerScreen.x;
                            const float dy = mousePos.y - data.centerScreen.y;
                            const float distance = std::max(8.0f, std::sqrt(dx * dx + dy * dy));
                            const float scaleFactor = std::clamp(distance / std::max(8.0f, m_DragStartHandleDistance), 0.1f, 20.0f);
                            instance->SetScale(m_DragStartScale * scaleFactor);
                            break;
                        }
                    }
                } else {
                    m_InteractionMode = InteractionMode::None;
                    m_InteractionInstanceId = 0U;
                }
                break;

            case InteractionMode::None:
            default:
                break;
        }

        HandleViewportKeyboard(canvasPos, canvasSize);
        projectedInstances = buildProjectedInstances();

        if (m_ShowGrid) {
            for (int line = -6; line <= 6; ++line) {
                const Vector3 a(-3.0, 0.0, static_cast<double>(line) * 0.5);
                const Vector3 b( 3.0, 0.0, static_cast<double>(line) * 0.5);
                const Vector3 c(static_cast<double>(line) * 0.5, 0.0, -3.0);
                const Vector3 d(static_cast<double>(line) * 0.5, 0.0,  3.0);
                drawList->AddLine(ProjectPoint(a, center, m_CameraYaw, m_CameraPitch, m_Zoom, m_CameraTarget),
                                  ProjectPoint(b, center, m_CameraYaw, m_CameraPitch, m_Zoom, m_CameraTarget),
                                  IM_COL32(42, 48, 56, 255), 1.0f);
                drawList->AddLine(ProjectPoint(c, center, m_CameraYaw, m_CameraPitch, m_Zoom, m_CameraTarget),
                                  ProjectPoint(d, center, m_CameraYaw, m_CameraPitch, m_Zoom, m_CameraTarget),
                                  IM_COL32(42, 48, 56, 255), 1.0f);
            }
        }

        if (!assembly || projectedInstances.empty()) {
            drawList->AddText(ImVec2(canvasPos.x + 16.0f, canvasPos.y + 16.0f),
                              IM_COL32(180, 185, 195, 255),
                              "No assembly instances yet. Add a part from Workshop Library.");
            return;
        }

        static const int edges[][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        std::sort(projectedInstances.begin(), projectedInstances.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.depth > rhs.depth;
        });

        for (const auto& projected : projectedInstances) {
            ImU32 color = ColorForInstance(projected.instanceId, *projected.instance, workshop);
            const bool selected = workshop.GetSelectedInstanceId() == projected.instanceId;
            const bool hovered = m_HoveredInstanceId == projected.instanceId;
            const float thickness = selected ? 3.0f : 2.0f;
            if (selected) {
                color = IM_COL32(255, 214, 102, 255);
            } else if (hovered) {
                color = IM_COL32(121, 193, 255, 255);
            }

            for (const auto& edge : edges) {
                drawList->AddLine(
                    projected.cornersScreen[static_cast<size_t>(edge[0])],
                    projected.cornersScreen[static_cast<size_t>(edge[1])],
                    color,
                    thickness
                );
            }

            if (m_ShowLabels) {
                const Vector3 labelAnchor = projected.instance->GetPosition() +
                                            Vector3(0.0, projected.instance->GetScaledBBoxMax().y + 0.08, 0.0);
                const ImVec2 labelPos = ProjectPoint(
                    labelAnchor,
                    center,
                    m_CameraYaw,
                    m_CameraPitch,
                    m_Zoom,
                    m_CameraTarget
                );
                drawList->AddText(labelPos, IM_COL32(220, 225, 235, 255), projected.instance->GetInstanceName().c_str());
            }
        }

        if (workshop.GetSelectedInstance()) {
            for (const auto& projected : projectedInstances) {
                if (projected.instanceId != workshop.GetSelectedInstanceId()) {
                    continue;
                }

                for (int i = 0; i < static_cast<int>(projected.cornersScreen.size()); ++i) {
                    const ImVec2 handleCenter = projected.cornersScreen[static_cast<size_t>(i)];
                    const float radius = m_HoveredHandleIndex == i ? 6.0f : 4.5f;
                    drawList->AddCircleFilled(
                        handleCenter,
                        radius,
                        m_HoveredHandleIndex == i ? IM_COL32(255, 244, 140, 255) : IM_COL32(255, 214, 102, 235)
                    );
                    drawList->AddCircle(handleCenter, radius, IM_COL32(40, 40, 40, 255), 0, 1.0f);
                }

                drawList->AddCircle(projected.centerScreen, 6.0f, IM_COL32(121, 193, 255, 255), 0, 2.0f);
                break;
            }
        }

        if (m_ShowControlsHint) {
            const char* status = "LMB: select/drag | corner handle: resize | Shift+drag: move vertically | RMB: orbit | MMB: pan | wheel: zoom | arrows/PageUp/PageDown/Q/E/Z/X: keyboard transform";
            drawList->AddText(
                ImVec2(canvasPos.x + 12.0f, canvasMax.y - 22.0f),
                IM_COL32(170, 178, 188, 255),
                status
            );
        }

        if (canvasHovered || canvasActive) {
            drawList->AddText(
                ImVec2(canvasPos.x + 12.0f, canvasPos.y + 12.0f),
                IM_COL32(190, 198, 210, 255),
                workshop.GetSelectedInstanceId() == 0U ? "Viewport ready: click a component to move or resize it." :
                                                         "Viewport active: drag selected component or use keyboard shortcuts."
            );
        }
    }

    void WorkshopViewportPanel::HandleViewportKeyboard(const ImVec2& canvasPos, const ImVec2& canvasSize) {
        auto& workshop = WorkshopManager::GetInstance();
        auto instance = workshop.GetSelectedInstance();
        if (!instance || m_InteractionMode != InteractionMode::None) {
            return;
        }

        if (ImGui::GetIO().WantTextInput || ImGui::IsAnyItemActive()) {
            return;
        }

        const ImVec2 canvasMax(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);
        const bool canvasHovered = ImGui::IsMouseHoveringRect(canvasPos, canvasMax, false);
        const bool windowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        if (!canvasHovered && !windowFocused) {
            return;
        }

        const float dt = std::max(ImGui::GetIO().DeltaTime, 1.0f / 240.0f);
        const float moveStep = (ImGui::GetIO().KeyShift ? 3.0f : 1.0f) * dt;
        const float rotationStep = 90.0f * dt;
        const float scaleStep = 1.2f * dt;

        Vector3 position = instance->GetPosition();
        Vector3 rotation = instance->GetRotation();
        Vector3 scale = instance->GetScale();
        bool transformChanged = false;
        bool scaleChanged = false;

        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
            position.x -= moveStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
            position.x += moveStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
            position.z -= moveStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
            position.z += moveStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_PageUp)) {
            position.y += moveStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_PageDown)) {
            position.y -= moveStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_Q)) {
            rotation.y -= rotationStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_E)) {
            rotation.y += rotationStep;
            transformChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_Z)) {
            scale = scale * std::max(0.25f, 1.0f - scaleStep);
            scaleChanged = true;
        }
        if (ImGui::IsKeyDown(ImGuiKey_X)) {
            scale = scale * (1.0f + scaleStep);
            scaleChanged = true;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_F, false)) {
            m_CameraTarget = instance->GetPosition();
            m_Zoom = std::clamp(
                180.0f + static_cast<float>(instance->GetScaledRadius()) * 120.0f,
                80.0f,
                460.0f
            );
        }

        if (transformChanged) {
            instance->SetTransform(position, rotation);
        }
        if (scaleChanged) {
            instance->SetScale(scale);
        }
    }

    SimulationSetupPanel::SimulationSetupPanel() {
        EnsureWorkshopReady();
    }

    void SimulationSetupPanel::Draw() {
        EnsureWorkshopReady();
        if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
            ImGui::End();
            return;
        }

        DrawSimulationSetup();
        ImGui::Separator();
        DrawSelectedInstanceLoads();
        ImGui::Separator();
        DrawSimulationResults();

        ImGui::End();
    }

    void SimulationSetupPanel::DrawSimulationSetup() {
        auto& workshop = WorkshopManager::GetInstance();
        ImGui::Text("Simulation Bridge");
        ImGui::BulletText("Parts: %zu", workshop.GetPartCount());
        ImGui::BulletText("Assemblies: %zu", workshop.GetAssemblyCount());
        ImGui::BulletText("Instances: %zu", workshop.GetInstanceCount());

        HelpMarker(WT::CodesFEA);
        ImGui::DragFloat("Duration (s)", &m_SimulationDuration, 0.1f, 0.1f, 20.0f, "%.1f");
        Hover(WT::SimDuration);
        ImGui::DragFloat("Timestep (s)", &m_TimeStep, 0.001f, 0.001f, 0.1f, "%.3f");
        Hover(WT::SimTimestep);

        const char* modes[] = { "Stress", "Temperature", "Danger", "None" };
        if (ImGui::Combo("Viewport Heatmap", &m_VisualizationMode, modes, IM_ARRAYSIZE(modes))) {
            Graphics::Heatmap::Mode mode = Graphics::Heatmap::Mode::Stress;
            switch (m_VisualizationMode) {
                case 1: mode = Graphics::Heatmap::Mode::Temperature; break;
                case 2: mode = Graphics::Heatmap::Mode::DangerIndicator; break;
                case 3: mode = Graphics::Heatmap::Mode::None; break;
                default: break;
            }
            workshop.UpdateVisualization("", mode);
        }
        Hover(WT::SimHeatmap);

        if (ImGui::Button("Sync Assembly To Simulation", ImVec2(-1, 0))) {
            workshop.SetupSimulation();
        }
        Hover(WT::SimSync);

        if (ImGui::Button("Run Workshop Simulation", ImVec2(-1, 0))) {
            workshop.SetupSimulation();
            const int steps = std::max(1, static_cast<int>(m_SimulationDuration / std::max(0.001f, m_TimeStep)));
            for (int step = 0; step < steps; ++step) {
                SimulationEngine::GetInstance().Tick(m_TimeStep);
            }
        }
        Hover(WT::SimRun);
    }

    void SimulationSetupPanel::DrawSelectedInstanceLoads() {
        auto& workshop = WorkshopManager::GetInstance();
        auto instance = workshop.GetSelectedInstance();
        if (!instance) {
            ImGui::TextDisabled("Pick an assembly instance to apply force and heat.");
            return;
        }

        const uint32_t componentId = workshop.GetSimulationComponentForInstance(workshop.GetSelectedInstanceId());
        if (componentId == 0U) {
            ImGui::TextDisabled("Selected instance is not mapped yet. Run Sync Assembly To Simulation.");
            return;
        }

        auto* component = SimulationEngine::GetInstance().GetComponent(componentId);
        if (!component) {
            ImGui::TextDisabled("Mapped simulation component is unavailable.");
            return;
        }

        ImGui::Text("Loads for %s", instance->GetInstanceName().c_str());
        m_AppliedForce = component->appliedForce;
        m_HeatInput = component->heatInput;

        if (ImGui::SliderFloat("Force (N)", &m_AppliedForce, 0.0f, 250000.0f, "%.0f")) {
            SimulationEngine::GetInstance().SetForce(componentId, m_AppliedForce);
        }
        Hover(WT::SimForce);
        if (ImGui::SliderFloat("Heat Input (W)", &m_HeatInput, 0.0f, 2500.0f, "%.1f")) {
            SimulationEngine::GetInstance().SetHeatInput(componentId, m_HeatInput);
        }
        Hover(WT::SimHeat);
    }

    void SimulationSetupPanel::DrawSimulationResults() {
        auto& workshop = WorkshopManager::GetInstance();
        auto assembly = workshop.GetActiveAssembly();
        if (!assembly || assembly->GetAllInstances().empty()) {
            ImGui::TextDisabled("No assembly data available.");
            return;
        }

        if (!ImGui::BeginTable("WorkshopResults", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
            return;
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip)) {
            ImGui::SetTooltip("%s", WT::SimResultsTable);
        }

        ImGui::TableSetupColumn("Instance");
        ImGui::TableSetupColumn("Stress");
        ImGui::TableSetupColumn("Temp");
        ImGui::TableSetupColumn("Force");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        for (const auto& [instanceId, instance] : assembly->GetAllInstances()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(instance->GetInstanceName().c_str());

            const uint32_t componentId = workshop.GetSimulationComponentForInstance(instanceId);
            auto* component = componentId == 0U ? nullptr : SimulationEngine::GetInstance().GetComponent(componentId);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(component ? "%.2f MPa" : "--", component ? component->stress / 1e6f : 0.0f);

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(component ? "%.1f C" : "--", component ? component->temperature - 273.15f : 0.0f);

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(component ? "%.0f N" : "--", component ? component->appliedForce : 0.0f);

            ImGui::TableSetColumnIndex(4);
            if (!component) {
                ImGui::TextDisabled("Not synced");
            } else if (component->isDangerous) {
                ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), "Warning");
            } else {
                ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "Stable");
            }
        }

        ImGui::EndTable();
    }

} // namespace SZM::Workshop3D
