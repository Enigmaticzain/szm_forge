#include "ViewportCameraController.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

namespace SZM::Graphics {

ViewportCameraController::ViewportCameraController() {
    UpdatePosition();
}

void ViewportCameraController::HandleMouseDrag(float deltaX, float deltaY, CameraMode mode) {
    switch (mode) {
        case CameraMode::Orbit:
            m_Yaw += deltaX * m_CameraSpeed;
            m_Pitch += deltaY * m_CameraSpeed;
            m_Pitch = glm::clamp(m_Pitch, -glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);
            UpdatePosition();
            break;

        case CameraMode::Pan:
            m_Target.x -= deltaX * m_CameraSpeed * 0.1f;
            m_Target.y += deltaY * m_CameraSpeed * 0.1f;
            UpdatePosition();
            break;

        case CameraMode::Zoom:
            m_Distance -= deltaY * m_ZoomSpeed;
            m_Distance = glm::max(m_Distance, 0.1f);
            UpdatePosition();
            break;

        case CameraMode::FirstPerson:
            m_Yaw += deltaX * m_CameraSpeed;
            m_Pitch += deltaY * m_CameraSpeed;
            m_Pitch = glm::clamp(m_Pitch, -glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);
            break;
    }
}

void ViewportCameraController::HandleMouseScroll(float delta) {
    m_Distance -= delta * m_ZoomSpeed;
    m_Distance = glm::max(m_Distance, 0.1f);
    UpdatePosition();
}

void ViewportCameraController::HandleKeyboard(int key, bool pressed) {
    // Keyboard input handling for future use
}

void ViewportCameraController::ResetView() {
    m_Position = glm::vec3(0.0f, 0.0f, 5.0f);
    m_Target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_Yaw = 0.0f;
    m_Pitch = 0.0f;
    m_Distance = 5.0f;
    UpdatePosition();
}

void ViewportCameraController::FrameAll(const glm::vec3& sceneCenter, float sceneRadius) {
    m_Target = sceneCenter;
    m_Distance = sceneRadius / std::tan(glm::radians(m_FOV / 2.0f));
    UpdatePosition();
}

void ViewportCameraController::FrameSelection(const glm::vec3& selectionCenter, float selectionRadius) {
    m_Target = selectionCenter;
    m_Distance = selectionRadius / std::tan(glm::radians(m_FOV / 2.0f)) * 1.5f;
    UpdatePosition();
}

glm::mat4 ViewportCameraController::GetViewMatrix() const {
    return glm::lookAt(m_Position, m_Target, m_Up);
}

glm::mat4 ViewportCameraController::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(m_FOV), m_AspectRatio, 0.1f, 1000.0f);
}

void ViewportCameraController::SetViewportSize(uint32_t width, uint32_t height) {
    m_ViewportWidth = width;
    m_ViewportHeight = height;
    m_AspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

void ViewportCameraController::UpdatePosition() {
    float x = m_Distance * std::sin(m_Yaw) * std::cos(m_Pitch);
    float y = m_Distance * std::sin(m_Pitch);
    float z = m_Distance * std::cos(m_Yaw) * std::cos(m_Pitch);

    m_Position = m_Target + glm::vec3(x, y, z);
}

} // namespace SZM::Graphics
