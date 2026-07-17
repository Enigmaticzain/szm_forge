#pragma once

#include "Viewport.hpp"
#include "Heatmap.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include <memory>

namespace SZM::Graphics {

/**
 * @class GraphicsManager
 * @brief Singleton managing all graphics subsystems
 *
 * Features:
 *  - Viewport management
 *  - Heatmap configuration
 *  - Render pipeline coordination
 *  - ImGui integration
 */
class GraphicsManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to GraphicsManager
     */
    static GraphicsManager& GetInstance();
    
    /**
     * @brief Initialize graphics subsystems
     */
    void Init();
    
    /**
     * @brief Shutdown graphics subsystems
     */
    void Shutdown();
    
    /**
     * @brief Update and render frame
     * @param engine Simulation engine for component data
     */
    void Update(const SimulationEngine& engine);
    
    /**
     * @brief Get viewport
     * @return Reference to viewport
     */
    Viewport& GetViewport() { return *m_Viewport; }
    
    /**
     * @brief Get heatmap
     * @return Reference to heatmap
     */
    Heatmap& GetHeatmap() { return m_Heatmap; }
    
    /**
     * @brief Set heatmap mode
     * @param mode Heatmap visualization mode
     */
    void SetHeatmapMode(Heatmap::Mode mode) { m_Heatmap.SetMode(mode); }

private:
    GraphicsManager() = default;
    ~GraphicsManager() = default;
    
    // Prevent copying
    GraphicsManager(const GraphicsManager&) = delete;
    GraphicsManager& operator=(const GraphicsManager&) = delete;
    
    std::unique_ptr<Viewport> m_Viewport;
    Heatmap m_Heatmap;
    bool m_Initialized = false;
};

} // namespace SZM::Graphics
