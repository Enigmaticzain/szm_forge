#pragma once

#include <glm/glm.hpp>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @enum GizmoMode
 * @brief Transform gizmo modes
 */
enum class GizmoMode {
    Translate,
    Rotate,
    Scale,
    None
};

/**
 * @enum GizmoAxis
 * @brief Gizmo axis selection
 */
enum class GizmoAxis {
    X,
    Y,
    Z,
    XY,
    YZ,
    XZ,
    None
};

/**
 * @struct GizmoState
 * @brief Current gizmo state
 */
struct GizmoState {
    GizmoMode mode = GizmoMode::None;
    GizmoAxis activeAxis = GizmoAxis::None;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    bool isDragging = false;
};

/**
 * @class ViewportGizmo
 * @brief Transform gizmo for viewport manipulation
 */
class ViewportGizmo {
public:
    ViewportGizmo();

    /**
     * @brief Set gizmo mode
     */
    void SetMode(GizmoMode mode) { m_State.mode = mode; }

    /**
     * @brief Get gizmo mode
     */
    GizmoMode GetMode() const { return m_State.mode; }

    /**
     * @brief Handle mouse click on gizmo
     */
    GizmoAxis HandleMouseClick(float screenX, float screenY);

    /**
     * @brief Handle mouse drag
     */
    void HandleMouseDrag(float deltaX, float deltaY);

    /**
     * @brief Handle mouse release
     */
    void HandleMouseRelease();

    /**
     * @brief Set gizmo position
     */
    void SetPosition(const glm::vec3& pos) { m_State.position = pos; }

    /**
     * @brief Get gizmo position
     */
    glm::vec3 GetPosition() const { return m_State.position; }

    /**
     * @brief Set gizmo rotation
     */
    void SetRotation(const glm::vec3& rot) { m_State.rotation = rot; }

    /**
     * @brief Get gizmo rotation
     */
    glm::vec3 GetRotation() const { return m_State.rotation; }

    /**
     * @brief Set gizmo scale
     */
    void SetScale(const glm::vec3& scale) { m_State.scale = scale; }

    /**
     * @brief Get gizmo scale
     */
    glm::vec3 GetScale() const { return m_State.scale; }

    /**
     * @brief Get gizmo state
     */
    const GizmoState& GetState() const { return m_State; }

    /**
     * @brief Render gizmo
     */
    void Render();

private:
    GizmoState m_State;
    glm::vec3 m_DragStart = glm::vec3(0.0f);
    float m_GizmoSize = 0.1f;

    void RenderTranslateGizmo();
    void RenderRotateGizmo();
    void RenderScaleGizmo();
};

} // namespace SZM::Graphics
