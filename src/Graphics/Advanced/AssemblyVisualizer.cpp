#include "AssemblyVisualizer.hpp"
#include <GL/gl.h>
#include <iostream>

namespace SZM::Graphics {

AssemblyVisualizer& AssemblyVisualizer::GetInstance() {
    static AssemblyVisualizer instance;
    return instance;
}

void AssemblyVisualizer::Init() {
    std::cout << "[AssemblyVisualizer] Initialized\n";
}

void AssemblyVisualizer::Shutdown() {
    m_RenderData.clear();
    m_PartVisibility.clear();
}

void AssemblyVisualizer::UpdateAssembly(const Assembly::AssemblyGraphManager& manager) {
    BuildRenderData(manager);
}

void AssemblyVisualizer::RenderAssembly(
    const Camera& camera,
    const Heatmap& heatmap,
    RenderTarget& target
) {
    if (!target.IsBound()) {
        target.Bind();
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    for (const auto& renderData : m_RenderData) {
        if (!IsPartVisible(renderData.partId)) {
            continue;
        }

        // Render part with transform and color
        // This would be integrated with the main renderer
    }
}

void AssemblyVisualizer::HighlightPart(uint32_t partId) {
    m_HighlightedPartId = partId;
}

void AssemblyVisualizer::ClearHighlight() {
    m_HighlightedPartId = 0;
}

void AssemblyVisualizer::SetPartVisibility(uint32_t partId, bool visible) {
    if (partId < m_PartVisibility.size()) {
        m_PartVisibility[partId] = visible;
    }
}

bool AssemblyVisualizer::IsPartVisible(uint32_t partId) const {
    if (partId >= m_PartVisibility.size()) {
        return true;
    }
    return m_PartVisibility[partId];
}

void AssemblyVisualizer::BuildRenderData(const Assembly::AssemblyGraphManager& manager) {
    m_RenderData.clear();

    // Build render data from assembly graph
    // This would traverse the assembly hierarchy and create render data for each part
    std::cout << "[AssemblyVisualizer] Built render data for assembly\n";
}

} // namespace SZM::Graphics
