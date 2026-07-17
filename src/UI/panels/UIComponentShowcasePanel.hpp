#pragma once

#include "../ModernPanelBase.hpp"

namespace SZM {

/**
 * @class UIComponentShowcasePanel
 * @brief Demonstrates all modern UI components and patterns
 */
class UIComponentShowcasePanel : public ModernPanelBase {
public:
    UIComponentShowcasePanel() = default;
    ~UIComponentShowcasePanel() override = default;

    const char* GetName() const override { return "UI Showcase"; }
    std::string GetTitle() const override { return "Component Showcase"; }
    const char* GetIcon() const override { return "🎨"; }

protected:
    void DrawContent() override;

private:
    void ShowColors();
    void ShowButtons();
    void ShowStatusBadges();
    void ShowProgressBars();
    void ShowCards();
};

} // namespace SZM
