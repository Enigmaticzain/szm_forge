#include "ThermalAnalysisPanel.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

namespace SZM::UI {

ThermalAnalysisPanel::ThermalAnalysisPanel()
    : m_SelectedComponentId(1), m_CurrentTab(0), m_IsAnalysisRunning(false), m_HasResults(false) {
    m_CurrentConfig.mode = SZM::FEA::AnalysisMode::STEADY_STATE;
    m_CurrentConfig.ambientTemperature = 20.0;
    m_CurrentConfig.heatInput = 100.0;
    m_CurrentConfig.convectionCoefficient = 10.0;
    m_CurrentConfig.duration = 60.0;
    m_CurrentConfig.timeSteps = 100;
    m_CurrentConfig.density = 7850.0;
    m_CurrentConfig.thermalConductivity = 50.0;
    m_CurrentConfig.specificHeat = 500.0;
}

void ThermalAnalysisPanel::Render() {
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Thermal Analysis", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    // Header
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "THERMAL ANALYSIS");
    ImGui::SameLine();
    if (m_HasResults) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Results Available]");
    }
    ImGui::Separator();

    // Tabbed interface
    if (ImGui::BeginTabBar("ThermalTabs")) {
        if (ImGui::BeginTabItem("Input", nullptr, 0)) {
            RenderInputSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Results", nullptr, 0)) {
            if (m_HasResults) {
                RenderResultsSection();
            } else {
                ImGui::TextDisabled("No analysis results yet. Run analysis from Input tab.");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Visualization", nullptr, 0)) {
            if (m_HasResults) {
                RenderVisualizationSection();
            } else {
                ImGui::TextDisabled("No visualization data yet.");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ThermalAnalysisPanel::RenderInputSection() {
    // Component ID
    ImGui::SliderInt("Component ID##thermal", &m_SelectedComponentId, 1, 100);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Analysis Parameters");

    // Analysis mode
    static int modeSelect = 0;
    if (ImGui::RadioButton("Steady State##mode", &modeSelect, 0)) {
        m_CurrentConfig.mode = SZM::FEA::AnalysisMode::STEADY_STATE;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Transient##mode", &modeSelect, 1)) {
        m_CurrentConfig.mode = SZM::FEA::AnalysisMode::TRANSIENT;
    }

    // Temperature parameters
    ImGui::DragFloat("Ambient Temp (°C)##thermal", &m_AmbientTempInput, 1.0f, -100.0f, 500.0f);
    m_CurrentConfig.ambientTemperature = m_AmbientTempInput;

    ImGui::DragFloat("Heat Input (W)##thermal", &m_HeatInputInput, 10.0f, 0.0f, 10000.0f);
    m_CurrentConfig.heatInput = m_HeatInputInput;

    ImGui::DragFloat("Convection Coeff (W/m²·K)##thermal", &m_ConvectionInput, 0.5f, 0.1f, 1000.0f);
    m_CurrentConfig.convectionCoefficient = m_ConvectionInput;

    if (m_CurrentConfig.mode == SZM::FEA::AnalysisMode::TRANSIENT) {
        ImGui::DragFloat("Duration (s)##thermal", &m_DurationInput, 5.0f, 0.1f, 10000.0f);
        m_CurrentConfig.duration = m_DurationInput;

        ImGui::SliderInt("Time Steps##thermal", &m_TimeStepsInput, 10, 1000);
        m_CurrentConfig.timeSteps = m_TimeStepsInput;
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Material Properties");

    ImGui::DragFloat("Density (kg/m³)##thermal", &m_CurrentConfig.density, 100.0f, 100.0f, 100000.0f);
    ImGui::DragFloat("Thermal Cond (W/m·K)##thermal", &m_CurrentConfig.thermalConductivity, 1.0f, 0.1f, 1000.0f);
    ImGui::DragFloat("Specific Heat (J/kg·K)##thermal", &m_CurrentConfig.specificHeat, 10.0f, 100.0f, 10000.0f);

    ImGui::Separator();

    // Run button
    if (ImGui::Button("Run Thermal Analysis##btn", ImVec2(-1, 0))) {
        m_IsAnalysisRunning = true;
        m_LastResults = m_Solver.RunThermalAnalysis(m_CurrentConfig);
        m_HasResults = m_LastResults.success;
        m_IsAnalysisRunning = false;
    }

    if (m_IsAnalysisRunning) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Running...");
    }
}

void ThermalAnalysisPanel::RenderResultsSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Analysis Results");
    ImGui::Separator();

    ImGui::Text("Engine Used:     %s", m_LastResults.engineUsed.c_str());
    ImGui::Text("Analysis Status: %s", m_LastResults.success ? "SUCCESS" : "FAILED");
    ImGui::Separator();

    // Temperature results
    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Temperature Results (°C)");
    ImGui::Text("  Ambient:         %.2f", m_CurrentConfig.ambientTemperature);
    ImGui::Text("  Maximum:         %.2f", m_LastResults.maxTemperature);
    ImGui::Text("  Minimum:         %.2f", m_LastResults.minTemperature);
    ImGui::Text("  Average:         %.2f", m_LastResults.averageTemperature);
    ImGui::Text("  Rise:            %.2f", m_LastResults.temperatureRise);

    ImGui::Separator();

    // Danger status
    if (m_LastResults.isDangerous) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "⚠ DANGEROUS CONDITIONS");
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Temperature exceeds safe operating threshold!");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ SAFE - Within acceptable temperature range");
    }
}

void ThermalAnalysisPanel::RenderVisualizationSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Temperature Field");
    ImGui::Separator();

    // Create temperature field plot
    if (!m_LastResults.temperatureField.empty()) {
        std::vector<float> tempData;
        float minTemp = *std::min_element(m_LastResults.temperatureField.begin(), 
                                         m_LastResults.temperatureField.end());
        float maxTemp = *std::max_element(m_LastResults.temperatureField.begin(), 
                                         m_LastResults.temperatureField.end());

        for (double temp : m_LastResults.temperatureField) {
            tempData.push_back(static_cast<float>(temp));
        }

        ImGui::Text("Temperature Range: %.2f°C to %.2f°C", minTemp, maxTemp);
        ImGui::PlotLines("Temperature (°C)", tempData.data(), tempData.size(), 0, 
                        nullptr, minTemp - 5, maxTemp + 5, ImVec2(450, 150));

        // Node temperature list
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Nodal Temperatures");

        if (ImGui::BeginChild("TempFieldTable", ImVec2(0, 150))) {
            ImGui::Columns(2, "TempColumns");
            ImGui::SetColumnWidth(0, 50);
            ImGui::Text("Node");
            ImGui::NextColumn();
            ImGui::Text("Temp (°C)");
            ImGui::NextColumn();
            ImGui::Separator();

            for (size_t i = 0; i < m_LastResults.temperatureField.size(); ++i) {
                ImGui::Text("%zu", i + 1);
                ImGui::NextColumn();
                ImGui::Text("%.2f", m_LastResults.temperatureField[i]);
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
            ImGui::EndChild();
        }
    } else {
        ImGui::TextDisabled("No temperature field data available.");
    }
}

void ThermalAnalysisPanel::RenderControlsSection() {
    ImGui::Separator();
    if (ImGui::Button("Export Results", ImVec2(-1, 0))) {
        // Export functionality would go here
    }
}

void ThermalAnalysisPanel::SetSelectedComponent(int componentId) {
    m_SelectedComponentId = componentId;
}

void ThermalAnalysisPanel::SetAnalysisConfig(const SZM::FEA::CalculiXThermalConfig& config) {
    m_CurrentConfig = config;
    m_AmbientTempInput = static_cast<float>(config.ambientTemperature);
    m_HeatInputInput = static_cast<float>(config.heatInput);
    m_ConvectionInput = static_cast<float>(config.convectionCoefficient);
    m_DurationInput = static_cast<float>(config.duration);
    m_TimeStepsInput = config.timeSteps;
}

} // namespace SZM::UI
