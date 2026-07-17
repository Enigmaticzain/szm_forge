#include "CouplingManager.hpp"
#include "SimulationComponent.hpp"
#include <iostream>
#include <algorithm>

namespace SZM {

void CouplingManager::Initialize() {
    std::cout << "[CouplingManager] Initialized.\n";
}

void CouplingManager::Shutdown() {
    m_CoupledComponents.clear();
    std::cout << "[CouplingManager] Shutdown.\n";
}

void CouplingManager::RegisterComponent(SimulationComponent* comp) {
    if (!comp) return;
    auto it = std::find(m_CoupledComponents.begin(), m_CoupledComponents.end(), comp);
    if (it == m_CoupledComponents.end()) {
        m_CoupledComponents.push_back(comp);
    }
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
    // Mock for Phase 2: Calculate thermal stress/expansion on component
    for (auto* comp : m_CoupledComponents) {
        // Assume a baseline temp of 293.15K (20C)
        double deltaT = comp->temperature - 293.15;
        if (deltaT > 1.0) { // arbitrary small threshold
            // In a real system, this translates to nodal forces or strain offsets
            // Here we just increase the stress proxy based on thermal expansion constraint
            // sigma = E * alpha * deltaT
            // Young's modulus ~ 200GPa for steel, alpha ~ 12e-6
            double E = 200e9;
            double alpha = 12e-6;
            double thermalStress = E * alpha * deltaT;
            
            // Add thermal stress to applied stress (very naive 1D proxy)
            comp->stress += static_cast<float>(thermalStress * 0.01); // scaled down for demo
        }
    }
}

void CouplingManager::TransferFluidToThermal(double dt) {
    // Mock for Phase 2: Fluids cooling the part (convection)
    for (auto* comp : m_CoupledComponents) {
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
