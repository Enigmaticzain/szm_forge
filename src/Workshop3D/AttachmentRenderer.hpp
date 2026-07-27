#pragma once

#include "Assembly/FastenerDataTypes.hpp"
#include "Graphics/RendererTypes.hpp"
#include <vector>

namespace SZM::Workshop3D {

/**
 * AttachmentRenderer - Renders mechanical attachments in the viewport
 * 
 * Draws:
 * - Bolts with heads, shafts, and threads
 * - Hinges with barrels and leaves
 * - Welds with bead visualization
 */
class AttachmentRenderer {
public:
    AttachmentRenderer();
    ~AttachmentRenderer() = default;
    
    // =========================================================================
    // BOLT RENDERING
    // =========================================================================
    
    /**
     * Render a single bolt at its position
     */
    void RenderBolt(const Assembly::BoltInstance& bolt);
    
    /**
     * Render bolt preview (during placement)
     */
    void RenderBoltPreview(
        const Geometry::Vector3& position,
        const Geometry::Vector3& normal,
        const Assembly::BoltSpec& spec
    );
    
    /**
     * Render all bolts
     */
    void RenderAllBolts(const std::vector<Assembly::BoltInstance>& bolts);
    
    // =========================================================================
    // HINGE RENDERING
    // =========================================================================
    
    /**
     * Render a single hinge
     */
    void RenderHinge(const Assembly::HingeInstance& hinge);
    
    /**
     * Render hinge preview
     */
    void RenderHingePreview(
        const Geometry::Vector3& pivot,
        const Geometry::Vector3& axis,
        float length,
        float width
    );
    
    /**
     * Render all hinges
     */
    void RenderAllHinges(const std::vector<Assembly::HingeInstance>& hinges);
    
    // =========================================================================
    // WELD RENDERING
    // =========================================================================
    
    /**
     * Render a single weld
     */
    void RenderWeld(const Assembly::WeldInstance& weld);
    
    /**
     * Render weld preview
     */
    void RenderWeldPreview(
        const Geometry::Vector3& start,
        const Geometry::Vector3& end,
        const Assembly::WeldSpec& spec
    );
    
    /**
     * Render all welds
     */
    void RenderAllWelds(const std::vector<Assembly::WeldInstance>& welds);
    
    // =========================================================================
    // SETTINGS
    // =========================================================================
    
    void SetSelectedId(const Assembly::UUID& id) { m_SelectedId = id; }
    void SetHoveredId(const Assembly::UUID& id) { m_HoveredId = id; }
    
    void SetRenderScale(float scale) { m_RenderScale = scale; }
    float GetRenderScale() const { return m_RenderScale; }
    
    void SetShowLabels(bool show) { m_ShowLabels = show; }
    void SetShowPreview(bool show) { m_ShowPreview = show; }
    
    // =========================================================================
    // COLORS
    // =========================================================================
    
    struct Colors {
        Graphics::Color4 boltHead = Graphics::Color4(0.4f, 0.4f, 0.45f, 1.0f);
        Graphics::Color4 boltShaft = Graphics::Color4(0.35f, 0.35f, 0.4f, 1.0f);
        Graphics::Color4 selected = Graphics::Color4(1.0f, 0.8f, 0.2f, 1.0f);
        Graphics::Color4 hovered = Graphics::Color4(0.8f, 0.8f, 0.4f, 1.0f);
        Graphics::Color4 preview = Graphics::Color4(0.5f, 0.7f, 1.0f, 0.7f);
        Graphics::Color4 hingeBarrel = Graphics::Color4(0.45f, 0.45f, 0.5f, 1.0f);
        Graphics::Color4 hingeLeaf = Graphics::Color4(0.5f, 0.5f, 0.55f, 1.0f);
        Graphics::Color4 weldBead = Graphics::Color4(1.0f, 0.6f, 0.2f, 0.9f);
        Graphics::Color4 weldSeam = Graphics::Color4(0.9f, 0.5f, 0.1f, 1.0f);
    };
    
    Colors& GetColors() { return m_Colors; }
    const Colors& GetColors() const { return m_Colors; }
    
private:
    Assembly::UUID m_SelectedId;
    Assembly::UUID m_HoveredId;
    float m_RenderScale = 1.0f;
    bool m_ShowLabels = true;
    bool m_ShowPreview = true;
    Colors m_Colors;
    
    // Helper: Determine color based on selection state
    Graphics::Color4 GetColorForBolt(const Assembly::BoltInstance& bolt);
    Graphics::Color4 GetColorForHinge(const Assembly::HingeInstance& hinge);
    Graphics::Color4 GetColorForWeld(const Assembly::WeldInstance& weld);
    
    // Helper: Get effective color
    Graphics::Color4 GetEffectiveColor(
        const Graphics::Color4& base,
        const Assembly::UUID& itemId,
        const Assembly::UUID& selectedId,
        const Assembly::UUID& hoveredId
    );
};

} // namespace SZM::Workshop3D
