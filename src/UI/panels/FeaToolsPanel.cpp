#include "FeaToolsPanel.hpp"
#include "../UIStyleHelper.hpp"
#include <imgui.h>
#include <cstdio>
#include <cmath>

void FeaToolsPanel::DrawRunAnalysis() {
    ImGui::Text("Cantilever beam FEA");
    ImGui::InputInt("Component ID##fea", &m_ComponentId);
    if (m_ComponentId < 0) {
        m_ComponentId = 0;
    }
    ImGui::DragFloat("Force (N)##fea", &m_ForceN, 10.f, 0.f, 1e6f);
    ImGui::DragFloat("Cross-section (mm²)##fea", &m_AreaMm2, 1.f, 1.f, 1e6f);
    ImGui::DragFloat("Length (mm)##fea", &m_ThicknessMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Young's modulus (GPa)##fea", &m_YoungsGpa, 1.f, 1.f, 500.f);
    ImGui::DragFloat("Yield strength (MPa)##fea", &m_YieldMpa, 1.f, 1.f, 2000.f);
    ImGui::DragFloat("Density (kg/m³)##fea", &m_Density, 10.f, 100.f, 20000.f);

    if (ImGui::Button("Run FEA (mesh + solver)##fea", ImVec2(-1, 0))) {
        const float areaM2 = m_AreaMm2 * 1e-6f;
        const float thicknessM = m_ThicknessMm * 1e-3f;
        m_LastResult = SZM::BridgeClient::RunFea(
            m_ComponentId,
            m_ForceN,
            areaM2,
            thicknessM,
            m_YoungsGpa * 1e9f,
            m_YieldMpa * 1e6f,
            m_Density);

        if (m_LastResult.success) {
            const float stressMpa = m_LastResult.raw.value("max_stress_pa", 0.f) / 1e6f;
            const float dispMm = m_LastResult.raw.value("max_displacement_m", 0.f) * 1000.f;
            const float ratio = m_LastResult.raw.value("stress_ratio", 0.f);
            const bool danger = m_LastResult.raw.value("is_dangerous", false);
            const std::string meshPath = m_LastResult.raw.value("mesh_path", "");
            if (!meshPath.empty()) {
                std::snprintf(m_MeshPath, sizeof(m_MeshPath), "%s", meshPath.c_str());
            } else {
                m_MeshPath[0] = '\0';
            }
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "OK solver=%s mesh=%s\n"
                "max stress=%.2f MPa  disp=%.4f mm\n"
                "stress ratio=%.1f%% %s",
                m_LastResult.raw.value("engine_used", "?").c_str(),
                m_LastResult.raw.value("mesh_engine", "none").c_str(),
                stressMpa, dispMm, ratio * 100.f,
                danger ? "WARNING: near yield" : "safe");
        } else {
            m_MeshPath[0] = '\0';
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "FAIL: %s", m_LastResult.message.c_str());
        }
    }
}

void FeaToolsPanel::DrawResults() {
    if (!m_LastResult.success) {
        return;
    }
    const bool danger = m_LastResult.raw.value("is_dangerous", false);
    SZM::UIStyleHelper::StatusBadge(danger ? "warning" : "online");
    ImGui::SameLine();
    ImGui::TextUnformatted(danger ? "High stress — review design" : "Stress within limits");

    if (m_MeshPath[0] != '\0') {
        ImGui::TextDisabled("Mesh used");
        ImGui::TextWrapped("%s", m_MeshPath);
        if (ImGui::Button("Copy mesh path##fea", ImVec2(120, 0))) {
            ImGui::SetClipboardText(m_MeshPath);
        }
    }
}

void FeaToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    m_BridgeOnline = SZM::BridgeClient::IsReachable();
    SZM::UIStyleHelper::StatusBadge(m_BridgeOnline ? "online" : "offline");
    ImGui::SameLine();
    ImGui::TextUnformatted(m_BridgeOnline
        ? "FEA stack on port 8003 (Group 4)"
        : "Start bridge: uvicorn external_engine_bridge:app --port 8003");

    if (!m_BridgeOnline) {
        ImGui::TextWrapped(
            "Runs mesh generation then CalculiX (or analytical fallback). "
            "Install scipy/numpy; optional: apt install calculix-ccx.");
        ImGui::End();
        return;
    }

    auto status = SZM::BridgeClient::FetchStackStatus();
    if (status) {
        if (status->feaReady) {
            ImGui::TextColored(ImVec4(0.2f, 0.85f, 0.45f, 1.f), "FEA ready (scipy + numpy)");
        } else {
            ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.f), "FEA partial — pip install scipy numpy");
        }
        if (status->calculixAvailable) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.2f, 0.85f, 0.45f, 1.f), "CalculiX available");
        } else {
            ImGui::SameLine();
            ImGui::TextDisabled("(analytical solver fallback)");
        }
    }

    if (ImGui::CollapsingHeader("Run analysis", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawRunAnalysis();
    }

    ImGui::Spacing();
    ImGui::Text("Results");
    ImGui::BeginChild("FeaResultLog", ImVec2(-1, 100), true);
    ImGui::TextUnformatted(m_ResultLog[0] ? m_ResultLog : "(no analysis yet)");
    ImGui::EndChild();
    DrawResults();

    ImGui::End();
}
