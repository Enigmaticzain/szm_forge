#pragma once

#include "IPanel.hpp"
#include "../Graphics/Advanced/SimulationVisualizationData.hpp"
#include "../Graphics/Advanced/SimulationResultsRenderer.hpp"

namespace SZM::UI {

/**
 * @class SimulationPlaybackPanel
 * @brief Controls for simulation playback and visualization
 */
class SimulationPlaybackPanel : public IPanel {
public:
    SimulationPlaybackPanel();
    ~SimulationPlaybackPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Simulation Playback"; }

private:
    Graphics::SimulationVisualizationData& m_VisData;
    Graphics::SimulationResultsRenderer& m_ResultsRenderer;

    void DrawPlaybackControls();
    void DrawFrameSlider();
    void DrawResultModeSelector();
    void DrawStatistics();
};

} // namespace SZM::UI
