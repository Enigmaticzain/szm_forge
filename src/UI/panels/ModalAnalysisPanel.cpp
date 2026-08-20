#include "ModalAnalysisPanel.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

namespace SZM::UI {

ModalAnalysisPanel::ModalAnalysisPanel()
    : m_SelectedComponentId(1), m_CurrentTab(0), m_IsAnalysisRunning(false), m_HasResults(false) {
    m_CurrentConfig.numModes = 10;
    m_CurrentConfig.density = 7850.0;
    m_CurrentConfig.youngsModulus = 210e9;
    m_CurrentConfig.poissonRatio = 0.3;
    m_CurrentConfig.minFrequency = 0.0;
    m_CurrentConfig.maxFrequency = 10000.0;
}

void ModalAnalysisPanel::Render() {
    ImGui::SetNextWindowSize(ImVec2(550, 700), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Modal Analysis", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    // Header
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "MODAL ANALYSIS");
    ImGui::SameLine();
    if (m_HasResults) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[%d Modes Computed]", m_LastResults.numModesComputed);
    }
    ImGui::Separator();

    // Tabbed interface
    if (ImGui::BeginTabBar("ModalTabs")) {
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

        if (ImGui::BeginTabItem("Frequencies", nullptr, 0)) {
            if (m_HasResults) {
                RenderFrequencySection();
            } else {
                ImGui::TextDisabled("No frequency data yet.");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Mode Shapes", nullptr, 0)) {
            if (m_HasResults && !m_LastResults.modeShapes.empty()) {
                RenderModeShapesSection();
            } else {
                ImGui::TextDisabled("No mode shape data yet.");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ModalAnalysisPanel::RenderInputSection() {
    // Component ID
    ImGui::SliderInt("Component ID##modal", &m_SelectedComponentId, 1, 100);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Analysis Parameters");

    // Number of modes
    ImGui::SliderInt("Number of Modes##modal", &m_NumModesInput, 1, 20);
    m_CurrentConfig.numModes = m_NumModesInput;

    // Frequency range
    ImGui::DragFloat("Min Frequency (Hz)##modal", &m_MinFrequencyInput, 10.0f, 0.0f, 100000.0f);
    m_CurrentConfig.minFrequency = m_MinFrequencyInput;

    ImGui::DragFloat("Max Frequency (Hz)##modal", &m_MaxFrequencyInput, 100.0f, 0.0f, 100000.0f);
    m_CurrentConfig.maxFrequency = m_MaxFrequencyInput;

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Material Properties");

    ImGui::DragFloat("Density (kg/m³)##modal", &m_DensityInput, 100.0f, 100.0f, 100000.0f);
    m_CurrentConfig.density = m_DensityInput;

    // Young's modulus (in scientific notation)
    ImGui::DragScalar("Young's Modulus (Pa)##modal", ImGuiDataType_Double, 
                     &m_CurrentConfig.youngsModulus, 1e8f, nullptr, nullptr, "%.2e");

    ImGui::DragFloat("Poisson's Ratio##modal", &m_PoissonRatioInput, 0.01f, 0.0f, 0.5f);
    m_CurrentConfig.poissonRatio = m_PoissonRatioInput;

    ImGui::Separator();

    // Run button
    if (ImGui::Button("Run Modal Analysis##btn", ImVec2(-1, 0))) {
        m_IsAnalysisRunning = true;
        m_LastResults = m_Solver.RunModalAnalysis(m_CurrentConfig);
        m_HasResults = m_LastResults.success;
        m_IsAnalysisRunning = false;
    }

    if (m_IsAnalysisRunning) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Computing...");
    }
}

void ModalAnalysisPanel::RenderResultsSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Analysis Summary");
    ImGui::Separator();

    ImGui::Text("Engine Used:       %s", m_LastResults.engineUsed.c_str());
    ImGui::Text("Status:            %s", m_LastResults.success ? "SUCCESS" : "FAILED");
    ImGui::Text("Modes Computed:    %d", m_LastResults.numModesComputed);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Frequency Summary");
    ImGui::Text("  1st Mode (Hz):   %.2f", m_LastResults.firstNaturalFrequency);
    ImGui::Text("  Frequency Range: %.2f - %.2f Hz", 
               m_LastResults.lowestFrequency, m_LastResults.highestFrequency);
    
    double freqSpan = m_LastResults.highestFrequency - m_LastResults.lowestFrequency;
    ImGui::Text("  Frequency Span:  %.2f Hz", freqSpan);
}

void ModalAnalysisPanel::RenderFrequencySection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Natural Frequencies");
    ImGui::Separator();

    // Create frequency plot
    if (!m_LastResults.naturalFrequencies.empty()) {
        std::vector<float> freqData;
        for (double freq : m_LastResults.naturalFrequencies) {
            freqData.push_back(static_cast<float>(freq));
        }

        float minFreq = *std::min_element(freqData.begin(), freqData.end());
        float maxFreq = *std::max_element(freqData.begin(), freqData.end());

        ImGui::PlotBars("Frequencies (Hz)##modal", freqData.data(), freqData.size(), 0,
                       nullptr, minFreq * 0.9f, maxFreq * 1.1f, ImVec2(500, 200));

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Frequency List");

        if (ImGui::BeginChild("FrequencyTable", ImVec2(0, 250))) {
            ImGui::Columns(3, "FreqColumns");
            ImGui::SetColumnWidth(0, 50);
            ImGui::SetColumnWidth(1, 100);
            ImGui::Text("Mode");
            ImGui::NextColumn();
            ImGui::Text("Freq (Hz)");
            ImGui::NextColumn();
            ImGui::Text("Wavelength (m)");
            ImGui::NextColumn();
            ImGui::Separator();

            for (size_t i = 0; i < m_LastResults.naturalFrequencies.size(); ++i) {
                ImGui::Text("%zu", i + 1);
                ImGui::NextColumn();
                ImGui::Text("%.4f", m_LastResults.naturalFrequencies[i]);
                ImGui::NextColumn();
                double wavelength = 1.0 / (m_LastResults.naturalFrequencies[i] + 0.001);
                ImGui::Text("%.6f", wavelength);
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
            ImGui::EndChild();
        }
    } else {
        ImGui::TextDisabled("No frequency data available.");
    }
}

void ModalAnalysisPanel::RenderModeShapesSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Mode Shapes");
    ImGui::Separator();

    if (ImGui::SliderInt("Selected Mode##shape", &m_SelectedModeIndex, 0, 
                        static_cast<int>(m_LastResults.modeShapes.size()) - 1)) {
        // Mode selection changed
    }

    if (m_SelectedModeIndex < static_cast<int>(m_LastResults.modeShapes.size())) {
        const auto& modeShape = m_LastResults.modeShapes[m_SelectedModeIndex];

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Mode Properties");
        ImGui::Text("Mode Number:      %d", modeShape.modeNumber);
        ImGui::Text("Frequency:        %.4f Hz", modeShape.frequency);
        ImGui::Text("Wavelength:       %.6f m", modeShape.wavelength);
        ImGui::Text("Damping Ratio:    %.2f%%", modeShape.dampingRatio);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Nodal Displacements");

        if (!modeShape.nodalDisplacements.empty()) {
            std::vector<float> dispData;
            for (double disp : modeShape.nodalDisplacements) {
                dispData.push_back(static_cast<float>(disp));
            }

            float minDisp = *std::min_element(dispData.begin(), dispData.end());
            float maxDisp = *std::max_element(dispData.begin(), dispData.end());

            ImGui::PlotLines("Displacement", dispData.data(), dispData.size(), 0,
                           nullptr, minDisp - 0.1f, maxDisp + 0.1f, ImVec2(500, 200));

            ImGui::Separator();

            if (ImGui::BeginChild("DisplacementTable", ImVec2(0, 150))) {
                ImGui::Columns(2, "DispColumns");
                ImGui::SetColumnWidth(0, 50);
                ImGui::Text("Node");
                ImGui::NextColumn();
                ImGui::Text("Displacement (m)");
                ImGui::NextColumn();
                ImGui::Separator();

                for (size_t i = 0; i < modeShape.nodalDisplacements.size(); ++i) {
                    ImGui::Text("%zu", i + 1);
                    ImGui::NextColumn();
                    ImGui::Text("%.6f", modeShape.nodalDisplacements[i]);
                    ImGui::NextColumn();
                }

                ImGui::Columns(1);
                ImGui::EndChild();
            }
        }
    }
}

void ModalAnalysisPanel::SetSelectedComponent(int componentId) {
    m_SelectedComponentId = componentId;
}

void ModalAnalysisPanel::SetAnalysisConfig(const SZM::FEA::CalculiXModalConfig& config) {
    m_CurrentConfig = config;
    m_NumModesInput = config.numModes;
    m_MinFrequencyInput = static_cast<float>(config.minFrequency);
    m_MaxFrequencyInput = static_cast<float>(config.maxFrequency);
    m_DensityInput = static_cast<float>(config.density);
    m_PoissonRatioInput = static_cast<float>(config.poissonRatio);
}

} // namespace SZM::UI
