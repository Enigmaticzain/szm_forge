#include "Scheduler.hpp"
#include "CouplingManager.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace SZM {

Scheduler::Scheduler() {
    m_CouplingManager = std::make_unique<CouplingManager>();
}

Scheduler::~Scheduler() {
    Shutdown();
}

void Scheduler::Initialize() {
    if (m_IsInitialized) return;

    std::cout << "[Scheduler] Initializing Multi-Physics Orchestrator...\n";
    m_CouplingManager->Initialize();

    for (auto& solver : m_Solvers) {
        solver->Initialize();
        std::cout << "[Scheduler] Initialized solver: " << solver->GetName() << "\n";
    }

    SortSolvers();
    m_GlobalTime = 0.0;
    m_IsInitialized = true;
}

void Scheduler::Shutdown() {
    if (!m_IsInitialized) return;

    std::cout << "[Scheduler] Shutting down...\n";
    
    for (auto& solver : m_Solvers) {
        solver->Shutdown();
    }
    m_Solvers.clear();

    if (m_CouplingManager) {
        m_CouplingManager->Shutdown();
    }

    m_IsInitialized = false;
}

void Scheduler::RegisterSolver(std::shared_ptr<ISolver> solver) {
    if (!solver) return;
    m_Solvers.push_back(solver);
    if (m_IsInitialized) {
        // If added late, initialize it
        solver->Initialize();
        SortSolvers();
    }
}

void Scheduler::SortSolvers() {
    // Operator Splitting Sequence:
    // 1. Fluid Dynamics (computes loads and convection)
    // 2. Thermal (computes temperature distribution)
    // 3. Mechanical / Rigid Body (computes deformation/stress using fluid/thermal loads)
    
    auto getPriority = [](SolverType type) -> int {
        switch (type) {
            case SolverType::FluidDynamics: return 0;
            case SolverType::Thermal:       return 1;
            case SolverType::Electrical:    return 2;
            case SolverType::MechanicalFEA: return 3;
            case SolverType::RigidBodyPhysics: return 4;
            default: return 99;
        }
    };

    std::sort(m_Solvers.begin(), m_Solvers.end(),
        [&getPriority](const std::shared_ptr<ISolver>& a, const std::shared_ptr<ISolver>& b) {
            return getPriority(a->GetType()) < getPriority(b->GetType());
        }
    );
}

void Scheduler::Tick(double globalDt) {
    if (!m_IsInitialized) return;

    // 1. Pre-step coupling (Map boundary conditions to solvers)
    m_CouplingManager->PreStepCoupling(globalDt);

    // 2. Execute Solvers (Operator Splitting)
    for (auto& solver : m_Solvers) {
        double maxDt = solver->GetMaxAllowedTimeStep();
        
        if (globalDt <= maxDt) {
            solver->Step(globalDt);
        } else {
            // Sub-stepping required
            int steps = static_cast<int>(std::ceil(globalDt / maxDt));
            double subDt = globalDt / steps;
            for (int i = 0; i < steps; ++i) {
                solver->Step(subDt);
            }
        }
    }

    // 3. Post-step coupling (Map resulting fields back)
    m_CouplingManager->PostStepCoupling(globalDt);

    m_GlobalTime += globalDt;
}

} // namespace SZM
