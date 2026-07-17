#pragma once

#include "../Geometry/MathTypes.hpp"
#include <string>
#include <cstdint>

namespace SZM {

/**
 * @struct SimulationComponent
 * @brief Minimal component with material properties and simulation state.
 */
struct SimulationComponent {
    uint32_t    id;
    std::string name;
    std::string materialId;
    std::string materialName;
    
    // Geometry
    Geometry::Vector3 position;
    float       area;           // m²
    float       thickness;      // m
    
    // Material properties
    float       density;        // kg/m³
    float       yieldStrength;  // Pa
    float       thermalConductivity; // W/(m·K)
    
    // Simulation state
    float       appliedForce;   // N
    float       stress;         // Pa (calculated)
    float       temperature;    // K
    float       heatInput;      // W

    // Phase 5 integration state (physics coupling)
    Geometry::Vector3 linearVelocity;
    float       massEstimate;   // kg (derived from rigid-body proxy)
    std::string physicsBackend;
    uint32_t    activeContactCount;
    float       maxContactPenetration;
    float       kineticEnergy;
    bool        isGrounded;

    // Danger state
    float       stressRatio;    // stress / yieldStrength [0..1]
    float       tempRatio;      // (temp - 273) / maxTemp [0..1]
    bool        isDangerous;    // true if either ratio > 0.8
    
    SimulationComponent()
        : id(0), name("Component"),
          materialId("MAT-STEEL-STRUCT"), materialName("Structural Steel"),
          position(0.0, 0.0, 0.0),
          area(0.01f), thickness(0.01f),
          density(7850.0f), yieldStrength(250e6f),
          thermalConductivity(50.0f), appliedForce(0.0f), stress(0.0f),
          temperature(293.15f), heatInput(0.0f),
          linearVelocity(0.0, 0.0, 0.0), massEstimate(0.0f),
          physicsBackend("Built-in Proxy"), activeContactCount(0),
          maxContactPenetration(0.0f), kineticEnergy(0.0f), isGrounded(false),
          stressRatio(0.0f),
          tempRatio(0.0f), isDangerous(false) {}
};

} // namespace SZM
