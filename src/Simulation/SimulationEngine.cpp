#include "SimulationEngine.hpp"
#include "CouplingManager.hpp"
#include "../Input/EventBus.hpp"
#include "../Materials/MaterialDatabase.hpp"
#include <iostream>
#include <algorithm>

namespace SZM {

SimulationEngine::SimulationEngine() {
    m_Scheduler = std::make_unique<Scheduler>();
}

void SimulationEngine::Init() {
    if (m_IsInitialized) {
        return;
    }

    Materials::MaterialDatabase::GetInstance().LoadStandardLibraries();
    
    m_Scheduler->Initialize();

    m_IsInitialized = true;
    std::cout << "[SZM Simulation] Engine initialized.\n";
}

void SimulationEngine::Tick(double deltaTime) {
    if (m_IsPaused) {
        return;
    }

    if (m_Scheduler) {
        m_Scheduler->Tick(deltaTime);
    }
    
    // Simple mechanics and thermal mock for demo purposes
    for (auto& comp : m_Components) {
        if (comp->area > 0.0f) {
            comp->stress = comp->appliedForce / comp->area;
        }
        
        if (comp->area > 0.0f && comp->thickness > 0.0f && comp->density > 0.0f) {
            float mass = comp->area * comp->thickness * comp->density;
            float specificHeat = 900.0f; // approx for Aluminum/Steel
            if (mass > 0.0f) {
                float tempDelta = (comp->heatInput * deltaTime) / (mass * specificHeat);
                comp->temperature += tempDelta;
            }
        }
        
        // Dissipate heat towards ambient
        if (comp->heatInput == 0.0f && comp->temperature > AMBIENT_TEMP_K) {
            float cooling = (comp->temperature - AMBIENT_TEMP_K) * 0.1f * deltaTime;
            comp->temperature -= cooling;
        }

        comp->stressRatio = (comp->yieldStrength > 0.0f) 
            ? std::min(1.0f, comp->stress / comp->yieldStrength)
            : 0.0f;
        comp->tempRatio = (MAX_TEMP_K > AMBIENT_TEMP_K)
            ? std::min(1.0f, (comp->temperature - AMBIENT_TEMP_K) / (MAX_TEMP_K - AMBIENT_TEMP_K))
            : 0.0f;
        comp->isDangerous = (comp->stressRatio > 0.8f) || (comp->tempRatio > 0.8f);
    }

    // Publish event for UI/visualization
    EventBus::GetInstance().Publish("SimulationTick", std::any());
}

void SimulationEngine::Shutdown() {
    if (m_Scheduler) {
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
        std::cout << "[SZM Simulation] Removed component ID: " << id << "\n";
        m_Components.erase(it);
    }
}

void SimulationEngine::ClearAllComponents() {
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

// Removed simple UpdateStress, UpdateTemperature, and UpdateDangerState 
// because the physics Solvers and CouplingManager handle this now.

} // namespace SZM
