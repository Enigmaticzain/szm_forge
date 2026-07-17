#pragma once

#include "../Heatmap.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @struct StressVisualizationData
 * @brief Stress data for visualization
 */
struct StressVisualizationData {
    uint32_t componentId;
    float vonMisesStress;
    float maxPrincipalStress;
    float minPrincipalStress;
    float safetyFactor;
    bool isFailed;
};

/**
 * @class StressVisualizer
 * @brief Real-time stress visualization
 */
class StressVisualizer {
public:
    static StressVisualizer& GetInstance();

    void UpdateStressData(const std::vector<StressVisualizationData>& stressData);
    glm::vec3 GetStressColor(uint32_t componentId) const;
    float GetStressValue(uint32_t componentId) const;
    float GetSafetyFactor(uint32_t componentId) const;
    bool IsFailed(uint32_t componentId) const;
    float GetMaxStress() const { return m_MaxStress; }
    float GetMinStress() const { return m_MinStress; }
    void SetFailureThreshold(float threshold) { m_FailureThreshold = threshold; }
    float GetFailureThreshold() const { return m_FailureThreshold; }
    const std::vector<StressVisualizationData>& GetStressData() const { return m_StressData; }
    size_t GetFailedComponentCount() const;

private:
    StressVisualizer() = default;
    ~StressVisualizer() = default;
    StressVisualizer(const StressVisualizer&) = delete;
    StressVisualizer& operator=(const StressVisualizer&) = delete;

    std::vector<StressVisualizationData> m_StressData;
    float m_MaxStress = 0.0f;
    float m_MinStress = 0.0f;
    float m_FailureThreshold = 1.0f;

    void UpdateMinMax();
};

} // namespace SZM::Graphics
