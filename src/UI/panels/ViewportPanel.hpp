#pragma once

#include "IPanel.hpp"
#include "../Graphics/VisualizationController.hpp"
#include "../Graphics/Viewport.hpp"
#include <memory>

namespace SZM::UI {

/**
 * @class ViewportPanel
 * @brief ImGui panel for 3D viewport with visualization controls
 */
class ViewportPanel : public IPanel {
public:
    ViewportPanel();
    ~ViewportPanel() override;

    void Draw() override;
    const char* GetName() const override { return "3D Viewport"; }

protected:
    std::unique_ptr<Graphics::Viewport> m_Viewport;
    Graphics::VisualizationController& m_VisController;

    void DrawVisualizationControls();
    void DrawRenderModeSelector();
    void DrawLightingControls();
    void DrawHeatmapSelector();
};

} // namespace SZM::UI
