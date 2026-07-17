#include <iostream>
#include "Simulation/SimulationEngine.hpp"

using namespace SZM;

int main() {
    std::cout << "\n=== SZM Forge Simulation MVP Test ===\n\n";

    // Initialize engine
    SimulationEngine::GetInstance().Init();

    // Create a steel component
    uint32_t comp1 = SimulationEngine::GetInstance().AddComponent("Steel Beam");
    
    // Create an aluminum component
    uint32_t comp2 = SimulationEngine::GetInstance().AddComponent("Aluminum Plate");
    
    // Get components and modify properties
    auto* steel = SimulationEngine::GetInstance().GetComponent(comp1);
    auto* aluminum = SimulationEngine::GetInstance().GetComponent(comp2);
    
    if (steel) {
        steel->area = 0.01f;           // 10 cm²
        steel->yieldStrength = 250e6f; // 250 MPa
        steel->density = 7850.0f;      // kg/m³
    }
    
    if (aluminum) {
        aluminum->area = 0.02f;        // 20 cm²
        aluminum->yieldStrength = 70e6f; // 70 MPa
        aluminum->density = 2700.0f;   // kg/m³
    }

    std::cout << "\n--- Scenario 1: Normal Load ---\n";
    SimulationEngine::GetInstance().SetForce(comp1, 1000.0f);  // 1 kN
    SimulationEngine::GetInstance().SetHeatInput(comp1, 10.0f); // 10 W
    
    for (int i = 0; i < 3; ++i) {
        SimulationEngine::GetInstance().Tick(0.016); // 60 FPS
        
        if (steel) {
            std::cout << "  Frame " << i << ": Stress=" << steel->stress / 1e6f << " MPa, "
                      << "Temp=" << steel->temperature - 273.15f << "°C, "
                      << "Dangerous=" << (steel->isDangerous ? "YES" : "NO") << "\n";
        }
    }

    std::cout << "\n--- Scenario 2: Overload ---\n";
    SimulationEngine::GetInstance().SetForce(comp1, 5000.0f);  // 5 kN (overload)
    SimulationEngine::GetInstance().SetHeatInput(comp1, 100.0f); // 100 W (high heat)
    
    for (int i = 0; i < 3; ++i) {
        SimulationEngine::GetInstance().Tick(0.016);
        
        if (steel) {
            std::cout << "  Frame " << i << ": Stress=" << steel->stress / 1e6f << " MPa, "
                      << "Temp=" << steel->temperature - 273.15f << "°C, "
                      << "Dangerous=" << (steel->isDangerous ? "YES" : "NO") << "\n";
        }
    }

    std::cout << "\n--- Component List ---\n";
    for (const auto& comp : SimulationEngine::GetInstance().GetComponents()) {
        std::cout << "  ID=" << comp->id << ", Name=" << comp->name 
                  << ", Stress=" << comp->stress / 1e6f << " MPa"
                  << ", Temp=" << comp->temperature - 273.15f << "°C\n";
    }

    SimulationEngine::GetInstance().Shutdown();
    std::cout << "\n=== Test Complete ===\n\n";

    return 0;
}
