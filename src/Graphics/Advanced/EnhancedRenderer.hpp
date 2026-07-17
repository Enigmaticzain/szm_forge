#pragma once

#include "RenderMode.hpp"
#include "AdvancedMesh.hpp"
#include "../Camera.hpp"
#include "../Heatmap.hpp"
#include "../RenderTarget.hpp"
#include "../../Geometry/MathTypes.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace SZM::Graphics {

/**
 * @class EnhancedRenderer
 * @brief Advanced 3D renderer with multiple render modes and lighting
 */
class EnhancedRenderer {
public:
    static EnhancedRenderer& GetInstance();

    /**
     * @brief Initialize renderer
     */
    void Init();

    /**
     * @brief Shutdown renderer
     */
    void Shutdown();

    /**
     * @brief Set render mode
     */
    void SetRenderMode(RenderMode mode) { m_RenderMode = mode; }

    /**
     * @brief Set lighting mode
     */
    void SetLightingMode(LightingMode lighting) { m_LightingMode = lighting; }

    /**
     * @brief Render mesh with current settings
     */
    void RenderMesh(
        const AdvancedMesh& mesh,
        const glm::mat4& model,
        const glm::vec3& color,
        const Camera& camera,
        RenderTarget& target
    );

    /**
     * @brief Render mesh with wireframe overlay
     */
    void RenderMeshWithWireframe(
        const AdvancedMesh& mesh,
        const glm::mat4& model,
        const glm::vec3& color,
        const glm::vec3& wireframeColor,
        const Camera& camera,
        RenderTarget& target
    );

    /**
     * @brief Get current render mode
     */
    RenderMode GetRenderMode() const { return m_RenderMode; }

    /**
     * @brief Get current lighting mode
     */
    LightingMode GetLightingMode() const { return m_LightingMode; }

    /**
     * @brief Set light position
     */
    void SetLightPosition(const glm::vec3& pos) { m_LightPos = pos; }

    /**
     * @brief Set view position (camera)
     */
    void SetViewPosition(const glm::vec3& pos) { m_ViewPos = pos; }

private:
    EnhancedRenderer() = default;
    ~EnhancedRenderer() = default;

    EnhancedRenderer(const EnhancedRenderer&) = delete;
    EnhancedRenderer& operator=(const EnhancedRenderer&) = delete;

    RenderMode m_RenderMode = RenderMode::Solid;
    LightingMode m_LightingMode = LightingMode::Phong;
    glm::vec3 m_LightPos = glm::vec3(5.0f, 5.0f, 5.0f);
    glm::vec3 m_ViewPos = glm::vec3(0.0f, 0.0f, 5.0f);

    void SetupShaders();
    void UpdateUniforms(const Camera& camera, const glm::vec3& color);
};

} // namespace SZM::Graphics
