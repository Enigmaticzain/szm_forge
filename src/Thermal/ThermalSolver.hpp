#pragma once

#include "ThermalDataTypes.hpp"
#include <memory>
#include <unordered_map>

namespace SZM::Thermal {

/**
 * @class ThermalSolver
 * @brief Solves heat transfer problems with conduction, convection, and radiation
 *
 * Features:
 *  - Steady-state and transient analysis
 *  - Conduction (Fourier's law)
 *  - Convection (Newton's law)
 *  - Radiation (Stefan-Boltzmann)
 *  - Multiple boundary condition types
 *  - Heat source/sink support
 */
class ThermalSolver {
public:
    static ThermalSolver& GetInstance() {
        static ThermalSolver instance;
        return instance;
    }

    ThermalSolver(const ThermalSolver&) = delete;
    ThermalSolver& operator=(const ThermalSolver&) = delete;

    // Lifecycle
    void Init();
    void Shutdown();

    // Mesh management
    uint32_t AddNode(const Geometry::Vector3& pos, float temp = 293.15f);
    uint32_t AddElement(const std::vector<uint32_t>& nodeIds);
    void AddBoundaryCondition(const ThermalBoundaryCondition& bc);
    void AddHeatSource(const ThermalHeatSource& source);

    // Simulation
    void AssembleSystem();
    void ApplyBoundaryConditions();
    void SolveSteadyState();
    void SolveTransient(double timeStep);

    // Results
    float GetNodeTemperature(uint32_t nodeId) const;
    float GetElementHeatFlux(uint32_t elemId) const;
    const ThermalResults& GetResults() const { return m_Results; }

    // Accessors
    const std::vector<ThermalNode>& GetNodes() const { return m_Nodes; }
    const std::vector<ThermalElement>& GetElements() const { return m_Elements; }
    size_t GetNodeCount() const { return m_Nodes.size(); }
    size_t GetElementCount() const { return m_Elements.size(); }

private:
    ThermalSolver() = default;
    ~ThermalSolver() = default;

    void CalculateConduction();
    void CalculateConvection();
    void CalculateRadiation();
    void UpdateTemperatures(double dt);
    void CalculateHeatFlux();

private:
    std::vector<ThermalNode> m_Nodes;
    std::vector<ThermalElement> m_Elements;
    std::vector<ThermalBoundaryCondition> m_BoundaryConditions;
    std::vector<ThermalHeatSource> m_HeatSources;
    ThermalResults m_Results;

    uint32_t m_NextNodeId = 1;
    uint32_t m_NextElementId = 1;

    // Solver state
    std::vector<float> m_TemperatureField;
    std::vector<float> m_HeatFlowField;
};

} // namespace SZM::Thermal
