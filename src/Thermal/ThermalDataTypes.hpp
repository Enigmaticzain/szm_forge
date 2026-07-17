#pragma once

#include "../Geometry/MathTypes.hpp"
#include <vector>
#include <cstdint>

namespace SZM::Thermal {

/**
 * @enum BoundaryConditionType
 * @brief Types of thermal boundary conditions
 */
enum class BoundaryConditionType {
    FixedTemperature,    ///< Dirichlet: T = constant
    HeatFlux,            ///< Neumann: q = constant
    Convection,          ///< Robin: q = h(T - T_ambient)
    Radiation            ///< Radiation: q = ε·σ·(T⁴ - T_ambient⁴)
};

/**
 * @struct ThermalNode
 * @brief Node in thermal mesh
 */
struct ThermalNode {
    uint32_t id;
    Geometry::Vector3 position;
    float temperature = 293.15f;        ///< Kelvin (20°C default)
    float heatCapacity = 500.0f;        ///< J/(kg·K)
    float density = 7850.0f;            ///< kg/m³
    float thermalConductivity = 50.0f;  ///< W/(m·K)
    float volume = 0.0f;                ///< m³
};

/**
 * @struct ThermalElement
 * @brief Element in thermal mesh (tetrahedron or triangle)
 */
struct ThermalElement {
    uint32_t id;
    std::vector<uint32_t> nodeIds;      ///< 3 (triangle) or 4 (tet) node IDs
    float area = 0.0f;                  ///< m² (for surface elements)
    float thickness = 0.01f;            ///< m (for shell elements)
    float thermalConductivity = 50.0f;  ///< W/(m·K)
};

/**
 * @struct ThermalBoundaryCondition
 * @brief Boundary condition for thermal analysis
 */
struct ThermalBoundaryCondition {
    uint32_t nodeId;
    BoundaryConditionType type;
    float value = 293.15f;              ///< Temperature (K) or heat flux (W/m²)
    float convectionCoeff = 10.0f;      ///< W/(m²·K) for convection
    float ambientTemp = 293.15f;        ///< K (20°C)
    float emissivity = 0.9f;            ///< 0-1 for radiation
};

/**
 * @struct ThermalHeatSource
 * @brief Heat source (e.g., electrical dissipation)
 */
struct ThermalHeatSource {
    uint32_t nodeId;
    float powerDensity = 0.0f;          ///< W/m³
    float power = 0.0f;                 ///< W (total)
};

/**
 * @struct ThermalResults
 * @brief Results from thermal analysis
 */
struct ThermalResults {
    std::vector<float> nodeTemperatures;     ///< K
    std::vector<float> elementHeatFlux;      ///< W/m²
    std::vector<float> elementTemperatureGradient; ///< K/m
    float maxTemperature = 0.0f;             ///< K
    float minTemperature = 0.0f;             ///< K
    float totalHeatFlow = 0.0f;              ///< W
};

// Physical constants
constexpr float STEFAN_BOLTZMANN = 5.67e-8f;  ///< W/(m²·K⁴)
constexpr float ABSOLUTE_ZERO = 0.0f;         ///< K
constexpr float ROOM_TEMP = 293.15f;          ///< K (20°C)
constexpr float MAX_TEMP = 373.15f;           ///< K (100°C)

} // namespace SZM::Thermal
