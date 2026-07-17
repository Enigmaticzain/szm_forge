#pragma once

#include "../ModernPanelBase.hpp"

namespace SZM {

/**
 * @class ModernDashboardPanel
 * @brief Modern dashboard with metrics cards and status overview
 */
class ModernDashboardPanel : public ModernPanelBase {
public:
    ModernDashboardPanel() = default;
    ~ModernDashboardPanel() override = default;

    const char* GetName() const override { return "Modern Dashboard"; }
    std::string GetTitle() const override { return "Dashboard"; }
    const char* GetIcon() const override { return "📊"; }

protected:
    void DrawContent() override;
    void DrawHeader() override;

private:
    void DrawMetricsCards();
    void DrawSystemHealth();
    void DrawRecentActivity();
};

} // namespace SZM
