# SZM Forge — Phase 1 MVP: Simulation Engine

## What Was Built

A minimal, event-driven simulation engine that runs the core physics loop:

```
for each frame:
    stress = force / area
    temperature += (heat - dissipation) * dt
    isDangerous = (stress > 80% yield) OR (temp > 80% max)
    publish "SimulationTick" event
```

## Files Created

### Core Module (`src/Simulation/`)

1. **SimulationComponent.hpp**
   - Struct holding component properties: force, area, material (density, yield strength, thermal conductivity)
   - Calculated state: stress, temperature, danger flags
   - Uses project's existing `Geometry::Vector3` (no external deps)

2. **SimulationEngine.hpp/cpp**
   - Singleton managing component lifecycle
   - `AddComponent()` / `RemoveComponent()` / `GetComponent()`
   - `Tick(deltaTime)` — runs physics calculations
   - Publishes `"SimulationTick"` event via EventBus
   - Constants: MAX_TEMP_K = 373.15K (100°C), HEAT_DISSIPATION_RATE = 0.1 W/K

3. **CMakeLists.txt**
   - Builds `SZM_Simulation_Module` library
   - Links to project's existing modules

### Integration

- **src/Core/Application.cpp** — hooked `SimulationEngine::Tick()` into `TickSystems()`
- **CMakeLists.txt (root)** — added `add_subdirectory(src/Simulation)` and linked to main executable

### Testing

- **tests/simulation_test.cpp** — standalone test demonstrating:
  - Component creation (Steel, Aluminum)
  - Force/heat application
  - Danger state detection
  - Output: stress (MPa), temperature (°C), danger flag

## Physics Model

### Stress Calculation
```cpp
stress = appliedForce / area
stressRatio = stress / yieldStrength  // [0..1]
```

### Temperature Calculation
```cpp
mass = density * area * 0.01f  // assume 1cm thickness
dissipation = HEAT_DISSIPATION_RATE * (temp - AMBIENT_TEMP)
dT/dt = (heatInput - dissipation) / mass
temp += dT * dt
```

### Danger Detection
```cpp
isDangerous = (stressRatio > 0.8) OR (tempRatio > 0.8)
```

## How to Use

### Add a Component
```cpp
uint32_t id = SimulationEngine::GetInstance().AddComponent("My Part");
auto* comp = SimulationEngine::GetInstance().GetComponent(id);
comp->area = 0.01f;           // 10 cm²
comp->yieldStrength = 250e6f; // 250 MPa
```

### Apply Load
```cpp
SimulationEngine::GetInstance().SetForce(id, 5000.0f);  // 5 kN
SimulationEngine::GetInstance().SetHeatInput(id, 50.0f); // 50 W
```

### Listen to Simulation Events
```cpp
EventBus::GetInstance().Subscribe("SimulationTick", [](const std::any& p) {
    // Render updated component states
    // Update UI danger indicators
});
```

## Next Steps (Phase 2)

1. **UI Panel** — display component list with color-coded danger zones
2. **3D Visualization** — render components with stress/temp heatmaps
3. **Component Builder** — click-to-add UI for building machines
4. **Material Library** — integrate with existing `MaterialDatabase`
5. **Advanced Physics** — integrate with existing `Physics::RigidBody` and `FEA` modules

## Architecture Notes

- **Zero coupling** — all communication via EventBus
- **Minimal dependencies** — uses only project's existing `Geometry::Vector3`
- **Extensible** — easy to add thermal/electrical domains later
- **Thread-safe ready** — can be extended with `std::shared_mutex` like `MaterialDatabase`

---

**Status:** ✅ Compiles | ✅ Runs | ✅ Event-driven | Ready for UI integration
