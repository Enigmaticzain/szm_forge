#include "MeshToolsPanel.hpp"
#include "../UIStyleHelper.hpp"
#include <imgui.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <nlohmann/json.hpp>

namespace {
const char* kPrimitives[] = {"box", "cylinder", "sphere"};
}

void MeshToolsPanel::DrawResultActions() {
    if (m_LastFilePath[0] == '\0') {
        return;
    }
    ImGui::TextDisabled("Output mesh");
    ImGui::TextWrapped("%s", m_LastFilePath);
    if (ImGui::Button("Copy path##mesh", ImVec2(100, 0))) {
        ImGui::SetClipboardText(m_LastFilePath);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reveal in folder##mesh", ImVec2(130, 0))) {
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

void MeshToolsPanel::DrawVolumeMesh() {
    ImGui::Text("Volume mesh (Gmsh / trimesh fallback)");
    ImGui::DragFloat("Length (mm)##meshvol", &m_LengthMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Width (mm)##meshvol", &m_WidthMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Height (mm)##meshvol", &m_HeightMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Max element (m)##meshvol", &m_MaxElementSize, 0.001f, 0.001f, 1.f, "%.3f");
    ImGui::InputText("STEP/CAD path (optional)##meshvol", m_GeometryPath, sizeof(m_GeometryPath));

    if (ImGui::Button("Generate volume mesh##meshvol", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::GenerateMesh(
            m_LengthMm, m_WidthMm, m_HeightMm, m_MaxElementSize, m_GeometryPath);
        if (r.success) {
            std::snprintf(m_LastFilePath, sizeof(m_LastFilePath), "%s", r.filePath.c_str());
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "OK backend=%s\n%s", r.backend.c_str(), r.filePath.c_str());
        } else {
            m_LastFilePath[0] = '\0';
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "FAIL: %s", r.message.c_str());
        }
    }
}

void MeshToolsPanel::DrawTessellate() {
    ImGui::Text("Surface tessellation (viewport LOD mesh)");
    ImGui::Combo("Primitive##meshtess", &m_PrimitiveIndex, kPrimitives, IM_ARRAYSIZE(kPrimitives));
    ImGui::DragFloat("Length (mm)##meshtess", &m_LengthMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Width (mm)##meshtess", &m_WidthMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Height (mm)##meshtess", &m_HeightMm, 1.f, 1.f, 5000.f);
    ImGui::DragFloat("Chordal error##meshtess", &m_ChordalError, 0.001f, 0.001f, 0.5f, "%.3f");

    if (ImGui::Button("Tessellate for viewport##meshtess", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::TessellateMesh(
            kPrimitives[m_PrimitiveIndex], m_LengthMm, m_WidthMm, m_HeightMm, m_ChordalError, false);
        if (r.success) {
            const std::string meshFile = r.raw.value("mesh_file", "");
            if (!meshFile.empty()) {
                std::snprintf(m_LastFilePath, sizeof(m_LastFilePath), "%s", meshFile.c_str());
            }
            const auto& lod = r.raw.value("lod", nlohmann::json::object());
            std::snprintf(m_ResultLog, sizeof(m_ResultLog),
                "OK engine=%s\nprimitive=%s triangles=%d\nmesh_file=%s",
                r.raw.value("engine_used", r.backend).c_str(),
                lod.value("primitive", "?").c_str(),
                lod.value("triangle_count", 0),
                meshFile.empty() ? "(analytic only)" : meshFile.c_str());
        } else {
            m_LastFilePath[0] = '\0';
            std::snprintf(m_ResultLog, sizeof(m_ResultLog), "FAIL: %s", r.message.c_str());
        }
    }
}

void MeshToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) {
        ImGui::End();
        return;
    }

    m_BridgeOnline = SZM::BridgeClient::IsReachable();
    SZM::UIStyleHelper::StatusBadge(m_BridgeOnline ? "online" : "offline");
    ImGui::SameLine();
    ImGui::TextUnformatted(m_BridgeOnline
        ? "Mesh stack on port 8003 (Group 3)"
        : "Start bridge: uvicorn external_engine_bridge:app --port 8003");

    if (!m_BridgeOnline) {
        ImGui::TextWrapped(
            "Install Group 3: pip install gmsh meshio. "
            "Volume meshes feed FEA/CFD pipelines.");
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Volume mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawVolumeMesh();
    }
    if (ImGui::CollapsingHeader("Tessellate (LOD)", ImGuiTreeNodeFlags_DefaultOpen)) {
        DrawTessellate();
    }

    ImGui::Spacing();
    ImGui::Text("Last result");
    ImGui::BeginChild("MeshResultLog", ImVec2(-1, 100), true);
    ImGui::TextUnformatted(m_ResultLog[0] ? m_ResultLog : "(no operations yet)");
    ImGui::EndChild();
    DrawResultActions();

    ImGui::End();
}
