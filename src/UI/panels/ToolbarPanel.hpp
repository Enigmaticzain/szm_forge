#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/BridgeClient.hpp"
#include <chrono>
#include <optional>
#include <string>

/**
 * @class ToolbarPanel
 * @brief Enhanced toolbar with simulation playback, speed control, and view modes.
 *
 * Features:
 *  - Play/Pause/Reset simulation controls
 *  - Simulation speed adjustment (0.1x - 2.0x)
 *  - View mode toggles (stress/temperature heatmap)
 *  - Quick component add buttons
 *  - Real-time status display
 */
class ToolbarPanel : public IPanel {
public:
    ToolbarPanel();
    ~ToolbarPanel() override = default;

    void        Draw()     override;
    std::string GetTitle() const override { return "Toolbar"; }

private:
    void DrawPlaybackControls();
    void DrawSpeedControl();
    void DrawViewModes();
    void DrawQuickAdd();
    void DrawStatus();

private:
    bool m_ShowStressHeatmap = true;
    bool m_ShowTemperatureHeatmap = false;
    float m_SimulationSpeed;
    std::optional<SZM::BridgeClient::StackStatus> m_BridgeStatus;
    std::chrono::steady_clock::time_point m_LastBridgeRefresh{};
    double m_BridgeRefreshIntervalSec = 10.0;
};
