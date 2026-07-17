# Fluid Statics and Dynamics Module - Integration Guide

## Quick Setup

### For C++ Integration

Add to your `CMakeLists.txt`:
```cmake
target_link_libraries(YourTarget SZMFluids)
```

### For Python Integration

```bash
cd /mnt/D/projects/szm_forge/src/Fluids/Python
```

Import in your Python code:
```python
from fluid_simulation import FluidSimulator, Fluid, DragModel
```

## C++ Usage Examples

### Example 1: Simple Pressure Calculation

```cpp
#include "Fluids/FluidStaticsEngine.hpp"

using namespace SZM::Fluids;

int main() {
    // Create engine with water
    FluidStaticsEngine engine(FluidProperties::Water());
    
    // Calculate pressure at 10 meters depth
    float depth = 10.0f;
    float pressure = engine.getPressureAtDepth(depth);
    
    std::cout << "Pressure at " << depth << "m: " 
              << pressure << " Pa" << std::endl;
    
    return 0;
}
```

### Example 2: Buoyancy Analysis

```cpp
#include "Fluids/FluidStaticsEngine.hpp"

using namespace SZM::Fluids;

int main() {
    FluidStaticsEngine engine(FluidProperties::Water());
    
    // Analyze floating object
    float objectVolume = 2.0f;  // m³
    float objectMass = 1500.0f;  // kg
    glm::vec3 centerOfGravity(0, -0.5f, 0);
    
    FloatingObjectResult result = engine.analyzeFloatingObject(
        objectVolume, objectMass, centerOfGravity
    );
    
    if (result.isFloating) {
        std::cout << "Object floats!" << std::endl;
        std::cout << "Submerged: " << result.submergedFraction * 100 << "%" << std::endl;
        std::cout << "Buoyant Force: " << result.buoyantForce.y << " N" << std::endl;
    } else {
        std::cout << "Object sinks!" << std::endl;
    }
    
    return 0;
}
```

### Example 3: Drag Force Calculation

```cpp
#include "Fluids/FluidDynamicsEngine.hpp"

using namespace SZM::Fluids;

int main() {
    FluidDynamicsEngine engine(FluidProperties::Air());
    
    // Calculate drag on sphere
    float velocity = 50.0f;  // m/s
    float referenceArea = 0.1f;  // m²
    
    DragForceResult result = engine.calculateDragForce(
        velocity, referenceArea, DragModel::Sphere
    );
    
    std::cout << "Velocity: " << velocity << " m/s" << std::endl;
    std::cout << "Drag Force: " << result.dragForce << " N" << std::endl;
    std::cout << "Reynolds Number: " << result.reynoldsNumber << std::endl;
    
    return 0;
}
```

## Python Usage Examples

### Example 1: Quick Pressure Calculation

```python
from fluid_simulation import FluidSimulator, Fluid

sim = FluidSimulator(Fluid.WATER)

# Get pressure at 50 meters depth
depth = 50
pressure = sim.get_pressure_at_depth(depth)
print(f"Pressure at {depth}m: {pressure:,.0f} Pa")

# Get absolute pressure (with atmosphere)
abs_pressure = sim.get_absolute_pressure(depth)
print(f"Absolute pressure: {abs_pressure:,.0f} Pa")
```

### Example 2: Floating Object Analysis

```python
from fluid_simulation import FluidSimulator, Fluid

sim = FluidSimulator(Fluid.WATER)

# Define object properties
volume = 5.0  # m³
mass = 3500  # kg

# Analyze
result = sim.analyze_floating_object(volume, mass)

print(f"Will float: {result.is_floating}")
print(f"Submerged: {result.submerged_fraction:.1%}")
print(f"Draft: {result.draft:.2f} m")
print(f"Buoyant Force: {result.buoyant_force[1]:.0f} N")
```

### Example 3: Aerodynamic Analysis

```python
from fluid_simulation import FluidSimulator, Fluid, DragModel

sim = FluidSimulator(Fluid.AIR)

# Wing specifications
wing_area = 50  # m²
velocity = 100  # m/s

# Calculate lift
lift = sim.get_lift_force(velocity, wing_area, lift_coefficient=1.0)

# Calculate drag
drag_result = sim.get_drag_force(
    velocity, wing_area, model=DragModel.FLAT_PLATE
)

print(f"Lift: {lift:.0f} N")
print(f"Drag: {drag_result.drag_force:.0f} N")
print(f"L/D Ratio: {lift / drag_result.drag_force:.1f}")
```

## Integration with Simulation System

### Adding Fluid Scenarios

Create a new scenario file in `src/Simulation/Scenarios/`:

```cpp
#include "Simulation/SimulationScenario.hpp"
#include "Fluids/FluidStaticsEngine.hpp"
#include "Physics/RigidBody.hpp"

class WaterPoolScenario : public SimulationScenario {
public:
    WaterPoolScenario() : SimulationScenario("Water Pool") {
        fluidEngine = new SZM::Fluids::FluidStaticsEngine(
            SZM::Fluids::FluidProperties::Water()
        );
    }
    
    void initialize() override {
        // Set up pool
        poolDepth = 2.0f;
        poolArea = 50.0f * 25.0f;
    }
    
    void update(float deltaTime) override {
        // Update simulation
    }
    
private:
    SZM::Fluids::FluidStaticsEngine* fluidEngine;
    float poolDepth;
    float poolArea;
};
```

### Extending PhysicsEngine for Fluid Forces

Modify `src/Physics/PhysicsEngine.hpp`:

```cpp
class PhysicsEngine {
    // ... existing code ...
    
    void applyFluidForces(float deltaTime);
    void setFluidSimulation(SZM::Fluids::FluidDynamicsEngine* engine);
    
private:
    SZM::Fluids::FluidDynamicsEngine* fluidEngine;
};
```

Implementation in `src/Physics/PhysicsEngine.cpp`:

```cpp
void PhysicsEngine::applyFluidForces(float deltaTime) {
    if (!fluidEngine) return;
    
    for (auto& body : rigidBodies) {
        if (body->isSubmerged()) {
            // Calculate drag
            float velocity = glm::length(body->getVelocity());
            auto dragResult = fluidEngine->calculateDragForce(
                velocity, 
                body->getReferenceArea()
            );
            
            // Apply drag force
            body->applyForce(-dragResult.dragDirection * dragResult.dragForce);
            
            // Calculate buoyancy
            float buoyancy = fluidEngine->getFluid().getDensity() * 
                           fluidEngine->getFluid().getGravity() * 
                           body->getSubmergedVolume();
            
            body->applyForce(glm::vec3(0, buoyancy, 0));
        }
    }
}
```

## Integration with WebAPI

Add endpoints to `src/WebAPI/FluidSimulationAPI.cpp`:

```cpp
// Calculate buoyancy
[this](const request& req) {
    auto params = parse_query(req);
    float volume = std::stof(params["volume"]);
    auto result = fluidEngine->getBuoyantForce(volume);
    
    return response(200, json_response({
        {"buoyant_force", result}
    }));
}

// Analyze floating object
[this](const request& req) {
    auto params = parse_query(req);
    float volume = std::stof(params["volume"]);
    float mass = std::stof(params["mass"]);
    
    auto result = fluidEngine->analyzeFloatingObject(volume, mass, {});
    
    return response(200, json_response({
        {"is_floating", result.isFloating},
        {"submerged_fraction", result.submergedFraction},
        {"buoyant_force", result.buoyantForce.y}
    }));
}
```

## Testing

### Unit Tests for C++

Create `tests/FluidStaticsTest.cpp`:

```cpp
#include <gtest/gtest.h>
#include "Fluids/FluidStaticsEngine.hpp"

using namespace SZM::Fluids;

TEST(FluidStaticsTest, PressureAtDepth) {
    FluidStaticsEngine engine(FluidProperties::Water());
    
    float depth = 10.0f;
    float pressure = engine.getPressureAtDepth(depth);
    
    // Expected: ρ * g * h = 998.2 * 9.81 * 10 ≈ 97927 Pa
    EXPECT_NEAR(pressure, 97927, 100);
}

TEST(FluidStaticsTest, Buoyancy) {
    FluidStaticsEngine engine(FluidProperties::Water());
    
    float volume = 1.0f;
    float buoyancy = engine.getBuoyantForce(volume);
    
    // Expected: ρ * g * V = 998.2 * 9.81 * 1 ≈ 9792 N
    EXPECT_NEAR(buoyancy, 9792, 100);
}
```

### Python Tests

Create `tests/test_fluids.py`:

```python
import unittest
from fluid_simulation import FluidSimulator, Fluid

class TestFluidStatics(unittest.TestCase):
    def setUp(self):
        self.sim = FluidSimulator(Fluid.WATER)
    
    def test_pressure_at_depth(self):
        depth = 10.0
        pressure = self.sim.get_pressure_at_depth(depth)
        # Expected: 998.2 * 9.81 * 10 ≈ 97927 Pa
        self.assertAlmostEqual(pressure, 97927, delta=100)
    
    def test_buoyancy(self):
        volume = 1.0
        buoyancy = self.sim.get_buoyant_force(volume)
        # Expected: 998.2 * 9.81 * 1 ≈ 9792 N
        self.assertAlmostEqual(buoyancy, 9792, delta=100)
    
    def test_floating_object(self):
        volume = 1.0
        mass = 800  # 800 kg, less than water
        result = self.sim.analyze_floating_object(volume, mass)
        self.assertTrue(result.is_floating)

if __name__ == '__main__':
    unittest.main()
```

## Performance Optimization

### For Large-Scale Simulations

1. **Use C++ for heavy calculations** - Physics calculations are faster in C++
2. **Cache results** - Store computed pressures/forces when they don't change
3. **Batch operations** - Calculate forces for multiple objects at once
4. **GPU Acceleration** - Particle systems with CUDA for fluid particles

### Profiling Tips

```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();

// Your calculation
float result = engine.calculateDragForce(velocity, area);

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
std::cout << "Calculation took " << duration.count() << " μs" << std::endl;
```

## Common Issues

### Issue: "Cannot find glm"
**Solution:** Ensure GLM is in your include path:
```bash
export GLM_INCLUDE_DIR=/path/to/glm
cmake ..
```

### Issue: Python numpy not found
**Solution:** Create a virtual environment:
```bash
python3 -m venv /mnt/D/projects/szm_forge/venv
source /mnt/D/projects/szm_forge/venv/bin/activate
pip install numpy
```

### Issue: Compilation errors with C++17/20
**Solution:** Use compiler flags:
```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## Next Steps

1. **Extend with pybind11** - Create Python bindings for C++ engine
2. **Add turbulence models** - Implement k-ε or k-ω models
3. **Multiphase flow** - Support air-water mixtures
4. **Real-time visualization** - Integrate with graphics module
5. **CFD solver** - Advanced flow field calculation

## Support & Documentation

- See `README.md` for detailed API reference
- Check `examples.py` for practical use cases
- Review `fluid_utils.py` for helper functions
- Refer to physics textbooks for equation details

---

**Module Version:** 1.0.0  
**Status:** Production Ready
