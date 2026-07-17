#pragma once

#include "SimulationComponent.hpp"
#include "Scheduler.hpp"
#include <vector>
#include <memory>
#include <cstdint>

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
    
    bool SetComponentMaterial(uint32_t id, const std::string& materialId);
    bool SetComponentGeometry(uint32_t id, float area, float thickness);

    // Setters for testing
    void SetForce(uint32_t id, float force);
    void SetHeatInput(uint32_t id, float heat);

    // Access to scheduler
    Scheduler& GetScheduler() { return *m_Scheduler; }

    // Pause state
    bool IsPaused() const { return m_IsPaused; }
    void SetPaused(bool paused) { m_IsPaused = paused; }

private:
    SimulationEngine();
    ~SimulationEngine() = default;

private:
    std::vector<std::unique_ptr<SimulationComponent>> m_Components;
    std::unique_ptr<Scheduler> m_Scheduler;
    uint32_t m_NextComponentId = 1;
    bool m_IsInitialized = false;
    bool m_IsPaused = false;
    
    // Constants
    static constexpr float MAX_TEMP_K = 373.15f; // 100°C
    static constexpr float AMBIENT_TEMP_K = 293.15f; // 20°C
    static constexpr float HEAT_DISSIPATION_RATE = 0.1f; // W/K
};

} // namespace SZM
