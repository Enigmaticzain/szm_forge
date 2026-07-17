#include "PickingSystem.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>

namespace SZM::Graphics {

PickingSystem& PickingSystem::GetInstance() {
    static PickingSystem instance;
    return instance;
}

void PickingSystem::Init() {
    // Picking system initialized
}

void PickingSystem::Shutdown() {
    // Cleanup if needed
}

glm::vec3 PickingSystem::GetRayDirection(
    float screenX,
    float screenY,
    uint32_t viewportWidth,
    uint32_t viewportHeight,
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix
) {
    // Normalize screen coordinates to NDC [-1, 1]
    float ndcX = (2.0f * screenX) / viewportWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / viewportHeight;

    // Create ray in clip space
    glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);

    // Transform to eye space
    glm::vec4 rayEye = glm::inverse(projMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Transform to world space
    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * rayEye));

    return rayWorld;
}

std::optional<PickResult> PickingSystem::Pick(
    float screenX,
    float screenY,
    uint32_t viewportWidth,
    uint32_t viewportHeight,
    const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix
) {
    glm::vec3 rayDir = GetRayDirection(screenX, screenY, viewportWidth, viewportHeight, viewMatrix, projMatrix);
    glm::vec3 rayOrigin = glm::vec3(glm::inverse(viewMatrix)[3]);

    // For now, return empty - will be populated by component renderer
    return std::nullopt;
}

std::optional<float> PickingSystem::RaySphereIntersection(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const glm::vec3& sphereCenter,
    float sphereRadius
) {
    glm::vec3 oc = rayOrigin - sphereCenter;
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
        return std::nullopt;
    }

    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    if (t < 0.0f) {
        t = (-b + std::sqrt(discriminant)) / (2.0f * a);
    }

    if (t < 0.0f) {
        return std::nullopt;
    }

    return t;
}

std::optional<float> PickingSystem::RayBoxIntersection(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const glm::vec3& boxMin,
    const glm::vec3& boxMax
) {
    float tMin = 0.0f;
    float tMax = 1e10f;

    for (int i = 0; i < 3; ++i) {
        float invD = 1.0f / rayDir[i];
        float t0 = (boxMin[i] - rayOrigin[i]) * invD;
        float t1 = (boxMax[i] - rayOrigin[i]) * invD;

        if (invD < 0.0f) {
            std::swap(t0, t1);
        }

        tMin = std::max(tMin, t0);
        tMax = std::min(tMax, t1);

        if (tMax < tMin) {
            return std::nullopt;
        }
    }

    if (tMin > 0.0f) {
        return tMin;
    }
    if (tMax > 0.0f) {
        return tMax;
    }

    return std::nullopt;
}

} // namespace SZM::Graphics
