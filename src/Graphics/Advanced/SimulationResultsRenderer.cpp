#include "SimulationResultsRenderer.hpp"
#include <GL/gl.h>
#include <iostream>

namespace SZM::Graphics {

SimulationResultsRenderer& SimulationResultsRenderer::GetInstance() {
    static SimulationResultsRenderer instance;
    return instance;
}

void SimulationResultsRenderer::Init() {
    std::cout << "[SimulationResultsRenderer] Initialized\n";
}

void SimulationResultsRenderer::Shutdown() {
    std::cout << "[SimulationResultsRenderer] Shutdown\n";
}

void SimulationResultsRenderer::RenderResults(
    const Camera& camera,
    RenderTarget& target
) {
    if (!target.IsBound()) {
        target.Bind();
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Render based on current mode
    switch (m_ResultMode) {
        case SimulationResultMode::Stress:
            // Render with stress heatmap
            break;
        case SimulationResultMode::Temperature:
            // Render with thermal heatmap
            break;
        case SimulationResultMode::Deformation:
            // Render with deformation
            break;
        case SimulationResultMode::Combined:
            // Render combined visualization
            break;
    }
}

void SimulationResultsRenderer::UpdateFromSimulation(
    const std::vector<StressVisualizationData>& stressData,
    const std::vector<ThermalVisualizationData>& thermalData,
    const std::vector<DeformationData>& deformationData
) {
    GetStressVisualizer().UpdateStressData(stressData);
    GetThermalVisualizer().UpdateThermalData(thermalData);
    GetDeformationVisualizer().UpdateDeformation(deformationData);
}

} // namespace SZM::Graphics
