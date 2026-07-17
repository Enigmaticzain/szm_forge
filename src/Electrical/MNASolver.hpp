#pragma once

#include <vector>
#include <string>
#include <memory>
#include <Eigen/Dense>

namespace SZM {
namespace Electrical {

/**
 * @brief Open-source inspired Modified Nodal Analysis (MNA) Simulator.
 * MNA is the standard algorithm used in SPICE and other open-source electronic design automation (EDA) tools
 * to solve linear circuits consisting of resistors, current sources, and voltage sources.
 * 
 * Formulates the system Ax = z, where:
 * A is the conductance matrix + voltage source stamps.
 * x is the vector of unknown node voltages and voltage source currents.
 * z is the vector of known independent current and voltage sources.
 */

struct Component {
    enum class Type { Resistor, VoltageSource, CurrentSource };
    std::string name;
    Type type;
    int node1; // positive node (0 = ground)
    int node2; // negative node (0 = ground)
    double value; // Resistance (Ohms), Voltage (Volts), or Current (Amps)
};

class MNASolver {
public:
    MNASolver();
    ~MNASolver() = default;

    // Build circuit
    void AddComponent(const std::string& name, Component::Type type, int n1, int n2, double value);
    void Clear();

    // Run MNA formulation and solve
    bool Solve();

    // Get results
    double GetNodeVoltage(int node) const;
    double GetVoltageSourceCurrent(const std::string& vsrcName) const;

private:
    std::vector<Component> m_Components;
    int m_NumNodes;
    int m_NumVoltageSources;
    
    // MNA System: A * x = z
    Eigen::MatrixXd m_A;
    Eigen::VectorXd m_z;
    Eigen::VectorXd m_x; // The solution vector

    void DetermineSystemSize();
    void StampMatrix();
};

} // namespace Electrical
} // namespace SZM
