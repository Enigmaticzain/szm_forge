#include "SimulationPanel.hpp"
#include "../../Input/EventBus.hpp"

#include <imgui.h>
#include <cstdio>
#include <cmath>

SimulationPanel::SimulationPanel() {
    SZM::EventBus::GetInstance().Subscribe(
        "SimulationTick",
        [this](const std::any& /*payload*/) { this->OnSimulationTick(); }
    );
}

SimulationPanel::~SimulationPanel() {
    // EventBus lifecycle is handled globally.
}

void SimulationPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Add Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawComponentBuilder();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawComponentList();
    }

    ImGui::Separator();

    if (m_SelectedComponentId > 0 &&
        ImGui::CollapsingHeader("Selected Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawSelectedComponentControls();
    }

    ImGui::End();
}

void SimulationPanel::DrawComponentBuilder() {
    ImGui::InputText("Name", m_ComponentNameBuffer, sizeof(m_ComponentNameBuffer));

    if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
        const std::string name = (m_ComponentNameBuffer[0] != '\0')
            ? std::string(m_ComponentNameBuffer)
            : std::string("Component");

        m_SelectedComponentId = SZM::SimulationEngine::GetInstance().AddComponent(name);
        std::snprintf(m_ComponentNameBuffer, sizeof(m_ComponentNameBuffer), "New Component");
    }
}

void SimulationPanel::DrawComponentList() {
    const auto& components = SZM::SimulationEngine::GetInstance().GetComponents();

    if (components.empty()) {
        ImGui::TextDisabled("No components yet.");
        return;
    }

    if (!ImGui::BeginTable("ComponentTable", 5,
                           ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                               ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }

    ImGui::TableSetupColumn("ID");
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Stress (MPa)");
    ImGui::TableSetupColumn("Temp (C)");
    ImGui::TableSetupColumn("Status");
    ImGui::TableHeadersRow();

    for (const auto& comp : components) {
        ImGui::PushID(static_cast<int>(comp->id));
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        const std::string idText = std::to_string(comp->id);
        if (ImGui::Selectable(idText.c_str(), m_SelectedComponentId == comp->id)) {
            m_SelectedComponentId = comp->id;
            m_SelectedForce = comp->appliedForce;
            m_SelectedHeatInput = comp->heatInput;
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(comp->name.c_str());

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.2f", comp->stress / 1e6f);

        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.1f", comp->temperature - 273.15f);

        ImGui::TableSetColumnIndex(4);
        const ImVec4 statusColor = comp->isDangerous
            ? ImVec4(1.0f, 0.2f, 0.2f, 1.0f)
            : ImVec4(0.2f, 0.9f, 0.2f, 1.0f);
        ImGui::TextColored(statusColor, "%s", comp->isDangerous ? "DANGER" : "OK");

        ImGui::PopID();
    }

    ImGui::EndTable();
}

void SimulationPanel::DrawSelectedComponentControls() {
    auto* comp = SZM::SimulationEngine::GetInstance().GetComponent(m_SelectedComponentId);
    if (!comp) {
        m_SelectedComponentId = 0;
        ImGui::TextDisabled("Selected component no longer exists.");
        return;
    }

    ImGui::Text("Component: %s (ID: %u)", comp->name.c_str(), comp->id);
    ImGui::Separator();

    ImGui::Text("Material");
    ImGui::BulletText("Material: %s", comp->materialName.c_str());
    ImGui::BulletText("Material ID: %s", comp->materialId.c_str());
    ImGui::BulletText("Density: %.0f kg/m^3", comp->density);
    ImGui::BulletText("Yield Strength: %.0f MPa", comp->yieldStrength / 1e6f);
    ImGui::BulletText("Thermal Conductivity: %.1f W/(m*K)", comp->thermalConductivity);
    ImGui::BulletText("Area: %.4f m^2", comp->area);
    ImGui::BulletText("Thickness: %.4f m", comp->thickness);

    ImGui::Separator();

    ImGui::Text("State");
    ImGui::BulletText("Stress: %.2f MPa (%.1f%%)", comp->stress / 1e6f,
                      comp->stressRatio * 100.0f);
    ImGui::BulletText("Temperature: %.1f C", comp->temperature - 273.15f);
    ImGui::BulletText("Position: (%.2f, %.2f, %.2f) m",
                      comp->position.x, comp->position.y, comp->position.z);
    const double speed = std::sqrt(
        comp->linearVelocity.x * comp->linearVelocity.x +
        comp->linearVelocity.y * comp->linearVelocity.y +
        comp->linearVelocity.z * comp->linearVelocity.z);
    ImGui::BulletText("Velocity: (%.2f, %.2f, %.2f) m/s | Speed: %.2f m/s",
                      comp->linearVelocity.x,
                      comp->linearVelocity.y,
                      comp->linearVelocity.z,
                      speed);
    ImGui::BulletText("Mass (physics proxy): %.3f kg", comp->massEstimate);
    ImGui::BulletText("Physics Backend: %s", comp->physicsBackend.c_str());
    ImGui::BulletText("Kinetic Energy: %.4f J", comp->kineticEnergy);
    ImGui::BulletText("Contacts: %u", comp->activeContactCount);
    ImGui::BulletText("Grounded: %s", comp->isGrounded ? "Yes" : "No");
    ImGui::BulletText("Max Penetration: %.4f m", comp->maxContactPenetration);
    ImGui::BulletText("Danger: %s", comp->isDangerous ? "Yes" : "No");

    ImGui::Separator();

    if (ImGui::SliderFloat("Force (N)", &m_SelectedForce, 0.0f, 1.0e6f, "%.0f")) {
        SZM::SimulationEngine::GetInstance().SetForce(m_SelectedComponentId, m_SelectedForce);
    }
    if (ImGui::SliderFloat("Heat Input (W)", &m_SelectedHeatInput, 0.0f, 2.0e3f, "%.1f")) {
        SZM::SimulationEngine::GetInstance().SetHeatInput(m_SelectedComponentId,
                                                          m_SelectedHeatInput);
    }

    if (ImGui::Button("Remove Component", ImVec2(-1, 0))) {
        SZM::SimulationEngine::GetInstance().RemoveComponent(m_SelectedComponentId);
        m_SelectedComponentId = 0;
    }
}

void SimulationPanel::OnSimulationTick() {
    // Panel redraw is driven by the main frame loop.
}
