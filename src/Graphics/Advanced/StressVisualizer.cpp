#include "StressVisualizer.hpp"
#include "../GlmInterop.hpp"
#include <algorithm>

namespace SZM::Graphics {

StressVisualizer& StressVisualizer::GetInstance() {
    static StressVisualizer instance;
    return instance;
}

void StressVisualizer::UpdateStressData(const std::vector<StressVisualizationData>& stressData) {
    m_StressData = stressData;
    UpdateMinMax();
}

void StressVisualizer::UpdateMinMax() {
    if (m_StressData.empty()) {
        m_MaxStress = 0.0f;
        m_MinStress = 0.0f;
        return;
    }

    m_MaxStress = m_StressData[0].vonMisesStress;
    m_MinStress = m_StressData[0].vonMisesStress;

    for (const auto& data : m_StressData) {
        m_MaxStress = std::max(m_MaxStress, data.vonMisesStress);
        m_MinStress = std::min(m_MinStress, data.vonMisesStress);
    }
}

glm::vec3 StressVisualizer::GetStressColor(uint32_t componentId) const {
    auto it = std::find_if(m_StressData.begin(), m_StressData.end(),
        [componentId](const StressVisualizationData& d) { return d.componentId == componentId; });

    if (it == m_StressData.end()) {
        return glm::vec3(0.5f);
    }

    float ratio = m_MaxStress > 0.0f ? it->vonMisesStress / m_MaxStress : 0.0f;
    Heatmap heatmap(Heatmap::Mode::Stress);
    return ToGlmVec3(heatmap.GetColor(ratio));
}

float StressVisualizer::GetStressValue(uint32_t componentId) const {
    auto it = std::find_if(m_StressData.begin(), m_StressData.end(),
        [componentId](const StressVisualizationData& d) { return d.componentId == componentId; });

    return it != m_StressData.end() ? it->vonMisesStress : 0.0f;
}

float StressVisualizer::GetSafetyFactor(uint32_t componentId) const {
    auto it = std::find_if(m_StressData.begin(), m_StressData.end(),
        [componentId](const StressVisualizationData& d) { return d.componentId == componentId; });

    return it != m_StressData.end() ? it->safetyFactor : 0.0f;
}

bool StressVisualizer::IsFailed(uint32_t componentId) const {
    auto it = std::find_if(m_StressData.begin(), m_StressData.end(),
        [componentId](const StressVisualizationData& d) { return d.componentId == componentId; });

    return it != m_StressData.end() ? it->isFailed : false;
}

size_t StressVisualizer::GetFailedComponentCount() const {
    return std::count_if(m_StressData.begin(), m_StressData.end(),
        [](const StressVisualizationData& d) { return d.isFailed; });
}

} // namespace SZM::Graphics
