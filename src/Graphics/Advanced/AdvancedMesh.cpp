#include "AdvancedMesh.hpp"
#include <glad/gl.h>

namespace SZM::Graphics {

AdvancedMesh::AdvancedMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    SetupMesh(vertices, indices);
}

AdvancedMesh::~AdvancedMesh() {
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void AdvancedMesh::SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    m_VertexCount = vertices.size();
    m_IndexCount = indices.size();

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void AdvancedMesh::Draw(RenderMode mode) const {
    if (!m_VAO || m_IndexCount == 0) return;

    glBindVertexArray(m_VAO);

    switch (mode) {
        case RenderMode::Wireframe:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;

        case RenderMode::FlatShaded:
            glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
            glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
            break;

        default: // Solid, WireframeShaded, NormalMap, DepthMap
            glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
            break;
    }

    glBindVertexArray(0);
}

void AdvancedMesh::DrawWireframeOverlay(float lineWidth) const {
    if (!m_VAO || m_IndexCount == 0) return;

    glBindVertexArray(m_VAO);
    glLineWidth(lineWidth);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
    glBindVertexArray(0);
}

} // namespace SZM::Graphics
