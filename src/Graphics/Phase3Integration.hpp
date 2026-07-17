#pragma once

#include "GraphicsManager.hpp"
#include "Viewport.hpp"
#include "Heatmap.hpp"
#include "../Simulation/SimulationEngine.hpp"

namespace SZM::Graphics {

/**
 * @class Phase3Integration
 * @brief Coordinates the Phase 3 graphics stack.
 *
 * Integrates render targets, heatmap visualization, mesh rendering,
 * viewport management, and high-level graphics orchestration.
 */
class Phase3Integration {
public:
    static void Init();
    static void Shutdown();
    static void Update(const SimulationEngine& engine);

    [[nodiscard]] static GraphicsManager& GetGraphicsManager();
    static void SetVisualizationMode(Heatmap::Mode mode);
    [[nodiscard]] static Heatmap::Mode GetVisualizationMode();

private:
    static bool s_Initialized;
};

} // namespace SZM::Graphics
