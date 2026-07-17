#pragma once

#include "../TabManager.hpp"

namespace SZM {

/**
 * @class SimulationDashboardPanel
 * @brief Example tabbed panel with modern animations
 */
class SimulationDashboardPanel : public ModernTabPanel {
public:
    SimulationDashboardPanel();
    ~SimulationDashboardPanel() override = default;

    const char* GetTitle() const override { return "Simulation Dashboard"; }
    const char* GetIcon() const override { return "⚡"; }

private:
    void DrawOverviewTab();
    void DrawTasksTab();
    void DrawAnalyticsTab();
    void DrawSettingsTab();
};

} // namespace SZM
