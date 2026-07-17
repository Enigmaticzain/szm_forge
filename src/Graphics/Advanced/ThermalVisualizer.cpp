#include "ThermalVisualizer.hpp"
#include "../Heatmap.hpp"
#include "../GlmInterop.hpp"
#include <algorithm>

namespace SZM::Graphics {

ThermalVisualizer& ThermalVisualizer::GetInstance() {
    static ThermalVisualizer instance;
    return instance;
}

void ThermalVisualizer::UpdateThermalData(const std::vector<ThermalVisualizationData>& thermalData) {
    m_ThermalData = thermalData;
    UpdateMinMax();
}

void ThermalVisualizer::UpdateMinMax() {
    if (m_ThermalData.empty()) {
        m_MaxTemperature = 0.0f;
        m_MinTemperature = 0.0f;
        return;
    }

    m_MaxTemperature = m_ThermalData[0].temperature;
    m_MinTemperature = m_ThermalData[0].temperature;

    for (const auto& data : m_ThermalData) {
        m_MaxTemperature = std::max(m_MaxTemperature, data.temperature);
        m_MinTemperature = std::min(m_MinTemperature, data.temperature);
    }
}

glm::vec3 ThermalVisualizer::GetThermalColor(uint32_t componentId) const {
    auto it = std::find_if(m_ThermalData.begin(), m_ThermalData.end(),
        [componentId](const ThermalVisualizationData& d) { return d.componentId == componentId; });

    if (it == m_ThermalData.end()) {
        return glm::vec3(0.5f);
    }

    float range = m_MaxTemperature - m_MinTemperature;
    float ratio = range > 0.0f ? (it->temperature - m_MinTemperature) / range : 0.0f;
    Heatmap heatmap(Heatmap::Mode::Temperature);
    return ToGlmVec3(heatmap.GetColor(ratio));
}

float ThermalVisualizer::GetTemperature(uint32_t componentId) const {
    auto it = std::find_if(m_ThermalData.begin(), m_ThermalData.end(),
        [componentId](const ThermalVisualizationData& d) { return d.componentId == componentId; });

    return it != m_ThermalData.end() ? it->temperature : 0.0f;
}

float ThermalVisualizer::GetHeatFlux(uint32_t componentId) const {
    auto it = std::find_if(m_ThermalData.begin(), m_ThermalData.end(),
        [componentId](const ThermalVisualizationData& d) { return d.componentId == componentId; });

    return it != m_ThermalData.end() ? it->heatFlux : 0.0f;
}

bool ThermalVisualizer::IsOverheated(uint32_t componentId) const {
    auto it = std::find_if(m_ThermalData.begin(), m_ThermalData.end(),
        [componentId](const ThermalVisualizationData& d) { return d.componentId == componentId; });

    return it != m_ThermalData.end() ? it->isOverheated : false;
}

size_t ThermalVisualizer::GetOverheatedComponentCount() const {
    return std::count_if(m_ThermalData.begin(), m_ThermalData.end(),
        [](const ThermalVisualizationData& d) { return d.isOverheated; });
}

} // namespace SZM::Graphics
