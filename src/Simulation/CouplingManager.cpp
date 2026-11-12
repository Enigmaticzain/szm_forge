#include "CouplingManager.hpp"
#include "SimulationComponent.hpp"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace SZM {

namespace {
std::mutex s_CouplingMutex;
std::unordered_map<const CouplingManager*, std::unordered_map<SimulationComponent*, float>>
    s_ThermalContributions;
} // namespace

void CouplingManager::Initialize() {
    std::cout << "[CouplingManager] Initialized.\n";
}

void CouplingManager::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(s_CouplingMutex);
        m_CoupledComponents.clear();
        s_ThermalContributions.erase(this);
    }
    std::cout << "[CouplingManager] Shutdown.\n";
}

void CouplingManager::RegisterComponent(SimulationComponent* comp) {
    if (!comp) return;
    std::lock_guard<std::mutex> lock(s_CouplingMutex);
    auto it = std::find(m_CoupledComponents.begin(), m_CoupledComponents.end(), comp);
    if (it == m_CoupledComponents.end()) {
        m_CoupledComponents.push_back(comp);
    }
}

void CouplingManager::UnregisterComponent(SimulationComponent* comp) {
    if (!comp) return;
    std::lock_guard<std::mutex> lock(s_CouplingMutex);
    std::erase(m_CoupledComponents, comp);
    auto it = s_ThermalContributions.find(this);
    if (it != s_ThermalContributions.end()) {
        it->second.erase(comp);
    }
}

void CouplingManager::ClearComponents() {
    std::lock_guard<std::mutex> lock(s_CouplingMutex);
    m_CoupledComponents.clear();
    s_ThermalContributions.erase(this);
}

void CouplingManager::PreStepCoupling(double dt) {
    // Before solvers run, map data between them.
    // e.g. Thermal expansion loads mapped to FEA mechanical solver.
    TransferThermalToMechanical(dt);
}

void CouplingManager::PostStepCoupling(double dt) {
    // After solvers run, map data back.
    // e.g. Fluid convection/heat transfer applied to thermal boundaries.
    TransferFluidToThermal(dt);
}

void CouplingManager::TransferThermalToMechanical(double /*dt*/) {
    std::lock_guard<std::mutex> lock(s_CouplingMutex);
    auto& contributions = s_ThermalContributions[this];
    // Mock for Phase 2: Calculate thermal stress/expansion on component
    for (auto* comp : m_CoupledComponents) {
        if (!comp) continue;
        // Assume a baseline temp of 293.15K (20C)
        double deltaT = comp->temperature - 293.15;
        float newThermal = 0.0f;
        if (deltaT > 1.0) { // arbitrary small threshold
            // In a real system, this translates to nodal forces or strain offsets
            // Here we just increase the stress proxy based on thermal expansion constraint
            // sigma = E * alpha * deltaT
            // Young's modulus ~ 200GPa for steel, alpha ~ 12e-6
            double E = 200e9;
            double alpha = 12e-6;
            double thermalStress = E * alpha * deltaT;
            newThermal = static_cast<float>(thermalStress * 0.01); // scaled down for demo
        }

        float prevThermal = 0.0f;
        if (auto it = contributions.find(comp); it != contributions.end()) {
            prevThermal = it->second;
        }
        // Remove previous frame's thermal offset before applying the current step's offset
        comp->stress = std::max(0.0f, comp->stress - prevThermal) + newThermal;
        if (newThermal > 0.0f) {
            contributions[comp] = newThermal;
        } else {
            contributions.erase(comp);
        }
    }
}

void CouplingManager::TransferFluidToThermal(double dt) {
    std::lock_guard<std::mutex> lock(s_CouplingMutex);
    // Mock for Phase 2: Fluids cooling the part (convection)
    for (auto* comp : m_CoupledComponents) {
        if (!comp) continue;
        // If heat input is 0, simulate convection cooling
        if (comp->heatInput < 1e-3) {
            double h = 10.0; // Convection coefficient W/(m^2*K)
            double T_env = 293.15;
            double area = comp->area;
            
            double Q_conv = h * area * (comp->temperature - T_env);
            
            // Simple Euler integration for component temp
            double mass = comp->density * comp->area * comp->thickness;
            double Cp = 500.0; // Specific heat J/(kg*K) roughly for steel
            
            if (mass > 0.0) {
                double dT = -(Q_conv / (mass * Cp)) * dt;
                comp->temperature += static_cast<float>(dT);
            }
        }
    }
}

} // namespace SZM
