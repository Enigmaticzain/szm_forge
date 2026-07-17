# SZM Forge — Phase 1 Foundation Demo

**Status:** 🟢 **ACTIVE DEVELOPMENT**  
**Target:** Industrial simulation + machine-building platform  
**This Document:** Phase 1 achievements and roadmap

---

## 🎯 What Is SZM Forge?

SZM Forge is a **next-generation engineering platform** that combines:
- **Visual machine design** (drag-and-drop like LEGO)
- **Real physics simulation** (stress, thermal, electrical)
- **Material-aware behavior** (steel ≠ aluminum)
- **Failure prediction** (lifecycle + maintenance timing)

**Think:** Combine LEGO + professional engineering software + real physics.

---

## ✅ Phase 1 Achievements

### Core Systems Implemented

#### 1. **Simulation Engine** ✓
- Multi-component management
- Real-time physics calculation
- Event-based architecture (pub/sub)
- Stress calculation: `Stress = Force / Area`
- Thermal dynamics: `dT/dt = Q / (m·c)` + natural cooling
- Safety monitoring with threshold detection

#### 2. **Component System** ✓
- Modular component design
- Material properties (density, yield strength, thermal conductivity)
- State tracking (force, temperature, stress)
- Danger state classification
- JSON-serializable data model

#### 3. **Application Framework** ✓
- GLFW-based window management (cross-platform)
- Event bus system (decoupled subsystems)
- Input handling (keyboard, mouse)
- Delta-time physics stepping
- FPS diagnostics

#### 4. **Demo & Visualization** ✓
- Real-time component status dashboard
- Safety warnings (yield strength, thermal limits)
- Stress and temperature monitoring
- Color-coded danger states

---

## 🔬 Physics Engine (Phase 1)

### Implemented

```
For each timestep (Δt):
  1. Calculate stress:
       stress = applied_force / component_area
       
  2. Update temperature:
       ΔT = (heat_input · Δt) / (mass · heat_capacity)
       temperature += ΔT
       temperature -= (T - ambient) · cooling_rate · Δt
       
  3. Check safety:
       is_dangerous = (stress > yield_strength) OR (temp > max_temp)
       
  4. Publish state via EventBus
```

### Material Properties

Components have real material data:
- **Steel**: density 7850 kg/m³, yield strength 250 MPa
- **Aluminum**: density 2700 kg/m³, yield strength 40 MPa (extendable)
- **Copper**: high thermal conductivity for thermal simulation

---

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                   SZM Forge                         │
├─────────────────────────────────────────────────────┤
│                                                     │
│  ┌─────────────┐    ┌──────────────┐              │
│  │ Application │◄───►│ SimulationEng│              │
│  │  (GLFW)     │    │   (Physics)  │              │
│  └─────────────┘    └──────────────┘              │
│        │                    │                       │
│  ┌─────▼─────────┐  ┌─────▼──────────┐           │
│  │ Event Bus     │  │ Components     │           │
│  │ (Pub/Sub)     │  │ (Properties)   │           │
│  └───────────────┘  └────────────────┘           │
│        │                    │                       │
│  ┌─────▼──────────┐      ┌──▼─────────┐          │
│  │ Input Manager  │      │ Scene Graph│          │
│  │ (Keyboard)     │      │ (Hierarchy)│          │
│  └────────────────┘      └────────────┘          │
│                                                     │
│  [Future: Graphics Layer] [REST API] [UI]         │
└─────────────────────────────────────────────────────┘
```

---

## 🚀 How To Run The Demo

### Prerequisites
- CMake 3.21+
- C++20 compiler (GCC/Clang/MSVC)
- GLFW development headers (Linux: `libglfw3-dev`)

### Build

```bash
cd szm_forge
rm -rf build && mkdir build && cd build
cmake ..
cmake --build .
```

### Run

```bash
./SZM_Forge
```

**Output Example:**
```
╔════════════════════════════════════════════════════════════╗
║                    SZM FORGE — PHASE 1                      ║
║              Industrial Simulation + Design                 ║
╚════════════════════════════════════════════════════════════╝

Creating components...

[SZM Simulation] Added component: Steel Beam (ID: 1)
[SZM Simulation] Added component: Aluminum Plate (ID: 2)
[SZM Simulation] Added component: Copper Pipe (ID: 3)

Initial configuration:

  Component: Steel Beam (ID: 1)
    Applied Force:   50000.00 N
    Area:            100.00 cm²
    Calculated Stress: 500.00 MPa
    Yield Strength:   250.00 MPa
    Temperature:     25.00°C
    Heat Input:      100.00 W
    ⚠️  STATUS: DANGEROUS

  Component: Aluminum Plate (ID: 2)
    Applied Force:   30000.00 N
    Area:            150.00 cm²
    Calculated Stress: 200.00 MPa
    Yield Strength:   40.00 MPa
    Temperature:     25.00°C
    Heat Input:      50.00 W
    ⚠️  STATUS: DANGEROUS

  Component: Copper Pipe (ID: 3)
    Applied Force:   10000.00 N
    Area:            50.00 cm²
    Calculated Stress: 200.00 MPa
    Yield Strength:   250.00 MPa
    Temperature:     25.00°C
    Heat Input:      200.00 W
    ✓  STATUS: NOMINAL

Starting simulation...
[SZM Core] FPS: 60 | dt: 0.016667s
```

---

## 📊 Key Metrics

| Component | Force | Area | Stress | Yield | Status |
|-----------|-------|------|--------|-------|--------|
| Steel Beam | 50 kN | 100 cm² | 500 MPa | 250 MPa | ⚠️ FAIL |
| Aluminum Plate | 30 kN | 150 cm² | 200 MPa | 40 MPa | ⚠️ FAIL |
| Copper Pipe | 10 kN | 50 cm² | 200 MPa | 250 MPa | ✓ OK |

---

## 🔄 Phase 2 Roadmap (Optional)

### Immediate Next Steps
1. **Three.js Web Interface**
   - 3D scene with draggable components
   - Real-time visualization (stress = color)
   - Property editor panel

2. **REST API Layer**
   - JSON component serialization
   - Simulation state API
   - Load/save workspace

3. **Advanced Physics**
   - Kinematics (constraint solver)
   - Electrical networks
   - Multi-body dynamics

4. **UI/UX**
   - Component library (gears, motors, circuits)
   - Grid-based assembly canvas
   - Result graphs and reports

---

## 🧠 Design Decisions

### Why This Architecture?
- **Modular**: Each system (Physics, Input, Graphics, UI) is independent
- **Scalable**: Easy to add thermal, electrical, kinematic solvers
- **Event-Driven**: Components communicate via pub/sub, no tight coupling
- **Physics-First**: Simulation logic is core, graphics are optional
- **Real Materials**: Properties based on actual engineering data

### Why C++ + Three.js?
- **C++**: Performance-critical physics engine
- **Three.js**: Accessible, web-based 3D interface (no special software needed)
- **Separation**: Backend handles simulation, frontend handles visualization

---

## 📁 Project Structure

```
szm_forge/
├── src/
│   ├── main.cpp                    # Demo entry point
│   ├── Core/
│   │   ├── Application.hpp/cpp     # GLFW window + main loop
│   │   └── CMakeLists.txt
│   ├── Simulation/
│   │   ├── SimulationEngine.hpp/cpp # Physics engine
│   │   ├── SimulationComponent.hpp  # Component data model
│   │   └── CMakeLists.txt
│   ├── Input/
│   │   ├── InputManager.hpp/cpp
│   │   ├── EventBus.hpp            # Pub/sub system
│   │   └── ...
│   ├── Graphics/                   # (Placeholder for Phase 2)
│   ├── UI/                         # (Placeholder for Phase 2)
│   └── ... (other modules)
├── CMakeLists.txt                  # Build configuration
├── README.md                        # Project overview
└── PHASE1_DEMO.md                  # This file

```

---

## ✨ Conclusions

**What We've Built:**
- ✅ A working physics engine
- ✅ Multi-component simulation
- ✅ Real material properties
- ✅ Safety monitoring and diagnostics
- ✅ Modular, extensible architecture

**What Makes This Special:**
- Physics-driven (not just visual)
- Material-aware (properties affect behavior)
- Event-driven (scalable)
- Real engineering concepts (stress, temperature, yield)

**What's Next:**
- Web interface (Three.js)
- Component library
- Advanced physics (kinematics, electrical)
- AI suggestions (design optimization)

---

## 📞 Contact & Questions

For questions about the architecture, physics implementation, or roadmap:
- Check the Phase 1 goals: "Build, Define Reality, Simulate, Predict"
- Architecture diagrams above show how systems interact
- Component/Material data is extensible (add alloys, composites, etc.)

---

**Last Updated:** April 17, 2026  
**Version:** Phase 1 (Foundation)  
**Status:** 🟢 ACTIVE DEVELOPMENT
