#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @struct CollisionVisualization
 * @brief Data for visualizing collisions
 */
struct CollisionVisualization {
    uint32_t part1Id;
    uint32_t part2Id;
    glm::vec3 contactPoint;
    glm::vec3 contactNormal;
    float penetrationDepth;
    bool isActive = true;
};

/**
 * @class CollisionVisualizer
 * @brief Visualizes collisions and interferences
 */
class CollisionVisualizer {
public:
    static CollisionVisualizer& GetInstance();

    /**
     * @brief Add collision visualization
     */
    void AddCollision(const CollisionVisualization& collision);

    /**
     * @brief Clear all collisions
     */
    void ClearCollisions();

    /**
     * @brief Get all active collisions
     */
    const std::vector<CollisionVisualization>& GetCollisions() const { return m_Collisions; }

    /**
     * @brief Render collision visualizations
     */
    void RenderCollisions();

    /**
     * @brief Toggle collision visualization
     */
    void SetVisualizationEnabled(bool enabled) { m_Enabled = enabled; }

    /**
     * @brief Check if visualization is enabled
     */
    bool IsVisualizationEnabled() const { return m_Enabled; }

    /**
     * @brief Set collision color
     */
    void SetCollisionColor(const glm::vec3& color) { m_CollisionColor = color; }

    /**
     * @brief Get collision count
     */
    size_t GetCollisionCount() const { return m_Collisions.size(); }

private:
    CollisionVisualizer() = default;
    ~CollisionVisualizer() = default;

    CollisionVisualizer(const CollisionVisualizer&) = delete;
    CollisionVisualizer& operator=(const CollisionVisualizer&) = delete;

    std::vector<CollisionVisualization> m_Collisions;
    bool m_Enabled = true;
    glm::vec3 m_CollisionColor = glm::vec3(1.0f, 0.0f, 0.0f);  // Red
};

} // namespace SZM::Graphics
