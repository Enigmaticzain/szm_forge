#pragma once

#include <cmath>
#include <numbers>
#include <algorithm>

namespace SZM::Graphics {

    // ----------------------------------------------------------------
    //  Math helpers local to Camera (avoids GLM dependency)
    // ----------------------------------------------------------------
    struct CameraVec3 {
        float x, y, z;
        CameraVec3 operator+(const CameraVec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
        CameraVec3 operator-(const CameraVec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
        CameraVec3 operator*(float s)             const { return {x*s,   y*s,   z*s  }; }
        CameraVec3 operator/(float s)             const { return {x/s,   y/s,   z/s  }; }
        float dot  (const CameraVec3& o) const { return x*o.x + y*o.y + z*o.z; }
        CameraVec3 cross(const CameraVec3& o) const {
            return { y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x };
        }
        float length() const { return std::sqrt(x*x + y*y + z*z); }
        CameraVec3 normalized() const {
            float l = length();
            return l > 0.0f ? *this / l : *this;
        }
    };

    struct Matrix4x4 {
        float m[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    };

    struct Quaternion {
        float w, x, y, z;
        static Quaternion FromAxisAngle(const CameraVec3& axis, float angle) {
            float half = angle * 0.5f;
            float s    = std::sin(half);
            return { std::cos(half), axis.x*s, axis.y*s, axis.z*s };
        }
        Quaternion operator*(const Quaternion& q) const {
            return {
                w*q.w - x*q.x - y*q.y - z*q.z,
                w*q.x + x*q.w + y*q.z - z*q.y,
                w*q.y - x*q.z + y*q.w + z*q.x,
                w*q.z + x*q.y - y*q.x + z*q.w
            };
        }
        CameraVec3 Rotate(const CameraVec3& v) const {
            CameraVec3 qv{x, y, z};
            CameraVec3 t = qv.cross(v) * 2.0f;
            return v + (t * w) + qv.cross(t);
        }
    };

    enum class ProjectionMode { Perspective, Orthographic };

    // ----------------------------------------------------------------
    //  Camera — 3-D viewport camera with trackball orbit
    // ----------------------------------------------------------------
    class Camera {
    public:
        Camera(CameraVec3 pos = {10.f, 10.f, 10.f},
               CameraVec3 tgt = {0.f,  0.f,  0.f});

        // --- Configuration -------------------------------------------
        void SetProjectionMode(ProjectionMode mode);
        void SetAspectRatio(float width, float height);

        // --- Viewport Controls (driven by EventBus / InputManager) ---
        void Orbit(float deltaX, float deltaY, float sensitivity = 0.005f);
        void Pan  (float deltaX, float deltaY, float sensitivity = 0.01f);
        void Zoom (float deltaScroll,           float sensitivity = 0.5f);

        // --- Matrix Outputs (fed to GraphicsContext UBO) -------------
        [[nodiscard]] Matrix4x4 GetViewMatrix()       const;
        [[nodiscard]] Matrix4x4 GetProjectionMatrix() const;

        // --- Accessors -----------------------------------------------
        CameraVec3     GetPosition() const { return m_Position; }
        CameraVec3     GetTarget()   const { return m_Target;   }
        ProjectionMode GetMode()     const { return m_Mode;     }

    private:
        CameraVec3     m_Position;
        CameraVec3     m_Target;
        CameraVec3     m_Up    = {0.f, 1.f, 0.f};
        ProjectionMode m_Mode  = ProjectionMode::Perspective;

        float m_FovDegrees  = 45.0f;
        float m_AspectRatio = 16.0f / 9.0f;
        float m_NearClip    = 0.1f;
        float m_FarClip     = 10000.0f;
        float m_OrbitRadius = 0.0f;
        float m_OrthoScale  = 0.0f;

        CameraVec3 GetRightVector() const;
        void       EnforceMinimumRadius();
    };

} // namespace SZM::Graphics
