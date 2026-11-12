#include "ThermalSolver.hpp"
#include <cmath>
#include <algorithm>

namespace SZM::Thermal {

    void ThermalSolver::Init() {
        m_Nodes.clear();
        m_Elements.clear();
        m_BoundaryConditions.clear();
        m_HeatSources.clear();
        m_TemperatureField.clear();
        m_HeatFlowField.clear();
        m_Results = ThermalResults{};
        m_NextNodeId = 1;
        m_NextElementId = 1;
    }

    void ThermalSolver::Shutdown() {
        m_Nodes.clear();
        m_Elements.clear();
        m_BoundaryConditions.clear();
        m_HeatSources.clear();
        m_TemperatureField.clear();
        m_HeatFlowField.clear();
        m_Results = ThermalResults{};
        m_NextNodeId = 1;
        m_NextElementId = 1;
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
                    if (bc.type == BoundaryConditionType::FixedTemperature &&
                        i < m_TemperatureField.size()) {
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
                if (m_Nodes[i].id == nodeId0) idx0 = static_cast<int>(i);
                if (m_Nodes[i].id == nodeId1) idx1 = static_cast<int>(i);
            }

            if (idx0 < 0 || idx1 < 0) continue;
            size_t u0 = static_cast<size_t>(idx0);
            size_t u1 = static_cast<size_t>(idx1);
            if (u0 >= m_TemperatureField.size() || u1 >= m_TemperatureField.size() ||
                u0 >= m_HeatFlowField.size() || u1 >= m_HeatFlowField.size()) {
                continue;
            }

            float T0 = m_TemperatureField[u0];
            float T1 = m_TemperatureField[u1];
            float dT = T1 - T0;

            // Distance between nodes
            Geometry::Vector3 diff = m_Nodes[u1].position - m_Nodes[u0].position;
            float distance = diff.Magnitude();
            if (distance < 1e-6f) continue;

            // Heat flow: q = k * A * dT / L
            float k = elem.thermalConductivity;
            float A = elem.area > 0 ? elem.area : 0.01f;
            float q = k * A * dT / distance;

            // Add to heat flow field
            m_HeatFlowField[u0] += q;
            m_HeatFlowField[u1] -= q;
        }
    }

    void ThermalSolver::CalculateConvection() {
        // Newton's law: q = h * (T_surface - T_ambient)
        for (const auto& bc : m_BoundaryConditions) {
            if (bc.type != BoundaryConditionType::Convection) continue;

            // Find node index
            for (size_t i = 0; i < m_Nodes.size(); ++i) {
                if (m_Nodes[i].id == bc.nodeId) {
                    if (i < m_TemperatureField.size() && i < m_HeatFlowField.size()) {
                        float T = m_TemperatureField[i];
                        float dT = T - bc.ambientTemp;
                        float q = bc.convectionCoeff * dT;
                        m_HeatFlowField[i] -= q;  // Heat loss
                    }
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
                    if (i < m_TemperatureField.size() && i < m_HeatFlowField.size()) {
                        float T = m_TemperatureField[i];
                        float T_amb = bc.ambientTemp;
                        float T4_diff = T*T*T*T - T_amb*T_amb*T_amb*T_amb;
                        float q = bc.emissivity * STEFAN_BOLTZMANN * T4_diff;
                        m_HeatFlowField[i] -= q;  // Heat loss
                    }
                    break;
                }
            }
        }
    }

    void ThermalSolver::UpdateTemperatures(double dt) {
        // dT/dt = Q / (m * c) = Q / (ρ * V * c)
        for (size_t i = 0; i < m_Nodes.size(); ++i) {
            if (i >= m_TemperatureField.size() || i >= m_HeatFlowField.size()) {
                continue;
            }
            double Q = static_cast<double>(m_HeatFlowField[i]);
            double thermalMass = static_cast<double>(m_Nodes[i].density) *
                                 static_cast<double>(m_Nodes[i].volume) *
                                 static_cast<double>(m_Nodes[i].heatCapacity);
            if (thermalMass > 1e-9) {
                double dT = (Q / thermalMass) * dt;
                m_TemperatureField[i] += static_cast<float>(dT);
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
        if (m_TemperatureField.size() != m_Nodes.size()) {
            AssembleSystem();
        }

        if (m_HeatFlowField.size() != m_Nodes.size()) {
            m_HeatFlowField.resize(m_Nodes.size(), 0.0f);
        } else {
            std::fill(m_HeatFlowField.begin(), m_HeatFlowField.end(), 0.0f);
        }

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
        m_Results.elementHeatFlux.assign(m_Elements.size(), 0.0f);

        // Compute heat flux per element from temperature gradient
        for (size_t e = 0; e < m_Elements.size(); ++e) {
            const auto& elem = m_Elements[e];
            if (elem.nodeIds.size() >= 2) {
                int idx0 = -1, idx1 = -1;
                for (size_t i = 0; i < m_Nodes.size(); ++i) {
                    if (m_Nodes[i].id == elem.nodeIds[0]) idx0 = i;
                    if (m_Nodes[i].id == elem.nodeIds[1]) idx1 = i;
                }
                if (idx0 >= 0 && idx1 >= 0 && idx0 < static_cast<int>(m_TemperatureField.size()) && idx1 < static_cast<int>(m_TemperatureField.size())) {
                    Geometry::Vector3 diff = m_Nodes[idx1].position - m_Nodes[idx0].position;
                    float distance = diff.Magnitude();
                    if (distance > 1e-6f) {
                        float dT = std::abs(m_TemperatureField[idx1] - m_TemperatureField[idx0]);
                        float k = elem.thermalConductivity > 0.0f ? elem.thermalConductivity : 50.0f;
                        m_Results.elementHeatFlux[e] = k * (dT / distance);
                    }
                }
            }
        }

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
                if (i < m_TemperatureField.size()) {
                    return m_TemperatureField[i];
                }
                return m_Nodes[i].temperature;
            }
        }
        return 0.0f;
    }

    float ThermalSolver::GetElementHeatFlux(uint32_t elemId) const {
        for (size_t i = 0; i < m_Elements.size(); ++i) {
            if (m_Elements[i].id == elemId) {
                if (i < m_Results.elementHeatFlux.size()) {
                    return m_Results.elementHeatFlux[i];
                }
                return 0.0f;
            }
        }
        return 0.0f;
    }

} // namespace SZM::Thermal
