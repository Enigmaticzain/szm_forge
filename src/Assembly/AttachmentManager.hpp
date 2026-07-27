#pragma once

#include "FastenerDataTypes.hpp"
#include "Mates.hpp"
#include "Data/UUID.hpp"
#include "Math/Vector3.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace SZM::Assembly {

// Forward declarations
class KinematicSolver;

/**
 * AttachmentManager - Comprehensive fastener, hinge, and weld management
 * 
 * This system provides precise placement and control of mechanical attachments
 * including:
 * - Bolts with patterns and specifications
 * - Hinges with pivot control and limits
 * - Welds with bead visualization
 * 
 * Integrates with the kinematic solver for hinge motion simulation.
 */
class AttachmentManager {
public:
    AttachmentManager() = default;
    ~AttachmentManager() = default;
    
    // ============================================================================
    // BOLT OPERATIONS
    // ============================================================================
    
    /**
     * Place a single bolt at a specific location
     */
    UUID PlaceBolt(
        uint32_t ownerInstanceId,
        const SurfaceReference& surface,
        const BoltSpec& spec
    );
    
    /**
     * Place multiple bolts in a pattern
     */
    std::vector<UUID> PlaceBoltPattern(
        uint32_t ownerInstanceId,
        const SurfaceReference& baseSurface,
        const BoltSpec& spec,
        const BoltPattern& pattern
    );
    
    /**
     * Place a bolt pattern centered on a point
     */
    std::vector<UUID> PlaceBoltPatternCentered(
        uint32_t ownerInstanceId,
        const Geometry::Vector3& center,
        const Geometry::Vector3& surfaceNormal,
        const BoltSpec& spec,
        const BoltPattern& pattern
    );
    
    /**
     * Modify an existing bolt
     */
    bool ModifyBolt(const UUID& id, const BoltSpec& newSpec);
    
    /**
     * Remove a bolt
     */
    bool RemoveBolt(const UUID& id);
    
    /**
     * Get bolt by ID
     */
    BoltInstance* GetBolt(const UUID& id);
    const BoltInstance* GetBolt(const UUID& id) const;
    
    /**
     * Get all bolts for an instance
     */
    std::vector<BoltInstance*> GetBoltsForInstance(uint32_t instanceId);
    
    /**
     * Get bolt count
     */
    size_t GetBoltCount() const { return m_Data.bolts.size(); }
    
    // ============================================================================
    // HINGE OPERATIONS
    // ============================================================================
    
    /**
     * Place a hinge between two parts
     */
    UUID PlaceHinge(
        uint32_t instanceA,
        uint32_t instanceB,
        const Geometry::Vector3& pivotPoint,
        const Geometry::Vector3& axis,
        const HingeSpec& spec
    );
    
    /**
     * Place a hinge by selecting surface references
     * (calculates pivot and axis automatically from surfaces)
     */
    UUID PlaceHingeFromSurfaces(
        uint32_t instanceA,
        uint32_t instanceB,
        const SurfaceReference& surfaceA,
        const SurfaceReference& surfaceB,
        const HingeSpec& spec
    );
    
    /**
     * Modify an existing hinge
     */
    bool ModifyHinge(const UUID& id, const HingeSpec& newSpec);
    
    /**
     * Update hinge angle (for simulation)
     */
    void UpdateHingeAngle(const UUID& id, float angleDegrees);
    
    /**
     * Remove a hinge
     */
    bool RemoveHinge(const UUID& id);
    
    /**
     * Get hinge by ID
     */
    HingeInstance* GetHinge(const UUID& id);
    const HingeInstance* GetHinge(const UUID& id) const;
    
    /**
     * Get all hinges for an instance
     */
    std::vector<HingeInstance*> GetHingesForInstance(uint32_t instanceId);
    
    /**
     * Get hinge count
     */
    size_t GetHingeCount() const { return m_Data.hinges.size(); }
    
    // ============================================================================
    // WELD OPERATIONS
    // ============================================================================
    
    /**
     * Place a weld along a line between two parts
     */
    UUID PlaceWeld(
        const std::vector<uint32_t>& instanceIds,
        const Geometry::Vector3& start,
        const Geometry::Vector3& end,
        const WeldSpec& spec
    );
    
    /**
     * Place a fillet weld along an edge
     */
    UUID PlaceFilletWeldAlongEdge(
        uint32_t edgeInstanceId,
        const Geometry::Vector3& start,
        const Geometry::Vector3& end,
        const WeldSpec& spec
    );
    
    /**
     * Place a weld-all-around on a part
     */
    UUID PlaceWeldAllAround(
        uint32_t instanceId,
        const WeldSpec& spec
    );
    
    /**
     * Place spot welds along a pattern
     */
    std::vector<UUID> PlaceSpotWelds(
        uint32_t instanceId,
        const SurfaceReference& surface,
        const WeldSpec& spec,
        const BoltPattern& pattern
    );
    
    /**
     * Modify an existing weld
     */
    bool ModifyWeld(const UUID& id, const WeldSpec& newSpec);
    
    /**
     * Remove a weld
     */
    bool RemoveWeld(const UUID& id);
    
    /**
     * Get weld by ID
     */
    WeldInstance* GetWeld(const UUID& id);
    const WeldInstance* GetWeld(const UUID& id) const;
    
    /**
     * Get welds for an instance
     */
    std::vector<WeldInstance*> GetWeldsForInstance(uint32_t instanceId);
    
    /**
     * Get weld count
     */
    size_t GetWeldCount() const { return m_Data.welds.size(); }
    
    // ============================================================================
    // SELECTION & EDITING
    // ============================================================================
    
    /**
     * Select an attachment by ID
     */
    void SelectAttachment(const UUID& id);
    
    /**
     * Clear selection
     */
    void ClearSelection();
    
    /**
     * Get selected attachment
     */
    BoltInstance* GetSelectedBolt();
    HingeInstance* GetSelectedHinge();
    WeldInstance* GetSelectedWeld();
    
    /**
     * Select attachment at world position (ray pick)
     */
    UUID PickAttachment(
        const Geometry::Vector3& rayOrigin,
        const Geometry::Vector3& rayDirection,
        float maxDistance = 1000.0f
    );
    
    // ============================================================================
    // PLACEMENT MODE
    // ============================================================================
    
    /**
     * Enter bolt placement mode
     */
    void StartBoltPlacement(const BoltSpec& spec);
    
    /**
     * Enter hinge placement mode
     */
    void StartHingePlacement(const HingeSpec& spec);
    
    /**
     * Enter weld placement mode
     */
    void StartWeldPlacement(const WeldSpec& spec);
    
    /**
     * Cancel placement mode
     */
    void CancelPlacement();
    
    /**
     * Confirm current placement
     */
    UUID ConfirmPlacement();
    
    /**
     * Update placement preview
     */
    void UpdatePlacementPreview(
        const SurfaceReference& surface,
        const Geometry::Vector3& position
    );
    
    /**
     * Check if in placement mode
     */
    bool IsInPlacementMode() const;
    AttachmentSystemData::AttachmentKind GetPlacementKind() const;
    
    // ============================================================================
    // SNAP & GRID
    // ============================================================================
    
    /**
     * Snap a position to nearest snap point
     */
    Geometry::Vector3 SnapPosition(const Geometry::Vector3& position) const;
    
    /**
     * Get surface info at position (for placement)
     */
    SurfaceReference GetSurfaceAt(const Geometry::Vector3& position) const;
    
    /**
     * Set snap settings
     */
    void SetSnapSettings(bool grid, bool edge, bool vertex, bool center, float gridSize, float snapRadius);
    
    /**
     * Get snap settings
     */
    void GetSnapSettings(bool& grid, bool& edge, bool& vertex, bool& center, 
                         float& gridSize, float& snapRadius) const;
    
    // ============================================================================
    // KINEMATIC INTEGRATION
    // ============================================================================
    
    /**
     * Connect to kinematic solver for hinge simulation
     */
    void SetKinematicSolver(KinematicSolver* solver);
    
    /**
     * Apply hinge constraints to solver
     */
    void ApplyHingeConstraints();
    
    /**
     * Remove hinge constraints from solver
     */
    void RemoveHingeConstraints();
    
    // ============================================================================
    // DEFAULT SPECIFICATIONS
    // ============================================================================
    
    /**
     * Get/set default bolt specification
     */
    const BoltSpec& GetDefaultBoltSpec() const { return m_Data.defaultBoltSpec; }
    void SetDefaultBoltSpec(const BoltSpec& spec) { m_Data.defaultBoltSpec = spec; }
    
    /**
     * Get/set default hinge specification
     */
    const HingeSpec& GetDefaultHingeSpec() const { return m_Data.defaultHingeSpec; }
    void SetDefaultHingeSpec(const HingeSpec& spec) { m_Data.defaultHingeSpec = spec; }
    
    /**
     * Get/set default weld specification
     */
    const WeldSpec& GetDefaultWeldSpec() const { return m_Data.defaultWeldSpec; }
    void SetDefaultWeldSpec(const WeldSpec& spec) { m_Data.defaultWeldSpec = spec; }
    
    // ============================================================================
    // SERIALIZATION
    // ============================================================================
    
    /**
     * Save all attachments to JSON
     */
    bool SaveToJSON(const std::string& filepath) const;
    
    /**
     * Load attachments from JSON
     */
    bool LoadFromJSON(const std::string& filepath);
    
    /**
     * Get data reference (for rendering)
     */
    const AttachmentSystemData& GetData() const { return m_Data; }
    AttachmentSystemData& GetData() { return m_Data; }
    
    // ============================================================================
    // ANALYSIS
    // ============================================================================
    
    /**
     * Calculate total clamp force from all bolts
     */
    float CalculateTotalClampForce() const;
    
    /**
     * Calculate total weld strength
     */
    float CalculateTotalWeldStrength() const;
    
    /**
     * Check if assembly is adequately fastened
     */
    bool IsAdequatelyFastened() const;
    
    /**
     * Generate report of all attachments
     */
    std::string GenerateAttachmentReport() const;

private:
    AttachmentSystemData m_Data;
    KinematicSolver* m_KinematicSolver = nullptr;
    
    // Generate unique IDs
    UUID GenerateUUID() {
        return UUID::Generate();
    }
    
    // Helper: Create surface reference from position and normal
    SurfaceReference CreateSurfaceRef(
        uint32_t instanceId,
        const Geometry::Vector3& position,
        const Geometry::Vector3& normal,
        float offset = 0.0f
    );
    
    // Helper: Transform surface reference to world coordinates
    Geometry::Vector3 SurfaceToWorld(
        const SurfaceReference& surface,
        const Geometry::Vector3& instancePosition,
        const Geometry::Vector3& instanceRotation
    ) const;
    
    // Helper: Find nearest snap point
    Geometry::Vector3 FindNearestSnapPoint(const Geometry::Vector3& position) const;
    
    // Helper: Add bolt to kinematic system if needed
    void RegisterBoltConstraints(BoltInstance& bolt);
    
    // Helper: Add hinge to kinematic system
    void RegisterHingeConstraint(HingeInstance& hinge);
};

} // namespace SZM::Assembly
