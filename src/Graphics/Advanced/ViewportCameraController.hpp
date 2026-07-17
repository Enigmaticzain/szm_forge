#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SZM::Graphics {

/**
 * @enum CameraMode
 * @brief Camera navigation modes
 */
enum class CameraMode {
    Orbit,      ///< Orbit around target
    Pan,        ///< Pan camera
    Zoom,       ///< Zoom in/out
    FirstPerson ///< First-person view
};

/**
 * @class ViewportCameraController
 * @brief Advanced camera control for viewport
 */
class ViewportCameraController {
public:
    ViewportCameraController();

    /**
     * @brief Handle mouse drag
     */
    void HandleMouseDrag(float deltaX, float deltaY, CameraMode mode);

    /**
     * @brief Handle mouse scroll
     */
    void HandleMouseScroll(float delta);

    /**
     * @brief Handle keyboard input
     */
    void HandleKeyboard(int key, bool pressed);

    /**
     * @brief Reset camera to default view
     */
    void ResetView();

    /**
     * @brief Frame all objects (fit to view)
     */
    void FrameAll(const glm::vec3& sceneCenter, float sceneRadius);

    /**
     * @brief Frame selected object
     */
    void FrameSelection(const glm::vec3& selectionCenter, float selectionRadius);

    /**
     * @brief Get view matrix
     */
    glm::mat4 GetViewMatrix() const;

    /**
     * @brief Get projection matrix
     */
    glm::mat4 GetProjectionMatrix() const;

    /**
     * @brief Get camera position
     */
    glm::vec3 GetPosition() const { return m_Position; }

    /**
     * @brief Get camera target
     */
    glm::vec3 GetTarget() const { return m_Target; }

    /**
     * @brief Set viewport size
     */
    void SetViewportSize(uint32_t width, uint32_t height);

    /**
     * @brief Set camera speed
     */
    void SetCameraSpeed(float speed) { m_CameraSpeed = speed; }

    /**
     * @brief Get camera speed
     */
    float GetCameraSpeed() const { return m_CameraSpeed; }

    /**
     * @brief Set zoom speed
     */
    void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }

    /**
     * @brief Get zoom speed
     */
    float GetZoomSpeed() const { return m_ZoomSpeed; }

private:
    glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    float m_Distance = 5.0f;
    float m_CameraSpeed = 0.01f;
    float m_ZoomSpeed = 0.1f;
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    uint32_t m_ViewportWidth = 1280;
    uint32_t m_ViewportHeight = 720;
    float m_AspectRatio = 16.0f / 9.0f;
    float m_FOV = 45.0f;

    void UpdatePosition();
};

} // namespace SZM::Graphics
