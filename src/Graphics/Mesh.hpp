#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @struct Vertex
 * @brief Vertex data for mesh rendering
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

/**
 * @class Mesh
 * @brief GPU mesh with VAO/VBO management
 *
 * Features:
 *  - Vertex/index buffer management
 *  - VAO binding
 *  - Draw operations
 */
class Mesh {
public:
    /**
     * @brief Create mesh from vertices and indices
     * @param vertices Vertex data
     * @param indices Triangle indices
     */
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    
    /**
     * @brief Destroy mesh and free GPU resources
     */
    ~Mesh();
    
    // Prevent copying
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    
    /**
     * @brief Draw mesh
     */
    void Draw() const;
    
    /**
     * @brief Get vertex count
     * @return Number of vertices
     */
    uint32_t GetVertexCount() const { return m_VertexCount; }
    
    /**
     * @brief Get index count
     * @return Number of indices
     */
    uint32_t GetIndexCount() const { return m_IndexCount; }

private:
    uint32_t m_VAO = 0;
    uint32_t m_VBO = 0;
    uint32_t m_EBO = 0;
    uint32_t m_VertexCount = 0;
    uint32_t m_IndexCount = 0;
    
    /**
     * @brief Setup VAO/VBO/EBO
     */
    void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
};

} // namespace SZM::Graphics
