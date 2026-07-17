# SZM Forge

**Physics-Driven Engineering Platform — C++20**

**STATUS: Phase 1 Demo Ready — AI Integrated**

**VISION:** Build machines like LEGO + simulate real physics + predict failures
**THIS PHASE:** Working stress and thermal safety simulation + autonomous AI assistant

---

## Quick Start (Presentation Mode)

```bash
cd szm_forge
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
```

The default build runs a terminal-based mechanical demo with three scenarios:
1. Normal operation
2. Mechanical overload
3. Thermal overload

---

## Desktop Mode with React UI

To build with the modern React-based UI (default for desktop mode):

```bash
cd szm_forge
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build --parallel
./build/SZM_Forge
```

The desktop mode uses the **new ui/** React app (default):

- **Theme 2 — Modern** — 3D viewport, project hierarchy, analysis, AI workspace
- **Theme 2 — Classic (Theme 1)** — Telemetry / FEA dashboard layout (switch in Settings → Appearance)
- **Tooltips** — Global on/off in Settings → Accessibility; works in both themes
- **C++ backend** — Simulation, Workshop3D, WebAPI on localhost:8000

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build --parallel
./build/SZM_Forge
```

Dev UI only: `cd "new ui" && npm run dev` → http://localhost:3000

Legacy ImGui workshop fallback: `SZM_NATIVE_WORKSHOP_UI=1 ./build/SZM_Forge`  
Older UI snapshots: see [old_versions/README.md](old_versions/README.md)

## Project Structure

```
szm_forge/
├── CMakeLists.txt                   ← Root build (wires all modules)
├── config/szm_forge.json            ← Module registry & project config
├── assets/shaders/
│   ├── PBR.vert                     ← Cook-Torrance BRDF vertex shader
│   └── PBR.frag                     ← PBR fragment shader (GGX/Smith/Fresnel)
├── external/imgui/                  ← Clone docking branch here
└── src/
    ├── main.cpp                     ← Unified entry point (all modes)
    ├── Core/                        ← Application singleton + main loop
    ├── Input/                       ← EventBus, Event system, InputManager
    ├── Graphics/                    ← GraphicsContext (OGL/Vulkan), Camera,
    │                                   MeshGenerator, PBR shaders
    ├── UI/                          ← UIManager, DockSpace, panels
    ├── Data/                        ← UUID-128, SceneGraph, SceneGraphManager
    ├── Math/                        ← ConstraintSolver2D (Newton-Raphson, Eigen)
    ├── Geometry/                    ← SolidBody (B-Rep), Extrude, Revolve, CSG
    ├── Materials/                   ← IMaterial, MaterialDatabase (thread-safe)
    ├── Physics/                     ← RigidBody (inertia from mesh), GJK+EPA
    ├── Assembly/                    ← AssemblyGraph, BOM, KinematicSolver,
    │                                   InterferenceChecker (multi-threaded)
    └── FEA/                         ← FEADataTypes, LinearStaticSolver (PCG),
                                        VolumetricMesher (Gmsh)
```

---

## Module Status — v0.3.0

| ID  | Module                    | Domain        | Status      | Library                   | v    |
|-----|---------------------------|---------------|-------------|---------------------------|------|
| 010 | Core / Window             | Control       | ✅ Complete | `SZM_Core`                | 0.1  |
| 020 | UI / ImGui DockSpace      | Control       | ✅ Complete | `SZM_UI_Module`           | 0.1  |
| 030 | Input / Events / EventBus | Control       | ✅ Complete | `AppCore_Input_Event_Bus` | 0.1  |
| 040 | Graphics (OGL+Vk+Camera)  | Control       | ✅ Complete | `SZM_Graphics_Module`     | 0.2  |
| 050 | Scene Graph / Data        | Mechanical    | ✅ Complete | `DataCore_SceneGraph`     | 0.1  |
| 060 | 2D Constraint Solver      | Math          | ✅ Complete | `SZM_Constraint_Solver`   | 0.1  |
| 070 | Geometry Ops (B-Rep)      | Mechanical    | ⚠️ Partial  | `SZM_Geometry_Module`     | 0.1  |
| 110 | Material Database         | All domains   | ✅ Complete | `SZM_Materials_Module`    | 0.2  |
| 120 | Assembly + Kinematics     | Mechanical    | ✅ Complete | `SZM_Assembly_Module`     | 0.2  |
| 130 | Physics (RigidBody/GJK)   | Mechanical    | ✅ Complete | `SZM_Physics_Module`      | 0.2  |
| 320 | FEA Linear Static Solver  | Mechanical    | ✅ Complete | `SZM_FEA_Module`          | 0.2  |
| 300 | Thermal Engine            | Thermal       | 🔲 Pending  | `SZM_Thermal_Module`      | -    |
| 400 | Electrical Engine         | Electrical    | 🔲 Pending  | `SZM_Electrical_Module`   | -    |
| 500 | CEA AI Assistant          | Control       | ✅ Complete | `SZM_AI_Module`           | 0.3  |

---

## Build Modes

### 1. Lightweight Presentation Mode (default)
No GLFW, Eigen, or ImGui dependencies required.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
```

Run simulation demo tests:

```bash
./build/SZM_TestRunner
```

### 2. Full Desktop Mode (optional)
Builds the desktop stack with Graphics/UI/Core modules.

Required dependencies:
1. GLFW/X11 development packages
2. Eigen3 >= 3.4
3. Dear ImGui docking branch cloned into `external/imgui`

```bash
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel
./build-full/SZM_Forge
```

---

## Adding a New Module

1. Create `src/<ModuleName>/` with `.hpp` / `.cpp` files
2. Add `CMakeLists.txt` inside defining `SZM_<ModuleName>_Module`
3. Add `add_subdirectory(src/<ModuleName>)` to root `CMakeLists.txt`
4. Link to `SZM_Forge` and `SZM_TestRunner` targets
5. Hook tick into `Application::TickSystems()` in `src/Core/Application.cpp`
6. Publish results via `SZM::EventBus::GetInstance().Publish("YourEvent", payload)`
7. Register status in `config/szm_forge.json`

---

## Architecture Principles

| Principle | Implementation |
|-----------|----------------|
| **Singleton Host** | `Application` owns the OS window, delta-time, and system tick |
| **Zero-coupling** | All modules communicate through `EventBus` — no raw cross-module pointers |
| **Load-once** | `MaterialDatabase` and `BasePart` registries follow the flyweight pattern |
| **Thread-safe reads** | `MaterialDatabase` uses `std::shared_mutex` — FEA threads read concurrently |
| **Manifold guarantee** | Every geometry operation calls `SolidBody::Stitch()` + `VerifyManifold()` |
| **Stable numerics** | All iterative solvers use Eigen BDCSVD or PCG — graceful failure on rank deficiency |
| **Physics accuracy** | Inertia tensors computed analytically from mesh via signed-tetrahedron integration |

---

## New in v0.3.0

- **CEA AI Assistant** — Integrated AI module with design analysis, failure prediction, and multi-agent system
- **AI Engine** — Design recommendations, reliability analysis, multi-objective optimization (cost/weight/reliability)
- **Agent System** — 7 specialized agents (Planner, Engineering, Review, Research, Memory, Simulation, Optimization)
- **Lightweight Build** — AI module now available in terminal mode without GUI dependencies
- **Predictions Module** — Failure predictor with feature extraction and stress analysis

## New in v0.2.0

- **Camera** — quaternion trackball orbit/pan/zoom, Perspective ↔ Orthographic, gimbal-lock-free
- **MeshGenerator** — B-Rep→GPU tessellation with chordal-error dynamic LOD
- **PBR Shaders** — Cook-Torrance BRDF (GGX · Smith · Fresnel-Schlick), HDR Reinhard tonemapping
- **MaterialDatabase** — thread-safe singleton with Steel, Al-6061, Ti-6Al-4V built-in
- **AssemblyGraph** — instance-based part graph, BOM generation, mass/CG computation, instanced draw data
- **KinematicSolver** — 3-D Newton-Raphson mate solver using Eigen Isometry3d (6-DOF per body)
- **InterferenceChecker** — multi-threaded `std::jthread` AABB broad phase + Boolean narrow phase
- **RigidBody** — mass & inertia tensor from triangle mesh (Mirtich/Eberly polynomial integration)
- **CollisionDetection** — GJK + EPA narrow phase with warm-starting contact cache
- **FEA LinearStaticSolver** — global sparse [K], Preconditioned Conjugate Gradient, Von Mises post-processing
- **VolumetricMesher** — Gmsh-based Tet4/Tet10 generation with micro-edge healing

---

*SZM Forge — Integration Architecture | v0.2.0 | C++20 Desktop Engineering Suite*
