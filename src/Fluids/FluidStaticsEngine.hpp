#ifndef FLUID_STATICS_ENGINE_HPP
#define FLUID_STATICS_ENGINE_HPP

#include "FluidProperties.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace SZM::Fluids {

struct HydrostaticResult {
    float pressure;              // Pressure at depth (Pa)
    float force;                 // Total force (N)
    float buoyantForce;          // Buoyant force (N)
    glm::vec3 centerOfBuoyancy; // Center of buoyancy location
    glm::vec3 centerOfGravity;   // Center of gravity location
    float stability;             // Stability metric (-1 to 1)
};

struct FloatingObjectResult {
    bool isFloating;             // Whether object floats or sinks
    float submergedVolume;       // Volume submerged (m^3)
    float submergedFraction;     // Fraction of volume submerged (0-1)
    glm::vec3 buoyantForce;      // Buoyant force vector (N)
    float draft;                 // Depth of immersion (m)
    float freeBoard;             // Height above water surface (m)
};

class FluidStaticsEngine {
public:
    FluidStaticsEngine();
    explicit FluidStaticsEngine(const FluidProperties& fluid);
    
    void setFluid(const FluidProperties& fluid);
    const FluidProperties& getFluid() const { return fluid; }
    
    // Pressure calculations
    float getPressureAtDepth(float depth) const;
    float getAbsolutePressure(float depth) const;
    float getGaugePressure(float depth) const;
    
    // Buoyancy calculations
    float getBuoyantForce(float objectVolume) const;
    float getArchimedesPrinciple(float displacedVolume) const;
    
    // Hydrostatic force on surfaces
    float getHydrostaticForceOnSubmergedSurface(float area, float centroidDepth) const;
    glm::vec3 getHydrostaticForceVector(float area, float centroidDepth, const glm::vec3& normalDirection) const;
    float getCenterOfPressure(float centroidDepth, float surfaceArea) const;
    
    // Floating object analysis
    FloatingObjectResult analyzeFloatingObject(
        float objectVolume,
        float objectMass,
        const glm::vec3& centerOfGravity
    ) const;
    
    // Stability analysis
    float calculateStability(
        const glm::vec3& centerOfGravity,
        const glm::vec3& centerOfBuoyancy
    ) const;
    
    float getMetacentricHeight(
        const glm::vec3& centerOfGravity,
        const glm::vec3& centerOfBuoyancy,
        float secondMomentOfArea
    ) const;
    
    // Container-level pressure
    float getPressureAtBottom(float containerHeight) const;
    float getTotalForceOnContainerBottom(float containerBaseArea, float fluidHeight) const;
    float getTotalForceOnContainerWall(float wallArea, float fluidHeight) const;
    
    // Pressure distribution
    std::vector<float> getPressureDistribution(float maxDepth, int numPoints) const;
    
    // Piezometric head
    float getPiezometricHead(float pressure) const;
    float getPressureFromPiezometricHead(float head) const;
    
private:
    FluidProperties fluid;
    
    // Helper functions
    float calculateStabilityMetric(const glm::vec3& cg, const glm::vec3& cb) const;
};

} // namespace SZM::Fluids

#endif // FLUID_STATICS_ENGINE_HPP
