#include "StackExtendedPanels.hpp"
#include "../UIStyleHelper.hpp"
#include <nlohmann/json.hpp>
#include <imgui.h>
#include <cstdio>

static bool BridgeOk() { return SZM::BridgeClient::IsReachable(); }

void OptimizationToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::DragFloat("Initial stress (MPa)", &m_InitialStress, 1.f, 100.f, 2000.f);
    ImGui::DragFloat("Initial mass (kg)", &m_InitialMass, 0.1f, 0.1f, 100.f);
    ImGui::DragFloat("Target stress (MPa)", &m_TargetStress, 1.f, 100.f, 2000.f);
    ImGui::InputInt("Iterations", &m_Iterations);
    if (ImGui::Button("Run optimization (Optuna/pymoo)", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::RunOptimization(m_InitialStress, m_InitialMass, m_TargetStress, m_Iterations);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "OK backend=%s\n%s" : "FAIL: %s",
            r.raw.value("backend", r.backend).c_str(), r.success ? r.raw.dump(2).substr(0, 400).c_str() : r.message.c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(run optimization)");
    ImGui::End();
}

void MaterialsToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::InputText("Formula/element", m_Formula, sizeof(m_Formula));
    if (ImGui::Button("Lookup material", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::LookupMaterial(m_Formula);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "%s" : "FAIL: %s",
            r.success ? r.raw.dump(2).substr(0, 500).c_str() : r.message.c_str());
    }
    ImGui::InputText("NIST query", m_NistQuery, sizeof(m_NistQuery));
    if (ImGui::Button("NIST search", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::SearchNistMaterial(m_NistQuery);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "%s" : "FAIL: %s",
            r.success ? r.raw.dump(2).substr(0, 500).c_str() : r.message.c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(lookup materials)");
    ImGui::End();
}

void ManufacturingToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::InputInt("Component ID", &m_ComponentId);
    ImGui::InputText("Part name", m_PartName, sizeof(m_PartName));
    ImGui::DragFloat("Layer height (mm)", &m_LayerHeight, 0.01f, 0.05f, 1.f);
    if (ImGui::Button("Run slicer (Cura/fallback G-code)", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::RunSlicer(m_ComponentId, m_PartName, m_LayerHeight);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "OK engine=%s\n%s" : "FAIL: %s",
            r.raw.value("engine_used", "?").c_str(),
            r.success ? "G-code generated" : r.message.c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(slice part)");
    ImGui::End();
}

void RoboticsToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::InputTextMultiline("URDF snippet", m_UrdfSnippet, sizeof(m_UrdfSnippet), ImVec2(-1, 60));
    if (ImGui::Button("Parse URDF", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::ParseUrdf(m_UrdfSnippet);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "Parsed: %s" : "FAIL: %s",
            r.success ? r.raw.dump(2).substr(0, 300).c_str() : r.message.c_str());
    }
    ImGui::InputInt("Sim steps", &m_SimSteps);
    if (ImGui::Button("PyBullet simulate", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::SimulateRobot(m_SimSteps);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "Sim OK: %s" : "FAIL: %s",
            r.success ? r.raw.dump(2).substr(0, 300).c_str() : r.message.c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(robotics tools)");
    ImGui::End();
}

void IoTToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::InputText("MQTT broker", m_Broker, sizeof(m_Broker));
    ImGui::InputInt("Port", &m_Port);
    ImGui::InputText("Topic", m_Topic, sizeof(m_Topic));
    if (ImGui::Button("Start MQTT subscriber", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::StartMqtt(m_Broker, m_Port, m_Topic);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "MQTT started" : "FAIL: %s", r.message.c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(IoT bridge)");
    ImGui::End();
}

void PhysicsMLToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::InputText("Surrogate name", m_ModelName, sizeof(m_ModelName));
    ImGui::InputInt("Epochs", &m_Epochs);
    if (ImGui::Button("Train surrogate", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::TrainSurrogate(m_ModelName, m_Epochs);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "Train OK: %s" : "FAIL: %s",
            r.success ? r.raw.dump(2).substr(0, 300).c_str() : r.message.c_str());
    }
    if (ImGui::Button("Predict (demo inputs)", ImVec2(-1, 0))) {
        nlohmann::json inputs = {{"stress", 400.f}, {"temp", 80.f}};
        auto r = SZM::BridgeClient::PredictSurrogate(m_ModelName, inputs);
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "Predict: %s" : "FAIL: %s",
            r.success ? r.raw.dump(2).substr(0, 300).c_str() : r.message.c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(physics ML — torch optional)");
    ImGui::End();
}

void PipelineToolsPanel::Draw() {
    if (!ImGui::Begin(GetTitle().c_str(), &IsVisible)) { ImGui::End(); return; }
    if (!BridgeOk()) { ImGui::TextWrapped("Bridge offline"); ImGui::End(); return; }
    ImGui::InputText("Geometry path", m_GeometryPath, sizeof(m_GeometryPath));
    if (ImGui::Button("Run CAD→mesh→simulate pipeline", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::RunPipeline(m_GeometryPath);
        if (r.success && r.raw.contains("job_id")) {
            std::snprintf(m_JobId, sizeof(m_JobId), "%s", r.raw["job_id"].get<std::string>().c_str());
        }
        std::snprintf(m_Log, sizeof(m_Log), r.success ? "Pipeline job=%s status=%s" : "FAIL: %s",
            m_JobId, r.raw.value("status", "?").c_str(), r.message.c_str());
    }
    if (m_JobId[0] && ImGui::Button("Poll job status", ImVec2(-1, 0))) {
        auto r = SZM::BridgeClient::GetPipelineJob(m_JobId);
        std::snprintf(m_Log, sizeof(m_Log), "Job %s: %s", m_JobId, r.raw.dump(2).substr(0, 400).c_str());
    }
    ImGui::TextWrapped("%s", m_Log[0] ? m_Log : "(engineering pipeline)");
    ImGui::End();
}
