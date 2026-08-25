#include "ThermalCfdToolsPanel.hpp"
#include "../UIStyleHelper.hpp"
#include <imgui.h>
#include <cstdio>

namespace {
const char* kModes[] = {"steady_state", "transient"};
}

void ThermalCfdToolsPanel::DrawThermal() {
    ImGui::InputInt("Component ID##thermal", &m_ComponentId);
    ImGui::Combo("Mode##thermal", &m_ModeIndex, kModes, IM_ARRAYSIZE(kModes));
    ImGui::DragFloat("Ambient (C)##thermal", &m_AmbientC, 0.5f, -50.f, 500.f);
    ImGui::DragFloat("Heat input (W)##thermal", &m_HeatW, 1.f, 0.f, 50000.f);
    ImGui::DragFloat("Convection (W/m2K)##thermal", &m_Convection, 0.1f, 0.1f, 1000.f);
    if (ImGui::Button("Run thermal analysis##thermal", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::RunThermal(m_ComponentId, kModes[m_ModeIndex], m_AmbientC, m_HeatW, m_Convection);
        if (r.success) {
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "Thermal OK engine=%s\nmax temp=%.1f C rise=%.1f C %s",
                r.raw.value("engine_used", "?").c_str(),
                r.raw.value("max_temperature_c", 0.f),
                r.raw.value("temperature_rise_c", 0.f),
                r.raw.value("is_dangerous", false) ? "WARNING" : "safe");
        } else {
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "Thermal FAIL: %s", r.message.c_str());
        }
    }
}

void ThermalCfdToolsPanel::DrawCfd() {
    ImGui::DragFloat("Inlet velocity (m/s)##cfd", &m_InletVelocity, 0.5f, 0.f, 200.f);
    if (ImGui::Button("Run CFD (drag estimate)##cfd", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::RunCfd(m_ComponentId, m_InletVelocity);
        if (r.success) {
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "CFD OK engine=%s\ndrag=%.1f N max v=%.1f m/s",
                r.raw.value("engine_used", "?").c_str(),
                r.raw.value("drag_force_n", 0.f),
                r.raw.value("max_velocity_m_s", 0.f));
        } else {
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "CFD FAIL: %s", r.message.c_str());
        }
    }
}

void ThermalCfdToolsPanel::DrawFluidLookup() {
    ImGui::InputText("Fluid##fluid", m_FluidName, sizeof(m_FluidName));
    ImGui::DragFloat("Temperature (K)##fluid", &m_FluidTempK, 1.f, 200.f, 800.f);
    if (ImGui::Button("CoolProp lookup##fluid", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::GetFluidProperties(m_FluidName, m_FluidTempK);
        if (r.success && r.raw.contains("properties")) {
            const auto& p = r.raw["properties"];
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "Fluid %s @ %.0f K\ndensity=%.1f kg/m3 k=%.3f W/mK",
                m_FluidName, m_FluidTempK,
                p.value("density_kg_m3", 0.f),
                p.value("thermal_conductivity_w_mk", 0.f));
        } else {
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "Fluid lookup failed: %s", r.message.c_str());
        }
    }
}

void ThermalCfdToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }
    m_BridgeOnline = SZM::BridgeClient::IsReachable();
    SZM::UIStyleHelper::StatusBadge(m_BridgeOnline ? "online" : "offline");
    ImGui::SameLine();
    ImGui::TextUnformatted(m_BridgeOnline ? "Group 5 thermal/CFD" : "Bridge offline");
    if (!m_BridgeOnline) {
        ImGui::End();
        return;
    }
    if (ImGui::CollapsingHeader("Thermal", ImGuiTreeNodeFlags_DefaultOpen)) DrawThermal();
    if (ImGui::CollapsingHeader("CFD", ImGuiTreeNodeFlags_DefaultOpen)) DrawCfd();
    if (ImGui::CollapsingHeader("Fluid properties (CoolProp)")) DrawFluidLookup();
    ImGui::BeginChild("ThermalCfdLog", ImVec2(-1, 90), true);
    ImGui::TextUnformatted(m_ResultLog[0] ? m_ResultLog : "(no results)");
    ImGui::EndChild();
    ImGui::End();
}
