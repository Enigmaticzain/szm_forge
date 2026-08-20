#include "SimulationVisualizerPanel.hpp"
#include "../../Input/EventBus.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

SimulationVisualizerPanel::SimulationVisualizerPanel() {
    SZM::EventBus::GetInstance().Subscribe(
        "SimulationTick",
        [this](const std::any& /*payload*/) { this->OnSimulationTick(); }
    );
}

SimulationVisualizerPanel::~SimulationVisualizerPanel() {}

void SimulationVisualizerPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputTextWithHint("##search", "Search components...", m_SearchFilter, sizeof(m_SearchFilter));
    ImGui::SameLine();
    
    RenderVisualizationControls();
    ImGui::Separator();

    ImGui::BeginChild("ViewportArea", ImVec2(ImGui::GetContentRegionAvail().x * 0.75f, 0), true);
    RenderViewport();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("DetailsPanel", ImVec2(0, 0), true);
    RenderComponentList();
    ImGui::EndChild();

    ImGui::End();
}

void SimulationVisualizerPanel::RenderVisualizationControls() {
    if (ImGui::Checkbox("Stress##viz", &m_ShowStressHeatmap)) {
        if (m_ShowStressHeatmap) m_ShowTemperatureHeatmap = false;
    }
    ImGui::SameLine();
    
    if (ImGui::Checkbox("Temp##viz", &m_ShowTemperatureHeatmap)) {
        if (m_ShowTemperatureHeatmap) m_ShowStressHeatmap = false;
    }
    ImGui::SameLine();
    
    ImGui::Checkbox("Wire##viz", &m_ShowWireframe);
    ImGui::SameLine();
    
    ImGui::Checkbox("Labels##viz", &m_ShowComponentLabels);
    ImGui::SameLine();
    
    ImGui::Checkbox("Danger##viz", &m_ShowDangerIndicators);
}

void SimulationVisualizerPanel::RenderViewport() {
    auto* scene = SZM::SimulationEngine::GetInstance().GetScene();
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f) {
        return;
    }

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 p1 = ImVec2(p0.x + viewportSize.x, p0.y + viewportSize.y);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Background
    drawList->AddRectFilled(p0, p1, IM_COL32(32, 36, 44, 255));
    drawList->AddRect(p0, p1, IM_COL32(80, 88, 100, 255));

    // Grid background
    const float gridSpacing = 40.0f;
    for (float x = p0.x; x < p1.x; x += gridSpacing) {
        drawList->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), IM_COL32(50, 55, 65, 100), 0.5f);
    }
    for (float y = p0.y; y < p1.y; y += gridSpacing) {
        drawList->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), IM_COL32(50, 55, 65, 100), 0.5f);
    }

    // Center crosshair
    const ImVec2 center = ImVec2((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
    drawList->AddLine(ImVec2(center.x - 10, center.y), ImVec2(center.x + 10, center.y), IM_COL32(100, 150, 200, 150), 1.0f);
    drawList->AddLine(ImVec2(center.x, center.y - 10), ImVec2(center.x, center.y + 10), IM_COL32(100, 150, 200, 150), 1.0f);

    if (!scene) {
        drawList->AddText(
            ImVec2(p0.x + 16.0f, p0.y + 16.0f),
            IM_COL32(180, 185, 195, 255),
            "No Scene loaded."
        );
    } else {
        auto entities = scene->View<SZM::SceneGraph::PhysicsStateComponent, SZM::SceneGraph::TransformComponent>();
        if (entities.empty()) {
            drawList->AddText(
                ImVec2(p0.x + 16.0f, p0.y + 16.0f),
                IM_COL32(180, 185, 195, 255),
                "No components. Add one from the toolbar."
            );
        } else {
            // Render components
            for (auto e : entities) {
                std::string name = "Entity_" + std::to_string(static_cast<uint32_t>(e));
                if (scene->HasComponent<SZM::SceneGraph::TagComponent>(e)) {
                    name = scene->GetComponent<SZM::SceneGraph::TagComponent>(e).name;
                }

                // Filter by search
                if (m_SearchFilter[0] != '\0' && name.find(m_SearchFilter) == std::string::npos) {
                    continue;
                }
                
                const auto& physics = scene->GetComponent<SZM::SceneGraph::PhysicsStateComponent>(e);
                const auto& transform = scene->GetComponent<SZM::SceneGraph::TransformComponent>(e);
                DrawComponentCube(e, physics, transform, name, drawList, p0, viewportSize);
            }
        }
    }

    // Camera info
    drawList->AddText(
        ImVec2(p0.x + 8.0f, p1.y - 24.0f),
        IM_COL32(150, 155, 165, 200),
        ("Dist: " + std::to_string(static_cast<int>(m_CameraDistance)) + 
         " | Yaw: " + std::to_string(static_cast<int>(m_CameraYaw)) + 
         " | Pitch: " + std::to_string(static_cast<int>(m_CameraPitch))).c_str()
    );

    ImGui::Dummy(viewportSize);
    UpdateCamera();
}

void SimulationVisualizerPanel::DrawComponentCube(SZM::SceneGraph::Entity e, const SZM::SceneGraph::PhysicsStateComponent& physics,
                                                  const SZM::SceneGraph::TransformComponent& transform, const std::string& name,
                                                  ImDrawList* drawList, 
                                                  const ImVec2& viewportPos, 
                                                  const ImVec2& viewportSize) {
    // Project component position to screen space
    const ImVec2 center = ImVec2(
        viewportPos.x + viewportSize.x * 0.5f + transform.position.x * 50.0f,
        viewportPos.y + viewportSize.y * 0.5f - transform.position.z * 50.0f
    );

    const float size = 16.0f * m_ComponentScale;
    const ImVec2 p0 = ImVec2(center.x - size, center.y - size);
    const ImVec2 p1 = ImVec2(center.x + size, center.y + size);

    // Get color based on visualization mode
    const auto color = GetComponentColor(physics);
    const ImU32 fillColor = IM_COL32(
        static_cast<int>(color.x * 255.0f),
        static_cast<int>(color.y * 255.0f),
        static_cast<int>(color.z * 255.0f),
        255
    );

    // Draw component
    if (m_ShowWireframe) {
        drawList->AddRect(p0, p1, fillColor, 2.0f, 0, 2.0f);
    } else {
        drawList->AddRectFilled(p0, p1, fillColor);
        drawList->AddRect(p0, p1, IM_COL32(255, 255, 255, 100), 0.0f, 0, 1.0f);
    }

    // Danger indicator (pulsing border)
    if (m_ShowDangerIndicators && physics.isDangerous) {
        const ImU32 dangerColor = IM_COL32(255, 100, 100, 200);
        drawList->AddRect(ImVec2(p0.x - 2, p0.y - 2), ImVec2(p1.x + 2, p1.y + 2), dangerColor, 0.0f, 0, 2.0f);
    }

    if (physics.activeContactCount > 0) {
        const ImU32 contactColor = physics.isGrounded
            ? IM_COL32(90, 190, 255, 220)
            : IM_COL32(255, 210, 120, 220);
        drawList->AddRect(ImVec2(p0.x - 3, p0.y - 3), ImVec2(p1.x + 3, p1.y + 3), contactColor, 0.0f, 0, 2.0f);
        drawList->AddLine(
            ImVec2(p0.x, p1.y + 4.0f),
            ImVec2(p1.x, p1.y + 4.0f),
            contactColor,
            2.0f
        );
    }

    // Label
    if (m_ShowComponentLabels) {
        drawList->AddText(
            ImVec2(center.x - 20.0f, center.y + size + 4.0f),
            IM_COL32(220, 225, 235, 255),
            name.c_str()
        );
    }

    // Selection highlight
    if (m_SelectedComponentId == e) {
        drawList->AddRect(ImVec2(p0.x - 4, p0.y - 4), ImVec2(p1.x + 4, p1.y + 4), 
                         IM_COL32(255, 200, 0, 255), 0.0f, 0, 3.0f);
    }
}

void SimulationVisualizerPanel::RenderComponentList() {
    auto* scene = SZM::SimulationEngine::GetInstance().GetScene();
    if (!scene) return;

    auto entities = scene->View<SZM::SceneGraph::PhysicsStateComponent, SZM::SceneGraph::TransformComponent>();
    ImGui::Text("Components (%zu)", entities.size());
    ImGui::Separator();
    
    for (auto e : entities) {
        std::string name = "Entity_" + std::to_string(static_cast<uint32_t>(e));
        if (scene->HasComponent<SZM::SceneGraph::TagComponent>(e)) {
            name = scene->GetComponent<SZM::SceneGraph::TagComponent>(e).name;
        }

        if (m_SearchFilter[0] != '\0' && name.find(m_SearchFilter) == std::string::npos) {
            continue;
        }

        ImGui::PushID(e);
        
        const bool isSelected = (m_SelectedComponentId == e);
        if (ImGui::Selectable(name.c_str(), isSelected)) {
            m_SelectedComponentId = e;
        }

        if (isSelected) {
            const auto& physics = scene->GetComponent<SZM::SceneGraph::PhysicsStateComponent>(e);
            const auto& transform = scene->GetComponent<SZM::SceneGraph::TransformComponent>(e);
            DrawComponentDetails(e, physics, transform, name);
        }

        ImGui::PopID();
    }
}

void SimulationVisualizerPanel::DrawComponentDetails(SZM::SceneGraph::Entity e, const SZM::SceneGraph::PhysicsStateComponent& physics,
                              const SZM::SceneGraph::TransformComponent& transform, const std::string& name) {
    ImGui::Indent();
    ImGui::TextDisabled("ID: %u", e);
    ImGui::TextDisabled("Pos: (%.2f, %.2f, %.2f)", transform.position.x, transform.position.y, transform.position.z);
    
    ImGui::Separator();

    ImGui::Text("Stress: %.2f MPa", physics.stress / 1e6f);
    ImGui::ProgressBar(physics.stressRatio, ImVec2(-1, 0), "");
    
    ImGui::Text("Temp: %.1f°C", physics.temperature - 273.15f);
    ImGui::ProgressBar(physics.tempRatio, ImVec2(-1, 0), "");

    ImGui::Text("Contacts: %u | Grounded: %s", physics.activeContactCount, physics.isGrounded ? "Yes" : "No");
    ImGui::Text("Kinetic Energy: %.4f J", physics.kineticEnergy);
    ImGui::Text("Penetration: %.4f m", physics.maxContactPenetration);

    ImGui::Separator();
    
    if (physics.isDangerous) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "⚠ DANGEROUS");
    } else {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "✓ SAFE");
    }
    
    ImGui::Unindent();
}

SZM::Geometry::Vector3 SimulationVisualizerPanel::GetComponentColor(const SZM::SceneGraph::PhysicsStateComponent& physics) const {
    if (m_ShowStressHeatmap) {
        return MapStressToColor(physics.stressRatio);
    }
    if (m_ShowTemperatureHeatmap) {
        return MapTemperatureToColor(physics.tempRatio);
    }
    return MapDangerToColor(physics.isDangerous);
}

SZM::Geometry::Vector3 SimulationVisualizerPanel::MapStressToColor(float ratio) const {
    // Blue (safe) -> Cyan -> Green -> Yellow -> Red (danger)
    if (ratio < 0.25f) {
        const float t = ratio / 0.25f;
        return SZM::Geometry::Vector3(0.0f, t, 1.0f);
    } else if (ratio < 0.5f) {
        const float t = (ratio - 0.25f) / 0.25f;
        return SZM::Geometry::Vector3(0.0f, 1.0f, 1.0f - t);
    } else if (ratio < 0.75f) {
        const float t = (ratio - 0.5f) / 0.25f;
        return SZM::Geometry::Vector3(t, 1.0f, 0.0f);
    } else {
        const float t = (ratio - 0.75f) / 0.25f;
        return SZM::Geometry::Vector3(1.0f, 1.0f - t, 0.0f);
    }
}

SZM::Geometry::Vector3 SimulationVisualizerPanel::MapTemperatureToColor(float ratio) const {
    // Blue (cold) -> Cyan -> Green -> Yellow -> Red (hot)
    if (ratio < 0.25f) {
        const float t = ratio / 0.25f;
        return SZM::Geometry::Vector3(0.0f, t, 1.0f);
    } else if (ratio < 0.5f) {
        const float t = (ratio - 0.25f) / 0.25f;
        return SZM::Geometry::Vector3(0.0f, 1.0f, 1.0f - t);
    } else if (ratio < 0.75f) {
        const float t = (ratio - 0.5f) / 0.25f;
        return SZM::Geometry::Vector3(t, 1.0f, 0.0f);
    } else {
        const float t = (ratio - 0.75f) / 0.25f;
        return SZM::Geometry::Vector3(1.0f, 1.0f - t, 0.0f);
    }
}

SZM::Geometry::Vector3 SimulationVisualizerPanel::MapDangerToColor(bool isDangerous) const {
    return isDangerous ? SZM::Geometry::Vector3(1.0f, 0.2f, 0.2f)
                       : SZM::Geometry::Vector3(0.2f, 1.0f, 0.2f);
}

void SimulationVisualizerPanel::UpdateCamera() {
    ImGuiIO& io = ImGui::GetIO();
    
    if (ImGui::IsItemHovered()) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            const ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            m_CameraYaw += delta.x * 0.5f;
            m_CameraPitch += delta.y * 0.5f;
            m_CameraPitch = std::clamp(m_CameraPitch, -89.0f, 89.0f);
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
        
        if (io.MouseWheel != 0.0f) {
            m_CameraDistance -= io.MouseWheel * 0.5f;
            m_CameraDistance = std::clamp(m_CameraDistance, 1.0f, 20.0f);
        }
    }
    
    if (m_AutoRotate) {
        m_CameraYaw += m_AutoRotateSpeed * io.DeltaTime;
    }
}

void SimulationVisualizerPanel::RenderComponents() {
    // Graphics module integration pending
}

void SimulationVisualizerPanel::OnSimulationTick() {
    // Panel redraw driven by main frame loop
}
