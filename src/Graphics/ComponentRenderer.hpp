#pragma once

#include "RenderTarget.hpp"
#include "Heatmap.hpp"
#include "Camera.hpp"
#include "../Simulation/SimulationComponent.hpp"
#include "../Assembly/AssemblyGraph.hpp"

namespace SZM::Graphics {

/**
 * @class ComponentRenderer
 * @brief Renders simulation components and assemblies with heatmap visualization
 *
 * Features:
 *  - Component mesh rendering (cube, sphere, cylinder)
 *  - Assembly rendering with hierarchy
 *  - Heatmap color application
 *  - Shader management
 *  - Lighting and material support
 */
class ComponentRenderer {
public:
    /**
     * @brief Initialize renderer (setup shaders, meshes)
     */
    static void Init();

    /**
     * @brief Shutdown renderer (cleanup resources)
     */
    static void Shutdown();

    /**
     * @brief Render a single component
     * @param comp Component to render
     * @param camera Camera for view/projection matrices
     * @param heatmap Heatmap for color mapping
     * @param target Render target (framebuffer)
     */
    static void RenderComponent(
        const SimulationComponent& comp,
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    );

    /**
     * @brief Render an assembly with all components
     * @param manager Assembly graph manager
     * @param camera Camera for view/projection matrices
     * @param heatmap Heatmap for color mapping
     * @param target Render target (framebuffer)
     */
    static void RenderAssembly(
        const Assembly::AssemblyGraphManager& manager,
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    );

    /**
     * @brief Render a cube at specified position
     * @param pos Position in world space
     * @param scale Scale factors (width, height, depth)
     * @param color RGB color
     */
    static void RenderCube(
        const Geometry::Vector3& pos,
        const Geometry::Vector3& scale,
        const Geometry::Vector3& color
    );

    /**
     * @brief Render a sphere at specified position
     * @param pos Position in world space
     * @param radius Sphere radius
     * @param color RGB color
     */
    static void RenderSphere(
        const Geometry::Vector3& pos,
        float radius,
        const Geometry::Vector3& color
    );

private:
    /**
     * @brief Setup shader programs
     */
    static void SetupShaders();

    /**
     * @brief Get color for component based on heatmap
     * @param comp Component to get color for
     * @param heatmap Heatmap for color mapping
     * @return RGB color
     */
    static Geometry::Vector3 GetComponentColor(
        const SimulationComponent& comp,
        const Heatmap& heatmap
    );
};

} // namespace SZM::Graphics
