#include "Phase3Integration.hpp"

namespace SZM::Graphics {

bool Phase3Integration::s_Initialized = false;

void Phase3Integration::Init() {
    if (s_Initialized) return;
    
    GraphicsManager::GetInstance().Init();
    s_Initialized = true;
}

void Phase3Integration::Shutdown() {
    if (!s_Initialized) return;
    
    GraphicsManager::GetInstance().Shutdown();
    s_Initialized = false;
}

void Phase3Integration::Update(const SimulationEngine& engine) {
    if (!s_Initialized) return;
    
    GraphicsManager::GetInstance().Update(engine);
}

GraphicsManager& Phase3Integration::GetGraphicsManager() {
    return GraphicsManager::GetInstance();
}

void Phase3Integration::SetVisualizationMode(Heatmap::Mode mode) {
    GraphicsManager::GetInstance().SetHeatmapMode(mode);
}

Heatmap::Mode Phase3Integration::GetVisualizationMode() {
    return GraphicsManager::GetInstance().GetHeatmap().GetMode();
}

} // namespace SZM::Graphics
