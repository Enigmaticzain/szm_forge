#pragma once

/**
 * @file INTEGRATION_GUIDE.md
 * @brief How to hook SimulationEngine into UI/Graphics for Phase 2
 */

/*

## Quick Integration Checklist

### 1. Subscribe to Simulation Events (in UIManager or a new SimulationPanel)

```cpp
#include "Simulation/SimulationEngine.hpp"
#include "Input/EventBus.hpp"

class SimulationPanel : public IPanel {
public:
    void Init() override {
        EventBus::GetInstance().Subscribe("SimulationTick", 
            [this](const std::any& p) { this->OnSimulationTick(); });
    }

private:
    void OnSimulationTick() {
        auto& engine = SimulationEngine::GetInstance();
        for (const auto& comp : engine.GetComponents()) {
            // Update UI with comp->stress, comp->temperature, comp->isDangerous
        }
    }
};
```

### 2. Render Component States (in Graphics module)

```cpp
// In MeshGenerator or a new SimulationRenderer:
for (const auto& comp : SimulationEngine::GetInstance().GetComponents()) {
    glm::vec3 color = comp->isDangerous 
        ? glm::vec3(1.0f, 0.0f, 0.0f)  // Red = danger
        : glm::vec3(0.0f, 1.0f, 0.0f); // Green = safe
    
    DrawCube(comp->position, color);
}
```

### 3. Add UI Controls (ImGui DockSpace)

```cpp
// In UIManager::Render():
if (ImGui::Begin("Simulation Control")) {
    if (ImGui::Button("Add Component")) {
        SimulationEngine::GetInstance().AddComponent("New Part");
    }
    
    ImGui::SliderFloat("Force (N)", &selectedForce, 0.0f, 10000.0f);
    ImGui::SliderFloat("Heat (W)", &selectedHeat, 0.0f, 500.0f);
    
    if (ImGui::Button("Apply")) {
        SimulationEngine::GetInstance().SetForce(selectedId, selectedForce);
        SimulationEngine::GetInstance().SetHeatInput(selectedId, selectedHeat);
    }
    
    ImGui::End();
}
```

### 4. Display Component List

```cpp
if (ImGui::Begin("Components")) {
    for (const auto& comp : SimulationEngine::GetInstance().GetComponents()) {
        ImGui::Text("ID: %u | %s", comp->id, comp->name.c_str());
        ImGui::Text("  Stress: %.1f MPa", comp->stress / 1e6f);
        ImGui::Text("  Temp: %.1f°C", comp->temperature - 273.15f);
        ImGui::TextColored(
            comp->isDangerous ? ImVec4(1,0,0,1) : ImVec4(0,1,0,1),
            "  Status: %s", comp->isDangerous ? "DANGER" : "OK"
        );
    }
    ImGui::End();
}
```

## Material Integration

Once you want to use real materials:

```cpp
#include "Materials/MaterialDatabase.hpp"

auto* comp = SimulationEngine::GetInstance().GetComponent(id);
auto* material = MaterialDatabase::GetInstance().GetMaterial("Steel");

if (material) {
    comp->density = material->GetDensity();
    comp->yieldStrength = material->GetYieldStrength();
    comp->thermalConductivity = material->GetThermalConductivity();
}
```

## Physics Integration (Phase 3+)

When ready to integrate with RigidBody/FEA:

```cpp
// In SimulationEngine::Tick():
for (auto& comp : m_Components) {
    // 1. Existing stress/temp calculations
    UpdateStress(comp);
    UpdateTemperature(comp, dt);
    
    // 2. Later: integrate with RigidBody
    // Physics::RigidBody rb = CreateRigidBodyFromComponent(comp);
    // rb.ApplyForce(comp->appliedForce);
    
    // 3. Later: integrate with FEA
    // FEA::LinearStaticSolver solver;
    // solver.Solve(rb.GetMesh(), comp->appliedForce);
    // comp->stress = solver.GetMaxVonMises();
}
```

## Event Flow Diagram

```
Application::Run()
    ↓
Application::TickSystems()
    ↓
SimulationEngine::Tick(dt)
    ├─ UpdateStress()
    ├─ UpdateTemperature()
    ├─ UpdateDangerState()
    └─ EventBus::Publish("SimulationTick")
        ↓
    UIManager::OnSimulationTick()
        ├─ Update ImGui panels
        └─ Trigger Graphics redraw
        ↓
    Graphics::Render()
        └─ Draw components with danger colors
```

## Testing the Integration

```bash
# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run main app
./build/SZM_Forge

# Run simulation test
./build/SZM_TestRunner  # (if you add simulation_test.cpp to CMakeLists)
```

*/
