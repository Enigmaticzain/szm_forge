#pragma once

#include "RenderTarget.hpp"
#include "Camera.hpp"
#include "Heatmap.hpp"
#include "../Simulation/SimulationEngine.hpp"
#include <glm/glm.hpp>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @class Viewport
 * @brief Manages 3D viewport rendering with ImGui integration
 *
 * Features:
 *  - Render-to-texture pipeline
 *  - Camera controls (orbit, pan, zoom)
 *  - Component rendering with heatmap
 *  - ImGui texture display
 */
class Viewport {
public:
    /**
     * @brief Create viewport with specified dimensions
     * @param width Viewport width in pixels
     * @param height Viewport height in pixels
     */
    Viewport(uint32_t width, uint32_t height);
    
    /**
     * @brief Destroy viewport
     */
    ~Viewport();
    
    // Prevent copying
    Viewport(const Viewport&) = delete;
    Viewport& operator=(const Viewport&) = delete;
    
    /**
     * @brief Resize viewport
     * @param width New width in pixels
     * @param height New height in pixels
     */
    void Resize(uint32_t width, uint32_t height);
    
    /**
     * @brief Render viewport contents
     * @param engine Simulation engine for component data
     * @param heatmap Heatmap for color mapping
     */
    void Render(const SimulationEngine& engine, const Heatmap& heatmap);
    
    /**
     * @brief Get ImGui texture ID for display
     * @return Texture ID for ImGui::Image()
     */
    uint32_t GetTextureID() const;

    /**
     * @brief Get viewport width in pixels
     */
    uint32_t GetWidth() const { return m_Width; }

    /**
     * @brief Get viewport height in pixels
     */
    uint32_t GetHeight() const { return m_Height; }
    
    /**
     * @brief Get camera
     * @return Reference to viewport camera
     */
    Camera& GetCamera() { return m_Camera; }
    
    /**
     * @brief Get camera (const)
     * @return Const reference to viewport camera
     */
    const Camera& GetCamera() const { return m_Camera; }
    
    /**
     * @brief Handle mouse input for camera control
     * @param deltaX Mouse X delta
     * @param deltaY Mouse Y delta
     * @param button Mouse button (0=left, 1=right, 2=middle)
     */
    void HandleMouseInput(float deltaX, float deltaY, int button);
    
    /**
     * @brief Handle scroll input for zoom
     * @param delta Scroll delta
     */
    void HandleScroll(float delta);

private:
    RenderTarget m_RenderTarget;
    Camera m_Camera;
    uint32_t m_Width;
    uint32_t m_Height;
    
    /**
     * @brief Clear viewport
     */
    void Clear();
    
    /**
     * @brief Render all components
     * @param engine Simulation engine
     * @param heatmap Heatmap for coloring
     */
    void RenderComponents(const SimulationEngine& engine, const Heatmap& heatmap);
};

} // namespace SZM::Graphics
