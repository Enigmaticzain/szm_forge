#include "InteractiveRenderer.hpp"
#include "../GlmInterop.hpp"
#include <iostream>
#include <glm/gtc/matrix_inverse.hpp>

namespace SZM::Graphics {

InteractiveRenderer& InteractiveRenderer::GetInstance() {
    static InteractiveRenderer instance;
    return instance;
}

void InteractiveRenderer::Init() {
    GetPickingSystem().Init();
    std::cout << "[InteractiveRenderer] Initialized\n";
}

void InteractiveRenderer::Shutdown() {
    GetPickingSystem().Shutdown();
}

void InteractiveRenderer::HandleMouseClick(
    float screenX,
    float screenY,
    uint32_t viewportWidth,
    uint32_t viewportHeight,
    const Camera& camera,
    bool multiSelect
) {
    auto& selectionManager = GetSelectionManager();
    auto& pickingSystem = GetPickingSystem();
    selectionManager.SetMultiSelectMode(multiSelect);

    const glm::mat4 viewMatrix = ToGlmMat4(camera.GetViewMatrix());
    const glm::mat4 projectionMatrix = ToGlmMat4(camera.GetProjectionMatrix());

    glm::vec3 rayDir = pickingSystem.GetRayDirection(
        screenX, screenY, viewportWidth, viewportHeight,
        viewMatrix, projectionMatrix
    );

    glm::vec3 rayOrigin = glm::vec3(glm::inverse(viewMatrix)[3]);

    // Test intersection with all renderable components
    float closestDistance = 1e10f;
    uint32_t closestComponentId = 0;
    bool hitFound = false;

    for (const auto& comp : m_RenderableComponents) {
        // Simple sphere intersection test
        glm::vec3 compPos(comp.position.x, comp.position.y, comp.position.z);
        float radius = std::sqrt(comp.area) * 0.5f;

        auto distance = PickingSystem::RaySphereIntersection(rayOrigin, rayDir, compPos, radius);
        if (distance && distance.value() < closestDistance) {
            closestDistance = distance.value();
            closestComponentId = comp.id;
            hitFound = true;
        }
    }

    if (hitFound) {
        selectionManager.Select(closestComponentId);
        std::cout << "[InteractiveRenderer] Selected component " << closestComponentId << "\n";
    } else {
        if (!multiSelect) {
            selectionManager.ClearSelection();
        }
    }
}

void InteractiveRenderer::HandleMouseMove(
    float screenX,
    float screenY,
    uint32_t viewportWidth,
    uint32_t viewportHeight,
    const Camera& camera
) {
    auto& pickingSystem = GetPickingSystem();
    auto& highlightingSystem = GetHighlightingSystem();
    const glm::mat4 viewMatrix = ToGlmMat4(camera.GetViewMatrix());
    const glm::mat4 projectionMatrix = ToGlmMat4(camera.GetProjectionMatrix());

    glm::vec3 rayDir = pickingSystem.GetRayDirection(
        screenX, screenY, viewportWidth, viewportHeight,
        viewMatrix, projectionMatrix
    );

    glm::vec3 rayOrigin = glm::vec3(glm::inverse(viewMatrix)[3]);

    float closestDistance = 1e10f;
    uint32_t closestComponentId = 0;
    bool hitFound = false;

    for (const auto& comp : m_RenderableComponents) {
        glm::vec3 compPos(comp.position.x, comp.position.y, comp.position.z);
        float radius = std::sqrt(comp.area) * 0.5f;

        auto distance = PickingSystem::RaySphereIntersection(rayOrigin, rayDir, compPos, radius);
        if (distance && distance.value() < closestDistance) {
            closestDistance = distance.value();
            closestComponentId = comp.id;
            hitFound = true;
        }
    }

    if (hitFound) {
        highlightingSystem.SetHovered(closestComponentId);
    } else {
        highlightingSystem.ClearHovered();
    }
}

void InteractiveRenderer::RenderComponent(
    const SimulationComponent& comp,
    const glm::mat4& model,
    const glm::vec3& baseColor,
    const Camera& camera,
    RenderTarget& target
) {
    auto& selectionManager = GetSelectionManager();
    auto& highlightingSystem = GetHighlightingSystem();
    glm::vec3 finalColor = baseColor;

    // Apply highlighting
    if (selectionManager.IsSelected(comp.id)) {
        finalColor = glm::mix(baseColor, highlightingSystem.GetSelectionColor(), 0.4f);
    } else if (highlightingSystem.IsHovered(comp.id)) {
        finalColor = glm::mix(baseColor, highlightingSystem.GetHoverColor(), 0.3f);
    }

    // Render with final color
    // This would be called by the main renderer
}

} // namespace SZM::Graphics
