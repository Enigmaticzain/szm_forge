#include "VisualizationController.hpp"
#include "Advanced/EnhancedRenderer.hpp"
#include <iostream>

namespace SZM::Graphics {

VisualizationController& VisualizationController::GetInstance() {
    static VisualizationController instance;
    return instance;
}

void VisualizationController::Init() {
    std::cout << "[VisualizationController] Initialized\n";
}

void VisualizationController::SetRenderMode(RenderMode mode) {
    m_RenderMode = mode;
    EnhancedRenderer::GetInstance().SetRenderMode(mode);
}

void VisualizationController::SetLightingMode(LightingMode lighting) {
    m_LightingMode = lighting;
    EnhancedRenderer::GetInstance().SetLightingMode(lighting);
}

void VisualizationController::SetHeatmapMode(Heatmap::Mode mode) {
    m_HeatmapMode = mode;
}

} // namespace SZM::Graphics
