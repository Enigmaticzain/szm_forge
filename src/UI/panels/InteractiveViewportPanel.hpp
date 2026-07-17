#pragma once

#include "ViewportPanel.hpp"
#include "../Graphics/Advanced/InteractiveRenderer.hpp"

namespace SZM::UI {

/**
 * @class InteractiveViewportPanel
 * @brief Viewport with selection, picking, and highlighting
 */
class InteractiveViewportPanel : public ViewportPanel {
public:
    InteractiveViewportPanel();
    ~InteractiveViewportPanel() override;

    void Draw() override;

protected:
    Graphics::InteractiveRenderer& m_InteractiveRenderer;
    bool m_IsMouseOver = false;
    float m_LastMouseX = 0.0f;
    float m_LastMouseY = 0.0f;

    void HandleMouseInput();
    void DrawSelectionInfo();
};

} // namespace SZM::UI
