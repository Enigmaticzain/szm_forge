#pragma once

#include "IPanel.hpp"
#include "UIStyleHelper.hpp"
#include <string>

namespace SZM {

/**
 * @class ModernPanelBase
 * @brief Base class for modern card-styled panels
 */
class ModernPanelBase : public IPanel {
public:
    virtual ~ModernPanelBase() = default;

    void Draw() final;

protected:
    // Override this to draw panel content
    virtual void DrawContent() = 0;

    // Optional: customize header
    virtual void DrawHeader() {}

    // Panel metadata
    virtual const char* GetIcon() const { return nullptr; }
    virtual bool ShowHeader() const { return true; }

    // Helper methods
    void BeginCard();
    void EndCard();
    void DrawStatusIndicator(const std::string& status);
    void DrawProgressBar(float progress, const char* label = nullptr);

private:
    bool m_CardOpen = true;
};

} // namespace SZM
