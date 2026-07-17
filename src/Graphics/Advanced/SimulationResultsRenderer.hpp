#pragma once

#include "SimulationVisualizationData.hpp"
#include "StressVisualizer.hpp"
#include "ThermalVisualizer.hpp"
#include "DeformationVisualizer.hpp"
#include "../Camera.hpp"
#include "../RenderTarget.hpp"
#include "../Heatmap.hpp"

namespace SZM::Graphics {

/**
 * @enum SimulationResultMode
 * @brief Visualization mode for simulation results
 */
enum class SimulationResultMode {
    Stress,
    Temperature,
    Deformation,
    Combined
};

/**
 * @class SimulationResultsRenderer
 * @brief Renders simulation results with real-time visualization
 */
class SimulationResultsRenderer {
public:
    static SimulationResultsRenderer& GetInstance();

    void Init();
    void Shutdown();

    void SetResultMode(SimulationResultMode mode) { m_ResultMode = mode; }
    SimulationResultMode GetResultMode() const { return m_ResultMode; }

    void RenderResults(
        const Camera& camera,
        RenderTarget& target
    );

    void UpdateFromSimulation(
        const std::vector<StressVisualizationData>& stressData,
        const std::vector<ThermalVisualizationData>& thermalData,
        const std::vector<DeformationData>& deformationData
    );

    StressVisualizer& GetStressVisualizer() { return StressVisualizer::GetInstance(); }
    ThermalVisualizer& GetThermalVisualizer() { return ThermalVisualizer::GetInstance(); }
    DeformationVisualizer& GetDeformationVisualizer() { return DeformationVisualizer::GetInstance(); }

private:
    SimulationResultsRenderer() = default;
    ~SimulationResultsRenderer() = default;

    SimulationResultsRenderer(const SimulationResultsRenderer&) = delete;
    SimulationResultsRenderer& operator=(const SimulationResultsRenderer&) = delete;

    SimulationResultMode m_ResultMode = SimulationResultMode::Stress;
};

} // namespace SZM::Graphics
