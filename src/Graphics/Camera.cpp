#include "Camera.hpp"

namespace SZM::Graphics {

    Camera::Camera(CameraVec3 pos, CameraVec3 tgt) 
        : m_Position(pos), m_Target(tgt) {
        CameraVec3 diff = m_Position - m_Target;
        m_OrbitRadius = diff.length();
        m_OrthoScale = m_OrbitRadius;
    }

    CameraVec3 Camera::GetRightVector() const {
        CameraVec3 forward = (m_Target - m_Position).normalized();
        return forward.cross(m_Up).normalized();
    }

    void Camera::EnforceMinimumRadius() {
        if (m_OrbitRadius < 0.1f) m_OrbitRadius = 0.1f;
    }

    void Camera::Orbit(float deltaX, float deltaY, float sensitivity) {
        CameraVec3 diff = m_Position - m_Target;
        
        // Yaw around up vector
        CameraVec3 right = GetRightVector();
        CameraVec3 forward = (m_Target - m_Position).normalized();
        
        // Pitch around right vector
        CameraVec3 pitchAxis = right;
        float pitchAngle = deltaY * sensitivity;
        
        // Yaw around up vector
        CameraVec3 yawAxis = m_Up;
        float yawAngle = deltaX * sensitivity;
        
        // Apply rotations
        Quaternion pitch = Quaternion::FromAxisAngle(pitchAxis, pitchAngle);
        Quaternion yaw = Quaternion::FromAxisAngle(yawAxis, yawAngle);
        Quaternion combined = yaw * pitch;
        
        diff = combined.Rotate(diff);
        m_Position = m_Target + diff;
        m_OrbitRadius = diff.length();
    }

    void Camera::Pan(float deltaX, float deltaY, float sensitivity) {
        CameraVec3 right = GetRightVector();
        CameraVec3 forward = (m_Target - m_Position).normalized();
        CameraVec3 up = forward.cross(right).normalized();
        
        CameraVec3 panVec = (right * deltaX + up * deltaY) * sensitivity;
        m_Position = m_Position + panVec;
        m_Target = m_Target + panVec;
    }

    void Camera::Zoom(float deltaScroll, float sensitivity) {
        CameraVec3 direction = (m_Target - m_Position).normalized();
        m_Position = m_Position + direction * deltaScroll * sensitivity;
        m_OrbitRadius = (m_Position - m_Target).length();
        EnforceMinimumRadius();
    }

    void Camera::SetProjectionMode(ProjectionMode mode) {
        m_Mode = mode;
    }

    void Camera::SetAspectRatio(float width, float height) {
        if (height > 0.0f) {
            m_AspectRatio = width / height;
        }
    }

    Matrix4x4 Camera::GetViewMatrix() const {
        CameraVec3 forward = (m_Target - m_Position).normalized();
        CameraVec3 right = forward.cross(m_Up).normalized();
        CameraVec3 up = right.cross(forward).normalized();
        
        Matrix4x4 view;
        view.m[0]  = right.x;   view.m[4]  = right.y;   view.m[8]   = right.z;   view.m[12] = -right.dot(m_Position);
        view.m[1]  = up.x;      view.m[5]  = up.y;      view.m[9]   = up.z;      view.m[13] = -up.dot(m_Position);
        view.m[2]  = -forward.x; view.m[6]  = -forward.y; view.m[10]  = -forward.z; view.m[14] = forward.dot(m_Position);
        view.m[3]  = 0.0f;      view.m[7]  = 0.0f;      view.m[11]  = 0.0f;      view.m[15] = 1.0f;
        
        return view;
    }

    Matrix4x4 Camera::GetProjectionMatrix() const {
        Matrix4x4 proj;
        
        if (m_Mode == ProjectionMode::Perspective) {
            float f = 1.0f / std::tan(m_FovDegrees * 3.14159265f / 360.0f);
            float n = m_NearClip;
            float far = m_FarClip;
            
            proj.m[0]  = f / m_AspectRatio;
            proj.m[5]  = f;
            proj.m[10] = (far + n) / (n - far);
            proj.m[11] = -1.0f;
            proj.m[14] = (2.0f * far * n) / (n - far);
            proj.m[15] = 0.0f;
        } else {
            // Orthographic
            float scale = m_OrthoScale;
            float right = scale * m_AspectRatio * 0.5f;
            float top = scale * 0.5f;
            
            proj.m[0]  = 1.0f / right;
            proj.m[5]  = 1.0f / top;
            proj.m[10] = -2.0f / (m_FarClip - m_NearClip);
            proj.m[14] = -(m_FarClip + m_NearClip) / (m_FarClip - m_NearClip);
            proj.m[15] = 1.0f;
        }
        
        return proj;
    }

} // namespace SZM::Graphics
