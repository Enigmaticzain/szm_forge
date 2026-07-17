#pragma once

#include "../IPanel.hpp"
#include "../../Simulation/SimulationEngine.hpp"
#include <string>
#include <cstdint>

/**
 * @class SimulationPanel
 * @brief Control panel for simulation engine.
 *
 * Features:
 *  - Add/remove components
 *  - Apply force and heat input
 *  - Display component list with stress/temp/danger status
 *  - Color-coded danger indicators (red = dangerous, green = safe)
 */
class SimulationPanel : public IPanel {
public:
    SimulationPanel();
    ~SimulationPanel();

    void        Draw()     override;
    std::string GetTitle() const override { return "Simulation Control"; }

private:
    void DrawComponentBuilder();
    void DrawComponentList();
    void DrawSelectedComponentControls();
    void OnSimulationTick();

private:
    uint32_t m_SelectedComponentId = 0;
    char     m_ComponentNameBuffer[64] = "New Component";
    float    m_SelectedForce = 0.0f;
    float    m_SelectedHeatInput = 0.0f;
};
