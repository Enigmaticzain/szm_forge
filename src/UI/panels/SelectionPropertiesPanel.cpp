#include "SelectionPropertiesPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

SelectionPropertiesPanel::SelectionPropertiesPanel()
    : m_SelectionManager(Graphics::SelectionManager::GetInstance()),
      m_SimEngine(::SZM::SimulationEngine::GetInstance()) {
}

SelectionPropertiesPanel::~SelectionPropertiesPanel() = default;

void SelectionPropertiesPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    size_t selectionCount = m_SelectionManager.GetSelectionCount();

    if (selectionCount == 0) {
        ImGui::TextDisabled("No components selected");
    } else if (selectionCount == 1) {
        uint32_t componentId = *m_SelectionManager.GetSelectedIds().begin();
        DrawComponentProperties(componentId);
    } else {
        ImGui::Text("Multiple components selected (%zu)", selectionCount);
        ImGui::Separator();

        for (uint32_t id : m_SelectionManager.GetSelectedIds()) {
            if (ImGui::TreeNode((void*)(intptr_t)id, "Component %u", id)) {
                DrawComponentProperties(id);
                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

void SelectionPropertiesPanel::DrawComponentProperties(uint32_t componentId) {
    const auto* component = m_SimEngine.GetComponent(componentId);
    if (!component) {
        ImGui::TextDisabled("Component data unavailable");
        return;
    }

    ImGui::Text("Component ID: %u", componentId);
    ImGui::Text("Name: %s", component->name.c_str());
    ImGui::Separator();

    ImGui::Text("Material Properties:");
    ImGui::Indent();
    ImGui::BulletText("Material: %s", component->materialName.c_str());
    ImGui::BulletText("Density: %.2f kg/m^3", component->density);
    ImGui::BulletText("Yield Strength: %.2f MPa", component->yieldStrength / 1.0e6f);
    ImGui::BulletText("Thermal Conductivity: %.2f W/(m*K)", component->thermalConductivity);
    ImGui::Unindent();

    ImGui::Separator();
    DrawStressInfo(*component);
    ImGui::Separator();
    DrawThermalInfo(*component);
}

void SelectionPropertiesPanel::DrawStressInfo(const ::SZM::SimulationComponent& comp) {
    ImGui::Text("Stress Analysis:");
    ImGui::Indent();
    ImGui::ProgressBar(comp.stressRatio, ImVec2(-1, 0), "Stress utilization");
    ImGui::BulletText("Von Mises Stress: %.2f MPa", comp.stress / 1.0e6f);
    ImGui::BulletText("Applied Force: %.2f N", comp.appliedForce);
    ImGui::BulletText("Safety Factor: %.2f", comp.stress > 0.0f ? comp.yieldStrength / comp.stress : 0.0f);
    ImGui::Unindent();
}

void SelectionPropertiesPanel::DrawThermalInfo(const ::SZM::SimulationComponent& comp) {
    ImGui::Text("Thermal Analysis:");
    ImGui::Indent();
    ImGui::ProgressBar(comp.tempRatio, ImVec2(-1, 0), "Thermal utilization");
    ImGui::BulletText("Temperature: %.2f K", comp.temperature);
    ImGui::BulletText("Heat Input: %.2f W", comp.heatInput);
    ImGui::BulletText("Danger State: %s", comp.isDangerous ? "Active" : "Safe");
    ImGui::Unindent();
}

} // namespace SZM::UI
