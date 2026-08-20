#pragma once

#include <string>
#include <vector>
#include "../../Electrical/NgspiceCircuitSolver.hpp"

namespace SZM::UI {

    /**
     * @class CircuitAnalysisPanel
     * @brief ImGui desktop panel for circuit simulation and analysis
     */
    class CircuitAnalysisPanel {
    public:
        CircuitAnalysisPanel();
        ~CircuitAnalysisPanel() = default;

        void Render();
        
        void SetSelectedComponent(int componentId);
        int GetSelectedComponent() const { return m_SelectedComponentId; }

        void AddComponent(const SZM::Electrical::CircuitComponent& component);
        void RemoveComponent(size_t index);

    private:
        void RenderNetlistSection();
        void RenderDCAnalysisSection();
        void RenderACAnalysisSection();
        void RenderTransientAnalysisSection();
        void RenderResultsSection();

        // State
        int m_SelectedComponentId = 1;
        int m_CurrentTab = 0;
        bool m_IsAnalysisRunning = false;

        // Circuit components
        std::vector<SZM::Electrical::CircuitComponent> m_Components;
        SZM::Electrical::CircuitNetlist m_CurrentCircuit;
        SZM::Electrical::NgspiceConfig m_CurrentConfig;
        SZM::Electrical::NgspiceCircuitSolver m_Solver;

        // Results
        SZM::Electrical::CircuitAnalysisResult m_LastResults;
        bool m_HasResults = false;

        // UI State - DC
        float m_VoltageSourceDC = 10.0f;
        float m_ResistanceDC = 1000.0f;
        int m_CircuitTypeDC = 0;  // 0=series, 1=parallel

        // UI State - AC
        float m_FrequencyAC = 1000.0f;
        float m_InductanceAC = 1e-3f;
        float m_CapacitanceAC = 1e-6f;

        // UI State - Transient
        float m_DurationTransient = 1.0f;
        int m_TimeStepsTransient = 100;

        static constexpr int CIRCUIT_TAB_NETLIST = 0;
        static constexpr int CIRCUIT_TAB_DC = 1;
        static constexpr int CIRCUIT_TAB_AC = 2;
        static constexpr int CIRCUIT_TAB_TRANSIENT = 3;
        static constexpr int CIRCUIT_TAB_RESULTS = 4;
    };

} // namespace SZM::UI
