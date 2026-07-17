#pragma once

#include <memory>
#include <vector>

namespace SZM {

    class ISolver;
    struct SimulationComponent;

    /**
     * @class CouplingManager
     * @brief Manages the exchange of boundary conditions and fields between disjoint physics solvers.
     */
    class CouplingManager {
    public:
        CouplingManager() = default;
        ~CouplingManager() = default;

        void Initialize();
        void Shutdown();

        /// Executes pre-step coupling (e.g., passing thermal loads to FEA)
        void PreStepCoupling(double dt);

        /// Executes post-step coupling (e.g., passing fluid heat transfer to Thermal)
        void PostStepCoupling(double dt);

        // --- Coupling Interfaces (Mock for Phase 2 implementation) ---
        // In a full system, this would map nodes to nodes or boundaries to boundaries.
        // For Phase 2, we will use the SimulationComponent state as a proxy board.

        /// Register a component to track its coupled states
        void RegisterComponent(SimulationComponent* comp);

    private:
        std::vector<SimulationComponent*> m_CoupledComponents;

        // Specific coupling transfers
        void TransferFluidToThermal(double dt);
        void TransferThermalToMechanical(double dt);
    };

} // namespace SZM
