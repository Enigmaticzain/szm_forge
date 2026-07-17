#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <optional>

namespace SZM::Graphics {

/**
 * @struct PickResult
 * @brief Result of a pick operation
 */
struct PickResult {
    uint32_t componentId;
    float distance;
    glm::vec3 hitPoint;
};

/**
 * @class PickingSystem
 * @brief Ray casting based picking for component selection
 */
class PickingSystem {
public:
    static PickingSystem& GetInstance();

    /**
     * @brief Initialize picking system
     */
    void Init();

    /**
     * @brief Shutdown picking system
     */
    void Shutdown();

    /**
     * @brief Pick component at screen coordinates
     * @param screenX Screen X coordinate (0-width)
     * @param screenY Screen Y coordinate (0-height)
     * @param viewportWidth Viewport width
     * @param viewportHeight Viewport height
     * @return Pick result if hit, empty optional otherwise
     */
    std::optional<PickResult> Pick(
        float screenX,
        float screenY,
        uint32_t viewportWidth,
        uint32_t viewportHeight,
        const glm::mat4& viewMatrix,
        const glm::mat4& projMatrix
    );

    /**
     * @brief Generate ray from screen coordinates
     */
    glm::vec3 GetRayDirection(
        float screenX,
        float screenY,
        uint32_t viewportWidth,
        uint32_t viewportHeight,
        const glm::mat4& viewMatrix,
        const glm::mat4& projMatrix
    );

    /**
     * @brief Test ray-sphere intersection
     */
    static std::optional<float> RaySphereIntersection(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDir,
        const glm::vec3& sphereCenter,
        float sphereRadius
    );

    /**
     * @brief Test ray-box intersection
     */
    static std::optional<float> RayBoxIntersection(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDir,
        const glm::vec3& boxMin,
        const glm::vec3& boxMax
    );

private:
    PickingSystem() = default;
    ~PickingSystem() = default;

    PickingSystem(const PickingSystem&) = delete;
    PickingSystem& operator=(const PickingSystem&) = delete;
};

} // namespace SZM::Graphics
