#include "CadToolsPanel.hpp"
#include "../UIStyleHelper.hpp"
#include <cstdlib>
#include <imgui.h>
#include <cstdio>
#include <cstring>
#include <nlohmann/json.hpp>

namespace {
const char* kBackends[] = {"auto", "build123d", "cadquery"};
}

void CadToolsPanel::DrawResultActions() {
    if (m_LastFilePath[0] == '\0') {
        return;
    }
    ImGui::TextDisabled("Output file");
    ImGui::TextWrapped("%s", m_LastFilePath);
    if (ImGui::Button("Copy path##cad", ImVec2(100, 0))) {
        ImGui::SetClipboardText(m_LastFilePath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reveal in folder##cad", ImVec2(130, 0))) {
        std::string cmd = "xdg-open \"";
        for (const char* p = m_LastFilePath; *p; ++p) {
            if (*p == '"') {
                cmd += '\\';
            }
            cmd += *p;
        }
        cmd += "\" 2>/dev/null || true";
        std::system(cmd.c_str());
    }
}

void CadToolsPanel::DrawTextToCad() {
    ImGui::Text("Text → CAD");
    ImGui::InputTextMultiline("Prompt##t2cad", m_Prompt, sizeof(m_Prompt), ImVec2(-1, 60));
    ImGui::InputText("Output dir##t2cad", m_OutputDir, sizeof(m_OutputDir));
    ImGui::Combo("Backend##t2cad", &m_T2CadBackendIndex, kBackends, IM_ARRAYSIZE(kBackends));

    if (ImGui::Button("Generate STEP##t2cad", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::TextToCad(m_Prompt, kBackends[m_T2CadBackendIndex], m_OutputDir);
        if (r.success) {
            if (!r.filePath.empty()) {
                std::snprintf(m_LastFilePath, sizeof(m_LastFilePath), "%s", r.filePath.c_str());
            }
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "OK backend=%s engine=%s\nfile=%s",
                r.backend.c_str(),
                r.raw.value("engine", "?").c_str(),
                r.filePath.empty() ? "(mesh spec only)" : r.filePath.c_str());
        } else {
            m_LastFilePath[0] = '\0';
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "FAIL: %s", r.message.c_str());
        }
    }
}

void CadToolsPanel::DrawExportBox() {
    ImGui::Text("Parametric export");
    ImGui::InputText("Shape##export", m_Shape, sizeof(m_Shape));
    ImGui::DragFloat("Length (mm)##export", &m_Length, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Width (mm)##export", &m_Width, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Height (mm)##export", &m_Height, 1.f, 1.f, 5000.f);
    ImGui::Combo("Backend##export", &m_ExportBackendIndex, kBackends, IM_ARRAYSIZE(kBackends));

    if (ImGui::Button("Export STEP##export", ImVec2(-1, 0))) {
        const std::string path = std::string(m_OutputDir) + "/szm_export.step";
        auto r = SZM::BridgeClient::ExportCad(
            m_Shape, m_Length, m_Width, m_Height, kBackends[m_ExportBackendIndex], path);
        if (r.success) {
            const std::string& out = r.filePath.empty() ? path : r.filePath;
            std::snprintf(m_LastFilePath, sizeof(m_LastFilePath), "%s", out.c_str());
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "Exported via %s\n%s", r.backend.c_str(), out.c_str());
        } else {
            m_LastFilePath[0] = '\0';
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "Export failed: %s", r.message.c_str());
        }
    }
}

void CadToolsPanel::DrawGeometricFit() {
    ImGui::Text("Geometric fit (scikit-spatial)");
    ImGui::TextDisabled("Uses demo plane points; replace with scan data in API.");

    if (ImGui::Button("Fit plane to demo points##fit", ImVec2(-1, 0))) {
        nlohmann::json pts = {
            {0, 0, 0}, {1, 0, 0.01}, {0, 1, -0.01}, {1, 1, 0.02},
        };
        auto r = SZM::BridgeClient::FitGeometry("plane", pts);
        if (r.success && r.raw.contains("result")) {
            const auto& n = r.raw["result"]["normal"];
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "Plane normal: [%.4f, %.4f, %.4f]",
                n[0].get<float>(), n[1].get<float>(), n[2].get<float>());
        } else {
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "Fit failed: %s", r.message.c_str());
        }
    }
}

void CadToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    m_BridgeOnline = SZM::BridgeClient::IsReachable();
    SZM::UIStyleHelper::StatusBadge(m_BridgeOnline ? "online" : "offline");
    ImGui::SameLine();
    ImGui::TextUnformatted(m_BridgeOnline
        ? "Python bridge connected (port 8003)"
        : "Start bridge: uvicorn external_engine_bridge:app --port 8003");

    if (!m_BridgeOnline) {
        ImGui::TextWrapped(
            "The desktop app talks to the Python engineering stack on port 8003. "
            "Swagger at /docs is the API — these panels bring it into SZM Forge.");
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Text to CAD", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawTextToCad();
    }
    if (ImGui::CollapsingHeader("Export solid", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawExportBox();
    }
    if (ImGui::CollapsingHeader("Reverse engineering fit")) {
        DrawGeometricFit();
    }

    ImGui::Spacing();
    ImGui::Text("Last result");
    ImGui::BeginChild("CadResultLog", ImVec2(-1, 100), true);
    ImGui::TextUnformatted(m_ResultLog[0] ? m_ResultLog : "(no operations yet)");
    ImGui::EndChild();
    DrawResultActions();

    ImGui::End();
}
