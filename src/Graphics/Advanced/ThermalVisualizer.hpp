#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @struct ThermalVisualizationData
 * @brief Thermal data for visualization
 */
struct ThermalVisualizationData {
    uint32_t componentId;
    float temperature;
    float heatFlux;
    float thermalStress;
    bool isOverheated;
};

/**
 * @class ThermalVisualizer
 * @brief Real-time thermal visualization
 */
class ThermalVisualizer {
public:
    static ThermalVisualizer& GetInstance();

    void UpdateThermalData(const std::vector<ThermalVisualizationData>& thermalData);
    glm::vec3 GetThermalColor(uint32_t componentId) const;
    float GetTemperature(uint32_t componentId) const;
    float GetHeatFlux(uint32_t componentId) const;
    bool IsOverheated(uint32_t componentId) const;
    float GetMaxTemperature() const { return m_MaxTemperature; }
    float GetMinTemperature() const { return m_MinTemperature; }
    void SetOverheatThreshold(float threshold) { m_OverheatThreshold = threshold; }
    float GetOverheatThreshold() const { return m_OverheatThreshold; }
    const std::vector<ThermalVisualizationData>& GetThermalData() const { return m_ThermalData; }
    size_t GetOverheatedComponentCount() const;

private:
    ThermalVisualizer() = default;
    ~ThermalVisualizer() = default;
    ThermalVisualizer(const ThermalVisualizer&) = delete;
    ThermalVisualizer& operator=(const ThermalVisualizer&) = delete;

    std::vector<ThermalVisualizationData> m_ThermalData;
    float m_MaxTemperature = 0.0f;
    float m_MinTemperature = 0.0f;
    float m_OverheatThreshold = 373.15f;  // 100°C

    void UpdateMinMax();
};

} // namespace SZM::Graphics
