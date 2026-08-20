#include "CircuitAnalysisPanel.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

namespace SZM::UI {

CircuitAnalysisPanel::CircuitAnalysisPanel()
    : m_SelectedComponentId(1), m_CurrentTab(0), m_IsAnalysisRunning(false), m_HasResults(false) {
    m_CurrentConfig.analysisType = SZM::Electrical::AnalysisType::DC;
    m_CurrentConfig.temperature_C = 27.0;
    m_CurrentConfig.frequency_Hz = 1000.0;
    m_CurrentConfig.duration_s = 1.0;
    m_CurrentConfig.timeSteps = 100;

    // Initialize with a default simple circuit
    m_CurrentCircuit.title = "SZM Circuit - Component 1";
}

void CircuitAnalysisPanel::Render() {
    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
    
    if (!ImGui::Begin("Circuit Analysis", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    // Header
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "CIRCUIT ANALYSIS");
    ImGui::SameLine();
    if (m_HasResults) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[Results Ready]");
    }
    ImGui::Separator();

    // Component ID selector
    ImGui::SliderInt("Component ID##circuit", &m_SelectedComponentId, 1, 100);

    // Tabbed interface
    if (ImGui::BeginTabBar("CircuitTabs")) {
        if (ImGui::BeginTabItem("Netlist", nullptr, 0)) {
            RenderNetlistSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("DC Analysis", nullptr, 0)) {
            RenderDCAnalysisSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("AC Analysis", nullptr, 0)) {
            RenderACAnalysisSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Transient", nullptr, 0)) {
            RenderTransientAnalysisSection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Results", nullptr, 0)) {
            if (m_HasResults) {
                RenderResultsSection();
            } else {
                ImGui::TextDisabled("No results yet. Run an analysis from any tab above.");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void CircuitAnalysisPanel::RenderNetlistSection() {
    ImGui::TextColored(ImVec4(0.8f, 1.0f, 0.2f, 1.0f), "Circuit Components");
    ImGui::Separator();

    ImGui::Text("Component Count: %zu", m_Components.size());

    // Display component list
    if (ImGui::BeginChild("ComponentList", ImVec2(0, 200))) {
        ImGui::Columns(5, "ComponentColumns");
        ImGui::SetColumnWidth(0, 40);
        ImGui::SetColumnWidth(1, 60);
        ImGui::SetColumnWidth(2, 80);
        ImGui::SetColumnWidth(3, 50);
        ImGui::SetColumnWidth(4, 60);

        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Type");
        ImGui::NextColumn();
        ImGui::Text("Value");
        ImGui::NextColumn();
        ImGui::Text("N1→N2");
        ImGui::NextColumn();
        ImGui::Text("Action");
        ImGui::NextColumn();
        ImGui::Separator();

        for (size_t i = 0; i < m_Components.size(); ++i) {
            const auto& comp = m_Components[i];
            ImGui::Text("%s", comp.name.c_str());
            ImGui::NextColumn();

            const char* type_str = "";
            switch (comp.type) {
                case SZM::Electrical::ComponentType::RESISTOR: type_str = "R"; break;
                case SZM::Electrical::ComponentType::CAPACITOR: type_str = "C"; break;
                case SZM::Electrical::ComponentType::INDUCTOR: type_str = "L"; break;
                case SZM::Electrical::ComponentType::VOLTAGE_SOURCE: type_str = "V"; break;
                case SZM::Electrical::ComponentType::CURRENT_SOURCE: type_str = "I"; break;
                case SZM::Electrical::ComponentType::DIODE: type_str = "D"; break;
                default: type_str = "?"; break;
            }
            ImGui::Text("%s", type_str);
            ImGui::NextColumn();

            ImGui::Text("%.2f", comp.value);
            ImGui::NextColumn();

            ImGui::Text("%d→%d", comp.node1, comp.node2);
            ImGui::NextColumn();

            ImGui::PushID(static_cast<int>(i));
            if (ImGui::SmallButton("Del##comp")) {
                RemoveComponent(i);
            }
            ImGui::PopID();
            ImGui::NextColumn();
        }

        ImGui::Columns(1);
        ImGui::EndChild();
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Add Component");

    static int comp_type_select = 0;
    static char comp_name[32] = "R1";
    static float comp_value = 1000.0f;
    static int comp_node1 = 1;
    static int comp_node2 = 0;

    ImGui::Combo("Component Type##add", &comp_type_select, 
                "Resistor\0Capacitor\0Inductor\0Voltage Source\0Current Source\0Diode\0\0");
    ImGui::InputText("Name##add", comp_name, sizeof(comp_name));
    ImGui::DragFloat("Value##add", &comp_value, 10.0f, 0.001f, 1e6);
    ImGui::SliderInt("Node 1##add", &comp_node1, 0, 10);
    ImGui::SliderInt("Node 2##add", &comp_node2, 0, 10);

    if (ImGui::Button("Add Component##btn", ImVec2(-1, 0))) {
        SZM::Electrical::CircuitComponent new_comp;
        new_comp.name = comp_name;
        new_comp.value = comp_value;
        new_comp.node1 = comp_node1;
        new_comp.node2 = comp_node2;

        static const SZM::Electrical::ComponentType types[] = {
            SZM::Electrical::ComponentType::RESISTOR,
            SZM::Electrical::ComponentType::CAPACITOR,
            SZM::Electrical::ComponentType::INDUCTOR,
            SZM::Electrical::ComponentType::VOLTAGE_SOURCE,
            SZM::Electrical::ComponentType::CURRENT_SOURCE,
            SZM::Electrical::ComponentType::DIODE,
        };
        new_comp.type = types[comp_type_select];

        AddComponent(new_comp);
        snprintf(comp_name, sizeof(comp_name), "R%zu", m_Components.size() + 1);
    }
}

void CircuitAnalysisPanel::RenderDCAnalysisSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "DC Operating Point Analysis");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Circuit Parameters");
    ImGui::DragFloat("Voltage Source (V)##dc", &m_VoltageSourceDC, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Total Resistance (Ω)##dc", &m_ResistanceDC, 10.0f, 1.0f, 1e6);

    ImGui::Text("Circuit Configuration:");
    ImGui::RadioButton("Series##dc", &m_CircuitTypeDC, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Parallel##dc", &m_CircuitTypeDC, 1);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Quick Calculation");

    double circuit_current = m_VoltageSourceDC / std::max(m_ResistanceDC, 1.0f);
    double power_dissipation = m_VoltageSourceDC * circuit_current;

    ImGui::Text("Circuit Current:    %.4f A", circuit_current);
    ImGui::Text("Power Dissipation:  %.4f W", power_dissipation);
    ImGui::Text("Node Voltage:       %.4f V", m_VoltageSourceDC / 2.0);

    ImGui::Separator();

    if (ImGui::Button("Run DC Analysis##btn", ImVec2(-1, 0))) {
        m_IsAnalysisRunning = true;
        m_CurrentConfig.analysisType = SZM::Electrical::AnalysisType::DC;
        m_LastResults = m_Solver.RunSimulation(m_CurrentCircuit, m_CurrentConfig);
        m_HasResults = m_LastResults.success;
        m_IsAnalysisRunning = false;
    }

    if (m_IsAnalysisRunning) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Computing...");
    }
}

void CircuitAnalysisPanel::RenderACAnalysisSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "AC Frequency Response Analysis");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Frequency Parameters");
    ImGui::DragFloat("Frequency (Hz)##ac", &m_FrequencyAC, 100.0f, 1.0f, 1e8);
    ImGui::DragFloat("Inductance (H)##ac", &m_InductanceAC, 1e-4f, 1e-9f, 1.0f, "%.3e");
    ImGui::DragFloat("Capacitance (F)##ac", &m_CapacitanceAC, 1e-7f, 1e-12f, 1e-3f, "%.3e");

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "AC Calculations");

    double omega = 2.0 * M_PI * m_FrequencyAC;
    double XL = omega * m_InductanceAC;
    double XC = (omega * m_CapacitanceAC > 0) ? 1.0 / (omega * m_CapacitanceAC) : 1e10;
    double Z = std::sqrt(1000.0 * 1000.0 + (XL - XC) * (XL - XC));  // Assume 1kΩ resistance
    double phase = std::atan2(XL - XC, 1000.0) * 180.0 / M_PI;
    double fres = 1.0 / (2.0 * M_PI * std::sqrt(m_InductanceAC * m_CapacitanceAC));

    ImGui::Text("Inductive Reactance:   %.4f Ω", XL);
    ImGui::Text("Capacitive Reactance:  %.4f Ω", XC);
    ImGui::Text("Total Impedance:       %.4f Ω", Z);
    ImGui::Text("Phase Shift:           %.2f°", phase);
    ImGui::Text("Resonant Frequency:    %.2f Hz", fres);

    ImGui::Separator();

    if (ImGui::Button("Run AC Analysis##btn", ImVec2(-1, 0))) {
        m_IsAnalysisRunning = true;
        m_CurrentConfig.analysisType = SZM::Electrical::AnalysisType::AC;
        m_CurrentConfig.frequency_Hz = m_FrequencyAC;
        m_LastResults = m_Solver.RunSimulation(m_CurrentCircuit, m_CurrentConfig);
        m_HasResults = m_LastResults.success;
        m_IsAnalysisRunning = false;
    }
}

void CircuitAnalysisPanel::RenderTransientAnalysisSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Transient Response Analysis");
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Time Domain Parameters");
    ImGui::DragFloat("Duration (s)##transient", &m_DurationTransient, 0.01f, 0.001f, 100.0f);
    ImGui::SliderInt("Time Steps##transient", &m_TimeStepsTransient, 10, 1000);

    double time_step = m_DurationTransient / std::max(1, m_TimeStepsTransient);
    ImGui::Text("Time Step: %.6f s", time_step);

    // RC time constant calculation
    double tau = 1000.0 * 1e-6;  // R*C default
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "RC Time Constant");
    ImGui::Text("τ (RC):                %.6f s", tau);
    ImGui::Text("5τ (settling):         %.6f s", 5.0 * tau);
    ImGui::Text("Simulation covers:     %.1f τ", m_DurationTransient / tau);

    ImGui::Separator();

    if (ImGui::Button("Run Transient Analysis##btn", ImVec2(-1, 0))) {
        m_IsAnalysisRunning = true;
        m_CurrentConfig.analysisType = SZM::Electrical::AnalysisType::TRANSIENT;
        m_CurrentConfig.duration_s = m_DurationTransient;
        m_CurrentConfig.timeSteps = m_TimeStepsTransient;
        m_LastResults = m_Solver.RunSimulation(m_CurrentCircuit, m_CurrentConfig);
        m_HasResults = m_LastResults.success;
        m_IsAnalysisRunning = false;
    }
}

void CircuitAnalysisPanel::RenderResultsSection() {
    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "Analysis Results");
    ImGui::Separator();

    ImGui::Text("Status:        %s", m_LastResults.success ? "SUCCESS" : "FAILED");
    ImGui::Text("Engine:        %s", m_LastResults.engineUsed.c_str());

    // Determine analysis type label
    const char* analysis_label = "";
    switch (m_LastResults.analysisType) {
        case SZM::Electrical::AnalysisType::DC: analysis_label = "DC Operating Point"; break;
        case SZM::Electrical::AnalysisType::AC: analysis_label = "AC Frequency"; break;
        case SZM::Electrical::AnalysisType::TRANSIENT: analysis_label = "Transient"; break;
        default: analysis_label = "Unknown"; break;
    }
    ImGui::Text("Type:          %s", analysis_label);

    ImGui::Separator();

    // Display results based on analysis type
    if (m_LastResults.analysisType == SZM::Electrical::AnalysisType::DC) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Node Voltages (V)");
        if (ImGui::BeginChild("DCResults", ImVec2(0, 150))) {
            for (const auto& [node, voltage] : m_LastResults.nodeVoltages) {
                ImGui::Text("Node %d:  %.4f V", node, voltage);
            }
            ImGui::EndChild();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Component Powers (W)");
        for (const auto& [comp_name, power] : m_LastResults.componentPowers) {
            ImGui::Text("%s:  %.4f W", comp_name.c_str(), power);
        }
    } else if (m_LastResults.analysisType == SZM::Electrical::AnalysisType::AC) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "AC Response");
        ImGui::Text("Frequency:       %.2f Hz", m_LastResults.frequency_Hz);
        ImGui::Text("Phase Shift:     %.2f°", m_LastResults.phaseShift_degrees);
        ImGui::Text("Resonant Freq:   %.2f Hz", m_LastResults.resonantFrequency_Hz);
    } else if (m_LastResults.analysisType == SZM::Electrical::AnalysisType::TRANSIENT) {
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "Transient Response");
        ImGui::Text("Settling Time:   %.6f s", m_LastResults.settlingTime_s);

        // Plot voltage waveform if available
        if (!m_LastResults.timePoints.empty() && !m_LastResults.voltageWaveforms.empty()) {
            auto& voltages = m_LastResults.voltageWaveforms.begin()->second;
            std::vector<float> float_voltages(voltages.begin(), voltages.end());
            ImGui::PlotLines("Voltage vs Time", float_voltages.data(), float_voltages.size(), 0,
                           nullptr, 0, 15, ImVec2(500, 150));
        }
    }
}

void CircuitAnalysisPanel::SetSelectedComponent(int componentId) {
    m_SelectedComponentId = componentId;
    m_CurrentCircuit.title = "SZM Circuit - Component " + std::to_string(componentId);
}

void CircuitAnalysisPanel::AddComponent(const SZM::Electrical::CircuitComponent& component) {
    m_Components.push_back(component);
    m_CurrentCircuit.components = m_Components;
}

void CircuitAnalysisPanel::RemoveComponent(size_t index) {
    if (index < m_Components.size()) {
        m_Components.erase(m_Components.begin() + index);
        m_CurrentCircuit.components = m_Components;
    }
}

} // namespace SZM::UI
