#include "PropertiesPanel.hpp"
#include "../../Input/EventBus.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <imgui.h>
#include <any>

PropertiesPanel::PropertiesPanel() {
    SZM::EventBus::GetInstance().Subscribe(
        "SelectionChanged",
        [this](const std::any& payload) {
            if (const auto* selected = std::any_cast<std::string>(&payload)) {
                SetSelectedEntityName(*selected);
            }
        }
    );
}

void PropertiesPanel::SetSelectedEntityName(const std::string& name) {
    m_SelectedEntity = name;
}

void PropertiesPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    if (m_SelectedEntity.empty()) {
        ImGui::TextDisabled("No entity selected.");
        ImGui::End();
        return;
    }

    uint32_t compId = 0;
    try {
        compId = std::stoul(m_SelectedEntity);
    } catch (...) {
        // Invalid ID string
    }

    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(compId);
    if (!comp) {
        ImGui::Text("Selected (Invalid/Unknown): %s", m_SelectedEntity.c_str());
        ImGui::End();
        return;
    }

    ImGui::Text("Component: %s", comp->name.c_str());
    ImGui::TextDisabled("ID: %u", comp->id);
    ImGui::Separator();

    ImGui::Text("Material");
    ImGui::BulletText("ID: %s", comp->materialId.c_str());
    ImGui::BulletText("Name: %s", comp->materialName.c_str());
    
    ImGui::Separator();
    ImGui::Text("Geometry");
    
    float area = comp->area;
    float thickness = comp->thickness;
    bool changed = false;
    
    if (ImGui::InputFloat("Area (m^2)", &area, 0.001f, 0.01f, "%.4f")) changed = true;
    if (ImGui::InputFloat("Thickness (m)", &thickness, 0.001f, 0.01f, "%.4f")) changed = true;
    
    if (changed) {
        SZM::SimulationEngine::GetInstance().SetComponentGeometry(comp->id, area, thickness);
    }

    ImGui::Separator();
    ImGui::Text("Simulation State");
    ImGui::Text("Stress: %.2f MPa", comp->stress / 1e6f);
    ImGui::Text("Temperature: %.2f K", comp->temperature);
    
    float yieldMPa = comp->yieldStrength / 1e6f;
    ImGui::Text("Yield Str: %.2f MPa", yieldMPa);

    if (comp->isDangerous) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "WARNING: DANGEROUS STATE");
    }

    ImGui::End();
}
