#pragma once

#include "Data/UUID.hpp"
#include "Math/Vector3.hpp"
#include <array>
#include <string>
#include <vector>

namespace SZM::Assembly {

// ============================================================================
// ENUMS
// ============================================================================

enum class FastenerType {
    HexBolt,
    SocketHeadCapScrew,
    MachineScrew,
    Rivet,
    Stud,
    AnchorBolt
};

enum class ThreadStandard {
    Metric,
    Imperial,
    UN,
    NPT
};

enum class BoltGrade {
    Grade8_8,    // Metric 8.8
    Grade10_9,   // Metric 10.9
    Grade12_9,   // Metric 12.9
    Grade5,      // Imperial Grade 5
    Grade8,      // Imperial Grade 8
    A325,        // High strength structural
    A490         // High strength structural
};

enum class HingeType {
    Standard,        // Basic door hinge
    Piano,           // Continuous hinge
    Butt,            // Butt hinge
    Barrel,          // Barrel/continuous hinge
    Spring,          // Spring-loaded return
    Detent           // Hinge with detent positions
};

enum class WeldType {
    Fillet,          // T-joint weld (triangular cross-section)
    Butt,            // Edge-to-edge weld
    Plug,            // Filled hole weld
    Slot,            // Elongated plug weld
    Spot,            // Spot weld
    Seam,            // Continuous seam weld
    PlugAndSlot,     // Combined plug/slot
    FilletAllAround  // Fillet weld around perimeter
};

enum class WeldProcess {
    SMAW,            // Stick welding (Shielded Metal Arc)
    GMAW,            // MIG/GMAW
    FCAW,            // Flux-cored
    GTAW,            // TIG
    SAW,             // Submerged Arc
    EBW,             // Electron Beam
    LBW              // Laser Beam
};

// ============================================================================
// SURFACE REFERENCE - Where an attachment is placed
// ============================================================================

struct SurfaceReference {
    uint32_t instanceId = 0U;          // Which assembly instance
    int faceIndex = -1;                // Which face (-1 = custom point)
    
    // Surface-local coordinates for precise placement
    float u = 0.5f;                   // 0-1 position on face (u-axis)
    float v = 0.5f;                   // 0-1 position on face (v-axis)
    
    // Actual world position (computed from surface + u,v)
    Geometry::Vector3 worldPosition;
    
    // Surface normal at placement point
    Geometry::Vector3 surfaceNormal;
    
    // Offset from surface (positive = away from surface)
    float offset = 0.0f;
    
    // Edge/vertex snapping
    enum class SnapTarget {
        None,
        FaceCenter,
        EdgeCenter,
        EdgeStart,
        EdgeEnd,
        Vertex,
        GridPoint
    };
    SnapTarget snapTarget = SnapTarget::FaceCenter;
    
    // Which edge/vertex if snapped
    int edgeIndex = -1;
    int vertexIndex = -1;
};

// ============================================================================
// THREAD SPECIFICATION
// ============================================================================

struct ThreadSpec {
    ThreadStandard standard = ThreadStandard::Metric;
    
    // Nominal diameter (mm for metric, inches for imperial)
    float nominalDiameter = 10.0f;    // M10, 3/8", etc.
    
    // Thread pitch (mm for metric, threads/inch for imperial)
    float pitch = 1.5f;
    
    // Common metric sizes for quick selection
    static constexpr std::array<float, 20> COMMON_METRIC = {
        3.0f, 4.0f, 5.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f,
        20.0f, 22.0f, 24.0f, 27.0f, 30.0f, 33.0f, 36.0f, 39.0f, 42.0f, 48.0f
    };
    
    // Common pitches for M10 (example)
    static constexpr std::array<float, 5> PITCHES_M10 = {0.75f, 1.0f, 1.25f, 1.5f, 2.0f};
    
    std::string ToString() const {
        if (standard == ThreadStandard::Metric) {
            return "M" + std::to_string(static_cast<int>(nominalDiameter)) + "x" + 
                   std::to_string(pitch);
        } else {
            return std::to_string(nominalDiameter) + "-" + 
                   std::to_string(static_cast<int>(24.0f / pitch)) + " UNC";
        }
    }
};

// ============================================================================
// BOLT SPECIFICATION
// ============================================================================

struct BoltSpec {
    FastenerType type = FastenerType::HexBolt;
    ThreadSpec thread;
    BoltGrade grade = BoltGrade::Grade8_8;
    
    // Dimensions
    float length = 30.0f;             // Total length (mm)
    float headDiameter = 0.0f;        // Auto-calculated from thread
    float headHeight = 0.0f;          // Auto-calculated from thread
    float wrenchSize = 0.0f;          // Auto-calculated from thread
    
    // Material
    std::string material = "Steel";
    std::string finish = "Zinc Plated"; // Plain, Zinc, HDG, Stainless, etc.
    
    // Installation
    float torque = 0.0f;              // Required torque (Nm)
    float preload = 0.0f;             // Clamp load (N)
    
    // Calculate standard dimensions from thread
    void CalculateFromThread() {
        if (thread.standard == ThreadStandard::Metric) {
            float d = thread.nominalDiameter;
            
            // Hex bolt head dimensions (DIN 931/933)
            headDiameter = d * 1.5f;
            headHeight = d * 0.65f;
            wrenchSize = d * 0.9f;  // ~17mm for M10
            
            // Estimate torque from clamp load
            // T = 0.2 * d * Fc (rough approximation)
            preload = 0.75f * (thread.nominalDiameter * thread.nominalDiameter) * 100.0f;
            torque = 0.2f * d * preload / 1000.0f; // Nm
        }
    }
    
    std::string GetDisplayName() const {
        std::string head;
        switch (type) {
            case FastenerType::HexBolt: head = "Hex Bolt"; break;
            case FastenerType::SocketHeadCapScrew: head = "SHCS"; break;
            case FastenerType::MachineScrew: head = "Machine Screw"; break;
            case FastenerType::Rivet: head = "Rivet"; break;
            case FastenerType::Stud: head = "Stud"; break;
            case FastenerType::AnchorBolt: head = "Anchor Bolt"; break;
        }
        return head + " " + thread.ToString() + "x" + std::to_string(static_cast<int>(length)) + "mm";
    }
};

// ============================================================================
// BOLT PATTERN - Arrays of bolts
// ============================================================================

enum class PatternType {
    Single,
    Grid2D,          // Rectangular array
    Grid3D,          // 3D grid
    Circular,         // Bolts in a circle
    CircularStaggered, // Two concentric circles
    BoltCircle,       // Multiple bolts on circle (common in flanges)
    AlongLine,        // Evenly spaced along a line
    AlongEdge,        // Follow an edge with even spacing
    Custom             // User-defined positions
};

struct BoltPattern {
    PatternType type = PatternType::Single;
    
    // Grid pattern parameters
    int rows = 1;
    int columns = 1;
    float spacingX = 25.0f;   // mm
    float spacingY = 25.0f;   // mm
    float patternAngle = 0.0f; // Rotation of entire pattern
    
    // Circular pattern parameters
    int boltCount = 6;        // Number of bolts in circle
    float circleRadius = 50.0f; // mm
    float startAngle = 0.0f;  // Degrees
    float endAngle = 360.0f; // Degrees (full circle = 360)
    
    // Line pattern parameters
    Geometry::Vector3 lineStart;
    Geometry::Vector3 lineEnd;
    int count = 2;
    bool evenSpacing = true;
    
    // Custom positions
    std::vector<Geometry::Vector3> customPositions;
    
    // Get all positions from pattern
    std::vector<Geometry::Vector3> GetPositions() const {
        std::vector<Geometry::Vector3> positions;
        
        switch (type) {
            case PatternType::Single:
                positions.push_back(Geometry::Vector3(0, 0, 0));
                break;
                
            case PatternType::Grid2D:
                for (int r = 0; r < rows; ++r) {
                    for (int c = 0; c < columns; ++c) {
                        positions.push_back(Geometry::Vector3(
                            c * spacingX - (columns - 1) * spacingX / 2.0f,
                            0,
                            r * spacingY - (rows - 1) * spacingY / 2.0f
                        ));
                    }
                }
                break;
                
            case PatternType::Circular:
            case PatternType::BoltCircle: {
                float angleStep = (endAngle - startAngle) / boltCount;
                for (int i = 0; i < boltCount; ++i) {
                    float a = (startAngle + i * angleStep) * 3.14159265358979 / 180.0;
                    positions.push_back(Geometry::Vector3(
                        std::cos(a) * circleRadius,
                        0,
                        std::sin(a) * circleRadius
                    ));
                }
                break;
            }
            
            case PatternType::AlongLine:
            case PatternType::AlongEdge: {
                Geometry::Vector3 dir = lineEnd - lineStart;
                float totalLength = dir.Length();
                if (totalLength > 0.001f) {
                    dir.Normalize();
                    float step = totalLength / (count - 1);
                    for (int i = 0; i < count; ++i) {
                        positions.push_back(lineStart + dir * (i * step));
                    }
                }
                break;
            }
            
            case PatternType::Custom:
                positions = customPositions;
                break;
                
            default:
                positions.push_back(Geometry::Vector3(0, 0, 0));
                break;
        }
        
        // Apply pattern rotation
        if (std::abs(patternAngle) > 0.001f) {
            float ca = std::cos(patternAngle * 3.14159265358979 / 180.0);
            float sa = std::sin(patternAngle * 3.14159265358979 / 180.0);
            for (auto& pos : positions) {
                float nx = pos.x * ca - pos.z * sa;
                float nz = pos.x * sa + pos.z * ca;
                pos.x = nx;
                pos.z = nz;
            }
        }
        
        return positions;
    }
};

// ============================================================================
// INDIVIDUAL BOLT INSTANCE
// ============================================================================

struct BoltInstance {
    UUID id;
    uint32_t ownerInstanceId;    // Which assembly instance this is attached to
    
    SurfaceReference surfaceRef;  // Where on the surface
    BoltSpec spec;
    
    // Visual properties
    bool isVisible = true;
    bool isSelected = false;
    
    // State
    bool isInstalled = false;     // Has bolt been "tightened"
    float actualTorque = 0.0f;    // Actual torque applied
    
    // Connected parts (for load distribution)
    std::vector<uint32_t> clampedInstanceIds; // Parts being clamped together
    
    Geometry::Vector3 GetWorldPosition() const {
        return surfaceRef.worldPosition + surfaceRef.surfaceNormal * surfaceRef.offset;
    }
};

// ============================================================================
// HINGE SPECIFICATION
// ============================================================================

struct HingeSpec {
    HingeType type = HingeType::Standard;
    
    // Dimensions
    float length = 100.0f;        // Overall hinge length (mm)
    float width = 40.0f;          // Overall hinge width (mm)
    float thickness = 3.0f;       // Material thickness (mm)
    float pinDiameter = 6.0f;     // Hinge pin diameter (mm)
    
    // Material
    std::string material = "Steel";
    bool isNylonBearing = false;  // Has nylon bearing washers
    
    // Motion limits
    float minAngle = -180.0f;     // Degrees
    float maxAngle = 180.0f;      // Degrees
    bool hasStops = true;         // Has physical stops
    
    // Detent settings (if detent hinge)
    float detentTorque = 0.0f;    // Nm to overcome detent
    std::vector<float> detentPositions; // Click positions in degrees
    
    std::string GetDisplayName() const {
        std::string typeStr;
        switch (type) {
            case HingeType::Standard: typeStr = "Standard"; break;
            case HingeType::Piano: typeStr = "Piano"; break;
            case HingeType::Butt: typeStr = "Butt"; break;
            case HingeType::Barrel: typeStr = "Barrel"; break;
            case HingeType::Spring: typeStr = "Spring"; break;
            case HingeType::Detent: typeStr = "Detent"; break;
        }
        return typeStr + " Hinge " + 
               std::to_string(static_cast<int>(length)) + "x" +
               std::to_string(static_cast<int>(width)) + "mm";
    }
};

// ============================================================================
// INDIVIDUAL HINGE INSTANCE
// ============================================================================

struct HingeInstance {
    UUID id;
    uint32_t instanceA;           // First assembly instance
    uint32_t instanceB;           // Second assembly instance (moving part)
    
    // Pivot point in world space
    Geometry::Vector3 pivotPoint;
    
    // Hinge axis (normalized, direction the hinge rotates around)
    Geometry::Vector3 axis;
    
    // Offset from part surfaces
    float offsetA = 0.0f;         // Distance from part A surface
    float offsetB = 0.0f;         // Distance from part B surface
    
    HingeSpec spec;
    
    // State
    float currentAngle = 0.0f;   // Current angle in degrees
    bool isConstrained = false;   // Has angle constraints applied
    
    // Visual properties
    bool isVisible = true;
    bool isSelected = false;
};

// ============================================================================
// WELD SPECIFICATION
// ============================================================================

struct WeldSpec {
    WeldType type = WeldType::Fillet;
    WeldProcess process = WeldProcess::GMAW;
    
    // Dimensions (in mm)
    float size = 5.0f;            // Leg size for fillet welds
    float throat = 3.5f;          // Effective throat thickness
    
    // Length (auto-calculated or manual override)
    float length = 0.0f;          // 0 = auto from geometry
    
    // Perimeter weld settings
    bool weldAllAround = false;   // Full perimeter
    float startGap = 0.0f;        // Gap at start
    float endGap = 0.0f;          // Gap at end
    
    // Spacing (for plug/slot welds)
    float spacing = 0.0f;         // Center-to-center spacing
    
    // Material
    std::string fillerMaterial = "ER70S-6"; // Common MIG wire
    std::string shieldingGas = "75% Ar / 25% CO2";
    
    // Process parameters
    float amperage = 200.0f;      // Typical for GMAW
    float voltage = 22.0f;
    float wireFeedSpeed = 300.0f; // ipm or mm/s
    
    // Strength calculation
    float legForce = 0.0f;        // Force per mm of weld (calculated)
    
    void CalculateStrength() {
        // Simplified weld strength: F = 0.707 * size * length * 490 MPa (steel)
        legForce = 0.707f * size * 490.0f; // N per mm of weld
    }
    
    std::string GetDisplayName() const {
        std::string typeStr;
        switch (type) {
            case WeldType::Fillet: typeStr = "Fillet"; break;
            case WeldType::Butt: typeStr = "Butt"; break;
            case WeldType::Plug: typeStr = "Plug"; break;
            case WeldType::Slot: typeStr = "Slot"; break;
            case WeldType::Spot: typeStr = "Spot"; break;
            case WeldType::Seam: typeStr = "Seam"; break;
            case WeldType::PlugAndSlot: typeStr = "Plug/Slot"; break;
            case WeldType::FilletAllAround: typeStr = "Fillet All-Around"; break;
        }
        std::string procStr;
        switch (process) {
            case WeldProcess::SMAW: procStr = "SMAW"; break;
            case WeldProcess::GMAW: procStr = "MIG"; break;
            case WeldProcess::FCAW: procStr = "FCAW"; break;
            case WeldProcess::GTAW: procStr = "TIG"; break;
            case WeldProcess::SAW: procStr = "SAW"; break;
            case WeldProcess::EBW: procStr = "EB"; break;
            case WeldProcess::LBW: procStr = "LB"; break;
        }
        return procStr + " " + typeStr + " " + 
               std::to_string(static_cast<int>(size)) + "mm";
    }
};

// ============================================================================
// WELD BEAD DEFINITION
// ============================================================================

struct WeldBead {
    Geometry::Vector3 start;
    Geometry::Vector3 end;
    Geometry::Vector3 direction;   // Tangent direction
    float length = 0.0f;
    
    // Cross-section properties
    float legSize = 5.0f;
    float throatThickness = 3.5f;
    
    // Position relative to workpieces
    int workpieceIndexA = -1;     // Which part is "base"
    int workpieceIndexB = -1;     // Which part is "attached"
};

// ============================================================================
// INDIVIDUAL WELD INSTANCE
// ============================================================================

struct WeldInstance {
    UUID id;
    
    // Connected parts (typically 2, can be more for complex joints)
    std::vector<uint32_t> instanceIds;
    
    // Weld geometry
    std::vector<WeldBead> beads;  // Individual weld passes/beads
    
    // Edge/face reference (for weld-all-around)
    uint32_t edgeInstanceId = 0U;  // If following an edge
    bool followsEdge = false;
    
    WeldSpec spec;
    
    // Calculated values
    float totalLength = 0.0f;
    float totalStrength = 0.0f;    // N
    
    // State
    bool isComplete = true;        // Weld quality (assumed complete for sim)
    float qualityFactor = 1.0f;    // 0-1 weld quality
    
    // Visual properties
    bool isVisible = true;
    bool isSelected = false;
    
    void CalculateTotalLength() {
        totalLength = 0.0f;
        for (const auto& bead : beads) {
            totalLength += bead.length;
        }
    }
    
    void CalculateTotalStrength() {
        CalculateTotalLength();
        spec.CalculateStrength();
        totalStrength = totalLength * spec.legForce * qualityFactor;
    }
};

// ============================================================================
// ATTACHMENT MANAGER DATA CONTAINER
// ============================================================================

struct AttachmentSystemData {
    // All bolts in the assembly
    std::vector<BoltInstance> bolts;
    
    // All hinges
    std::vector<HingeInstance> hinges;
    
    // All welds
    std::vector<WeldInstance> welds;
    
    // Currently selected attachment
    UUID selectedAttachmentId;
    enum class AttachmentKind { None, Bolt, Hinge, Weld } selectedKind;
    
    // Placement mode state
    bool isPlacingBolt = false;
    bool isPlacingHinge = false;
    bool isPlacingWeld = false;
    
    // Placement preview
    SurfaceReference previewSurface;
    Geometry::Vector3 previewPosition;
    bool showPreview = false;
    
    // Snap settings
    bool snapToGrid = true;
    float gridSize = 5.0f;         // mm
    bool snapToEdge = true;
    bool snapToVertex = true;
    bool snapToCenter = true;
    float snapRadius = 10.0f;       // mm - how close to snap
    
    // Default specifications (for quick placement)
    BoltSpec defaultBoltSpec;
    HingeSpec defaultHingeSpec;
    WeldSpec defaultWeldSpec;
};

} // namespace SZM::Assembly
