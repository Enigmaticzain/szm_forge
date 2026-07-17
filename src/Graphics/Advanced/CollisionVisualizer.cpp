#include "CollisionVisualizer.hpp"
#include <GL/gl.h>
#include <iostream>

namespace SZM::Graphics {

CollisionVisualizer& CollisionVisualizer::GetInstance() {
    static CollisionVisualizer instance;
    return instance;
}

void CollisionVisualizer::AddCollision(const CollisionVisualization& collision) {
    m_Collisions.push_back(collision);
}

void CollisionVisualizer::ClearCollisions() {
    m_Collisions.clear();
}

void CollisionVisualizer::RenderCollisions() {
    if (!m_Enabled || m_Collisions.empty()) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    glPointSize(8.0f);

    for (const auto& collision : m_Collisions) {
        if (!collision.isActive) continue;

        // Render contact point as sphere
        glBegin(GL_POINTS);
        glColor3f(m_CollisionColor.x, m_CollisionColor.y, m_CollisionColor.z);
        glVertex3f(collision.contactPoint.x, collision.contactPoint.y, collision.contactPoint.z);
        glEnd();

        // Render contact normal as line
        glm::vec3 normalEnd = collision.contactPoint + collision.contactNormal * 0.1f;
        glBegin(GL_LINES);
        glColor3f(m_CollisionColor.x, m_CollisionColor.y, m_CollisionColor.z);
        glVertex3f(collision.contactPoint.x, collision.contactPoint.y, collision.contactPoint.z);
        glVertex3f(normalEnd.x, normalEnd.y, normalEnd.z);
        glEnd();
    }

    glPointSize(1.0f);
    glEnable(GL_DEPTH_TEST);
}

} // namespace SZM::Graphics
