#include "GraphicsManager.hpp"
#include "ComponentRenderer.hpp"

namespace SZM::Graphics {

GraphicsManager& GraphicsManager::GetInstance() {
    static GraphicsManager instance;
    return instance;
}

void GraphicsManager::Init() {
    if (m_Initialized) return;
    
    m_Viewport = std::make_unique<Viewport>(1280, 720);
    ComponentRenderer::Init();
    m_Initialized = true;
}

void GraphicsManager::Shutdown() {
    if (!m_Initialized) return;
    
    ComponentRenderer::Shutdown();
    m_Viewport.reset();
    m_Initialized = false;
}

void GraphicsManager::Update(const SimulationEngine& engine) {
    if (!m_Initialized) return;
    
    m_Viewport->Render(engine, m_Heatmap);
}

} // namespace SZM::Graphics
