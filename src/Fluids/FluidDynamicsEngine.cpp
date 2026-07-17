#include "FluidDynamicsEngine.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Fluids {

FluidDynamicsEngine::FluidDynamicsEngine()
    : fluid(FluidProperties::Water()) {
}

FluidDynamicsEngine::FluidDynamicsEngine(const FluidProperties& fluidProps)
    : fluid(fluidProps) {
}

void FluidDynamicsEngine::setFluid(const FluidProperties& fluidProps) {
    fluid = fluidProps;
}

float FluidDynamicsEngine::getDragCoefficient(DragModel model, float reynoldsNumber) const {
    // Empirical drag coefficients for common shapes
    switch (model) {
        case DragModel::Sphere: {
            // Cd for sphere varies with Reynolds number
            if (reynoldsNumber < 1.0f) {
                return 24.0f / reynoldsNumber;  // Stokes flow
            } else if (reynoldsNumber < 1000.0f) {
                // Intermediate range
                return 24.0f / reynoldsNumber + 4.0f / std::sqrt(reynoldsNumber) + 0.4f;
            } else {
                return 0.47f;  // Turbulent range
            }
        }
        case DragModel::Cylinder:
            // For cylinders perpendicular to flow
            if (reynoldsNumber < 100.0f) return 1.2f;
            if (reynoldsNumber < 1e5) return 1.0f;
            return 0.3f;  // Turbulent
            
        case DragModel::Flat:
            return 1.28f;  // Flat plate perpendicular to flow
            
        case DragModel::Custom:
        default:
            return 0.5f;  // Default value
    }
}

float FluidDynamicsEngine::getDynamicPressure(float velocity) const {
    // q = 0.5 * ρ * v²
    return 0.5f * fluid.getDensity() * velocity * velocity;
}

DragForceResult FluidDynamicsEngine::calculateDragForce(
    float velocity,
    float referenceArea,
    DragModel model,
    float customDragCoefficient) const {
    
    DragForceResult result;
    
    float re = getReynoldsNumber(velocity, std::sqrt(referenceArea));
    result.reynoldsNumber = re;
    
    if (model == DragModel::Custom) {
        result.dragCoefficient = customDragCoefficient;
    } else {
        result.dragCoefficient = getDragCoefficient(model, re);
    }
    
    result.dynamicPressure = getDynamicPressure(velocity);
    result.dragForce = result.dragCoefficient * result.dynamicPressure * referenceArea;
    result.dragDirection = glm::vec3(-1.0f, 0.0f, 0.0f);  // Opposite to flow
    
    return result;
}

float FluidDynamicsEngine::getReynoldsNumber(float velocity, float characteristicLength) const {
    // Re = ρ * v * L / μ
    if (fluid.getDynamicViscosity() <= 0.0f) return 0.0f;
    return (fluid.getDensity() * velocity * characteristicLength) / fluid.getDynamicViscosity();
}

LiftForceResult FluidDynamicsEngine::calculateLiftForce(
    float velocity,
    float wingArea,
    float liftCoefficient,
    const glm::vec3& flowDirection) const {
    
    LiftForceResult result;
    result.liftCoefficient = liftCoefficient;
    
    float q = getDynamicPressure(velocity);
    result.liftForce = liftCoefficient * q * wingArea;
    
    // Lift is perpendicular to flow direction
    glm::vec3 normalized = glm::normalize(flowDirection);
    result.liftDirection = glm::vec3(-normalized.z, 0.0f, normalized.x);
    
    return result;
}

float FluidDynamicsEngine::getViscousResistance(
    float pipeLength, float pipeDiameter, float velocity) const {
    
    // Pressure loss: Δp = f * (L/D) * (ρ * v² / 2)
    float re = getReynoldsNumber(velocity, pipeDiameter);
    float f = getPipeFrictionFactor(re);
    
    return f * (pipeLength / pipeDiameter) * getDynamicPressure(velocity);
}

float FluidDynamicsEngine::getPipeFrictionFactor(float reynoldsNumber, float pipeRoughness) const {
    if (reynoldsNumber <= 0.0f) return 0.0f;
    
    // For laminar flow
    if (reynoldsNumber < 2300.0f) {
        return 64.0f / reynoldsNumber;
    }
    
    // For turbulent flow - using Swamee-Jain approximation
    return swameeJainEquation(reynoldsNumber, pipeRoughness);
}

float FluidDynamicsEngine::swameeJainEquation(float re, float relativeRoughness) const {
    // Swamee-Jain equation (explicit approximation of Colebrook-White)
    // f = 0.25 / [log10(ε/(3.7*D) + 5.74/Re^0.9)]²
    
    if (re <= 0.0f) return 0.0f;
    
    float denominator = std::log10(relativeRoughness / 3.7f + 5.74f / std::pow(re, 0.9f));
    float f = 0.25f / (denominator * denominator);
    
    return glm::clamp(f, 0.01f, 0.1f);
}

BernoulliResult FluidDynamicsEngine::applyBernoulli(
    float h1, float v1, float p1,
    float h2, float p2) const {
    
    BernoulliResult result;
    
    float g = fluid.getGravity();
    float rho = fluid.getDensity();
    
    // Bernoulli equation: h + v²/(2g) + P/(ρg) = constant
    float head1 = h1 + (v1 * v1) / (2.0f * g) + p1 / (rho * g);
    
    // Total head is constant
    result.totalHead = head1;
    result.elevationHead = h1;
    result.velocityHead = (v1 * v1) / (2.0f * g);
    result.pressureHead = p1 / (rho * g);
    
    // Calculate velocity at point 2
    float head2 = h2 + p2 / (rho * g);
    float velocityHead2 = result.totalHead - head2;
    result.velocityAtPoint2 = std::sqrt(2.0f * g * glm::max(velocityHead2, 0.0f));
    
    return result;
}

FlowRateResult FluidDynamicsEngine::calculateFlowRate(float area, float velocity) const {
    FlowRateResult result;
    result.volumeFlowRate = area * velocity;
    result.massFlowRate = fluid.getDensity() * result.volumeFlowRate;
    result.continuityVelocity = velocity;
    return result;
}

FlowRateResult FluidDynamicsEngine::applyContinuity(
    float area1, float velocity1, float area2) const {
    
    // Continuity equation: A1 * v1 = A2 * v2
    FlowRateResult result;
    
    if (area2 <= 0.0f) {
        result.volumeFlowRate = area1 * velocity1;
        result.continuityVelocity = velocity1;
    } else {
        result.volumeFlowRate = area1 * velocity1;
        result.continuityVelocity = (area1 * velocity1) / area2;
    }
    
    result.massFlowRate = fluid.getDensity() * result.volumeFlowRate;
    return result;
}

float FluidDynamicsEngine::getOrificeVelocity(float pressureDifference) const {
    // From Bernoulli: v = sqrt(2 * Δp / ρ)
    if (fluid.getDensity() <= 0.0f) return 0.0f;
    return std::sqrt(2.0f * pressureDifference / fluid.getDensity());
}

float FluidDynamicsEngine::getOrificeFlowRate(
    float orificeArea, float pressureDifference) const {
    
    float velocity = getOrificeVelocity(pressureDifference);
    // Include discharge coefficient (typically 0.6-0.8)
    float dischargeCoeff = 0.65f;
    return dischargeCoeff * orificeArea * velocity;
}

float FluidDynamicsEngine::getPressureLoss(
    float flowVelocity, float pipeDiameter, float pipeLength, float pipeRoughness) const {
    
    return getViscousResistance(pipeLength, pipeDiameter, flowVelocity);
}

float FluidDynamicsEngine::getPumpPower(float flowRate, float pressureDifference) const {
    // Hydraulic power: P = Q * Δp
    return flowRate * pressureDifference;
}

float FluidDynamicsEngine::getShaftPower(float flowRate, float headDifference) const {
    // P = ρ * g * Q * h
    return fluid.getDensity() * fluid.getGravity() * flowRate * headDifference;
}

float FluidDynamicsEngine::getTurbulenceIntensity(
    float velocity, float characteristicLength) const {
    
    float re = getReynoldsNumber(velocity, characteristicLength);
    
    // Turbulence intensity typically increases with Reynolds number
    if (isFlowLaminar(re)) return 0.01f;
    return 0.05f + 0.01f * std::log10(glm::max(re, 1.0f) / 4000.0f);
}

bool FluidDynamicsEngine::isFlowTurbulent(float reynoldsNumber) const {
    return reynoldsNumber > 4000.0f;
}

bool FluidDynamicsEngine::isFlowLaminar(float reynoldsNumber) const {
    return reynoldsNumber < 2300.0f;
}

std::vector<glm::vec3> FluidDynamicsEngine::getVelocityField(
    const glm::vec3& flowDirection, float flowVelocity, int gridResolution) const {
    
    std::vector<glm::vec3> field;
    glm::vec3 normalized = glm::normalize(flowDirection) * flowVelocity;
    
    for (int i = 0; i < gridResolution * gridResolution * gridResolution; ++i) {
        field.push_back(normalized);
    }
    
    return field;
}

glm::vec3 FluidDynamicsEngine::getWakeVelocity(
    const glm::vec3& objectPosition,
    const glm::vec3& flowDirection,
    float flowVelocity,
    float downstreamDistance) const {
    
    // Wake velocity reduces downstream (simplified model)
    float wake_reduction = 1.0f / (1.0f + downstreamDistance);
    return glm::normalize(flowDirection) * flowVelocity * wake_reduction;
}

} // namespace SZM::Fluids
