#pragma once

#include "SimulationComponent.hpp"
#include "Scheduler.hpp"
#include "../Scene/Scene.hpp"
#include <vector>
#include <memory>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace SZM {

/**
 * @class SimulationEngine
 * @brief Minimal physics engine: stress = force/area, temp += heat*dt
 *        Publishes "SimulationTick" event with component states.
 */
class SimulationEngine {
public:
    static SimulationEngine& GetInstance() {
        static SimulationEngine instance;
        return instance;
    }

    SimulationEngine(const SimulationEngine&)            = delete;
    SimulationEngine& operator=(const SimulationEngine&) = delete;

    // Lifecycle
    void Init();
    void Tick(double deltaTime);
    void Shutdown();

    // Component management
    uint32_t AddComponent(const std::string& name);
    uint32_t AddComponent(const std::string& name,
                          const std::string& materialId,
                          float area,
                          float thickness);
    void RemoveComponent(uint32_t id);
    void ClearAllComponents();
    
    /// Retrieve a component by ID (non-const accessor for modification)
    SimulationComponent* GetComponent(uint32_t id);
    
    /// Retrieve a component by ID (const accessor for read-only access)
    const SimulationComponent* GetComponent(uint32_t id) const;
    
    /// Get all components (const reference)
    const std::vector<std::unique_ptr<SimulationComponent>>& GetComponents() const {
        return m_Components;
    }
    
    /// Access the new ECS hierarchical scene graph
    SceneGraph::Scene* GetScene() { return m_Scene.get(); }
    
    bool SetComponentMaterial(uint32_t id, const std::string& materialId);
    bool SetComponentGeometry(uint32_t id, float area, float thickness);

    // Setters for testing
    void SetForce(uint32_t id, float force);
    void SetHeatInput(uint32_t id, float heat);

    // Dispatch an async FEA job to the Python bridge (port 8003)
    void DispatchCalculiXJob(uint32_t componentId);

    // Access to scheduler
    Scheduler& GetScheduler() { return *m_Scheduler; }

    // Pause state
    bool IsPaused() const { return m_IsPaused; }
    void SetPaused(bool paused) { m_IsPaused = paused; }

    // Time scale for simulation speed control (1.0 = realtime)
    void SetTimeScale(double scale) { m_TimeScale = std::max(0.0, scale); }
    double GetTimeScale() const { return m_TimeScale; }

private:
    SimulationEngine();
    ~SimulationEngine() = default;

private:
    std::vector<std::unique_ptr<SimulationComponent>> m_Components;
    std::unique_ptr<Scheduler> m_Scheduler;
    std::unique_ptr<SceneGraph::Scene> m_Scene;
    uint32_t m_NextComponentId = 1;
    bool m_IsInitialized = false;
    bool m_IsPaused = false;
    double m_TimeScale = 1.0;

    // CalculiX async dispatch: tracks which component IDs have a job in-flight
    std::unordered_map<uint32_t, std::thread> m_CcxThreads;
    std::mutex m_CcxMutex;
    
    // Constants
    static constexpr float MAX_TEMP_K = 373.15f; // 100°C
    static constexpr float AMBIENT_TEMP_K = 293.15f; // 20°C
    static constexpr float HEAT_DISSIPATION_RATE = 0.1f; // W/K
};

} // namespace SZM
