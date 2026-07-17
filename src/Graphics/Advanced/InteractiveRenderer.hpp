#pragma once

#include "SelectionManager.hpp"
#include "PickingSystem.hpp"
#include "HighlightingSystem.hpp"
#include "../Camera.hpp"
#include "../RenderTarget.hpp"
#include "../../Simulation/SimulationComponent.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace SZM::Graphics {

/**
 * @class InteractiveRenderer
 * @brief Renderer with selection, picking, and highlighting support
 */
class InteractiveRenderer {
public:
    static InteractiveRenderer& GetInstance();

    /**
     * @brief Initialize interactive renderer
     */
    void Init();

    /**
     * @brief Shutdown interactive renderer
     */
    void Shutdown();

    /**
     * @brief Handle mouse click for picking
     */
    void HandleMouseClick(
        float screenX,
        float screenY,
        uint32_t viewportWidth,
        uint32_t viewportHeight,
        const Camera& camera,
        bool multiSelect = false
    );

    /**
     * @brief Handle mouse move for hover detection
     */
    void HandleMouseMove(
        float screenX,
        float screenY,
        uint32_t viewportWidth,
        uint32_t viewportHeight,
        const Camera& camera
    );

    /**
     * @brief Render component with selection/hover highlighting
     */
    void RenderComponent(
        const SimulationComponent& comp,
        const glm::mat4& model,
        const glm::vec3& baseColor,
        const Camera& camera,
        RenderTarget& target
    );

    /**
     * @brief Get selection manager
     */
    SelectionManager& GetSelectionManager() { return SelectionManager::GetInstance(); }

    /**
     * @brief Get highlighting system
     */
    HighlightingSystem& GetHighlightingSystem() { return HighlightingSystem::GetInstance(); }

    /**
     * @brief Get picking system
     */
    PickingSystem& GetPickingSystem() { return PickingSystem::GetInstance(); }

private:
    InteractiveRenderer() = default;
    ~InteractiveRenderer() = default;

    InteractiveRenderer(const InteractiveRenderer&) = delete;
    InteractiveRenderer& operator=(const InteractiveRenderer&) = delete;

    std::vector<SimulationComponent> m_RenderableComponents;
};

} // namespace SZM::Graphics
