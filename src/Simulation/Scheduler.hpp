#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <future>
#include <atomic>

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
     * @class ThreadPool
     * @brief Minimal fixed-size thread pool for parallel solver dispatch.
     */
    class ThreadPool {
    public:
        explicit ThreadPool(size_t numThreads);
        ~ThreadPool();

        template<typename F>
        std::future<void> Submit(F&& f) {
            auto task = std::make_shared<std::packaged_task<void()>>(std::forward<F>(f));
            std::future<void> fut = task->get_future();
            {
                std::lock_guard<std::mutex> lock(m_Mutex);
                m_Queue.push([task]() { (*task)(); });
            }
            m_CV.notify_one();
            return fut;
        }

        size_t Size() const { return m_Workers.size(); }

    private:
        std::vector<std::thread>          m_Workers;
        std::queue<std::function<void()>> m_Queue;
        std::mutex                        m_Mutex;
        std::condition_variable           m_CV;
        std::atomic<bool>                 m_Stop{false};
    };

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

        /// Number of worker threads used for parallel solver dispatch
        size_t GetThreadCount() const;

    private:
        std::vector<std::shared_ptr<ISolver>> m_Solvers;
        std::unique_ptr<CouplingManager>      m_CouplingManager;
        std::unique_ptr<ThreadPool>           m_ThreadPool;

        double m_GlobalTime = 0.0;
        bool   m_IsInitialized = false;

        void SortSolvers();
    };

} // namespace SZM
