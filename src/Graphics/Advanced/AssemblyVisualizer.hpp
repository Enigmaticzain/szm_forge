#pragma once

#include "../../Assembly/AssemblyGraph.hpp"
#include "../Camera.hpp"
#include "../RenderTarget.hpp"
#include "../Heatmap.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace SZM::Graphics {

/**
 * @struct AssemblyRenderData
 * @brief Rendering data for assembly parts
 */
struct AssemblyRenderData {
    uint32_t partId;
    glm::mat4 transform;
    glm::vec3 color;
    bool isSelected = false;
    bool isHovered = false;
};

/**
 * @class AssemblyVisualizer
 * @brief Visualizes assembly hierarchy with part rendering
 */
class AssemblyVisualizer {
public:
    static AssemblyVisualizer& GetInstance();

    /**
     * @brief Initialize visualizer
     */
    void Init();

    /**
     * @brief Shutdown visualizer
     */
    void Shutdown();

    /**
     * @brief Update assembly visualization data
     */
    void UpdateAssembly(const Assembly::AssemblyGraphManager& manager);

    /**
     * @brief Render assembly
     */
    void RenderAssembly(
        const Camera& camera,
        const Heatmap& heatmap,
        RenderTarget& target
    );

    /**
     * @brief Get render data for all parts
     */
    const std::vector<AssemblyRenderData>& GetRenderData() const { return m_RenderData; }

    /**
     * @brief Highlight part by ID
     */
    void HighlightPart(uint32_t partId);

    /**
     * @brief Clear highlighting
     */
    void ClearHighlight();

    /**
     * @brief Toggle part visibility
     */
    void SetPartVisibility(uint32_t partId, bool visible);

    /**
     * @brief Get part visibility
     */
    bool IsPartVisible(uint32_t partId) const;

private:
    AssemblyVisualizer() = default;
    ~AssemblyVisualizer() = default;

    AssemblyVisualizer(const AssemblyVisualizer&) = delete;
    AssemblyVisualizer& operator=(const AssemblyVisualizer&) = delete;

    std::vector<AssemblyRenderData> m_RenderData;
    std::vector<bool> m_PartVisibility;
    uint32_t m_HighlightedPartId = 0;

    void BuildRenderData(const Assembly::AssemblyGraphManager& manager);
};

} // namespace SZM::Graphics
