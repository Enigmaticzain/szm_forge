#pragma once

#include "../Mesh.hpp"
#include "RenderMode.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @class AdvancedMesh
 * @brief Enhanced mesh with wireframe, normal visualization, and multiple render modes
 */
class AdvancedMesh {
public:
    /**
     * @brief Create mesh from vertices and indices
     */
    AdvancedMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~AdvancedMesh();

    AdvancedMesh(const AdvancedMesh&) = delete;
    AdvancedMesh& operator=(const AdvancedMesh&) = delete;

    /**
     * @brief Draw mesh in specified mode
     */
    void Draw(RenderMode mode) const;

    /**
     * @brief Draw with wireframe overlay
     */
    void DrawWireframeOverlay(float lineWidth = 1.0f) const;

    /**
     * @brief Get vertex count
     */
    uint32_t GetVertexCount() const { return m_VertexCount; }

    /**
     * @brief Get index count
     */
    uint32_t GetIndexCount() const { return m_IndexCount; }

private:
    uint32_t m_VAO = 0;
    uint32_t m_VBO = 0;
    uint32_t m_EBO = 0;
    uint32_t m_VertexCount = 0;
    uint32_t m_IndexCount = 0;

    void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
};

} // namespace SZM::Graphics
