#include "Scheduler.hpp"
#include "CouplingManager.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace SZM {

// ============================================================================
// ThreadPool
// ============================================================================

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_Workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    m_CV.wait(lock, [this]{ return m_Stop || !m_Queue.empty(); });
                    if (m_Stop && m_Queue.empty()) return;
                    task = std::move(m_Queue.front());
                    m_Queue.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    m_Stop = true;
    m_CV.notify_all();
    for (auto& w : m_Workers) {
        if (w.joinable()) w.join();
    }
}

Scheduler::Scheduler() {
    m_CouplingManager = std::make_unique<CouplingManager>();
    const size_t hw = std::thread::hardware_concurrency();
    m_ThreadPool = std::make_unique<ThreadPool>(hw > 1 ? hw - 1 : 1);
    std::cout << "[Scheduler] Thread pool: " << (hw > 1 ? hw - 1 : 1) << " workers\n";
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

    m_CouplingManager->PreStepCoupling(globalDt);

    // Group solvers by priority level — same-priority solvers run in parallel,
    // different-priority groups run sequentially (operator splitting order).
    auto getPriority = [](SolverType type) -> int {
        switch (type) {
            case SolverType::FluidDynamics:    return 0;
            case SolverType::Thermal:          return 1;
            case SolverType::Electrical:       return 2;
            case SolverType::MechanicalFEA:    return 3;
            case SolverType::RigidBodyPhysics: return 4;
            default: return 99;
        }
    };

    int currentPriority = -1;
    std::vector<std::future<void>> batch;

    auto flushBatch = [&]() {
        for (auto& f : batch) f.get();
        batch.clear();
    };

    for (auto& solver : m_Solvers) {
        int p = getPriority(solver->GetType());
        if (p != currentPriority) {
            flushBatch(); // wait for previous priority group
            currentPriority = p;
        }
        double maxDt = solver->GetMaxAllowedTimeStep();
        auto s = solver; // capture by value for thread safety
        batch.push_back(m_ThreadPool->Submit([s, globalDt, maxDt]() {
            if (globalDt <= maxDt) {
                s->Step(globalDt);
            } else {
                int steps = static_cast<int>(std::ceil(globalDt / maxDt));
                double subDt = globalDt / steps;
                for (int i = 0; i < steps; ++i) s->Step(subDt);
            }
        }));
    }
    flushBatch();

    m_CouplingManager->PostStepCoupling(globalDt);
    m_GlobalTime += globalDt;
}

size_t Scheduler::GetThreadCount() const {
    return m_ThreadPool ? m_ThreadPool->Size() : 0;
}

} // namespace SZM
