#include "CircuitSolver.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Electrical {

    void CircuitSolver::Init() {
        m_Nodes.clear();
        m_Components.clear();
        m_Connections.clear();
        m_NextNodeId = 1;
        m_NextComponentId = 1;
    }

    void CircuitSolver::Shutdown() {
        m_Nodes.clear();
        m_Components.clear();
        m_Connections.clear();
    }

    uint32_t CircuitSolver::AddNode(const std::string& name) {
        ElectricalNode node;
        node.id = m_NextNodeId++;
        node.name = name;
        m_Nodes.push_back(node);
        return node.id;
    }

    uint32_t CircuitSolver::AddComponent(ComponentType type, float value, const std::string& name) {
        ElectricalComponent comp;
        comp.id = m_NextComponentId++;
        comp.type = type;
        comp.value = value;
        comp.name = name.empty() ? ("Component_" + std::to_string(comp.id)) : name;
        m_Components.push_back(comp);
        return comp.id;
    }

    void CircuitSolver::Connect(uint32_t nodeAId, uint32_t nodeBId, uint32_t componentId) {
        ElectricalConnection conn;
        conn.nodeAId = nodeAId;
        conn.nodeBId = nodeBId;
        conn.componentId = componentId;
        m_Connections.push_back(conn);
    }

    void CircuitSolver::SolveDC() {
        // Simple DC analysis using Ohm's law
        m_DCResults.nodeVoltages.resize(m_Nodes.size(), 0.0f);
        m_DCResults.componentCurrents.resize(m_Components.size(), 0.0f);
        m_DCResults.componentPowers.resize(m_Components.size(), 0.0f);
        m_DCResults.faultyComponents.resize(m_Components.size(), false);

        // For each connection, calculate current using Ohm's law
        for (const auto& conn : m_Connections) {
            // Find nodes
            int idxA = -1, idxB = -1;
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == conn.nodeAId) idxA = i;
                if (m_Nodes[i].id == conn.nodeBId) idxB = i;
            }
            if (idxA < 0 || idxB < 0) continue;

            // Find component
            int idxComp = -1;
            for (size_t i = 0; i < m_Components.size(); ++i) {
                if (m_Components[i].id == conn.componentId) {
                    idxComp = i;
                    break;
                }
            }
            if (idxComp < 0) continue;

            float VA = m_DCResults.nodeVoltages[idxA];
            float VB = m_DCResults.nodeVoltages[idxB];
            float V = VA - VB;

            // Calculate current based on component type
            float I = 0.0f;
            if (m_Components[idxComp].type == ComponentType::Resistor) {
                if (m_Components[idxComp].value > 1e-6f) {
                    I = V / m_Components[idxComp].value;
                }
            } else if (m_Components[idxComp].type == ComponentType::VoltageSource) {
                // Voltage source sets voltage
                m_DCResults.nodeVoltages[idxB] = m_DCResults.nodeVoltages[idxA] - m_Components[idxComp].value;
            }

            m_DCResults.componentCurrents[idxComp] = I;
            m_DCResults.componentPowers[idxComp] = V * I;
        }

        CalculatePower();
        CheckThermalStress();
    }

    void CircuitSolver::SolveAC(float frequency) {
        // AC analysis using impedance
        m_ACResults.frequency = frequency;
        m_ACResults.nodeVoltages.resize(m_Nodes.size(), std::complex<float>(0.0f, 0.0f));
        m_ACResults.componentCurrents.resize(m_Components.size(), std::complex<float>(0.0f, 0.0f));
        m_ACResults.componentImpedances.resize(m_Components.size(), 0.0f);
        m_ACResults.componentPhaseAngles.resize(m_Components.size(), 0.0f);

        float omega = 2.0f * 3.14159265f * frequency;

        // Calculate impedances
        for (size_t i = 0; i < m_Components.size(); ++i) {
            std::complex<float> Z(0.0f, 0.0f);

            if (m_Components[i].type == ComponentType::Resistor) {
                Z = std::complex<float>(m_Components[i].value, 0.0f);
            } else if (m_Components[i].type == ComponentType::Inductor) {
                Z = std::complex<float>(0.0f, omega * m_Components[i].value);
            } else if (m_Components[i].type == ComponentType::Capacitor) {
                Z = std::complex<float>(0.0f, -1.0f / (omega * m_Components[i].value));
            }

            m_ACResults.componentImpedances[i] = std::abs(Z);
            m_ACResults.componentPhaseAngles[i] = std::arg(Z);
        }
    }

    void CircuitSolver::SolveTransient(double timeStep) {
        // Transient analysis for RC, RL, RLC circuits
        // dI/dt = (V - I*R) / L
        // dV/dt = I / C

        for (const auto& conn : m_Connections) {
            int idxComp = -1;
            for (size_t i = 0; i < m_Components.size(); ++i) {
                if (m_Components[i].id == conn.componentId) {
                    idxComp = i;
                    break;
                }
            }
            if (idxComp < 0) continue;

            float I = m_DCResults.componentCurrents[idxComp];

            if (m_Components[idxComp].type == ComponentType::Inductor) {
                // dI/dt = V / L
                float V = m_DCResults.nodeVoltages[0];  // Simplified
                float dI = (V / m_Components[idxComp].value) * static_cast<float>(timeStep);
                m_DCResults.componentCurrents[idxComp] += dI;
            } else if (m_Components[idxComp].type == ComponentType::Capacitor) {
                // dV/dt = I / C
                float dV = (I / m_Components[idxComp].value) * static_cast<float>(timeStep);
                // Update node voltage
                for (size_t i = 0; i < m_Nodes.size(); ++i) {
                    if (m_Nodes[i].id == conn.nodeBId) {
                        m_DCResults.nodeVoltages[i] += dV;
                        break;
                    }
                }
            }
        }

        CalculatePower();
    }

    void CircuitSolver::AssembleAdmittanceMatrix() {
        // Build admittance matrix for circuit analysis
        // Y = 1/Z for each component
    }

    void CircuitSolver::ApplyKirchhoffLaws() {
        // Apply Kirchhoff's voltage and current laws
        // KVL: Sum of voltages around loop = 0
        // KCL: Sum of currents at node = 0
    }

    void CircuitSolver::CheckThermalStress() {
        // Check for thermal stress on components
        for (size_t i = 0; i < m_Components.size(); ++i) {
            float P = m_DCResults.componentPowers[i];
            float I = m_DCResults.componentCurrents[i];

            // Check power rating
            if (P > m_Components[i].powerRating * OVERCURRENT_THRESHOLD) {
                m_Components[i].isFaulty = true;
                m_DCResults.faultyComponents[i] = true;
            }

            // Estimate temperature rise: ΔT = P * R_thermal
            float R_thermal = 50.0f;  // K/W (typical)
            float dT = P * R_thermal;
            m_Components[i].temperature = 293.15f + dT;

            if (m_Components[i].temperature > OVERTEMP_THRESHOLD) {
                m_Components[i].isFaulty = true;
                m_DCResults.faultyComponents[i] = true;
            }
        }
    }

    void CircuitSolver::CalculatePower() {
        m_DCResults.totalPower = 0.0f;
        m_DCResults.totalCurrent = 0.0f;

        for (float P : m_DCResults.componentPowers) {
            m_DCResults.totalPower += std::abs(P);
        }

        for (float I : m_DCResults.componentCurrents) {
            m_DCResults.totalCurrent += std::abs(I);
        }
    }

    bool CircuitSolver::DetectFault(uint32_t componentId) {
        for (size_t i = 0; i < m_Components.size(); ++i) {
            if (m_Components[i].id == componentId) {
                return m_Components[i].isFaulty;
            }
        }
        return false;
    }

    std::vector<uint32_t> CircuitSolver::GetFaultyComponents() const {
        std::vector<uint32_t> faultyIds;
        for (const auto& comp : m_Components) {
            if (comp.isFaulty) {
                faultyIds.push_back(comp.id);
            }
        }
        return faultyIds;
    }

    float CircuitSolver::GetNodeVoltage(uint32_t nodeId) const {
        for (size_t i = 0; i < m_Nodes.size(); ++i) {
            if (m_Nodes[i].id == nodeId) {
                return m_DCResults.nodeVoltages[i];
            }
        }
        return 0.0f;
    }

    float CircuitSolver::GetComponentCurrent(uint32_t componentId) const {
        for (size_t i = 0; i < m_Components.size(); ++i) {
            if (m_Components[i].id == componentId) {
                return m_DCResults.componentCurrents[i];
            }
        }
        return 0.0f;
    }

    float CircuitSolver::GetComponentPower(uint32_t componentId) const {
        for (size_t i = 0; i < m_Components.size(); ++i) {
            if (m_Components[i].id == componentId) {
                return m_DCResults.componentPowers[i];
            }
        }
        return 0.0f;
    }

} // namespace SZM::Electrical
