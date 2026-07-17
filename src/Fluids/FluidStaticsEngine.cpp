#include "FluidStaticsEngine.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Fluids {

FluidStaticsEngine::FluidStaticsEngine()
    : fluid(FluidProperties::Water()) {
}

FluidStaticsEngine::FluidStaticsEngine(const FluidProperties& fluid)
    : fluid(fluid) {
}

void FluidStaticsEngine::setFluid(const FluidProperties& fluidProps) {
    fluid = fluidProps;
}

float FluidStaticsEngine::getPressureAtDepth(float depth) const {
    // P = ρ * g * h (gauge pressure)
    return fluid.getDensity() * fluid.getGravity() * depth;
}

float FluidStaticsEngine::getAbsolutePressure(float depth) const {
    // P_abs = P_atm + ρ * g * h
    return fluid.getPressure() + getPressureAtDepth(depth);
}

float FluidStaticsEngine::getGaugePressure(float depth) const {
    // Gauge pressure is relative to atmospheric pressure
    return getPressureAtDepth(depth);
}

float FluidStaticsEngine::getBuoyantForce(float objectVolume) const {
    // F_b = ρ * g * V (Archimedes' Principle)
    return fluid.getDensity() * fluid.getGravity() * objectVolume;
}

float FluidStaticsEngine::getArchimedesPrinciple(float displacedVolume) const {
    // Same as buoyant force - weight of displaced fluid
    return getBuoyantForce(displacedVolume);
}

float FluidStaticsEngine::getHydrostaticForceOnSubmergedSurface(
    float area, float centroidDepth) const {
    // F = P_c * A where P_c is pressure at centroid
    return getPressureAtDepth(centroidDepth) * area;
}

glm::vec3 FluidStaticsEngine::getHydrostaticForceVector(
    float area, float centroidDepth, const glm::vec3& normalDirection) const {
    float forceMagnitude = getHydrostaticForceOnSubmergedSurface(area, centroidDepth);
    glm::vec3 normalized = glm::normalize(normalDirection);
    return normalized * forceMagnitude;
}

float FluidStaticsEngine::getCenterOfPressure(float centroidDepth, float surfaceArea) const {
    // Center of pressure is always deeper than centroid for submerged surfaces
    // Approximation: CoP depth ≈ Centroid depth + (Centroid depth / 2)
    // More accurate calculation depends on surface shape
    if (surfaceArea <= 0.0f) return centroidDepth;
    
    float secondMoment = (surfaceArea * centroidDepth * centroidDepth) / 12.0f;
    return centroidDepth + (secondMoment / (centroidDepth * surfaceArea));
}

FloatingObjectResult FluidStaticsEngine::analyzeFloatingObject(
    float objectVolume,
    float objectMass,
    const glm::vec3& centerOfGravity) const {
    
    FloatingObjectResult result;
    
    float objectWeight = objectMass * fluid.getGravity();
    float maxBuoyancy = getBuoyantForce(objectVolume);
    
    result.isFloating = maxBuoyancy >= objectWeight;
    
    if (result.isFloating) {
        // Calculate submerged volume needed for equilibrium
        result.submergedVolume = objectMass / fluid.getDensity();
        result.submergedFraction = result.submergedVolume / objectVolume;
        result.draft = (result.submergedVolume / objectVolume); // Normalized draft
    } else {
        // Object sinks - entire volume is submerged
        result.submergedVolume = objectVolume;
        result.submergedFraction = 1.0f;
        result.draft = 1.0f;
    }
    
    result.buoyantForce = glm::vec3(0.0f, getBuoyantForce(result.submergedVolume), 0.0f);
    result.freeBoard = objectVolume - result.submergedVolume;
    
    return result;
}

float FluidStaticsEngine::calculateStability(
    const glm::vec3& centerOfGravity,
    const glm::vec3& centerOfBuoyancy) const {
    
    // Stability is higher when center of buoyancy is below center of gravity
    // For floating objects: CB should be below CG for stability
    float verticalDistance = centerOfBuoyancy.y - centerOfGravity.y;
    
    // Normalize to -1 (unstable) to +1 (very stable)
    float maxDistance = 10.0f;  // Reference distance
    return glm::clamp(verticalDistance / maxDistance, -1.0f, 1.0f);
}

float FluidStaticsEngine::getMetacentricHeight(
    const glm::vec3& centerOfGravity,
    const glm::vec3& centerOfBuoyancy,
    float secondMomentOfArea) const {
    
    // BM = I / V (metacentric radius)
    // GM = BM - (CG - CB) (metacentric height)
    if (secondMomentOfArea <= 0.0f) return 0.0f;
    
    float displaceVolume = 1.0f;  // Assume unit volume for this calculation
    float BM = secondMomentOfArea / displaceVolume;
    
    float verticalDistance = glm::distance(centerOfGravity, centerOfBuoyancy);
    float GM = BM - verticalDistance;
    
    return glm::max(GM, 0.0f);
}

float FluidStaticsEngine::getPressureAtBottom(float containerHeight) const {
    return getAbsolutePressure(containerHeight);
}

float FluidStaticsEngine::getTotalForceOnContainerBottom(
    float containerBaseArea, float fluidHeight) const {
    // Total force = pressure at bottom * area
    return getPressureAtBottom(fluidHeight) * containerBaseArea;
}

float FluidStaticsEngine::getTotalForceOnContainerWall(
    float wallArea, float fluidHeight) const {
    // Average pressure on wall = ρ * g * h/2 (varies linearly from 0 to ρgh)
    float averagePressure = getPressureAtDepth(fluidHeight / 2.0f);
    return averagePressure * wallArea;
}

std::vector<float> FluidStaticsEngine::getPressureDistribution(
    float maxDepth, int numPoints) const {
    
    std::vector<float> pressures;
    pressures.reserve(numPoints);
    
    for (int i = 0; i < numPoints; ++i) {
        float depth = (maxDepth * i) / (numPoints - 1.0f);
        pressures.push_back(getPressureAtDepth(depth));
    }
    
    return pressures;
}

float FluidStaticsEngine::getPiezometricHead(float pressure) const {
    // h = P / (ρ * g)
    float denominator = fluid.getDensity() * fluid.getGravity();
    if (denominator <= 0.0f) return 0.0f;
    return pressure / denominator;
}

float FluidStaticsEngine::getPressureFromPiezometricHead(float head) const {
    // P = ρ * g * h
    return fluid.getDensity() * fluid.getGravity() * head;
}

} // namespace SZM::Fluids
