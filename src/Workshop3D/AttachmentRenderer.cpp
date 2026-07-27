#include "AttachmentRenderer.hpp"
#include "Math/Matrix4x4.hpp"
#include "Math/Vector3.hpp"
#include <cmath>

namespace SZM::Workshop3D {

using namespace SZM::Assembly;
using namespace SZM::Graphics;

// ============================================================================
// CONSTRUCTION
// ============================================================================

AttachmentRenderer::AttachmentRenderer() = default;

// ============================================================================
// BOLT RENDERING
// ============================================================================

void AttachmentRenderer::RenderBolt(const BoltInstance& bolt) {
    Color4 color = GetColorForBolt(bolt);
    RenderBoltPreview(bolt.GetWorldPosition(), bolt.surfaceRef.surfaceNormal, bolt.spec);
}

void AttachmentRenderer::RenderBoltPreview(
    const Geometry::Vector3& position,
    const Geometry::Vector3& normal,
    const BoltSpec& spec
) {
    // Build transformation matrix
    // Normal points away from surface, bolt goes opposite direction
    Geometry::Vector3 axis = -normal.Normalized();
    
    // Create rotation to align Y-axis with axis
    Geometry::Vector3 yAxis(0, 1, 0);
    Geometry::Vector3 cross = yAxis.Cross(axis);
    float dot = yAxis.Dot(axis);
    
    float angle = std::acos(std::clamp(dot, -1.0f, 1.0f));
    
    Matrix4x4 transform;
    transform.Identity();
    
    if (cross.Length() > 0.001f) {
        cross.Normalize();
        transform.SetRotation(axis, angle);
    }
    
    transform.SetTranslation(position);
    
    // Draw hex head
    float headRadius = spec.headDiameter / 2.0f * m_RenderScale;
    float headHeight = spec.headHeight * m_RenderScale;
    DrawHexHead(transform, headRadius, headHeight, Color4(0.4f, 0.4f, 0.45f, 1.0f));
    
    // Draw shaft
    float shaftRadius = spec.thread.nominalDiameter / 2.0f * m_RenderScale * 0.9f;
    float shaftLength = spec.length * m_RenderScale;
    DrawCylinder(transform * Matrix4x4::Translation(Geometry::Vector3(0, -shaftLength/2, 0)),
                 shaftRadius, shaftLength, Color4(0.35f, 0.35f, 0.4f, 1.0f));
    
    // Draw thread lines
    float pitch = spec.thread.pitch * m_RenderScale;
    DrawThreadLines(transform, shaftRadius, shaftLength, pitch);
    
    // Draw washer face indicator
    DrawCircle(transform, shaftRadius * 1.2f, Color4(0.5f, 0.5f, 0.55f, 1.0f));
}

void AttachmentRenderer::RenderAllBolts(const std::vector<BoltInstance>& bolts) {
    for (const auto& bolt : bolts) {
        if (bolt.isVisible) {
            RenderBolt(bolt);
        }
    }
}

// ============================================================================
// HINGE RENDERING
// ============================================================================

void AttachmentRenderer::RenderHinge(const HingeInstance& hinge) {
    Color4 color = GetColorForHinge(hinge);
    RenderHingePreview(hinge.pivotPoint, hinge.axis, hinge.spec.length, hinge.spec.width);
}

void AttachmentRenderer::RenderHingePreview(
    const Geometry::Vector3& pivot,
    const Geometry::Vector3& axis,
    float length,
    float width
) {
    length *= m_RenderScale;
    width *= m_RenderScale;
    float thickness = 3.0f * m_RenderScale;
    float pinRadius = 3.0f * m_RenderScale;
    
    // Draw hinge barrel (center)
    Geometry::Vector3 perpAxis1 = axis.Orthogonal().Normalized();
    Geometry::Vector3 perpAxis2 = axis.Cross(perpAxis1).Normalized();
    
    // Draw barrel
    DrawCylinderAt(pivot, axis, pinRadius * 1.5f, width, m_Colors.hingeBarrel);
    
    // Draw pin
    DrawCylinderAt(pivot + perpAxis2 * (width/2), axis, pinRadius, width * 0.9f, Color4(0.6f, 0.6f, 0.65f, 1.0f));
    
    // Draw hinge leaves
    float leafLength = length / 2.0f;
    
    // Left leaf
    DrawBoxAt(pivot - perpAxis2 * (width/2 + thickness/2), 
              Geometry::Vector3(leafLength, width * 0.8f, thickness), m_Colors.hingeLeaf);
    
    // Right leaf
    DrawBoxAt(pivot + perpAxis2 * (width/2 + thickness/2),
              Geometry::Vector3(leafLength, width * 0.8f, thickness), m_Colors.hingeLeaf);
    
    // Draw mounting holes
    for (int i = 0; i < 3; ++i) {
        float holeOffset = -leafLength/2 + leafLength * (i + 1) / 4.0f;
        DrawCircleAt(pivot - perpAxis2 * (width/2 + thickness/2) + perpAxis1 * holeOffset,
                    perpAxis2, 2.0f * m_RenderScale, Color4(0.2f, 0.2f, 0.25f, 1.0f));
    }
    
    // Draw axis indicator
    DrawLine(pivot - axis * 20, pivot + axis * 20, Color4(1.0f, 0.0f, 0.0f, 0.5f));
}

void AttachmentRenderer::RenderAllHinges(const std::vector<HingeInstance>& hinges) {
    for (const auto& hinge : hinges) {
        if (hinge.isVisible) {
            RenderHinge(hinge);
        }
    }
}

// ============================================================================
// WELD RENDERING
// ============================================================================

void AttachmentRenderer::RenderWeld(const WeldInstance& weld) {
    Color4 color = GetColorForWeld(weld);
    
    for (const auto& bead : weld.beads) {
        RenderWeldPreview(bead.start, bead.end, weld.spec);
    }
}

void AttachmentRenderer::RenderWeldPreview(
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    const WeldSpec& spec
) {
    float legSize = spec.size * m_RenderScale;
    
    switch (spec.type) {
        case WeldType::Fillet:
        case WeldType::FilletAllAround:
            // Draw triangular fillet weld
            DrawFilletWeld(start, end, legSize);
            break;
            
        case WeldType::Butt:
            // Draw butt weld as raised ridge
            DrawButtWeld(start, end, spec.throat * m_RenderScale);
            break;
            
        case WeldType::Spot:
            // Draw spot weld as circle
            DrawSpotWeld(start, legSize * 2);
            break;
            
        case WeldType::Seam:
            // Draw continuous seam
            DrawSeamWeld(start, end, spec.throat * m_RenderScale);
            break;
            
        default:
            // Generic weld representation
            DrawWeldBead(start, end, legSize);
            break;
    }
    
    // Draw weld labels if enabled
    if (m_ShowLabels) {
        Geometry::Vector3 midpoint = (start + end) / 2.0f;
        DrawLabel(midpoint, "W");
    }
}

void AttachmentRenderer::RenderAllWelds(const std::vector<WeldInstance>& welds) {
    for (const auto& weld : welds) {
        if (weld.isVisible) {
            RenderWeld(weld);
        }
    }
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

Graphics::Color4 AttachmentRenderer::GetColorForBolt(const BoltInstance& bolt) {
    return GetEffectiveColor(
        m_Colors.boltHead,
        bolt.id,
        m_SelectedId,
        m_HoveredId
    );
}

Graphics::Color4 AttachmentRenderer::GetColorForHinge(const HingeInstance& hinge) {
    return GetEffectiveColor(
        m_Colors.hingeBarrel,
        hinge.id,
        m_SelectedId,
        m_HoveredId
    );
}

Graphics::Color4 AttachmentRenderer::GetColorForWeld(const WeldInstance& weld) {
    return GetEffectiveColor(
        m_Colors.weldBead,
        weld.id,
        m_SelectedId,
        m_HoveredId
    );
}

Graphics::Color4 AttachmentRenderer::GetEffectiveColor(
    const Color4& base,
    const UUID& itemId,
    const UUID& selectedId,
    const UUID& hoveredId
) {
    if (itemId == selectedId) {
        return m_Colors.selected;
    } else if (itemId == hoveredId) {
        return m_Colors.hovered;
    }
    return base;
}

// ============================================================================
// PRIMITIVE DRAW FUNCTIONS
// These would use the actual graphics API in a real implementation
// ============================================================================

void AttachmentRenderer::DrawHexHead(
    const Matrix4x4& transform,
    float radius,
    float height,
    const Color4& color
) {
    // Draw 6-sided polygon for hex head
    // In real implementation: use geometry buffer or immediate mode
    Geometry::Vector3 center = transform.GetTranslation();
    
    for (int i = 0; i < 6; ++i) {
        float angle1 = i * 3.14159f / 3.0f;
        float angle2 = (i + 1) * 3.14159f / 3.0f;
        
        Geometry::Vector3 p1(center.x + radius * std::cos(angle1),
                            center.y,
                            center.z + radius * std::sin(angle1));
        Geometry::Vector3 p2(center.x + radius * std::cos(angle2),
                            center.y,
                            center.z + radius * std::sin(angle2));
        Geometry::Vector3 p3(p2.x, p2.y - height, p2.z);
        Geometry::Vector3 p4(p1.x, p1.y - height, p1.z);
        
        DrawQuad(p1, p2, p3, p4, color);
    }
    
    // Top face
    DrawHexagon(center, radius, color);
}

void AttachmentRenderer::DrawCylinder(
    const Matrix4x4& transform,
    float radius,
    float height,
    const Color4& color
) {
    Geometry::Vector3 center = transform.GetTranslation();
    int segments = 16;
    
    for (int i = 0; i < segments; ++i) {
        float angle1 = i * 2.0f * 3.14159f / segments;
        float angle2 = (i + 1) * 2.0f * 3.14159f / segments;
        
        float x1 = center.x + radius * std::cos(angle1);
        float z1 = center.z + radius * std::sin(angle1);
        float x2 = center.x + radius * std::cos(angle2);
        float z2 = center.z + radius * std::sin(angle2);
        
        // Side quad
        Geometry::Vector3 v1(x1, center.y, z1);
        Geometry::Vector3 v2(x2, center.y, z2);
        Geometry::Vector3 v3(x2, center.y + height, z2);
        Geometry::Vector3 v4(x1, center.y + height, z1);
        
        DrawQuad(v1, v2, v3, v4, color);
    }
}

void AttachmentRenderer::DrawCylinderAt(
    const Geometry::Vector3& center,
    const Geometry::Vector3& axis,
    float radius,
    float height,
    const Color4& color
) {
    Matrix4x4 transform;
    transform.Identity();
    transform.SetTranslation(center);
    // Would also set rotation to align with axis
    DrawCylinder(transform, radius, height, color);
}

void AttachmentRenderer::DrawThreadLines(
    const Matrix4x4& transform,
    float radius,
    float length,
    float pitch
) {
    // Draw helical thread lines (simplified as rings)
    int numRings = static_cast<int>(length / pitch);
    
    for (int i = 0; i < numRings; ++i) {
        float y = -i * pitch;
        DrawRing(transform.GetTranslation() + Geometry::Vector3(0, y, 0), radius, color);
    }
}

void AttachmentRenderer::DrawFilletWeld(
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    float legSize
) {
    // Draw triangular cross-section fillet weld
    Geometry::Vector3 direction = (end - start).Normalized();
    Geometry::Vector3 perpendicular(0, 1, 0); // Would be computed from context
    
    Geometry::Vector3 v1 = start;
    Geometry::Vector3 v2 = end;
    Geometry::Vector3 v3 = start + perpendicular * legSize;
    Geometry::Vector3 v4 = end + perpendicular * legSize;
    
    DrawQuad(v1, v2, v4, v3, m_Colors.weldBead);
    
    // Draw triangular ends
    DrawTriangle(start, start + direction * legSize, v3, m_Colors.weldBead);
    DrawTriangle(end - direction * legSize, end, v4, m_Colors.weldBead);
}

void AttachmentRenderer::DrawButtWeld(
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    float height
) {
    // Draw raised butt weld as elongated octagon
    Geometry::Vector3 direction = (end - start).Normalized();
    Geometry::Vector3 perpendicular(0, 1, 0);
    
    Geometry::Vector3 v1 = start + perpendicular * height;
    Geometry::Vector3 v2 = end + perpendicular * height;
    Geometry::Vector3 v3 = end - perpendicular * height;
    Geometry::Vector3 v4 = start - perpendicular * height;
    
    DrawQuad(v1, v2, v3, v4, m_Colors.weldBead);
}

void AttachmentRenderer::DrawSpotWeld(
    const Geometry::Vector3& position,
    float diameter
) {
    DrawCircle(position, diameter / 2.0f, m_Colors.weldBead);
}

void AttachmentRenderer::DrawSeamWeld(
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    float height
) {
    DrawButtWeld(start, end, height);
}

void AttachmentRenderer::DrawWeldBead(
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    float thickness
) {
    DrawLine(start, end, m_Colors.weldBead);
}

// ============================================================================
// PLACEHOLDER IMPLEMENTATIONS
// These would be implemented using the actual graphics API
// ============================================================================

void AttachmentRenderer::DrawBoxAt(
    const Geometry::Vector3& center,
    const Geometry::Vector3& size,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawCircleAt(
    const Geometry::Vector3& center,
    const Geometry::Vector3& normal,
    float radius,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawCircle(
    const Geometry::Vector3& center,
    float radius,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawHexagon(
    const Geometry::Vector3& center,
    float radius,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawRing(
    const Geometry::Vector3& center,
    float radius,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawQuad(
    const Geometry::Vector3& p1,
    const Geometry::Vector3& p2,
    const Geometry::Vector3& p3,
    const Geometry::Vector3& p4,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawTriangle(
    const Geometry::Vector3& p1,
    const Geometry::Vector3& p2,
    const Geometry::Vector3& p3,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawLine(
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    const Color4& color
) {
    // Placeholder - would use actual rendering API
}

void AttachmentRenderer::DrawLabel(
    const Geometry::Vector3& position,
    const char* text
) {
    // Placeholder - would use actual UI rendering
}

} // namespace SZM::Workshop3D
