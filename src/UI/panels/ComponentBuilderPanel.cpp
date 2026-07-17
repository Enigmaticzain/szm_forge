#include "ComponentBuilderPanel.hpp"
#include "../../Materials/MaterialDatabase.hpp"

#include <imgui.h>
#include <vector>

ComponentBuilderPanel::ComponentBuilderPanel() {
    SZM::Materials::MaterialDatabase::GetInstance().LoadStandardLibraries();
    ApplyMaterialPreset(m_SelectedMaterialId);
}

void ComponentBuilderPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Material Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawMaterialPresets();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawGeometryInput();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Quick Templates", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawComponentTemplates();
    }

    ImGui::End();
}

void ComponentBuilderPanel::DrawMaterialPresets() {
    const auto materials = SZM::Materials::MaterialDatabase::GetInstance().GetAllMaterials();
    if (materials.empty()) {
        ImGui::TextDisabled("No materials available.");
        return;
    }

    int selectedIndex = 0;
    bool foundSelectedMaterial = false;
    std::vector<const char*> materialNames;
    materialNames.reserve(materials.size());

    for (std::size_t i = 0; i < materials.size(); ++i) {
        materialNames.push_back(materials[i].name.c_str());
        if (materials[i].id == m_SelectedMaterialId) {
            selectedIndex = static_cast<int>(i);
            foundSelectedMaterial = true;
        }
    }

    if (!foundSelectedMaterial) {
        ApplyMaterialPreset(materials.front().id);
    }

    if (ImGui::Combo("Material", &selectedIndex, materialNames.data(),
                     static_cast<int>(materialNames.size()))) {
        ApplyMaterialPreset(materials[static_cast<std::size_t>(selectedIndex)].id);
    }

    ImGui::TextDisabled("Material ID: %s", m_SelectedMaterialId.c_str());

    ImGui::Text("Density: %.0f kg/m^3", m_Density);
    ImGui::Text("Yield Strength: %.0f MPa", m_YieldStrength / 1e6f);
    ImGui::Text("Thermal Conductivity: %.1f W/(m*K)", m_ThermalConductivity);
}

void ComponentBuilderPanel::DrawGeometryInput() {
    ImGui::InputText("Component Name", m_ComponentName, sizeof(m_ComponentName));
    ImGui::SliderFloat("Area (m^2)", &m_Area, 0.001f, 1.0f, "%.4f");
    ImGui::SliderFloat("Thickness (m)", &m_Thickness, 0.001f, 0.1f, "%.4f");

    if (ImGui::Button("Create Component", ImVec2(-1, 0))) {
        SZM::SimulationEngine::GetInstance().AddComponent(
            m_ComponentName,
            m_SelectedMaterialId,
            m_Area,
            m_Thickness
        );
    }
}

void ComponentBuilderPanel::DrawComponentTemplates() {
    ImGui::Text("Quick-Add Templates");

    if (ImGui::Button("Steel Beam (10cm^2)", ImVec2(-1, 0))) {
        CreateTemplateComponent("Steel Beam", "MAT-STEEL-STRUCT", 0.01f, 0.05f);
    }

    if (ImGui::Button("Aluminum Plate (20cm^2)", ImVec2(-1, 0))) {
        CreateTemplateComponent("Aluminum Plate", "MAT-AL-6061-T6", 0.02f, 0.008f);
    }

    if (ImGui::Button("Copper Wire (1cm^2)", ImVec2(-1, 0))) {
        CreateTemplateComponent("Copper Wire", "MAT-CU-C110", 0.0001f, 0.002f);
    }

    if (ImGui::Button("Titanium Alloy (15cm^2)", ImVec2(-1, 0))) {
        CreateTemplateComponent("Titanium Alloy", "MAT-TI-6AL4V", 0.015f, 0.02f);
    }
}

void ComponentBuilderPanel::ApplyMaterialPreset(const std::string& materialId) {
    const auto material = SZM::Materials::MaterialDatabase::GetInstance().GetMaterial(materialId);
    if (!material.has_value()) {
        return;
    }

    m_SelectedMaterialId = material->id;
    m_Density = static_cast<float>(material->density);
    m_YieldStrength = static_cast<float>(material->yieldStrength * 1.0e6);
    m_ThermalConductivity = static_cast<float>(material->thermalConductivity);
}

void ComponentBuilderPanel::CreateTemplateComponent(const char* name,
                                                    const std::string& materialId,
                                                    float area,
                                                    float thickness) {
    SZM::SimulationEngine::GetInstance().AddComponent(name, materialId, area, thickness);
}
