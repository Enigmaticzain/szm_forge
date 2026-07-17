# SZM Forge — Integrated Architecture (v0.3.0)

## Integration Status: Phase 1 + Phase 2 + Advanced Modules

This document outlines how the new code modules integrate with the existing SZM Forge MVP.

---

## Module Integration Map

### Existing (Phase 1 & 2)
- ✅ `src/Simulation/` — Core physics engine
- ✅ `src/UI/panels/` — ImGui panels
- ✅ `src/Core/Application.cpp` — Main loop

### New (Code Files)
- 🔧 `src/Graphics/` — Enhanced with OpenGL/Vulkan contexts
- 🔧 `src/Input/` — Enhanced with EventBus patterns
- 🔧 `src/Physics/` — Enhanced with GJK/EPA collision detection
- 🔧 `src/Geometry/` — Enhanced with Extrude/Revolve/Boolean CSG
- 🔧 `src/Assembly/` — Enhanced with Kinematics solver
- 🔧 `src/FEA/` — Enhanced with linear static solver
- 🔧 `src/Materials/` — Enhanced with thread-safe database
- 🔧 `src/Math/` — Enhanced with constraint solver

### New TypeScript Modules (Optional Web/Desktop UI)
- 📦 `modules/010_x/` — Window/Graphics/Input managers
- 📦 `modules/020_x/` — Scene graph & history
- 📦 `modules/110_4/` — Topology manager
- 📦 `modules/120_1/` — Extrusion manager
- 📦 `modules/130_1/` — Constraint solver
- 📦 `modules/150_x/` — Camera & tessellation

---

## Architecture Principles (Maintained)

| Principle | Implementation |
|-----------|----------------|
| **Singleton Host** | `Application` owns OS window + delta-time + system tick |
| **Zero-coupling** | All modules communicate via `EventBus` |
| **Load-once** | `MaterialDatabase` + `BasePart` registries (flyweight) |
| **Thread-safe reads** | `std::shared_mutex` for concurrent FEA threads |
| **Manifold guarantee** | `SolidBody::Stitch()` + `VerifyManifold()` |
| **Stable numerics** | Eigen BDCSVD/PCG for iterative solvers |
| **Physics accuracy** | Inertia from mesh via signed-tetrahedron integration |

---

## Build Integration

### C++ (Existing + Enhanced)
```cmake
# Root CMakeLists.txt additions:
add_subdirectory(src/Graphics)      # Enhanced with Vulkan
add_subdirectory(src/Physics)       # Enhanced with GJK/EPA
add_subdirectory(src/Geometry)      # Enhanced with CSG
add_subdirectory(src/Assembly)      # Enhanced with Kinematics
add_subdirectory(src/FEA)           # Enhanced with PCG solver
add_subdirectory(src/Materials)     # Enhanced with thread-safety
add_subdirectory(src/Math)          # Enhanced with constraints
```

### TypeScript (Optional)
```bash
# For web/desktop UI layer:
npm install
npm run build
```

---

## Data Flow

```
Application::Run()
    ↓
TickSystems()
    ├─ SimulationEngine::Tick(dt)
    │   ├─ UpdateStress()
    │   ├─ UpdateTemperature()
    │   └─ UpdateDangerState()
    │
    ├─ Physics::CollisionDetection::generateManifold()
    │   ├─ GJK (broad phase)
    │   └─ EPA (narrow phase)
    │
    ├─ Assembly::KinematicSolver::Solve()
    │   └─ Newton-Raphson (6-DOF per body)
    │
    ├─ FEA::LinearStaticSolver::Solve()
    │   └─ PCG (sparse matrix)
    │
    └─ EventBus::Publish("SimulationTick")
        ↓
    UIManager::RenderUI()
        ├─ SimulationPanel::Draw()
        ├─ SimulationVisualizerPanel::Draw()
        └─ ComponentBuilderPanel::Draw()
```

---

## Key Enhancements

### 1. Graphics Context (Enhanced)
- **OpenGL 4.6** — Cook-Torrance PBR shaders
- **Vulkan** — Multi-threaded rendering pipeline
- **Camera** — Quaternion trackball (gimbal-lock-free)
- **MeshGenerator** — B-Rep → GPU tessellation with LOD

### 2. Physics Engine (Enhanced)
- **GJK+EPA** — Narrow-phase collision detection
- **RigidBody** — Inertia from mesh (Mirtich/Eberly)
- **Contact Caching** — Warm-starting for stability
- **CCD** — Continuous collision detection (tunneling prevention)

### 3. Geometry Operations (Enhanced)
- **Extrude** — Linear sweep with draft angle
- **Revolve** — Rotational sweep with axis validation
- **Boolean CSG** — Union/Difference/Intersect with manifold verification
- **B-Rep** — Half-edge topology with Euler characteristic check

### 4. Assembly System (Enhanced)
- **KinematicSolver** — 3-D Newton-Raphson (6-DOF per body)
- **AssemblyGraph** — Instance-based part graph
- **BOM Generation** — Bill of materials from hierarchy
- **Mass Properties** — CG + inertia tensor computation

### 5. FEA Solver (Enhanced)
- **LinearStaticSolver** — PCG with Gmsh volumetric meshing
- **Constitutive Matrix** — Isotropic linear elastic
- **Von Mises Stress** — Post-processing visualization
- **Boundary Conditions** — Penalty method for constraints

### 6. Materials Database (Enhanced)
- **Thread-safe** — `std::shared_mutex` for concurrent reads
- **Presets** — Steel, Al-6061, Ti-6Al-4V, Copper
- **Derived Properties** — Shear modulus from E and ν

---

## Repair Notes

### Issues Fixed
1. ✅ **GLM Dependency** — Replaced with project's `Geometry::Vector3`
2. ✅ **EventBus Integration** — All modules publish via EventBus
3. ✅ **Thread Safety** — `std::shared_mutex` for concurrent access
4. ✅ **Manifold Verification** — Euler characteristic check (V - E + F = 2)
5. ✅ **Numerical Stability** — BDCSVD + PCG for rank-deficient systems

### Features Preserved
- ✅ All Phase 1 simulation features
- ✅ All Phase 2 UI panels
- ✅ Zero-coupling architecture
- ✅ Singleton pattern
- ✅ EventBus communication

---

## Next Steps (Phase 3+)

### Immediate
1. Enable ImGui in CMakeLists.txt
2. Test UI panels with graphics context
3. Integrate camera controls

### Short-term
1. Graphics integration (render components)
2. Material database integration
3. Assembly graph visualization

### Medium-term
1. Physics integration (RigidBody + FEA)
2. Advanced visualization (heatmaps)
3. Component library (save/load)

### Long-term
1. Thermal engine (full heat transfer)
2. Electrical engine (circuit simulation)
3. AI assistant (design optimization)
4. Digital twin (real-world control)

---

## Build Commands

### Lightweight Mode (No GUI)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
```

### Full Desktop Mode (With GUI)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build --parallel
./build/SZM_Forge
```

### TypeScript Modules (Optional)
```bash
cd modules
npm install
npm run build
```

---

## File Statistics

| Category | Files | Lines |
|----------|-------|-------|
| Phase 1 (Simulation) | 3 | 189 |
| Phase 2 (UI Panels) | 6 | 370 |
| Enhanced C++ | 15+ | 3000+ |
| TypeScript Modules | 20+ | 2000+ |
| Documentation | 5 | 800+ |
| **Total** | **50+** | **6000+** |

---

**Version:** 0.3.0  
**Status:** 🚀 Ready for Integration  
**Last Updated:** 2024
