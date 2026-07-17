#ifndef BOUNDARY_CONDITIONS_HPP
#define BOUNDARY_CONDITIONS_HPP

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace SZM::Fluids {

enum class BoundaryType {
    Inlet,           // Fluid enters
    Outlet,          // Fluid exits
    Wall,            // No-slip boundary
    Symmetry,        // Mirror boundary
    Pressure,        // Constant pressure
    Velocity,        // Constant velocity
    FreeSlip,        // Slip boundary
    Periodic         // Repeating boundary
};

enum class GeometryType {
    Rectangle,       // 2D rectangle
    Circle,          // 2D circle
    Sphere,          // 3D sphere
    Cylinder,        // 3D cylinder
    Box,             // 3D rectangular box
    Cone,            // 3D cone
    Custom           // User-defined shape
};

struct BoundarySegment {
    BoundaryType type;
    glm::vec3 position;              // Center position
    glm::vec3 normal;                // Normal direction
    float area;                      // Area/length
    float value;                     // Pressure or velocity magnitude
    glm::vec3 direction;             // Flow direction (for velocity boundaries)
    bool isActive;
};

struct ContainerGeometry {
    GeometryType type;
    glm::vec3 position;              // Center/reference point
    glm::vec3 dimensions;            // Length, width, height
    float radius;                    // For circular shapes
    float height;                    // For cylindrical shapes
    std::string name;
};

struct FluidDomain {
    glm::vec3 minBound;              // Lower corner
    glm::vec3 maxBound;              // Upper corner
    float gridResolution;            // Grid cell size
    int xCells, yCells, zCells;      // Grid dimensions
    
    // Calculated properties
    glm::vec3 getCenter() const {
        return (minBound + maxBound) * 0.5f;
    }
    
    glm::vec3 getSize() const {
        return maxBound - minBound;
    }
    
    float getVolume() const {
        glm::vec3 size = getSize();
        return size.x * size.y * size.z;
    }
};

class BoundaryConditionManager {
public:
    BoundaryConditionManager();
    explicit BoundaryConditionManager(const ContainerGeometry& geometry);
    
    // Geometry management
    void setContainerGeometry(const ContainerGeometry& geometry);
    const ContainerGeometry& getGeometry() const { return containerGeometry; }
    
    void setFluidDomain(const FluidDomain& domain);
    const FluidDomain& getFluidDomain() const { return domain; }
    
    // Boundary segment management
    void addBoundarySegment(const BoundarySegment& segment);
    void removeBoundarySegment(size_t index);
    size_t getNumBoundarySegments() const { return boundarySegments.size(); }
    const BoundarySegment& getBoundarySegment(size_t index) const;
    BoundarySegment& getBoundarySegment(size_t index);
    
    // Predefined boundary configurations
    void createRectangularContainer(
        float length, float width, float height,
        float inletVelocity, float outletPressure
    );
    
    void createCircularPipeflow(
        float diameter, float length,
        float inletVelocity, float inletPressure
    );
    
    void createTankWithInletOutlet(
        float tankLength, float tankWidth, float tankHeight,
        float inletArea, float outletArea,
        glm::vec3 inletPosition, glm::vec3 outletPosition
    );
    
    void createChannelFlow(
        float channelLength, float channelWidth, float channelHeight,
        float flowVelocity
    );
    
    // Boundary analysis
    float getTotalInletArea() const;
    float getTotalOutletArea() const;
    float getTotalWallArea() const;
    
    glm::vec3 getTotalInletVelocity() const;
    float getAverageInletPressure() const;
    float getAverageOutletPressure() const;
    
    // Check if point is in domain
    bool isPointInDomain(const glm::vec3& point) const;
    bool isPointNearBoundary(const glm::vec3& point, float tolerance) const;
    
    // Get nearest boundary
    int getNearestBoundarySegment(const glm::vec3& point) const;
    float getDistanceToBoundary(const glm::vec3& point) const;
    
    // Vessel properties
    float getContainerVolume() const;
    float getContainerSurfaceArea() const;
    
    // Flow analysis
    float calculateMassFlowRateIn(float fluidDensity) const;
    float calculateMassFlowRateOut(float fluidDensity) const;
    bool isFlowBalanced(float fluidDensity, float tolerance = 0.01f) const;
    
    // Reset boundaries
    void clearAllBoundaries();
    void resetToDefault();
    
    // Get all boundaries by type
    std::vector<size_t> getBoundariesOfType(BoundaryType type) const;
    
    // Visualization helpers
    struct BoundaryVisualization {
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> colors;
        std::vector<unsigned int> indices;
    };
    
    BoundaryVisualization getVisualizationMesh() const;
    
    // Export/Import
    bool exportToFile(const std::string& filename) const;
    bool importFromFile(const std::string& filename);

private:
    ContainerGeometry containerGeometry;
    FluidDomain domain;
    std::vector<BoundarySegment> boundarySegments;
    
    // Helper methods
    void initializeRectangularDomain();
    void initializeCircularDomain();
    void createWallSegments();
    void updateGridResolution();
};

// Helper struct for common vessel types
struct VesselPreset {
    std::string name;
    ContainerGeometry geometry;
    std::vector<BoundarySegment> boundaries;
    float recommendedGridSize;
};

class VesselLibrary {
public:
    static VesselPreset getSwimmingPool();
    static VesselPreset getInfinityPool();
    static VesselPreset getLabBeaker();
    static VesselPreset getOilTank();
    static VesselPreset getPipeline();
    static VesselPreset getReactorVessel();
    static VesselPreset getChannelFlow();
};

} // namespace SZM::Fluids

#endif // BOUNDARY_CONDITIONS_HPP
