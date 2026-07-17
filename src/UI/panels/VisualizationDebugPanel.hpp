#pragma once

#include "IPanel.hpp"
#include "../Graphics/Advanced/CollisionVisualizer.hpp"
#include "../Graphics/Advanced/DeformationVisualizer.hpp"

namespace SZM::UI {

/**
 * @class VisualizationDebugPanel
 * @brief Debug panel for visualization system
 */
class VisualizationDebugPanel : public IPanel {
public:
    VisualizationDebugPanel();
    ~VisualizationDebugPanel() override;

    void Draw() override;
    const char* GetName() const override { return "Visualization Debug"; }

private:
    Graphics::CollisionVisualizer& m_CollisionViz;
    Graphics::DeformationVisualizer& m_DeformationViz;

    void DrawCollisionControls();
    void DrawDeformationControls();
    void DrawPerformanceStats();
};

} // namespace SZM::UI
