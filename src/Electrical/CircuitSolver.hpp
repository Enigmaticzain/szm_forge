#pragma once

#include "ElectricalDataTypes.hpp"
#include <memory>
#include <unordered_map>

namespace SZM::Electrical {

/**
 * @class CircuitSolver
 * @brief Solves electrical circuits with DC, AC, and transient analysis
 *
 * Features:
 *  - DC analysis (Ohm's law, Kirchhoff's laws)
 *  - AC analysis (impedance, frequency response)
 *  - Transient analysis (RC, RL, RLC circuits)
 *  - Fault detection (overcurrent, overvoltage)
 *  - Power calculation
 */
class CircuitSolver {
public:
    static CircuitSolver& GetInstance() {
        static CircuitSolver instance;
        return instance;
    }

    CircuitSolver(const CircuitSolver&) = delete;
    CircuitSolver& operator=(const CircuitSolver&) = delete;

    // Lifecycle
    void Init();
    void Shutdown();

    // Circuit construction
    uint32_t AddNode(const std::string& name);
    uint32_t AddComponent(ComponentType type, float value, const std::string& name = "");
    void Connect(uint32_t nodeAId, uint32_t nodeBId, uint32_t componentId);

    // Analysis methods
    void SolveDC();
    void SolveAC(float frequency);
    void SolveTransient(double timeStep);

    // Results
    float GetNodeVoltage(uint32_t nodeId) const;
    float GetComponentCurrent(uint32_t componentId) const;
    float GetComponentPower(uint32_t componentId) const;
    const CircuitAnalysisResults& GetDCResults() const { return m_DCResults; }
    const ACAnalysisResults& GetACResults() const { return m_ACResults; }

    // Fault detection
    bool DetectFault(uint32_t componentId);
    std::vector<uint32_t> GetFaultyComponents() const;

    // Accessors
    const std::vector<ElectricalNode>& GetNodes() const { return m_Nodes; }
    const std::vector<ElectricalComponent>& GetComponents() const { return m_Components; }
    const std::vector<ElectricalConnection>& GetConnections() const { return m_Connections; }
    size_t GetNodeCount() const { return m_Nodes.size(); }
    size_t GetComponentCount() const { return m_Components.size(); }

private:
    CircuitSolver() = default;
    ~CircuitSolver() = default;

    void AssembleAdmittanceMatrix();
    void ApplyKirchhoffLaws();
    void CheckThermalStress();
    void CalculatePower();

private:
    std::vector<ElectricalNode> m_Nodes;
    std::vector<ElectricalComponent> m_Components;
    std::vector<ElectricalConnection> m_Connections;

    CircuitAnalysisResults m_DCResults;
    ACAnalysisResults m_ACResults;

    uint32_t m_NextNodeId = 1;
    uint32_t m_NextComponentId = 1;
};

} // namespace SZM::Electrical
