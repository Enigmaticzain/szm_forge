#pragma once

#include "Advanced/RenderMode.hpp"
#include "Heatmap.hpp"
#include <glm/glm.hpp>

namespace SZM::Graphics {

/**
 * @class VisualizationController
 * @brief Manages visualization settings and render modes
 */
class VisualizationController {
public:
    static VisualizationController& GetInstance();

    /**
     * @brief Initialize controller
     */
    void Init();

    /**
     * @brief Set render mode
     */
    void SetRenderMode(RenderMode mode);

    /**
     * @brief Set lighting mode
     */
    void SetLightingMode(LightingMode lighting);

    /**
     * @brief Set heatmap mode
     */
    void SetHeatmapMode(Heatmap::Mode mode);

    /**
     * @brief Toggle wireframe overlay
     */
    void ToggleWireframe() { m_ShowWireframe = !m_ShowWireframe; }

    /**
     * @brief Toggle lighting
     */
    void ToggleLighting() { m_LightingEnabled = !m_LightingEnabled; }

    /**
     * @brief Get current render mode
     */
    RenderMode GetRenderMode() const { return m_RenderMode; }

    /**
     * @brief Get current lighting mode
     */
    LightingMode GetLightingMode() const { return m_LightingMode; }

    /**
     * @brief Get current heatmap mode
     */
    Heatmap::Mode GetHeatmapMode() const { return m_HeatmapMode; }

    /**
     * @brief Check if wireframe is shown
     */
    bool IsWireframeVisible() const { return m_ShowWireframe; }

    /**
     * @brief Check if lighting is enabled
     */
    bool IsLightingEnabled() const { return m_LightingEnabled; }

    /**
     * @brief Set light position
     */
    void SetLightPosition(const glm::vec3& pos) { m_LightPos = pos; }

    /**
     * @brief Get light position
     */
    glm::vec3 GetLightPosition() const { return m_LightPos; }

    /**
     * @brief Set ambient strength
     */
    void SetAmbientStrength(float strength) { m_AmbientStrength = strength; }

    /**
     * @brief Get ambient strength
     */
    float GetAmbientStrength() const { return m_AmbientStrength; }

private:
    VisualizationController() = default;
    ~VisualizationController() = default;

    VisualizationController(const VisualizationController&) = delete;
    VisualizationController& operator=(const VisualizationController&) = delete;

    RenderMode m_RenderMode = RenderMode::Solid;
    LightingMode m_LightingMode = LightingMode::Phong;
    Heatmap::Mode m_HeatmapMode = Heatmap::Mode::Stress;
    bool m_ShowWireframe = false;
    bool m_LightingEnabled = true;
    glm::vec3 m_LightPos = glm::vec3(5.0f, 5.0f, 5.0f);
    float m_AmbientStrength = 0.3f;
};

} // namespace SZM::Graphics
