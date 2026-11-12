#include "SimulationEngine.hpp"
#include "CouplingManager.hpp"
#include "../Input/EventBus.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>
#include <execution>

namespace SZM {

static std::atomic<int64_t> s_LastBridgeFailureNs{0};

SimulationEngine::SimulationEngine()
    : m_Scheduler(std::make_unique<Scheduler>()),
      m_Scene(std::make_unique<SceneGraph::Scene>()) {
}

SimulationEngine::~SimulationEngine() {
    Shutdown();
}

void SimulationEngine::Init() {
    if (m_IsInitialized) {
        return;
    }

    m_ShuttingDown = false;
    Materials::MaterialDatabase::GetInstance().LoadStandardLibraries();
    
    m_Scheduler->Initialize();

    m_IsInitialized = true;
    std::cout << "[SZM Simulation] Engine initialized.\n";
}

void SimulationEngine::Tick(double deltaTime) {
    if (m_IsPaused || m_ShuttingDown) {
        return;
    }

    const double effectiveDelta = deltaTime * m_TimeScale;

    if (m_Scheduler) {
        m_Scheduler->Tick(effectiveDelta);
    }

    // Reap completed CalculiX worker threads and apply marshaled stress updates safely
    std::vector<std::thread> finishedThreads;
    std::vector<CcxResultUpdate> resultUpdates;
    {
        std::lock_guard<std::mutex> lock(m_CcxMutex);
        for (uint32_t cid : m_FinishedCcxIds) {
            auto it = m_CcxThreads.find(cid);
            if (it != m_CcxThreads.end()) {
                finishedThreads.push_back(std::move(it->second));
                m_CcxThreads.erase(it);
            }
        }
        m_FinishedCcxIds.clear();
        resultUpdates.swap(m_PendingCcxResults);
    }
    for (auto& t : finishedThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Simple mechanics and thermal step
    for (const auto& compPtr : m_Components) {
        auto& comp = *compPtr;
        if (comp.area > 0.0f) {
            comp.stress = comp.appliedForce / comp.area;
        }

        if (comp.area > 0.0f && comp.thickness > 0.0f && comp.density > 0.0f) {
            float mass = comp.area * comp.thickness * comp.density;
            float specificHeat = 900.0f;
            if (mass > 0.0f) {
                float tempDelta = static_cast<float>((comp.heatInput * effectiveDelta) / (mass * specificHeat));
                comp.temperature += tempDelta;
            }
        }
        
        if (comp.heatInput == 0.0f && comp.temperature > AMBIENT_TEMP_K) {
            float cooling = (comp.temperature - AMBIENT_TEMP_K) * 0.1f * static_cast<float>(effectiveDelta);
            comp.temperature -= cooling;
        }
    }

    // Overlay marshaled CalculiX results after local baseline step
    for (const auto& upd : resultUpdates) {
        if (auto* c = GetComponent(upd.componentId)) {
            c->stress      = upd.stress;
            c->stressRatio = upd.stressRatio;
            c->isDangerous = upd.isDangerous;
        }
    }

    // Check if CalculiX jobs should be dispatched (with 10s cooldown if bridge failed)
    const int64_t lastFailureNs = s_LastBridgeFailureNs.load(std::memory_order_relaxed);
    const auto now = std::chrono::steady_clock::now();
    const bool bridgeInCooldown = (lastFailureNs != 0) &&
        (std::chrono::duration_cast<std::chrono::seconds>(
             now - std::chrono::steady_clock::time_point(
                 std::chrono::steady_clock::duration(lastFailureNs)))
             .count() < 10);

    std::vector<uint32_t> jobsToDispatch;
    for (const auto& compPtr : m_Components) {
        auto& comp = *compPtr;

        if (comp.appliedForce > 0.0f && !bridgeInCooldown) {
            std::lock_guard<std::mutex> lock(m_CcxMutex);
            if (m_CcxThreads.find(comp.id) == m_CcxThreads.end()) {
                jobsToDispatch.push_back(comp.id);
            }
        }

        comp.stressRatio = (comp.yieldStrength > 0.0f)
            ? std::min(1.0f, comp.stress / comp.yieldStrength)
            : 0.0f;
        comp.tempRatio = (MAX_TEMP_K > AMBIENT_TEMP_K)
            ? std::min(1.0f, (comp.temperature - AMBIENT_TEMP_K) / (MAX_TEMP_K - AMBIENT_TEMP_K))
            : 0.0f;
        comp.isDangerous = (comp.stressRatio > 0.8f) || (comp.tempRatio > 0.8f);
    }

    for (uint32_t cid : jobsToDispatch) {
        DispatchCalculiXJob(cid);
    }

    // Publish event for UI/visualization
    EventBus::GetInstance().Publish("SimulationTick", std::any());
}

void SimulationEngine::Shutdown() {
    m_ShuttingDown = true;

    std::vector<std::thread> activeThreads;
    {
        std::lock_guard<std::mutex> lock(m_CcxMutex);
        for (auto& [cid, t] : m_CcxThreads) {
            (void)cid;
            if (t.joinable()) {
                activeThreads.push_back(std::move(t));
            }
        }
        m_CcxThreads.clear();
        m_FinishedCcxIds.clear();
        m_PendingCcxResults.clear();
    }
    for (auto& t : activeThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    if (m_Scheduler) {
        if (auto* coupling = m_Scheduler->GetCouplingManager()) {
            coupling->ClearComponents();
        }
        m_Scheduler->Shutdown();
    }
    m_Components.clear();
    m_NextComponentId = 1;
    m_IsInitialized = false;
    std::cout << "[SZM Simulation] Engine shutdown.\n";
}

uint32_t SimulationEngine::AddComponent(const std::string& name) {
    auto comp = std::make_unique<SimulationComponent>();
    comp->id = m_NextComponentId;
    comp->name = name;
    if (m_Scheduler && m_Scheduler->GetCouplingManager()) {
        m_Scheduler->GetCouplingManager()->RegisterComponent(comp.get());
    }

    m_Components.push_back(std::move(comp));
    
    // Sync with new ECS Scene Graph
    if (m_Scene) {
        SceneGraph::Entity e = m_Scene->CreateEntity(name);
        SceneGraph::PhysicsStateComponent physics;
        m_Scene->AddComponent<SceneGraph::PhysicsStateComponent>(e, physics);
    }
    
    std::cout << "[SZM Simulation] Added component: " << name 
              << " (ID: " << m_NextComponentId << ")\n";
    
    return m_NextComponentId++;
}

uint32_t SimulationEngine::AddComponent(const std::string& name,
                                        const std::string& materialId,
                                        float area,
                                        float thickness) {
    const uint32_t id = AddComponent(name);
    SetComponentGeometry(id, area, thickness);
    SetComponentMaterial(id, materialId);
    return id;
}

void SimulationEngine::RemoveComponent(uint32_t id) {
    auto it = std::find_if(m_Components.begin(), m_Components.end(),
        [id](const auto& comp) { return comp->id == id; });
    
    if (it != m_Components.end()) {
        if (m_Scheduler && m_Scheduler->GetCouplingManager()) {
            m_Scheduler->GetCouplingManager()->UnregisterComponent(it->get());
        }
        std::cout << "[SZM Simulation] Removed component ID: " << id << "\n";
        m_Components.erase(it);
    }
}

void SimulationEngine::ClearAllComponents() {
    if (m_Scheduler && m_Scheduler->GetCouplingManager()) {
        m_Scheduler->GetCouplingManager()->ClearComponents();
    }
    m_Components.clear();
    m_NextComponentId = 1;
    std::cout << "[SZM Simulation] Cleared all components.\n";
}

SimulationComponent* SimulationEngine::GetComponent(uint32_t id) {
    auto it = std::find_if(m_Components.begin(), m_Components.end(),
        [id](const auto& comp) { return comp->id == id; });
    
    return (it != m_Components.end()) ? it->get() : nullptr;
}

const SimulationComponent* SimulationEngine::GetComponent(uint32_t id) const {
    auto it = std::find_if(m_Components.begin(), m_Components.end(),
        [id](const auto& comp) { return comp->id == id; });
    
    return (it != m_Components.end()) ? it->get() : nullptr;
}

bool SimulationEngine::SetComponentMaterial(uint32_t id, const std::string& materialId) {
    auto* comp = GetComponent(id);
    if (!comp) {
        return false;
    }

    const auto material = Materials::MaterialDatabase::GetInstance().GetMaterial(materialId);
    if (!material.has_value()) {
        return false;
    }

    comp->materialId = material->id;
    comp->materialName = material->name;
    comp->density = static_cast<float>(material->density);
    comp->yieldStrength = static_cast<float>(material->yieldStrength * 1.0e6);
    comp->thermalConductivity = static_cast<float>(material->thermalConductivity);
    return true;
}

bool SimulationEngine::SetComponentGeometry(uint32_t id, float area, float thickness) {
    auto* comp = GetComponent(id);
    if (!comp) {
        return false;
    }

    comp->area = std::max(area, 1.0e-6f);
    comp->thickness = std::max(thickness, 1.0e-4f);
    return true;
}

void SimulationEngine::SetForce(uint32_t id, float force) {
    if (auto* comp = GetComponent(id)) {
        comp->appliedForce = force;
    }
}

void SimulationEngine::SetHeatInput(uint32_t id, float heat) {
    if (auto* comp = GetComponent(id)) {
        comp->heatInput = heat;
    }
}

void SimulationEngine::DispatchCalculiXJob(uint32_t componentId) {
    if (m_ShuttingDown) {
        return;
    }

    auto* comp = GetComponent(componentId);
    if (!comp) return;

    const float force    = comp->appliedForce;
    const float area     = comp->area;
    const float thick    = comp->thickness;
    const float E        = 210e9f; // default; override from material if available
    const float yld      = comp->yieldStrength;
    const float rho      = comp->density;
    const uint32_t cid   = componentId;
    const float fallbackStress = comp->stress;
    const float fallbackRatio  = comp->stressRatio;
    const bool  fallbackDanger = comp->isDangerous;

    std::lock_guard<std::mutex> lock(m_CcxMutex);
    if (m_ShuttingDown || m_CcxThreads.find(cid) != m_CcxThreads.end()) {
        return;
    }

    m_CcxThreads[cid] = std::thread([this, cid, force, area, thick, E, yld, rho, fallbackStress, fallbackRatio, fallbackDanger]() {
        try {
            httplib::Client cli("127.0.0.1", 8003);
            cli.set_connection_timeout(2);
            cli.set_read_timeout(15);

            nlohmann::json body = {
                {"component_id",      cid},
                {"force_n",           force},
                {"area_m2",           area},
                {"thickness_m",       thick},
                {"youngs_modulus_pa", E},
                {"yield_strength_pa", yld},
                {"density_kg_m3",     rho}
            };

            if (!m_ShuttingDown) {
                auto res = cli.Post("/simulation/fea/run", body.dump(), "application/json");
                if (res && res->status == 200) {
                    auto j = nlohmann::json::parse(res->body);
                    if (j.value("status", "") == "success") {
                        CcxResultUpdate update;
                        update.componentId = cid;
                        update.stress      = j.value("max_stress_pa", fallbackStress);
                        update.stressRatio = j.value("stress_ratio",  fallbackRatio);
                        update.isDangerous = j.value("is_dangerous",  fallbackDanger);

                        std::cout << "[CalculiX] Component " << cid
                                  << " stress=" << update.stress << " Pa"
                                  << " engine=" << j.value("engine_used", "?")
                                  << "\n";

                        std::lock_guard<std::mutex> lock(m_CcxMutex);
                        if (!m_ShuttingDown) {
                            m_PendingCcxResults.push_back(update);
                        }
                    }
                } else {
                    s_LastBridgeFailureNs.store(
                        std::chrono::steady_clock::now().time_since_epoch().count(),
                        std::memory_order_relaxed);
                    std::cerr << "[CalculiX] Bridge unreachable for component " << cid << "\n";
                }
            }
        } catch (const std::exception& e) {
            s_LastBridgeFailureNs.store(
                std::chrono::steady_clock::now().time_since_epoch().count(),
                std::memory_order_relaxed);
            std::cerr << "[CalculiX] Exception: " << e.what() << "\n";
        }

        std::lock_guard<std::mutex> lock(m_CcxMutex);
        if (!m_ShuttingDown) {
            m_FinishedCcxIds.push_back(cid);
        }
    });
}

// Removed simple UpdateStress, UpdateTemperature, and UpdateDangerState 
// because the physics Solvers and CouplingManager handle this now.

} // namespace SZM
