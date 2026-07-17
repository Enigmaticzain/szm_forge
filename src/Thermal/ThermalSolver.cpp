#include "ThermalSolver.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Thermal {

    void ThermalSolver::Init() {
        m_Nodes.clear();
        m_Elements.clear();
        m_BoundaryConditions.clear();
        m_HeatSources.clear();
        m_NextNodeId = 1;
        m_NextElementId = 1;
    }

    void ThermalSolver::Shutdown() {
        m_Nodes.clear();
        m_Elements.clear();
        m_BoundaryConditions.clear();
        m_HeatSources.clear();
    }

    uint32_t ThermalSolver::AddNode(const Geometry::Vector3& pos, float temp) {
        ThermalNode node;
        node.id = m_NextNodeId++;
        node.position = pos;
        node.temperature = temp;
        m_Nodes.push_back(node);
        return node.id;
    }

    uint32_t ThermalSolver::AddElement(const std::vector<uint32_t>& nodeIds) {
        if (nodeIds.size() < 3 || nodeIds.size() > 4) {
            return 0;  // Invalid element
        }

        ThermalElement elem;
        elem.id = m_NextElementId++;
        elem.nodeIds = nodeIds;
        m_Elements.push_back(elem);
        return elem.id;
    }

    void ThermalSolver::AddBoundaryCondition(const ThermalBoundaryCondition& bc) {
        m_BoundaryConditions.push_back(bc);
    }

    void ThermalSolver::AddHeatSource(const ThermalHeatSource& source) {
        m_HeatSources.push_back(source);
    }

    void ThermalSolver::AssembleSystem() {
        // Initialize temperature field
        m_TemperatureField.resize(m_Nodes.size());
        for (size_t i = 0; i < m_Nodes.size(); ++i) {
            m_TemperatureField[i] = m_Nodes[i].temperature;
        }

        // Initialize heat flow field
        m_HeatFlowField.resize(m_Nodes.size(), 0.0f);
    }

    void ThermalSolver::ApplyBoundaryConditions() {
        for (const auto& bc : m_BoundaryConditions) {
            // Find node index
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == bc.nodeId) {
                    if (bc.type == BoundaryConditionType::FixedTemperature) {
                        m_TemperatureField[i] = bc.value;
                    }
                    break;
                }
            }
        }
    }

    void ThermalSolver::CalculateConduction() {
        // Fourier's law: q = -k * dT/dx
        // For each element, calculate heat flow between nodes
        for (const auto& elem : m_Elements) {
            if (elem.nodeIds.size() < 2) continue;

            // Simple 1D conduction between first two nodes
            uint32_t nodeId0 = elem.nodeIds[0];
            uint32_t nodeId1 = elem.nodeIds[1];

            // Find node indices
            int idx0 = -1, idx1 = -1;
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == nodeId0) idx0 = i;
                if (m_Nodes[i].id == nodeId1) idx1 = i;
            }

            if (idx0 < 0 || idx1 < 0) continue;

            float T0 = m_TemperatureField[idx0];
            float T1 = m_TemperatureField[idx1];
            float dT = T1 - T0;

            // Distance between nodes
            Geometry::Vector3 diff = m_Nodes[idx1].position - m_Nodes[idx0].position;
            float distance = diff.Magnitude();
            if (distance < 1e-6f) continue;

            // Heat flow: q = k * A * dT / L
            float k = elem.thermalConductivity;
            float A = elem.area > 0 ? elem.area : 0.01f;
            float q = k * A * dT / distance;

            // Add to heat flow field
            m_HeatFlowField[idx0] += q;
            m_HeatFlowField[idx1] -= q;
        }
    }

    void ThermalSolver::CalculateConvection() {
        // Newton's law: q = h * (T_surface - T_ambient)
        for (const auto& bc : m_BoundaryConditions) {
            if (bc.type != BoundaryConditionType::Convection) continue;

            // Find node index
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == bc.nodeId) {
                    float T = m_TemperatureField[i];
                    float dT = T - bc.ambientTemp;
                    float q = bc.convectionCoeff * dT;
                    m_HeatFlowField[i] -= q;  // Heat loss
                    break;
                }
            }
        }
    }

    void ThermalSolver::CalculateRadiation() {
        // Stefan-Boltzmann: q = ε * σ * (T⁴ - T_ambient⁴)
        for (const auto& bc : m_BoundaryConditions) {
            if (bc.type != BoundaryConditionType::Radiation) continue;

            // Find node index
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == bc.nodeId) {
                    float T = m_TemperatureField[i];
                    float T_amb = bc.ambientTemp;
                    float T4_diff = T*T*T*T - T_amb*T_amb*T_amb*T_amb;
                    float q = bc.emissivity * STEFAN_BOLTZMANN * T4_diff;
                    m_HeatFlowField[i] -= q;  // Heat loss
                    break;
                }
            }
        }
    }

    void ThermalSolver::UpdateTemperatures(double dt) {
        // dT/dt = Q / (m * c) = Q / (ρ * V * c)
        for (size_t i = 0; i < m_Nodes.size(); ++i) {
            float Q = m_HeatFlowField[i];
            float mass = m_Nodes[i].density * m_Nodes[i].volume;
            if (mass > 1e-6f) {
                float dT = Q / (mass * m_Nodes[i].heatCapacity) * static_cast<float>(dt);
                m_TemperatureField[i] += dT;
            }
        }
    }

    void ThermalSolver::SolveSteadyState() {
        AssembleSystem();
        ApplyBoundaryConditions();

        // Iterative steady-state solution
        for (int iter = 0; iter < 100; ++iter) {
            std::fill(m_HeatFlowField.begin(), m_HeatFlowField.end(), 0.0f);

            CalculateConduction();
            CalculateConvection();
            CalculateRadiation();

            // Add heat sources
            for (const auto& source : m_HeatSources) {
                for (size_t i = 0; i < m_Nodes.size(); ++i) {
                    if (m_Nodes[i].id == source.nodeId) {
                        m_HeatFlowField[i] += source.power;
                        break;
                    }
                }
            }

            // Check convergence (heat flow near zero)
            float maxHeatFlow = 0.0f;
            for (float q : m_HeatFlowField) {
                maxHeatFlow = std::max(maxHeatFlow, std::abs(q));
            }
            if (maxHeatFlow < 1e-3f) break;

            // Update temperatures (small time step for stability)
            UpdateTemperatures(0.001);
            ApplyBoundaryConditions();
        }

        CalculateHeatFlux();
    }

    void ThermalSolver::SolveTransient(double timeStep) {
        AssembleSystem();

        std::fill(m_HeatFlowField.begin(), m_HeatFlowField.end(), 0.0f);

        CalculateConduction();
        CalculateConvection();
        CalculateRadiation();

        // Add heat sources
        for (const auto& source : m_HeatSources) {
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == source.nodeId) {
                    m_HeatFlowField[i] += source.power;
                    break;
                }
            }
        }

        UpdateTemperatures(timeStep);
        ApplyBoundaryConditions();
        CalculateHeatFlux();
    }

    void ThermalSolver::CalculateHeatFlux() {
        m_Results.nodeTemperatures = m_TemperatureField;
        m_Results.elementHeatFlux.resize(m_Elements.size(), 0.0f);

        if (!m_TemperatureField.empty()) {
            m_Results.maxTemperature = *std::max_element(m_TemperatureField.begin(), m_TemperatureField.end());
            m_Results.minTemperature = *std::min_element(m_TemperatureField.begin(), m_TemperatureField.end());
        } else {
            m_Results.maxTemperature = 0.0f;
            m_Results.minTemperature = 0.0f;
        }

        m_Results.totalHeatFlow = 0.0f;
        for (float q : m_HeatFlowField) {
            m_Results.totalHeatFlow += std::abs(q);
        }
    }

    float ThermalSolver::GetNodeTemperature(uint32_t nodeId) const {
        for (size_t i = 0; i < m_Nodes.size(); ++i) {
            if (m_Nodes[i].id == nodeId) {
                return m_TemperatureField[i];
            }
        }
        return 0.0f;
    }

    float ThermalSolver::GetElementHeatFlux(uint32_t elemId) const {
        for (size_t i = 0; i < m_Elements.size(); ++i) {
            if (m_Elements[i].id == elemId) {
                return m_Results.elementHeatFlux[i];
            }
        }
        return 0.0f;
    }

} // namespace SZM::Thermal
