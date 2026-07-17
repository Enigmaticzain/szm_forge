#pragma once

#include "../ModernPanelBase.hpp"
#include <vector>
#include <string>

namespace SZM {

/**
 * @class ModernSimulationPanel
 * @brief Modern simulation control with task queue and progress
 */
class ModernSimulationPanel : public ModernPanelBase {
public:
    ModernSimulationPanel() = default;
    ~ModernSimulationPanel() override = default;

    const char* GetName() const override { return "Modern Simulation"; }
    std::string GetTitle() const override { return "Simulation Control"; }
    const char* GetIcon() const override { return "⚡"; }

protected:
    void DrawContent() override;
    void DrawHeader() override;

private:
    struct SimulationTask {
        std::string name;
        std::string type;
        std::string status;
        float progress;
    };

    void DrawSimulationControls();
    void DrawTaskQueue();
    void DrawProgressMetrics();

    std::vector<SimulationTask> m_Tasks = {
        {"Thermal Analysis", "thermal", "running", 0.65f},
        {"Stress Test", "structural", "complete", 1.0f},
        {"CFD Simulation", "fluid", "pending", 0.0f}
    };
};

} // namespace SZM
