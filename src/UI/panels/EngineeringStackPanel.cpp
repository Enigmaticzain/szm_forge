#include "EngineeringStackPanel.hpp"
#include "../UIStyleHelper.hpp"
#include <imgui.h>
#include <cstdio>

EngineeringStackPanel::EngineeringStackPanel() {
    RefreshStatus();
}

void EngineeringStackPanel::RefreshStatus() {
    m_LastRefresh = std::chrono::steady_clock::now();
    m_Status = SZM::BridgeClient::FetchStackStatus();
    if (m_Status && m_Status->connected) {
        m_StatusMessage = "Bridge online — " + std::to_string(m_Status->available) +
                          "/" + std::to_string(m_Status->totalTools) + " tools";
    } else if (m_Status) {
        m_StatusMessage = m_Status->lastError.empty()
            ? "Bridge offline — run: uvicorn external_engine_bridge:app --port 8003"
            : m_Status->lastError;
    } else {
        m_StatusMessage = "Bridge unreachable";
    }
}

void EngineeringStackPanel::DrawGroupTable() {
    if (!m_Status || !m_Status->raw.contains("groups")) {
        ImGui::TextDisabled("No stack data.");
        return;
    }

    if (!ImGui::BeginTable("StackGroups", 4,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        return;
    }
    ImGui::TableSetupColumn("Grp");
    ImGui::TableSetupColumn("Layer");
    ImGui::TableSetupColumn("OK");
    ImGui::TableSetupColumn("Health");
    ImGui::TableHeadersRow();

    for (const auto& g : m_Status->raw["groups"]) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%d", g.value("group", 0));
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(g.value("name", "").c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%d/%d", g.value("ok", 0), g.value("total", 0));
        ImGui::TableNextColumn();
        const float pct = g.value("health_pct", 0.0f) / 100.0f;
        SZM::UIStyleHelper::ProgressBar(pct, ImVec2(-1, 0), nullptr);
    }
    ImGui::EndTable();
}

void EngineeringStackPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    const bool online = m_Status && m_Status->connected;
    SZM::UIStyleHelper::StatusBadge(online ? "online" : "offline");
    ImGui::SameLine();
    ImGui::TextUnformatted(m_StatusMessage.c_str());

    ImGui::Checkbox("Auto refresh##stack", &m_AutoRefresh);
    ImGui::SameLine();
    if (ImGui::Button("Refresh now##stack")) {
        RefreshStatus();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Docs: http://127.0.0.1:8003/docs");

    if (m_AutoRefresh) {
        const auto now = std::chrono::steady_clock::now();
        const double elapsed = std::chrono::duration<double>(now - m_LastRefresh).count();
        if (elapsed >= m_RefreshIntervalSec) {
            RefreshStatus();
        }
    }

    if (m_Status && m_Status->cadReady) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.2f, 0.85f, 0.45f, 1.f), "Group 2 CAD: READY");
        ImGui::TextDisabled("build123d, cadquery, trimesh, shapely, scikit-spatial");
    } else if (m_Status && m_Status->connected) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.f), "Group 2 CAD: partial — install group 2 packages");
    }

    if (m_Status && m_Status->meshReady) {
        ImGui::TextColored(ImVec4(0.2f, 0.85f, 0.45f, 1.f), "Group 3 Mesh: READY");
        ImGui::TextDisabled("gmsh, meshio — use Mesh Tools panel");
    } else if (m_Status && m_Status->connected) {
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.f), "Group 3 Mesh: partial — pip install gmsh meshio");
    }

    if (m_Status && m_Status->feaReady) {
        ImGui::TextColored(ImVec4(0.2f, 0.85f, 0.45f, 1.f), "Group 4 FEA: READY");
        ImGui::TextDisabled("scipy, numpy%s", m_Status->calculixAvailable ? ", CalculiX" : " (analytical fallback)");
    } else if (m_Status && m_Status->connected) {
        ImGui::TextColored(ImVec4(0.95f, 0.75f, 0.2f, 1.f), "Group 4 FEA: partial — pip install scipy numpy");
    }

    ImGui::Spacing();
    SZM::UIStyleHelper::HorizontalSeparator();
    DrawGroupTable();

    ImGui::End();
}
