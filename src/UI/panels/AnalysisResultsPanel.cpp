#include "AnalysisResultsPanel.hpp"
#include <imgui.h>

namespace SZM::UI {

AnalysisResultsPanel::AnalysisResultsPanel()
    : m_StressViz(Graphics::StressVisualizer::GetInstance()),
      m_ThermalViz(Graphics::ThermalVisualizer::GetInstance()) {
}

AnalysisResultsPanel::~AnalysisResultsPanel() = default;

void AnalysisResultsPanel::Draw() {
    if (!ImGui::Begin(GetName(), &m_IsOpen)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##AnalysisTabs")) {
        if (ImGui::BeginTabItem("Stress")) {
            DrawStressResults();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Thermal")) {
            DrawThermalResults();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Combined")) {
            DrawCombinedResults();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void AnalysisResultsPanel::DrawStressResults() {
    ImGui::Text("Stress Analysis Summary");
    ImGui::Separator();

    ImGui::Text("Global Statistics:");
    ImGui::Indent();
    ImGui::BulletText("Max Stress: %.2f MPa", m_StressViz.GetMaxStress());
    ImGui::BulletText("Min Stress: %.2f MPa", m_StressViz.GetMinStress());
    ImGui::BulletText("Failed Components: %zu", m_StressViz.GetFailedComponentCount());
    ImGui::BulletText("Failure Threshold: %.2f", m_StressViz.GetFailureThreshold());
    ImGui::Unindent();

    ImGui::Separator();
    ImGui::Text("Component Details:");

    if (ImGui::BeginTable("StressTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Stress (MPa)");
        ImGui::TableSetupColumn("Safety Factor");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        for (const auto& data : m_StressViz.GetStressData()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%u", data.componentId);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.2f", data.vonMisesStress);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.2f", data.safetyFactor);
            ImGui::TableSetColumnIndex(3);
            ImGui::TextColored(
                data.isFailed ? ImVec4(1, 0, 0, 1) : ImVec4(0, 1, 0, 1),
                data.isFailed ? "FAILED" : "OK"
            );
        }

        ImGui::EndTable();
    }
}

void AnalysisResultsPanel::DrawThermalResults() {
    ImGui::Text("Thermal Analysis Summary");
    ImGui::Separator();

    ImGui::Text("Global Statistics:");
    ImGui::Indent();
    ImGui::BulletText("Max Temperature: %.2f K", m_ThermalViz.GetMaxTemperature());
    ImGui::BulletText("Min Temperature: %.2f K", m_ThermalViz.GetMinTemperature());
    ImGui::BulletText("Overheated Components: %zu", m_ThermalViz.GetOverheatedComponentCount());
    ImGui::BulletText("Overheat Threshold: %.2f K", m_ThermalViz.GetOverheatThreshold());
    ImGui::Unindent();

    ImGui::Separator();
    ImGui::Text("Component Details:");

    if (ImGui::BeginTable("ThermalTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("ID");
        ImGui::TableSetupColumn("Temperature (K)");
        ImGui::TableSetupColumn("Heat Flux (W/m²)");
        ImGui::TableSetupColumn("Status");
        ImGui::TableHeadersRow();

        for (const auto& data : m_ThermalViz.GetThermalData()) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%u", data.componentId);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.2f", data.temperature);
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.2f", data.heatFlux);
            ImGui::TableSetColumnIndex(3);
            ImGui::TextColored(
                data.isOverheated ? ImVec4(1, 0, 0, 1) : ImVec4(0, 1, 0, 1),
                data.isOverheated ? "HOT" : "OK"
            );
        }

        ImGui::EndTable();
    }
}

void AnalysisResultsPanel::DrawCombinedResults() {
    ImGui::Text("Combined Analysis Summary");
    ImGui::Separator();

    ImGui::Text("Critical Issues:");
    ImGui::Indent();
    ImGui::BulletText("Failed Components: %zu", m_StressViz.GetFailedComponentCount());
    ImGui::BulletText("Overheated Components: %zu", m_ThermalViz.GetOverheatedComponentCount());
    ImGui::Unindent();

    ImGui::Separator();
    ImGui::Text("Recommendations:");
    ImGui::Indent();
    if (m_StressViz.GetFailedComponentCount() > 0) {
        ImGui::BulletText("Increase material strength or reduce load");
    }
    if (m_ThermalViz.GetOverheatedComponentCount() > 0) {
        ImGui::BulletText("Improve cooling or reduce heat generation");
    }
    ImGui::Unindent();
}

} // namespace SZM::UI
