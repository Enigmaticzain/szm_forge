# Fluid Statics and Dynamics Module

## Overview

The Fluid Statics and Dynamics module is a comprehensive physics library integrated into SZM Forge for engineers to simulate, create, and innovate with fluid mechanics. It provides both C++ performance for calculations and Python convenience for rapid prototyping and experimentation.

## Features

### Fluid Statics
- **Pressure Calculations**: Hydrostatic pressure at any depth
- **Buoyancy Analysis**: Archimedes' principle and buoyant force
- **Stability Assessment**: Floating object equilibrium and stability metrics
- **Surface Forces**: Hydrostatic forces on submerged surfaces
- **Container Analysis**: Pressure distribution in vessels

### Fluid Dynamics
- **Drag Forces**: Sphere, cylinder, flat plate, and custom geometries
- **Lift Forces**: Wing and airfoil calculations
- **Reynolds Number**: Flow regime classification (laminar/transitional/turbulent)
- **Bernoulli Equation**: Energy conservation in flow
- **Continuity Equation**: Flow rate calculations
- **Pipe Flow**: Pressure loss and friction factor
- **Flow Visualization**: Velocity fields and streamlines

### Materials & Fluids
Pre-configured properties for:
- Water (fresh/sea water at various temperatures)
- Air (various temperatures and altitudes)
- Oil (various types)
- Mercury
- Honey (extreme viscosity testing)

## Architecture

```
src/Fluids/
├── C++ Engine
│   ├── FluidProperties.hpp/cpp       - Fluid characteristic properties
│   ├── FluidStaticsEngine.hpp/cpp    - Hydrostatics calculations
│   ├── FluidDynamicsEngine.hpp/cpp   - Flow and drag calculations
│   ├── CMakeLists.txt
│   └── PyBindings/                   - Python bindings (future)
│
└── Python API
    ├── Python/
    │   ├── __init__.py               - Package initialization
    │   ├── fluid_simulation.py       - Main simulation class
    │   ├── fluid_utils.py            - Utility functions
    │   └── examples.py               - Engineering examples
    └── Documentation/
        └── README.md                 - This file
```

## Installation

### Python Usage (Recommended for Most Users)

```bash
cd /mnt/D/projects/szm_forge/src/Fluids/Python
import sys
sys.path.insert(0, '/mnt/D/projects/szm_forge/src/Fluids')

from fluid_simulation import FluidSimulator, Fluid
```

### C++ Integration

Include in your CMakeLists.txt:
```cmake
target_link_libraries(YourTarget SZMFluids)
```

## Quick Start

### Basic Buoyancy Calculation

```python
from fluid_simulation import FluidSimulator, Fluid

# Create simulator for water
sim = FluidSimulator(Fluid.WATER)

# Calculate buoyant force on 1 m³ object
volume = 1.0  # m³
buoyant_force = sim.get_buoyant_force(volume)
print(f"Buoyant force: {buoyant_force:.2f} N")
```

### Floating Object Analysis

```python
# Analyze if object floats or sinks
result = sim.analyze_floating_object(
    volume=2.0,      # m³
    mass=1500        # kg
)

print(f"Floats: {result.is_floating}")
print(f"Submerged: {result.submerged_fraction:.1%}")
print(f"Buoyant force: {result.buoyant_force[1]:.2f} N")
```

### Drag Force Calculation

```python
from fluid_simulation import DragModel

# Calculate drag on moving sphere
drag_result = sim.get_drag_force(
    velocity=5.0,            # m/s
    reference_area=1.0,      # m²
    model=DragModel.SPHERE
)

print(f"Drag force: {drag_result.drag_force:.2f} N")
print(f"Reynolds number: {drag_result.reynolds_number:.0f}")
print(f"Flow regime: {drag_result.flow_regime}")
```

### Pressure Analysis

```python
# Get pressure at depth
depth = 10.0  # meters
pressure = sim.get_pressure_at_depth(depth)
print(f"Pressure at {depth}m: {pressure:.0f} Pa")

# Get absolute pressure (including atmosphere)
abs_pressure = sim.get_absolute_pressure(depth)
print(f"Absolute pressure: {abs_pressure:.0f} Pa")

# Get pressure distribution
pressures = sim.get_pressure_distribution(max_depth=100, num_points=10)
```

### Bernoulli Equation

```python
# Apply Bernoulli between two points
result = sim.apply_bernoulli(
    h1=0,      # elevation at point 1
    v1=5.0,    # velocity at point 1
    p1=101325, # pressure at point 1
    h2=5,      # elevation at point 2
    p2=101325  # pressure at point 2
)

print(f"Total head: {result.total_head:.2f} m")
print(f"Velocity at point 2: {result.velocity_at_point2:.2f} m/s")
```

### Continuity Equation

```python
# Find velocity after pipe diameter change
area1 = 0.1  # m²
velocity1 = 2.0  # m/s
area2 = 0.05  # m² (smaller pipe)

velocity2 = sim.get_continuity_equation(area1, velocity1, area2)
print(f"Velocity in smaller pipe: {velocity2:.2f} m/s")
```

## API Reference

### FluidSimulator Class

#### Constructor
```python
FluidSimulator(fluid_type: Fluid = Fluid.WATER, gravity: float = 9.81)
```

#### Statics Methods

| Method | Description | Returns |
|--------|-------------|---------|
| `get_pressure_at_depth(depth)` | Gauge pressure at depth | float (Pa) |
| `get_absolute_pressure(depth)` | Absolute pressure at depth | float (Pa) |
| `get_buoyant_force(volume)` | Buoyant force on volume | float (N) |
| `analyze_floating_object(volume, mass)` | Complete floating analysis | BuoyancyResult |
| `get_hydrostatic_force(area, depth)` | Force on submerged surface | float (N) |
| `get_pressure_distribution(max_depth, num_points)` | Pressure profile | List[float] |

#### Dynamics Methods

| Method | Description | Returns |
|--------|-------------|---------|
| `get_reynolds_number(velocity, length)` | Reynolds number | float |
| `get_dynamic_pressure(velocity)` | Dynamic pressure | float (Pa) |
| `get_drag_force(velocity, area, model, cd)` | Drag force | DragResult |
| `get_lift_force(velocity, area, cl)` | Lift force | float (N) |
| `apply_bernoulli(h1, v1, p1, h2, p2)` | Bernoulli equation | BernoulliResult |
| `get_continuity_equation(a1, v1, a2)` | Continuity equation | float (m/s) |
| `get_orifice_flow_rate(area, pressure_diff)` | Orifice flow | float (m³/s) |
| `get_pump_power(flow_rate, pressure_diff)` | Pump power | float (W) |
| `get_viscous_resistance(length, diameter, velocity)` | Pipe friction loss | float (Pa) |

### Fluid Types

```python
Fluid.WATER      # Fresh water @ 20°C
Fluid.AIR        # Air @ 15°C, sea level
Fluid.OIL        # Generic oil @ 20°C
Fluid.MERCURY    # Mercury @ 20°C
Fluid.HONEY      # Honey @ 20°C (very viscous)
Fluid.CUSTOM     # User-defined properties
```

### Drag Models

```python
DragModel.SPHERE       # For spheres
DragModel.CYLINDER     # For cylinders
DragModel.FLAT_PLATE   # For flat surfaces
DragModel.CUSTOM       # Custom drag coefficient
```

## Utility Functions

### FlowVisualizer

```python
from fluid_utils import FlowVisualizer

# Create uniform velocity field
field = FlowVisualizer.create_velocity_field(
    flow_direction=(1, 0, 0),
    flow_velocity=5.0,
    grid_size=10
)

# Create field with wake effects
wake_field = FlowVisualizer.create_wake_field(
    object_position=(0, 0, 0),
    flow_direction=(1, 0, 0),
    flow_velocity=5.0
)

# Calculate streamlines
streamlines = FlowVisualizer.get_streamlines(
    field,
    start_positions=[(0, 0, 0), (0, 1, 0)],
    num_steps=100
)
```

### GeometryHelpers

```python
from fluid_utils import GeometryHelpers

# Get sphere properties
sphere = GeometryHelpers.get_sphere_properties(radius=0.5)
print(sphere['volume'])       # m³
print(sphere['surface_area']) # m²

# Get cylinder properties
cylinder = GeometryHelpers.get_cylinder_properties(radius=0.5, height=2.0)

# Get box properties
box = GeometryHelpers.get_rectangular_box_properties(1.0, 2.0, 0.5)
```

### ThermodynamicCalculator

```python
from fluid_utils import ThermodynamicCalculator

# Temperature effect on viscosity
visc = ThermodynamicCalculator.get_temperature_dependent_viscosity(
    base_viscosity=0.001,
    base_temp=20,
    new_temp=40,
    fluid_type="water"
)

# Temperature effect on density
density = ThermodynamicCalculator.get_temperature_dependent_density(
    base_density=1000,
    base_temp=20,
    new_temp=40
)
```

## Engineering Examples

### 1. Submarine Stability
See `example_submarine_stability()` in examples.py
- Neutral buoyancy calculations
- Ballast system sizing
- Pressure analysis at depth

### 2. Pipe Flow Design
See `example_pipe_flow_design()` in examples.py
- Flow rate calculations
- Reynolds number determination
- Pressure drop analysis
- Pump power requirements

### 3. Swimming Pool Design
See `example_swimming_pool_design()` in examples.py
- Hydrostatic force calculations
- Wall and floor pressure analysis
- Structural requirements

### 4. Wing Aerodynamics
See `example_wing_aerodynamics()` in examples.py
- Lift and drag calculations
- Altitude effects on aerodynamics
- Takeoff performance analysis

### 5. Oil Pipeline
See `example_oil_pipeline()` in examples.py
- High-viscosity flow analysis
- Temperature effects on viscosity
- Multi-station pump requirements

### 6. Floating Barge
See `example_floating_barge()` in examples.py
- Floating stability analysis
- Draft and freeboard calculations
- Cargo capacity assessment

## Running Examples

```bash
cd /mnt/D/projects/szm_forge/src/Fluids/Python

# Run the main example suite
python examples.py

# Or import and run individually
python -c "from examples import example_submarine_stability; example_submarine_stability()"
```

## Physics Equations

### Statics

**Hydrostatic Pressure:**
```
P = ρ * g * h
```

**Buoyant Force (Archimedes' Principle):**
```
F_b = ρ * g * V_displaced
```

**Piezometric Head:**
```
H = h + v²/(2g) + P/(ρg)
```

### Dynamics

**Drag Force:**
```
F_d = C_d * q * A = C_d * (1/2 * ρ * v²) * A
```

**Lift Force:**
```
F_l = C_l * q * A
```

**Reynolds Number:**
```
Re = ρ * v * L / μ
```

**Bernoulli Equation:**
```
h₁ + v₁²/(2g) + P₁/(ρg) = h₂ + v₂²/(2g) + P₂/(ρg)
```

**Continuity Equation:**
```
A₁ * v₁ = A₂ * v₂
```

## Common Fluid Properties

| Fluid | Density (kg/m³) | Viscosity (Pa·s) | Temperature |
|-------|-----------------|------------------|-------------|
| Water | 998.2 | 0.001002 | 20°C |
| Air | 1.225 | 1.81e-5 | 15°C |
| Oil | 900 | 0.1 | 20°C |
| Mercury | 13546 | 0.001523 | 20°C |
| Honey | 1420 | 10.0 | 20°C |

## Performance Considerations

- C++ engine handles heavy calculations for speed
- Python API provides flexibility for rapid development
- Suitable for real-time applications with GPU support
- Scalable from single-object analysis to complex systems

## Integration with SZM Forge

This module integrates with:
- **Physics Engine**: Fluid forces on rigid bodies
- **Simulation System**: Fluid scenarios and environments
- **Materials Module**: Fluid property definitions
- **WebAPI**: REST endpoints for simulations
- **Visualization**: 3D rendering of flow fields and results

## Future Enhancements

- [ ] Turbulence modeling (k-ε, k-ω)
- [ ] Multiphase flow (air-water mixtures)
- [ ] Compressible flow at high Mach numbers
- [ ] Thermal effects integration
- [ ] GPU acceleration for large domains
- [ ] Real-time simulation in 3D viewport
- [ ] Advanced material property database
- [ ] CFD solver integration

## Contributing

To extend the module:
1. Add new fluid types to `FluidProperties`
2. Implement specialized engines for new physics
3. Create Python wrappers for C++ engines
4. Add test cases for accuracy validation
5. Document with examples

## References

- "Fluid Mechanics Fundamentals and Applications" - Cengel & Cimbala
- "Introduction to Fluid Mechanics" - Fox, McDonald & Pritchard
- Crane Co. Technical Paper "Flow of Fluids Through Valves, Fittings, and Pipe"
- NACA Reports on Aerodynamic Coefficients

## License

Part of SZM Forge project. See LICENSE file for details.

## Support

For questions or issues:
1. Check examples.py for similar use cases
2. Review API reference section
3. Consult physics equations section
4. Contact SZM Forge team

---

**Module Version:** 1.0.0  
**Last Updated:** May 2026  
**Status:** Production Ready
