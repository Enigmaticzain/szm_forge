#include "AttachmentManager.hpp"
#include "KinematicSolver.hpp"
#include "Workshop3D/WorkshopManager.hpp"
#include "Workshop3D/PartLibrary.hpp"
#include <fstream>
#include <sstream>
#include <cmath>

namespace SZM::Assembly {

// ============================================================================
// BOLT OPERATIONS
// ============================================================================

UUID AttachmentManager::PlaceBolt(
    uint32_t ownerInstanceId,
    const SurfaceReference& surface,
    const BoltSpec& spec
) {
    BoltInstance bolt;
    bolt.id = GenerateUUID();
    bolt.ownerInstanceId = ownerInstanceId;
    bolt.surfaceRef = surface;
    bolt.spec = spec;
    bolt.isVisible = true;
    bolt.isInstalled = false;
    
    // Calculate dimensions from thread specification
    bolt.spec.CalculateFromThread();
    
    // Calculate world position
    bolt.surfaceRef.worldPosition = surface.worldPosition + 
                                    surface.surfaceNormal * surface.offset;
    
    m_Data.bolts.push_back(bolt);
    
    // Register with kinematic system
    RegisterBoltConstraints(m_Data.bolts.back());
    
    return bolt.id;
}

std::vector<UUID> AttachmentManager::PlaceBoltPattern(
    uint32_t ownerInstanceId,
    const SurfaceReference& baseSurface,
    const BoltSpec& spec,
    const BoltPattern& pattern
) {
    std::vector<UUID> ids;
    
    // Get pattern positions in local space
    auto localPositions = pattern.GetPositions();
    
    // Transform to world space and place bolts
    for (const auto& localPos : localPositions) {
        SurfaceReference surface = baseSurface;
        surface.worldPosition = baseSurface.worldPosition + localPos;
        
        UUID id = PlaceBolt(ownerInstanceId, surface, spec);
        ids.push_back(id);
    }
    
    return ids;
}

std::vector<UUID> AttachmentManager::PlaceBoltPatternCentered(
    uint32_t ownerInstanceId,
    const Geometry::Vector3& center,
    const Geometry::Vector3& surfaceNormal,
    const BoltSpec& spec,
    const BoltPattern& pattern
) {
    std::vector<UUID> ids;
    
    auto localPositions = pattern.GetPositions();
    
    for (const auto& localPos : localPositions) {
        SurfaceReference surface;
        surface.instanceId = ownerInstanceId;
        surface.worldPosition = center + localPos;
        surface.surfaceNormal = surfaceNormal;
        surface.offset = 0.0f;
        surface.snapTarget = SurfaceReference::SnapTarget::None;
        
        UUID id = PlaceBolt(ownerInstanceId, surface, spec);
        ids.push_back(id);
    }
    
    return ids;
}

bool AttachmentManager::ModifyBolt(const UUID& id, const BoltSpec& newSpec) {
    for (auto& bolt : m_Data.bolts) {
        if (bolt.id == id) {
            bolt.spec = newSpec;
            bolt.spec.CalculateFromThread();
            return true;
        }
    }
    return false;
}

bool AttachmentManager::RemoveBolt(const UUID& id) {
    for (auto it = m_Data.bolts.begin(); it != m_Data.bolts.end(); ++it) {
        if (it->id == id) {
            m_Data.bolts.erase(it);
            return true;
        }
    }
    return false;
}

BoltInstance* AttachmentManager::GetBolt(const UUID& id) {
    for (auto& bolt : m_Data.bolts) {
        if (bolt.id == id) return &bolt;
    }
    return nullptr;
}

const BoltInstance* AttachmentManager::GetBolt(const UUID& id) const {
    for (const auto& bolt : m_Data.bolts) {
        if (bolt.id == id) return &bolt;
    }
    return nullptr;
}

std::vector<BoltInstance*> AttachmentManager::GetBoltsForInstance(uint32_t instanceId) {
    std::vector<BoltInstance*> result;
    for (auto& bolt : m_Data.bolts) {
        if (bolt.ownerInstanceId == instanceId) {
            result.push_back(&bolt);
        }
    }
    return result;
}

// ============================================================================
// HINGE OPERATIONS
// ============================================================================

UUID AttachmentManager::PlaceHinge(
    uint32_t instanceA,
    uint32_t instanceB,
    const Geometry::Vector3& pivotPoint,
    const Geometry::Vector3& axis,
    const HingeSpec& spec
) {
    HingeInstance hinge;
    hinge.id = GenerateUUID();
    hinge.instanceA = instanceA;
    hinge.instanceB = instanceB;
    hinge.pivotPoint = pivotPoint;
    hinge.axis = axis;
    hinge.axis.Normalize();
    hinge.spec = spec;
    hinge.currentAngle = 0.0f;
    hinge.isConstrained = true;
    hinge.isVisible = true;
    
    m_Data.hinges.push_back(hinge);
    
    // Register with kinematic solver
    RegisterHingeConstraint(m_Data.hinges.back());
    
    return hinge.id;
}

UUID AttachmentManager::PlaceHingeFromSurfaces(
    uint32_t instanceA,
    uint32_t instanceB,
    const SurfaceReference& surfaceA,
    const SurfaceReference& surfaceB,
    const HingeSpec& spec
) {
    // Calculate pivot point as midpoint between surfaces
    Geometry::Vector3 pivotPoint = (surfaceA.worldPosition + surfaceB.worldPosition) * 0.5f;
    
    // Calculate axis from surface normals (hinge axis is perpendicular to both normals)
    Geometry::Vector3 axis = surfaceA.surfaceNormal.Cross(surfaceB.surfaceNormal);
    
    // If normals are parallel/opposite, use default axis
    if (axis.Length() < 0.001f) {
        axis = Geometry::Vector3(0.0f, 1.0f, 0.0f); // Default Y axis
    } else {
        axis.Normalize();
    }
    
    return PlaceHinge(instanceA, instanceB, pivotPoint, axis, spec);
}

bool AttachmentManager::ModifyHinge(const UUID& id, const HingeSpec& newSpec) {
    for (auto& hinge : m_Data.hinges) {
        if (hinge.id == id) {
            hinge.spec = newSpec;
            return true;
        }
    }
    return false;
}

void AttachmentManager::UpdateHingeAngle(const UUID& id, float angleDegrees) {
    for (auto& hinge : m_Data.hinges) {
        if (hinge.id == id) {
            // Clamp to limits
            hinge.currentAngle = std::clamp(
                angleDegrees,
                hinge.spec.minAngle,
                hinge.spec.maxAngle
            );
            return;
        }
    }
}

bool AttachmentManager::RemoveHinge(const UUID& id) {
    for (auto it = m_Data.hinges.begin(); it != m_Data.hinges.end(); ++it) {
        if (it->id == id) {
            m_Data.hinges.erase(it);
            return true;
        }
    }
    return false;
}

HingeInstance* AttachmentManager::GetHinge(const UUID& id) {
    for (auto& hinge : m_Data.hinges) {
        if (hinge.id == id) return &hinge;
    }
    return nullptr;
}

const HingeInstance* AttachmentManager::GetHinge(const UUID& id) const {
    for (const auto& hinge : m_Data.hinges) {
        if (hinge.id == id) return &hinge;
    }
    return nullptr;
}

std::vector<HingeInstance*> AttachmentManager::GetHingesForInstance(uint32_t instanceId) {
    std::vector<HingeInstance*> result;
    for (auto& hinge : m_Data.hinges) {
        if (hinge.instanceA == instanceId || hinge.instanceB == instanceId) {
            result.push_back(&hinge);
        }
    }
    return result;
}

// ============================================================================
// WELD OPERATIONS
// ============================================================================

UUID AttachmentManager::PlaceWeld(
    const std::vector<uint32_t>& instanceIds,
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    const WeldSpec& spec
) {
    WeldInstance weld;
    weld.id = GenerateUUID();
    weld.instanceIds = instanceIds;
    weld.spec = spec;
    weld.isVisible = true;
    
    // Create single bead
    WeldBead bead;
    bead.start = start;
    bead.end = end;
    bead.direction = end - start;
    bead.length = bead.direction.Length();
    bead.direction.Normalize();
    bead.legSize = spec.size;
    bead.throatThickness = spec.throat;
    
    weld.beads.push_back(bead);
    weld.CalculateTotalLength();
    weld.CalculateTotalStrength();
    
    m_Data.welds.push_back(weld);
    return weld.id;
}

UUID AttachmentManager::PlaceFilletWeldAlongEdge(
    uint32_t edgeInstanceId,
    const Geometry::Vector3& start,
    const Geometry::Vector3& end,
    const WeldSpec& spec
) {
    WeldInstance weld;
    weld.id = GenerateUUID();
    weld.instanceIds = {edgeInstanceId};
    weld.edgeInstanceId = edgeInstanceId;
    weld.followsEdge = true;
    weld.spec = spec;
    weld.isVisible = true;
    
    WeldBead bead;
    bead.start = start;
    bead.end = end;
    bead.direction = end - start;
    bead.length = bead.direction.Length();
    bead.direction.Normalize();
    bead.legSize = spec.size;
    bead.throatThickness = spec.throat;
    bead.workpieceIndexA = 0;
    
    weld.beads.push_back(bead);
    weld.CalculateTotalLength();
    weld.CalculateTotalStrength();
    
    m_Data.welds.push_back(weld);
    return weld.id;
}

UUID AttachmentManager::PlaceWeldAllAround(
    uint32_t instanceId,
    const WeldSpec& spec
) {
    WeldInstance weld;
    weld.id = GenerateUUID();
    weld.instanceIds = {instanceId};
    weld.spec = spec;
    weld.spec.weldAllAround = true;
    weld.isVisible = true;
    // Note: Actual edge geometry would need to come from the part's mesh
    // For now, this creates a placeholder
    
    m_Data.welds.push_back(weld);
    return weld.id;
}

std::vector<UUID> AttachmentManager::PlaceSpotWelds(
    uint32_t instanceId,
    const SurfaceReference& surface,
    const WeldSpec& spec,
    const BoltPattern& pattern
) {
    std::vector<UUID> ids;
    
    // Override type to spot
    WeldSpec spotSpec = spec;
    spotSpec.type = WeldType::Spot;
    
    auto positions = pattern.GetPositions();
    for (const auto& localPos : positions) {
        SurfaceReference spotSurface = surface;
        spotSurface.worldPosition = surface.worldPosition + localPos;
        
        UUID id = PlaceWeld(
            {instanceId},
            spotSurface.worldPosition,
            spotSurface.worldPosition + spotSurface.surfaceNormal * 1.0f,
            spotSpec
        );
        ids.push_back(id);
    }
    
    return ids;
}

bool AttachmentManager::ModifyWeld(const UUID& id, const WeldSpec& newSpec) {
    for (auto& weld : m_Data.welds) {
        if (weld.id == id) {
            weld.spec = newSpec;
            weld.CalculateTotalStrength();
            return true;
        }
    }
    return false;
}

bool AttachmentManager::RemoveWeld(const UUID& id) {
    for (auto it = m_Data.welds.begin(); it != m_Data.welds.end(); ++it) {
        if (it->id == id) {
            m_Data.welds.erase(it);
            return true;
        }
    }
    return false;
}

WeldInstance* AttachmentManager::GetWeld(const UUID& id) {
    for (auto& weld : m_Data.welds) {
        if (weld.id == id) return &weld;
    }
    return nullptr;
}

const WeldInstance* AttachmentManager::GetWeld(const UUID& id) const {
    for (const auto& weld : m_Data.welds) {
        if (weld.id == id) return &weld;
    }
    return nullptr;
}

std::vector<WeldInstance*> AttachmentManager::GetWeldsForInstance(uint32_t instanceId) {
    std::vector<WeldInstance*> result;
    for (auto& weld : m_Data.welds) {
        for (uint32_t id : weld.instanceIds) {
            if (id == instanceId) {
                result.push_back(&weld);
                break;
            }
        }
    }
    return result;
}

// ============================================================================
// SELECTION & EDITING
// ============================================================================

void AttachmentManager::SelectAttachment(const UUID& id) {
    m_Data.selectedAttachmentId = id;
    m_Data.selectedKind = AttachmentSystemData::AttachmentKind::None;
    
    // Determine kind
    for (const auto& bolt : m_Data.bolts) {
        if (bolt.id == id) {
            m_Data.selectedKind = AttachmentSystemData::AttachmentKind::Bolt;
            return;
        }
    }
    for (const auto& hinge : m_Data.hinges) {
        if (hinge.id == id) {
            m_Data.selectedKind = AttachmentSystemData::AttachmentKind::Hinge;
            return;
        }
    }
    for (const auto& weld : m_Data.welds) {
        if (weld.id == id) {
            m_Data.selectedKind = AttachmentSystemData::AttachmentKind::Weld;
            return;
        }
    }
}

void AttachmentManager::ClearSelection() {
    m_Data.selectedAttachmentId = UUID::Null();
    m_Data.selectedKind = AttachmentSystemData::AttachmentKind::None;
}

BoltInstance* AttachmentManager::GetSelectedBolt() {
    if (m_Data.selectedKind == AttachmentSystemData::AttachmentKind::Bolt) {
        return GetBolt(m_Data.selectedAttachmentId);
    }
    return nullptr;
}

HingeInstance* AttachmentManager::GetSelectedHinge() {
    if (m_Data.selectedKind == AttachmentSystemData::AttachmentKind::Hinge) {
        return GetHinge(m_Data.selectedAttachmentId);
    }
    return nullptr;
}

WeldInstance* AttachmentManager::GetSelectedWeld() {
    if (m_Data.selectedKind == AttachmentSystemData::AttachmentKind::Weld) {
        return GetWeld(m_Data.selectedAttachmentId);
    }
    return nullptr;
}

UUID AttachmentManager::PickAttachment(
    const Geometry::Vector3& rayOrigin,
    const Geometry::Vector3& rayDirection,
    float maxDistance
) {
    UUID closestId;
    float closestDist = maxDistance;
    
    // Check bolts
    for (const auto& bolt : m_Data.bolts) {
        Geometry::Vector3 toBolt = bolt.GetWorldPosition() - rayOrigin;
        float t = toBolt.Dot(rayDirection);
        if (t > 0 && t < closestDist) {
            Geometry::Vector3 closest = rayOrigin + rayDirection * t;
            float dist = (closest - bolt.GetWorldPosition()).Length();
            if (dist < 5.0f) { // Pick radius 5mm
                closestDist = t;
                closestId = bolt.id;
            }
        }
    }
    
    // Check hinges
    for (const auto& hinge : m_Data.hinges) {
        Geometry::Vector3 toHinge = hinge.pivotPoint - rayOrigin;
        float t = toHinge.Dot(rayDirection);
        if (t > 0 && t < closestDist) {
            Geometry::Vector3 closest = rayOrigin + rayDirection * t;
            float dist = (closest - hinge.pivotPoint).Length();
            if (dist < 10.0f) { // Larger pick radius for hinges
                closestDist = t;
                closestId = hinge.id;
            }
        }
    }
    
    // Check welds
    for (const auto& weld : m_Data.welds) {
        for (const auto& bead : weld.beads) {
            // Simple line distance check
            Geometry::Vector3 toStart = bead.start - rayOrigin;
            float t = toStart.Dot(rayDirection);
            if (t > 0 && t < closestDist) {
                Geometry::Vector3 closest = rayOrigin + rayDirection * t;
                float dist = PointToLineDistance(closest, bead.start, bead.end);
                if (dist < 5.0f) {
                    closestDist = t;
                    closestId = weld.id;
                    break;
                }
            }
        }
    }
    
    return closestId;
}

// ============================================================================
// PLACEMENT MODE
// ============================================================================

void AttachmentManager::StartBoltPlacement(const BoltSpec& spec) {
    CancelPlacement();
    m_Data.defaultBoltSpec = spec;
    m_Data.isPlacingBolt = true;
    m_Data.selectedKind = AttachmentSystemData::AttachmentKind::Bolt;
}

void AttachmentManager::StartHingePlacement(const HingeSpec& spec) {
    CancelPlacement();
    m_Data.defaultHingeSpec = spec;
    m_Data.isPlacingHinge = true;
    m_Data.selectedKind = AttachmentSystemData::AttachmentKind::Hinge;
}

void AttachmentManager::StartWeldPlacement(const WeldSpec& spec) {
    CancelPlacement();
    m_Data.defaultWeldSpec = spec;
    m_Data.isPlacingWeld = true;
    m_Data.selectedKind = AttachmentSystemData::AttachmentKind::Weld;
}

void AttachmentManager::CancelPlacement() {
    m_Data.isPlacingBolt = false;
    m_Data.isPlacingHinge = false;
    m_Data.isPlacingWeld = false;
    m_Data.showPreview = false;
}

UUID AttachmentManager::ConfirmPlacement() {
    if (!m_Data.showPreview) return UUID::Null();
    
    UUID result;
    
    if (m_Data.isPlacingBolt) {
        result = PlaceBolt(
            m_Data.previewSurface.instanceId,
            m_Data.previewSurface,
            m_Data.defaultBoltSpec
        );
    } else if (m_Data.isPlacingHinge) {
        // For hinges, need two instances
        // This would need UI interaction to complete
        // Placeholder for now
    } else if (m_Data.isPlacingWeld) {
        // Similar - needs UI interaction
    }
    
    m_Data.showPreview = false;
    return result;
}

void AttachmentManager::UpdatePlacementPreview(
    const SurfaceReference& surface,
    const Geometry::Vector3& position
) {
    m_Data.previewSurface = surface;
    m_Data.previewPosition = position;
    m_Data.showPreview = true;
}

bool AttachmentManager::IsInPlacementMode() const {
    return m_Data.isPlacingBolt || m_Data.isPlacingHinge || m_Data.isPlacingWeld;
}

AttachmentSystemData::AttachmentKind AttachmentManager::GetPlacementKind() const {
    if (m_Data.isPlacingBolt) return AttachmentSystemData::AttachmentKind::Bolt;
    if (m_Data.isPlacingHinge) return AttachmentSystemData::AttachmentKind::Hinge;
    if (m_Data.isPlacingWeld) return AttachmentSystemData::AttachmentKind::Weld;
    return AttachmentSystemData::AttachmentKind::None;
}

// ============================================================================
// SNAP & GRID
// ============================================================================

Geometry::Vector3 AttachmentManager::SnapPosition(const Geometry::Vector3& position) const {
    Geometry::Vector3 result = position;
    
    if (m_Data.snapToGrid) {
        float gridSize = m_Data.gridSize;
        result.x = std::round(result.x / gridSize) * gridSize;
        result.y = std::round(result.y / gridSize) * gridSize;
        result.z = std::round(result.z / gridSize) * gridSize;
    }
    
    // Additional edge/vertex snapping would require mesh data
    // This is a placeholder that could be enhanced
    
    return result;
}

SurfaceReference AttachmentManager::GetSurfaceAt(const Geometry::Vector3& position) const {
    SurfaceReference ref;
    ref.worldPosition = position;
    ref.surfaceNormal = Geometry::Vector3(0, 0, 1); // Default normal
    ref.u = 0.5f;
    ref.v = 0.5f;
    ref.offset = 0.0f;
    ref.snapTarget = SurfaceReference::SnapTarget::None;
    return ref;
}

void AttachmentManager::SetSnapSettings(
    bool grid, bool edge, bool vertex, bool center,
    float gridSize, float snapRadius
) {
    m_Data.snapToGrid = grid;
    m_Data.snapToEdge = edge;
    m_Data.snapToVertex = vertex;
    m_Data.snapToCenter = center;
    m_Data.gridSize = gridSize;
    m_Data.snapRadius = snapRadius;
}

void AttachmentManager::GetSnapSettings(
    bool& grid, bool& edge, bool& vertex, bool& center,
    float& gridSize, float& snapRadius
) const {
    grid = m_Data.snapToGrid;
    edge = m_Data.snapToEdge;
    vertex = m_Data.snapToVertex;
    center = m_Data.snapToCenter;
    gridSize = m_Data.gridSize;
    snapRadius = m_Data.snapRadius;
}

// ============================================================================
// KINEMATIC INTEGRATION
// ============================================================================

void AttachmentManager::SetKinematicSolver(KinematicSolver* solver) {
    m_KinematicSolver = solver;
}

void AttachmentManager::ApplyHingeConstraints() {
    if (!m_KinematicSolver) return;
    
    for (const auto& hinge : m_Data.hinges) {
        RegisterHingeConstraint(const_cast<HingeInstance&>(hinge));
    }
}

void AttachmentManager::RemoveHingeConstraints() {
    // Would need to track constraint IDs to remove them
    // This is a simplified implementation
}

// ============================================================================
// SERIALIZATION
// ============================================================================

bool AttachmentManager::SaveToJSON(const std::string& filepath) const {
    // Simplified JSON serialization
    // Full implementation would use proper JSON library
    return false; // Placeholder
}

bool AttachmentManager::LoadFromJSON(const std::string& filepath) {
    // Simplified JSON deserialization
    return false; // Placeholder
}

// ============================================================================
// ANALYSIS
// ============================================================================

float AttachmentManager::CalculateTotalClampForce() const {
    float total = 0.0f;
    for (const auto& bolt : m_Data.bolts) {
        if (bolt.isInstalled) {
            total += bolt.spec.preload;
        }
    }
    return total;
}

float AttachmentManager::CalculateTotalWeldStrength() const {
    float total = 0.0f;
    for (const auto& weld : m_Data.welds) {
        total += weld.totalStrength;
    }
    return total;
}

bool AttachmentManager::IsAdequatelyFastened() const {
    // Simple check: has at least one attachment
    return !m_Data.bolts.empty() || !m_Data.welds.empty() || !m_Data.hinges.empty();
}

std::string AttachmentManager::GenerateAttachmentReport() const {
    std::ostringstream oss;
    oss << "=== ATTACHMENT REPORT ===\n\n";
    
    oss << "BOLTS: " << m_Data.bolts.size() << " total\n";
    for (const auto& bolt : m_Data.bolts) {
        oss << "  - " << bolt.spec.GetDisplayName() 
            << " at (" << bolt.GetWorldPosition().x << ", " 
            << bolt.GetWorldPosition().y << ", " 
            << bolt.GetWorldPosition().z << ")\n";
        oss << "    Torque: " << bolt.spec.torque << " Nm, "
            << "Preload: " << bolt.spec.preload << " N\n";
    }
    
    oss << "\nHINGES: " << m_Data.hinges.size() << " total\n";
    for (const auto& hinge : m_Data.hinges) {
        oss << "  - " << hinge.spec.GetDisplayName() << "\n";
        oss << "    Pivot: (" << hinge.pivotPoint.x << ", " 
            << hinge.pivotPoint.y << ", " << hinge.pivotPoint.z << ")\n";
        oss << "    Angle: " << hinge.currentAngle << " deg\n";
    }
    
    oss << "\nWELDS: " << m_Data.welds.size() << " total\n";
    for (const auto& weld : m_Data.welds) {
        oss << "  - " << weld.spec.GetDisplayName() << "\n";
        oss << "    Length: " << weld.totalLength << " mm, "
            << "Strength: " << weld.totalStrength << " N\n";
    }
    
    oss << "\nTotal Clamp Force: " << CalculateTotalClampForce() << " N\n";
    oss << "Total Weld Strength: " << CalculateTotalWeldStrength() << " N\n";
    
    return oss.str();
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

SurfaceReference AttachmentManager::CreateSurfaceRef(
    uint32_t instanceId,
    const Geometry::Vector3& position,
    const Geometry::Vector3& normal,
    float offset
) {
    SurfaceReference ref;
    ref.instanceId = instanceId;
    ref.worldPosition = position;
    ref.surfaceNormal = normal;
    ref.offset = offset;
    ref.u = 0.5f;
    ref.v = 0.5f;
    ref.snapTarget = SurfaceReference::SnapTarget::None;
    return ref;
}

Geometry::Vector3 AttachmentManager::SurfaceToWorld(
    const SurfaceReference& surface,
    const Geometry::Vector3& instancePosition,
    const Geometry::Vector3& instanceRotation
) const {
    // Simplified transformation
    return surface.worldPosition + surface.surfaceNormal * surface.offset;
}

Geometry::Vector3 AttachmentManager::FindNearestSnapPoint(const Geometry::Vector3& position) const {
    // Placeholder for edge/vertex snapping
    // Would need mesh geometry data
    return position;
}

void AttachmentManager::RegisterBoltConstraints(BoltInstance& bolt) {
    // Bolt adds rigidity between clamped parts
    // Could register with kinematic solver for detailed analysis
}

void AttachmentManager::RegisterHingeConstraint(HingeInstance& hinge) {
    if (!m_KinematicSolver) return;
    
    // Create a revolute joint constraint
    // This connects instanceA and instanceB at the hinge pivot
    
    // Get instance states (would need access to workshop)
    // Create ConcentricMate at hinge pivot
    // Register with kinematic solver
}

} // namespace SZM::Assembly
