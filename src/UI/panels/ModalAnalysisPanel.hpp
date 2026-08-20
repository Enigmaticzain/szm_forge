#pragma once

#include <string>
#include <vector>
#include "../../FEA/CalculiXModalSolver.hpp"

namespace SZM::UI {

    /**
     * @class ModalAnalysisPanel
     * @brief ImGui desktop panel for modal/eigenvalue analysis using CalculiX
     */
    class ModalAnalysisPanel {
    public:
        ModalAnalysisPanel();
        ~ModalAnalysisPanel() = default;

        void Render();
        
        void SetSelectedComponent(int componentId);
        int GetSelectedComponent() const { return m_SelectedComponentId; }

        void SetAnalysisConfig(const SZM::FEA::CalculiXModalConfig& config);
        const SZM::FEA::CalculiXModalConfig& GetAnalysisConfig() const { return m_CurrentConfig; }

    private:
        void RenderInputSection();
        void RenderResultsSection();
        void RenderFrequencySection();
        void RenderModeShapesSection();

        // State
        int m_SelectedComponentId = 1;
        int m_CurrentTab = 0;
        bool m_IsAnalysisRunning = false;

        // Configuration
        SZM::FEA::CalculiXModalConfig m_CurrentConfig;
        SZM::FEA::CalculiXModalSolver m_Solver;

        // Results
        SZM::FEA::ModalAnalysisResult m_LastResults;
        bool m_HasResults = false;

        // UI State
        int m_NumModesInput = 10;
        float m_MinFrequencyInput = 0.0f;
        float m_MaxFrequencyInput = 10000.0f;
        float m_DensityInput = 7850.0f;
        float m_YoungsModulusInput = 210e9f;
        float m_PoissonRatioInput = 0.3f;
        int m_SelectedModeIndex = 0;
        
        static constexpr int MODAL_TAB_INPUT = 0;
        static constexpr int MODAL_TAB_RESULTS = 1;
        static constexpr int MODAL_TAB_FREQUENCIES = 2;
        static constexpr int MODAL_TAB_MODES = 3;
    };

} // namespace SZM::UI
