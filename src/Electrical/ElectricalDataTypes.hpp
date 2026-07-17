#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <complex>

namespace SZM::Electrical {

/**
 * @enum ComponentType
 * @brief Types of electrical components
 */
enum class ComponentType {
    Resistor,       ///< R (Ohms)
    Capacitor,      ///< C (Farads)
    Inductor,       ///< L (Henries)
    VoltageSource,  ///< V (Volts)
    CurrentSource,  ///< I (Amps)
    Diode,          ///< Non-linear
    Transistor      ///< Non-linear
};

/**
 * @struct ElectricalComponent
 * @brief Electrical component (resistor, capacitor, etc.)
 */
struct ElectricalComponent {
    uint32_t id;
    std::string name;
    ComponentType type;
    float value = 0.0f;             ///< Ohms, Farads, Henries, Volts, Amps
    float tolerance = 0.05f;        ///< 5% default
    float powerRating = 1.0f;       ///< Watts
    float temperature = 293.15f;    ///< Kelvin
    bool isFaulty = false;
};

/**
 * @struct ElectricalNode
 * @brief Node in electrical circuit
 */
struct ElectricalNode {
    uint32_t id;
    std::string name;
    float voltage = 0.0f;           ///< Volts (DC)
    float current = 0.0f;           ///< Amps (DC)
    std::complex<float> impedance;  ///< Ohms (AC)
};

/**
 * @struct ElectricalConnection
 * @brief Connection between two nodes through a component
 */
struct ElectricalConnection {
    uint32_t nodeAId;
    uint32_t nodeBId;
    uint32_t componentId;
};

/**
 * @struct CircuitAnalysisResults
 * @brief Results from circuit analysis
 */
struct CircuitAnalysisResults {
    std::vector<float> nodeVoltages;        ///< Volts
    std::vector<float> componentCurrents;   ///< Amps
    std::vector<float> componentPowers;     ///< Watts
    std::vector<bool> faultyComponents;     ///< Fault flags
    float totalPower = 0.0f;                ///< Watts
    float totalCurrent = 0.0f;              ///< Amps
};

/**
 * @struct ACAnalysisResults
 * @brief Results from AC analysis
 */
struct ACAnalysisResults {
    std::vector<std::complex<float>> nodeVoltages;      ///< Volts (complex)
    std::vector<std::complex<float>> componentCurrents;  ///< Amps (complex)
    std::vector<float> componentImpedances;             ///< Ohms
    std::vector<float> componentPhaseAngles;            ///< Radians
    float frequency = 0.0f;                            ///< Hz
};

// Fault detection thresholds
constexpr float OVERCURRENT_THRESHOLD = 1.5f;   ///< 150% of rated
constexpr float OVERVOLTAGE_THRESHOLD = 1.2f;   ///< 120% of rated
constexpr float OVERTEMP_THRESHOLD = 373.15f;   ///< 100°C

} // namespace SZM::Electrical
