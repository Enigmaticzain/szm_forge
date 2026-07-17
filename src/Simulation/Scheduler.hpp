#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace SZM {

    enum class SolverType {
        RigidBodyPhysics,
        Thermal,
        FluidDynamics,
        MechanicalFEA,
        Electrical
    };

    /**
     * @class ISolver
     * @brief Interface for physics solvers managed by the Scheduler
     */
    class ISolver {
    public:
        virtual ~ISolver() = default;
        
        virtual void Initialize() = 0;
        virtual void Step(double dt) = 0;
        virtual void Shutdown() = 0;
        
        virtual SolverType GetType() const = 0;
        virtual std::string GetName() const = 0;
        
        // Allows the scheduler to sub-step if a solver requires a smaller timestep
        virtual double GetMaxAllowedTimeStep() const { return 0.0166666667; /* 60Hz default */ }
    };

    class CouplingManager;

    /**
     * @class Scheduler
     * @brief Orchestrates multi-physics integration using operator-splitting.
     *        Executes solvers in a defined sequence and coordinates data exchange via CouplingManager.
     */
    class Scheduler {
    public:
        Scheduler();
        ~Scheduler();

        void Initialize();
        void Shutdown();

        /// Advance the global simulation time by globalDt
        void Tick(double globalDt);

        /// Register a physics solver
        void RegisterSolver(std::shared_ptr<ISolver> solver);

        /// Get the coupling manager to set up boundary conditions
        CouplingManager* GetCouplingManager() const { return m_CouplingManager.get(); }

        double GetGlobalTime() const { return m_GlobalTime; }

    private:
        std::vector<std::shared_ptr<ISolver>> m_Solvers;
        std::unique_ptr<CouplingManager>      m_CouplingManager;

        double m_GlobalTime = 0.0;
        bool   m_IsInitialized = false;

        // Sequence of execution for operator splitting
        // Typically: Fluid -> Thermal -> Mechanical -> Physics
        void SortSolvers();
    };

} // namespace SZM
