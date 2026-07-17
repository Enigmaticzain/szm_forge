#include "DeformationVisualizer.hpp"
#include <algorithm>

namespace SZM::Graphics {

DeformationVisualizer& DeformationVisualizer::GetInstance() {
    static DeformationVisualizer instance;
    return instance;
}

void DeformationVisualizer::UpdateDeformation(const std::vector<DeformationData>& deformations) {
    m_Deformations = deformations;
}

glm::vec3 DeformationVisualizer::GetDeformedPosition(uint32_t componentId) const {
    auto it = std::find_if(m_Deformations.begin(), m_Deformations.end(),
        [componentId](const DeformationData& d) { return d.componentId == componentId; });

    if (it != m_Deformations.end()) {
        return it->originalPosition + (it->deformedPosition - it->originalPosition) * m_DeformationScale;
    }

    return glm::vec3(0.0f);
}

float DeformationVisualizer::GetDeformationMagnitude(uint32_t componentId) const {
    auto it = std::find_if(m_Deformations.begin(), m_Deformations.end(),
        [componentId](const DeformationData& d) { return d.componentId == componentId; });

    if (it != m_Deformations.end()) {
        return it->deformationMagnitude * m_DeformationScale;
    }

    return 0.0f;
}

void DeformationVisualizer::Clear() {
    m_Deformations.clear();
}

} // namespace SZM::Graphics
