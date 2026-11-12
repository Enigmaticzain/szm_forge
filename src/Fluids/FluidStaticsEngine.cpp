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
    float len = glm::length(normalDirection);
    if (len <= 1e-6f) {
        return glm::vec3(0.0f);
    }
    float forceMagnitude = getHydrostaticForceOnSubmergedSurface(area, centroidDepth);
    return (normalDirection / len) * forceMagnitude;
}

float FluidStaticsEngine::getCenterOfPressure(float centroidDepth, float surfaceArea) const {
    // Center of pressure is deeper than centroid for submerged surfaces
    // Guard against zero area or non-positive depth
    if (surfaceArea <= 0.0f || centroidDepth <= 1e-6f) return centroidDepth;
    
    float secondMoment = (surfaceArea * centroidDepth * centroidDepth) / 12.0f;
    return centroidDepth + (secondMoment / (centroidDepth * surfaceArea));
}

FloatingObjectResult FluidStaticsEngine::analyzeFloatingObject(
    float objectVolume,
    float objectMass,
    const glm::vec3& centerOfGravity) const {
    
    FloatingObjectResult result;
    if (objectVolume <= 0.0f || fluid.getDensity() <= 0.0f) {
        return result;
    }
    
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
    
    // For submerged/floating bodies, positive stability occurs when
    // center of buoyancy is above the center of gravity (CB.y > CG.y)
    float verticalDistance = centerOfBuoyancy.y - centerOfGravity.y;
    
    // Normalize to -1 (unstable) to +1 (very stable)
    float maxDistance = 10.0f;  // Reference distance
    return glm::clamp(verticalDistance / maxDistance, -1.0f, 1.0f);
}

float FluidStaticsEngine::getMetacentricHeight(
    const glm::vec3& centerOfGravity,
    const glm::vec3& centerOfBuoyancy,
    float secondMomentOfArea,
    float displacedVolume) const {
    
    // BM = I / V (metacentric radius)
    // GM = BM - (z_G - z_B) (metacentric height; vertical separation only)
    if (secondMomentOfArea <= 0.0f || displacedVolume <= 1e-6f) return 0.0f;

    float BM = secondMomentOfArea / displacedVolume;

    // Vertical coordinate separation (z_G - z_B), not full 3D Euclidean distance
    float verticalDistance = centerOfGravity.y - centerOfBuoyancy.y;
    float GM = BM - verticalDistance;

    return GM;
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
    if (numPoints <= 0) {
        return {};
    }
    if (numPoints == 1) {
        return {getPressureAtDepth(0.0f)};
    }
    
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
