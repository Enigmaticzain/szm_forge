#pragma once

#include "IPanel.hpp"
#include "../Graphics/Advanced/StressVisualizer.hpp"
#include "../Graphics/Advanced/ThermalVisualizer.hpp"

namespace SZM::UI {

/**
 * @class AnalysisResultsPanel
 * @brief Displays detailed analysis results
 */
class AnalysisResultsPanel : public IPanel {
public:
    AnalysisResultsPanel();
    ~AnalysisResultsPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Analysis Results"; }

private:
    Graphics::StressVisualizer& m_StressViz;
    Graphics::ThermalVisualizer& m_ThermalViz;

    void DrawStressResults();
    void DrawThermalResults();
    void DrawCombinedResults();
};

} // namespace SZM::UI
