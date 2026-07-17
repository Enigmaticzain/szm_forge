#include "ViewportGizmo.hpp"
#include <GL/gl.h>
#include <cmath>

namespace SZM::Graphics {

ViewportGizmo::ViewportGizmo() = default;

GizmoAxis ViewportGizmo::HandleMouseClick(float screenX, float screenY) {
    // Simple hit detection for gizmo axes
    // In a full implementation, this would use ray casting
    m_State.isDragging = true;
    m_DragStart = glm::vec3(screenX, screenY, 0.0f);
    return GizmoAxis::None;
}

void ViewportGizmo::HandleMouseDrag(float deltaX, float deltaY) {
    if (!m_State.isDragging) return;

    switch (m_State.mode) {
        case GizmoMode::Translate:
            m_State.position.x += deltaX * 0.01f;
            m_State.position.y -= deltaY * 0.01f;
            break;

        case GizmoMode::Rotate:
            m_State.rotation.x += deltaY * 0.01f;
            m_State.rotation.y += deltaX * 0.01f;
            break;

        case GizmoMode::Scale:
            m_State.scale += glm::vec3(deltaX * 0.001f);
            break;

        case GizmoMode::None:
            break;
    }
}

void ViewportGizmo::HandleMouseRelease() {
    m_State.isDragging = false;
}

void ViewportGizmo::Render() {
    switch (m_State.mode) {
        case GizmoMode::Translate:
            RenderTranslateGizmo();
            break;
        case GizmoMode::Rotate:
            RenderRotateGizmo();
            break;
        case GizmoMode::Scale:
            RenderScaleGizmo();
            break;
        case GizmoMode::None:
            break;
    }
}

void ViewportGizmo::RenderTranslateGizmo() {
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    // X axis (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(m_State.position.x, m_State.position.y, m_State.position.z);
    glVertex3f(m_State.position.x + m_GizmoSize, m_State.position.y, m_State.position.z);

    // Y axis (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(m_State.position.x, m_State.position.y, m_State.position.z);
    glVertex3f(m_State.position.x, m_State.position.y + m_GizmoSize, m_State.position.z);

    // Z axis (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(m_State.position.x, m_State.position.y, m_State.position.z);
    glVertex3f(m_State.position.x, m_State.position.y, m_State.position.z + m_GizmoSize);
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void ViewportGizmo::RenderRotateGizmo() {
    glDisable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    // Render rotation circles
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 32; ++i) {
        float angle = 2.0f * 3.14159f * i / 32.0f;
        float x = m_State.position.x + m_GizmoSize * std::cos(angle);
        float y = m_State.position.y;
        float z = m_State.position.z + m_GizmoSize * std::sin(angle);
        glVertex3f(x, y, z);
    }
    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void ViewportGizmo::RenderScaleGizmo() {
    glDisable(GL_DEPTH_TEST);
    glPointSize(8.0f);

    glBegin(GL_POINTS);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(m_State.position.x, m_State.position.y, m_State.position.z);
    glEnd();

    glPointSize(1.0f);
    glEnable(GL_DEPTH_TEST);
}

} // namespace SZM::Graphics
