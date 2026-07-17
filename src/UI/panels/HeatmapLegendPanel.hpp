#pragma once

#include "IPanel.hpp"
#include "../Graphics/Heatmap.hpp"

namespace SZM::UI {

/**
 * @class HeatmapLegendPanel
 * @brief Displays heatmap legend and color scale
 */
class HeatmapLegendPanel : public IPanel {
public:
    HeatmapLegendPanel();
    ~HeatmapLegendPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Heatmap Legend"; }

private:
    Graphics::Heatmap m_Heatmap;

    void DrawColorScale();
    void DrawLegendInfo();
};

} // namespace SZM::UI
