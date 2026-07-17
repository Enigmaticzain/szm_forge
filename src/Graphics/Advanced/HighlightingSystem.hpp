#pragma once

#include <glm/glm.hpp>
#include <cstdint>
#include <unordered_set>

namespace SZM::Graphics {

/**
 * @class HighlightingSystem
 * @brief Manages visual highlighting of selected and hovered components
 */
class HighlightingSystem {
public:
    static HighlightingSystem& GetInstance();

    /**
     * @brief Set hovered component
     */
    void SetHovered(uint32_t componentId);

    /**
     * @brief Clear hovered component
     */
    void ClearHovered();

    /**
     * @brief Get hovered component ID
     */
    uint32_t GetHovered() const { return m_HoveredId; }

    /**
     * @brief Check if component is hovered
     */
    bool IsHovered(uint32_t componentId) const { return m_HoveredId == componentId; }

    /**
     * @brief Get highlight color for component
     */
    glm::vec3 GetHighlightColor(uint32_t componentId, const glm::vec3& baseColor) const;

    /**
     * @brief Get selection highlight color
     */
    glm::vec3 GetSelectionColor() const { return m_SelectionColor; }

    /**
     * @brief Get hover highlight color
     */
    glm::vec3 GetHoverColor() const { return m_HoverColor; }

    /**
     * @brief Set selection highlight color
     */
    void SetSelectionColor(const glm::vec3& color) { m_SelectionColor = color; }

    /**
     * @brief Set hover highlight color
     */
    void SetHoverColor(const glm::vec3& color) { m_HoverColor = color; }

    /**
     * @brief Get highlight intensity for selected components
     */
    float GetSelectionIntensity() const { return m_SelectionIntensity; }

    /**
     * @brief Get highlight intensity for hovered components
     */
    float GetHoverIntensity() const { return m_HoverIntensity; }

    /**
     * @brief Set selection intensity
     */
    void SetSelectionIntensity(float intensity) { m_SelectionIntensity = intensity; }

    /**
     * @brief Set hover intensity
     */
    void SetHoverIntensity(float intensity) { m_HoverIntensity = intensity; }

private:
    HighlightingSystem() = default;
    ~HighlightingSystem() = default;

    HighlightingSystem(const HighlightingSystem&) = delete;
    HighlightingSystem& operator=(const HighlightingSystem&) = delete;

    uint32_t m_HoveredId = 0;
    glm::vec3 m_SelectionColor = glm::vec3(1.0f, 0.8f, 0.0f);  // Gold
    glm::vec3 m_HoverColor = glm::vec3(1.0f, 1.0f, 0.5f);      // Light yellow
    float m_SelectionIntensity = 1.5f;
    float m_HoverIntensity = 1.2f;
};

} // namespace SZM::Graphics
