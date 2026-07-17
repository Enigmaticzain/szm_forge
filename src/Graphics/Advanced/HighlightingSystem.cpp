#include "HighlightingSystem.hpp"

namespace SZM::Graphics {

HighlightingSystem& HighlightingSystem::GetInstance() {
    static HighlightingSystem instance;
    return instance;
}

void HighlightingSystem::SetHovered(uint32_t componentId) {
    m_HoveredId = componentId;
}

void HighlightingSystem::ClearHovered() {
    m_HoveredId = 0;
}

glm::vec3 HighlightingSystem::GetHighlightColor(uint32_t componentId, const glm::vec3& baseColor) const {
    if (IsHovered(componentId)) {
        return glm::mix(baseColor, m_HoverColor, m_HoverIntensity * 0.3f);
    }
    return baseColor;
}

} // namespace SZM::Graphics
