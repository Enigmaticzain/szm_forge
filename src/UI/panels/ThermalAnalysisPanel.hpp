#pragma once

#include <string>
#include <vector>
#include <map>
#include "../../FEA/CalculiXThermalSolver.hpp"

namespace SZM::UI {

    /**
     * @class ThermalAnalysisPanel
     * @brief ImGui desktop panel for thermal analysis using CalculiX
     */
    class ThermalAnalysisPanel {
    public:
        ThermalAnalysisPanel();
        ~ThermalAnalysisPanel() = default;

        void Render();
        
        void SetSelectedComponent(int componentId);
        int GetSelectedComponent() const { return m_SelectedComponentId; }

        void SetAnalysisConfig(const SZM::FEA::CalculiXThermalConfig& config);
        const SZM::FEA::CalculiXThermalConfig& GetAnalysisConfig() const { return m_CurrentConfig; }

    private:
        void RenderInputSection();
        void RenderResultsSection();
        void RenderVisualizationSection();
        void RenderControlsSection();

        // State
        int m_SelectedComponentId = 1;
        int m_CurrentTab = 0;
        bool m_IsAnalysisRunning = false;

        // Configuration
        SZM::FEA::CalculiXThermalConfig m_CurrentConfig;
        SZM::FEA::CalculiXThermalSolver m_Solver;

        // Results
        SZM::FEA::ThermalAnalysisResult m_LastResults;
        bool m_HasResults = false;

        // UI State
        int m_ModeSelect = 0;
        float m_AmbientTempInput = 20.0f;
        float m_HeatInputInput = 100.0f;
        float m_ConvectionInput = 10.0f;
        float m_DurationInput = 60.0f;
        int m_TimeStepsInput = 100;
        float m_DensityInput = 7850.0f;
        float m_ThermalCondInput = 50.0f;
        float m_SpecificHeatInput = 490.0f;
        
        static constexpr int THERMAL_TAB_INPUT = 0;
        static constexpr int THERMAL_TAB_RESULTS = 1;
        static constexpr int THERMAL_TAB_VISUALIZATION = 2;
    };

} // namespace SZM::UI
