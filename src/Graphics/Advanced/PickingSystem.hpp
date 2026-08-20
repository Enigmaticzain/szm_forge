#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <optional>
#include "SelectionManager.hpp"

namespace SZM::Graphics {

struct PickResult {
    uint32_t componentId  = UINT32_MAX;
    float    distance     = 0.0f;
    glm::vec3 hitPoint    = {};
    // BRep sub-element
    BRepSelectionMode subMode  = BRepSelectionMode::None;
    uint32_t          subIndex = UINT32_MAX; ///< face/edge/vertex index
};

class PickingSystem {
public:
    static PickingSystem& GetInstance();

    void Init();
    void Shutdown();

    std::optional<PickResult> Pick(
        float screenX, float screenY,
        uint32_t viewportWidth, uint32_t viewportHeight,
        const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    glm::vec3 GetRayDirection(
        float screenX, float screenY,
        uint32_t viewportWidth, uint32_t viewportHeight,
        const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    static std::optional<float> RaySphereIntersection(
        const glm::vec3& rayOrigin, const glm::vec3& rayDir,
        const glm::vec3& sphereCenter, float sphereRadius);

    static std::optional<float> RayBoxIntersection(
        const glm::vec3& rayOrigin, const glm::vec3& rayDir,
        const glm::vec3& boxMin, const glm::vec3& boxMax);

private:
    PickingSystem() = default;
    ~PickingSystem() = default;
    PickingSystem(const PickingSystem&) = delete;
    PickingSystem& operator=(const PickingSystem&) = delete;
};

} // namespace SZM::Graphics
