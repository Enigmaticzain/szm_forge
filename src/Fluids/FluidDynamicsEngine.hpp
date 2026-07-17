#ifndef FLUID_DYNAMICS_ENGINE_HPP
#define FLUID_DYNAMICS_ENGINE_HPP

#include "FluidProperties.hpp"
#include <glm/glm.hpp>
#include <vector>

namespace SZM::Fluids {

enum class DragModel {
    Sphere,
    Cylinder,
    Flat,
    Custom
};

struct DragForceResult {
    float dragCoefficient;       // Cd (dimensionless)
    float dynamicPressure;       // q = 0.5 * ρ * v^2
    float dragForce;             // F_d (N)
    glm::vec3 dragDirection;     // Direction of drag (opposite to velocity)
    float reynoldsNumber;        // Re = ρ * v * L / μ
};

struct LiftForceResult {
    float liftCoefficient;       // Cl
    float liftForce;             // F_l (N)
    glm::vec3 liftDirection;     // Perpendicular to flow direction
};

struct BernoulliResult {
    float totalHead;             // H = h + v²/(2g) + P/(ρg)
    float elevationHead;         // h (elevation)
    float velocityHead;          // v²/(2g)
    float pressureHead;          // P/(ρg)
    float velocityAtPoint2;      // Velocity at another point
};

struct FlowRateResult {
    float volumeFlowRate;        // Q = A * v (m³/s)
    float massFlowRate;          // m = ρ * Q (kg/s)
    float continuityVelocity;    // Velocity from continuity equation
};

class FluidDynamicsEngine {
public:
    FluidDynamicsEngine();
    explicit FluidDynamicsEngine(const FluidProperties& fluid);
    
    void setFluid(const FluidProperties& fluid);
    const FluidProperties& getFluid() const { return fluid; }
    
    // Drag force calculations
    DragForceResult calculateDragForce(
        float velocity,
        float referenceArea,
        DragModel model = DragModel::Sphere,
        float customDragCoefficient = 1.0f
    ) const;
    
    float getDragCoefficient(DragModel model, float reynoldsNumber) const;
    float getDynamicPressure(float velocity) const;
    
    // Lift force calculations
    LiftForceResult calculateLiftForce(
        float velocity,
        float wingArea,
        float liftCoefficient = 1.0f,
        const glm::vec3& flowDirection = glm::vec3(1, 0, 0)
    ) const;
    
    // Reynolds number
    float getReynoldsNumber(float velocity, float characteristicLength) const;
    
    // Viscous resistance in flow
    float getViscousResistance(float pipeLength, float pipeDiameter, float velocity) const;
    float getPipeFrictionFactor(float reynoldsNumber, float pipeRoughness = 0.0f) const;
    
    // Bernoulli equation
    BernoulliResult applyBernoulli(
        float h1, float v1, float p1,
        float h2, float p2
    ) const;
    
    // Flow rate calculations
    FlowRateResult calculateFlowRate(float area, float velocity) const;
    FlowRateResult applyContinuity(float area1, float velocity1, float area2) const;
    
    // Orifice and nozzle flow
    float getOrificeVelocity(float pressureDifference) const;
    float getOrificeFlowRate(float orificeArea, float pressureDifference) const;
    
    // Pipe flow pressure loss
    float getPressureLoss(
        float flowVelocity,
        float pipeDiameter,
        float pipeLength,
        float pipeRoughness = 0.0001f
    ) const;
    
    // Power requirements
    float getPumpPower(float flowRate, float pressureDifference) const;
    float getShaftPower(float flowRate, float headDifference) const;
    
    // Turbulence indicators
    float getTurbulenceIntensity(float velocity, float characteristicLength) const;
    bool isFlowTurbulent(float reynoldsNumber) const;
    bool isFlowLaminar(float reynoldsNumber) const;
    
    // Velocity field
    std::vector<glm::vec3> getVelocityField(
        const glm::vec3& flowDirection,
        float flowVelocity,
        int gridResolution = 10
    ) const;
    
    // Wake and vortex patterns
    glm::vec3 getWakeVelocity(
        const glm::vec3& objectPosition,
        const glm::vec3& flowDirection,
        float flowVelocity,
        float downstreamDistance
    ) const;
    
private:
    FluidProperties fluid;
    
    // Helper functions
    float colebrookWhiteEquation(float re, float relativeRoughness) const;
    float swameeJainEquation(float re, float relativeRoughness) const;
};

} // namespace SZM::Fluids

#endif // FLUID_DYNAMICS_ENGINE_HPP
