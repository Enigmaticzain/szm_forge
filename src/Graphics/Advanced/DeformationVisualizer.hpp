#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @struct DeformationData
 * @brief Deformation data for visualization
 */
struct DeformationData {
    uint32_t componentId;
    glm::vec3 originalPosition;
    glm::vec3 deformedPosition;
    float deformationMagnitude;
    float stressLevel;
};

/**
 * @class DeformationVisualizer
 * @brief Visualizes component deformations from simulation
 */
class DeformationVisualizer {
public:
    static DeformationVisualizer& GetInstance();

    /**
     * @brief Update deformation data
     */
    void UpdateDeformation(const std::vector<DeformationData>& deformations);

    /**
     * @brief Get deformation data
     */
    const std::vector<DeformationData>& GetDeformations() const { return m_Deformations; }

    /**
     * @brief Set deformation scale factor
     */
    void SetDeformationScale(float scale) { m_DeformationScale = scale; }

    /**
     * @brief Get deformation scale factor
     */
    float GetDeformationScale() const { return m_DeformationScale; }

    /**
     * @brief Enable/disable deformation visualization
     */
    void SetVisualizationEnabled(bool enabled) { m_Enabled = enabled; }

    /**
     * @brief Check if visualization is enabled
     */
    bool IsVisualizationEnabled() const { return m_Enabled; }

    /**
     * @brief Get deformed position for component
     */
    glm::vec3 GetDeformedPosition(uint32_t componentId) const;

    /**
     * @brief Get deformation magnitude for component
     */
    float GetDeformationMagnitude(uint32_t componentId) const;

    /**
     * @brief Clear all deformations
     */
    void Clear();

private:
    DeformationVisualizer() = default;
    ~DeformationVisualizer() = default;

    DeformationVisualizer(const DeformationVisualizer&) = delete;
    DeformationVisualizer& operator=(const DeformationVisualizer&) = delete;

    std::vector<DeformationData> m_Deformations;
    float m_DeformationScale = 1.0f;
    bool m_Enabled = true;
};

} // namespace SZM::Graphics
