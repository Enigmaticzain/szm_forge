# SZM Forge — Integration Checklist (v0.3.0)

## ✅ Phase 1 & 2 Status (Already Complete)

- [x] Simulation engine (stress + temperature)
- [x] UI panels (control + visualizer + builder)
- [x] EventBus integration
- [x] Application lifecycle hooks
- [x] Launch script
- [x] Documentation

---

## 🔧 Integration Tasks (New Code)

### Step 1: Graphics Module Enhancement

**Files to integrate:**
- `code` → Graphics context (OpenGL/Vulkan)
- `code2` → PBR shaders + MeshGenerator

**Actions:**
- [ ] Merge `GraphicsContext.hpp` into `src/Graphics/`
- [ ] Add `OpenGLContext.cpp` implementation
- [ ] Add `VulkanContext.cpp` implementation (optional)
- [ ] Add PBR vertex/fragment shaders to `assets/shaders/`
- [ ] Update `src/Graphics/CMakeLists.txt` with new files
- [ ] Link Vulkan SDK (optional)

**Status:** Ready to integrate

---

### Step 2: Physics Module Enhancement

**Files to integrate:**
- `code2` → GJK+EPA collision detection
- `code2` → RigidBody with inertia calculation

**Actions:**
- [ ] Merge `CollisionDetection.hpp/cpp` into `src/Physics/`
- [ ] Merge `RigidBody.hpp/cpp` into `src/Physics/`
- [ ] Add contact manifold structures
- [ ] Update `src/Physics/CMakeLists.txt`
- [ ] Link Eigen3 for matrix operations

**Status:** Ready to integrate

---

### Step 3: Geometry Module Enhancement

**Files to integrate:**
- `code` → Extrude, Revolve, Boolean CSG operations
- `code` → B-Rep solid body with half-edge topology

**Actions:**
- [ ] Merge `ExtrudeOp.hpp/cpp` into `src/Geometry/operations/`
- [ ] Merge `RevolveOp.hpp/cpp` into `src/Geometry/operations/`
- [ ] Merge `BooleanOp.hpp/cpp` into `src/Geometry/operations/`
- [ ] Merge `SolidBody.hpp/cpp` into `src/Geometry/`
- [ ] Update `src/Geometry/CMakeLists.txt`
- [ ] Verify manifold checks (Euler characteristic)

**Status:** Ready to integrate

---

### Step 4: Assembly Module Enhancement

**Files to integrate:**
- `code` → KinematicSolver (Newton-Raphson)
- `code` → AssemblyGraph with BOM generation
- `code` → InterferenceChecker (multi-threaded)

**Actions:**
- [ ] Merge `KinematicSolver.cpp` into `src/Assembly/`
- [ ] Merge `KinematicTypes.hpp` + `Mates.hpp` into `src/Assembly/`
- [ ] Merge `AssemblyGraph.hpp/cpp` into `src/Assembly/`
- [ ] Merge `InterferenceCheck.hpp/cpp` into `src/Assembly/`
- [ ] Update `src/Assembly/CMakeLists.txt`
- [ ] Link Eigen3 for 6-DOF solver

**Status:** Ready to integrate

---

### Step 5: FEA Module Enhancement

**Files to integrate:**
- `code` → LinearStaticSolver (PCG)
- `code` → VolumetricMesher (Gmsh)
- `code` → FEADataTypes

**Actions:**
- [ ] Merge `LinearSolver.hpp/cpp` into `src/FEA/`
- [ ] Merge `FEADataTypes.hpp` into `src/FEA/`
- [ ] Merge `VolumetricMesher.cpp` into `src/FEA/`
- [ ] Update `src/FEA/CMakeLists.txt`
- [ ] Link Gmsh library
- [ ] Link Eigen3 for sparse matrix solver

**Status:** Ready to integrate

---

### Step 6: Materials Module Enhancement

**Files to integrate:**
- `code` → MaterialDatabase (thread-safe)
- `code` → IMaterial interface

**Actions:**
- [ ] Merge `IMaterial.hpp` into `src/Materials/`
- [ ] Merge `MaterialDatabase.hpp/cpp` into `src/Materials/`
- [ ] Update `src/Materials/CMakeLists.txt`
- [ ] Add thread-safe `std::shared_mutex`
- [ ] Preload standard materials (Steel, Al, Ti, Cu)

**Status:** Ready to integrate

---

### Step 7: Math Module Enhancement

**Files to integrate:**
- `code` → ConstraintSolver2D (Newton-Raphson)
- `code` → DistanceConstraint example

**Actions:**
- [ ] Merge `ConstraintSolver.hpp/cpp` into `src/Math/`
- [ ] Merge constraint implementations into `src/Math/constraints/`
- [ ] Update `src/Math/CMakeLists.txt`
- [ ] Link Eigen3 for SVD solver

**Status:** Ready to integrate

---

### Step 8: Camera System Enhancement

**Files to integrate:**
- `code2` → Camera class (quaternion trackball)

**Actions:**
- [ ] Merge `Camera.hpp/cpp` into `src/Graphics/`
- [ ] Add quaternion-based orbit/pan/zoom
- [ ] Implement gimbal-lock-free rotation
- [ ] Update `src/Graphics/CMakeLists.txt`

**Status:** Ready to integrate

---

### Step 9: UI Panels Enhancement

**Files to integrate:**
- `code2` → UIManager with docking
- `code2` → EventDispatcher patterns

**Actions:**
- [ ] Merge enhanced `UIManager.cpp` into `src/UI/`
- [ ] Uncomment ImGui code in panels
- [ ] Add ImGui backend initialization
- [ ] Update `src/UI/CMakeLists.txt` with ImGui sources

**Status:** Ready to integrate

---

### Step 10: TypeScript Modules (Optional)

**Files to integrate:**
- `code 3` → Window/Graphics/Input managers (010.x)
- `code 3` → Scene graph & history (020.x)
- `code 3` → Topology/Constraint/Extrusion/Tessellation managers

**Actions:**
- [ ] Create `modules/` directory
- [ ] Copy TypeScript modules into `modules/`
- [ ] Create `modules/package.json`
- [ ] Create `modules/tsconfig.json`
- [ ] Create `modules/webpack.config.js` (optional)
- [ ] Add build scripts to root `package.json`

**Status:** Optional (for web UI layer)

---

## 🔗 CMakeLists.txt Updates

### Root CMakeLists.txt

Add these lines in the "Sub-Module Libraries" section:

```cmake
# Enhanced modules (new code integration)
add_subdirectory(src/Graphics)      # Now with OpenGL/Vulkan
add_subdirectory(src/Physics)       # Now with GJK/EPA
add_subdirectory(src/Geometry)      # Now with CSG operations
add_subdirectory(src/Assembly)      # Now with Kinematics
add_subdirectory(src/FEA)           # Now with PCG solver
add_subdirectory(src/Materials)     # Now with thread-safety
add_subdirectory(src/Math)          # Now with constraints
```

### Link Libraries

Update `SZM_Forge` target:

```cmake
target_link_libraries(SZM_Forge PRIVATE
    # ... existing ...
    SZM_Graphics_Module         # Enhanced
    SZM_Physics_Module          # Enhanced
    SZM_Geometry_Module         # Enhanced
    SZM_Assembly_Module         # Enhanced
    SZM_FEA_Module              # Enhanced
    SZM_Materials_Module        # Enhanced
    SZM_Constraint_Solver       # Enhanced
    # New external deps
    Eigen3::Eigen
    gmsh                        # For FEA meshing
    Vulkan::Vulkan              # Optional
)
```

---

## 🧪 Testing Checklist

### Unit Tests
- [ ] Test SimulationEngine (stress + temp)
- [ ] Test GJK collision detection
- [ ] Test RigidBody inertia calculation
- [ ] Test Extrude/Revolve operations
- [ ] Test Boolean CSG operations
- [ ] Test KinematicSolver convergence
- [ ] Test LinearStaticSolver PCG
- [ ] Test MaterialDatabase thread-safety
- [ ] Test ConstraintSolver2D

### Integration Tests
- [ ] Test Application lifecycle
- [ ] Test EventBus communication
- [ ] Test UI panel rendering
- [ ] Test Graphics context initialization
- [ ] Test Physics + Assembly integration
- [ ] Test FEA solver with real geometry

### Scenario Tests
- [ ] Normal operation (stress + temp)
- [ ] Mechanical overload
- [ ] Thermal overload
- [ ] Collision detection
- [ ] Assembly kinematics
- [ ] FEA stress analysis

---

## 📋 Dependency Checklist

### Required (Already in project)
- [x] C++20 compiler
- [x] CMake 3.21+
- [x] GLFW 3.3.8
- [x] Eigen3 3.4

### New Required
- [ ] Gmsh (for FEA volumetric meshing)
- [ ] Vulkan SDK (optional, for Vulkan backend)

### Optional
- [ ] ImGui docking branch (for full desktop mode)
- [ ] Node.js + npm (for TypeScript modules)

---

## 🚀 Build & Launch

### After Integration

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run lightweight demo
./build/SZM_Forge

# Run full desktop mode (if ImGui enabled)
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel
./build-full/SZM_Forge
```

---

## ✅ Verification Checklist

After integration, verify:

- [ ] All modules compile without errors
- [ ] No linker errors
- [ ] Simulation engine ticks correctly
- [ ] Physics collision detection works
- [ ] Geometry operations produce valid manifolds
- [ ] Assembly kinematics converges
- [ ] FEA solver produces reasonable results
- [ ] Materials database loads correctly
- [ ] UI panels render (if ImGui enabled)
- [ ] EventBus communication works
- [ ] No memory leaks (valgrind/ASAN)
- [ ] Thread-safety verified (ThreadSanitizer)

---

## 📊 Integration Progress

```
Phase 1 (Simulation):     ████████████████████ 100%
Phase 2 (UI):             ████████████████████ 100%
Graphics Enhancement:     ░░░░░░░░░░░░░░░░░░░░   0%
Physics Enhancement:      ░░░░░░░░░░░░░░░░░░░░   0%
Geometry Enhancement:     ░░░░░░░░░░░░░░░░░░░░   0%
Assembly Enhancement:     ░░░░░░░░░░░░░░░░░░░░   0%
FEA Enhancement:          ░░░░░░░░░░░░░░░░░░░░   0%
Materials Enhancement:    ░░░░░░░░░░░░░░░░░░░░   0%
Math Enhancement:         ░░░░░░░░░░░░░░░░░░░░   0%
TypeScript Modules:       ░░░░░░░░░░░░░░░░░░░░   0%
─────────────────────────────────────────────────
Overall:                  ████░░░░░░░░░░░░░░░░  20%
```

---

## 🎯 Next Action

Start with **Step 1: Graphics Module Enhancement** and work through sequentially. Each step is independent and can be tested before moving to the next.

**Estimated Time:** 2-3 hours per step (with testing)

---

**Version:** 0.3.0  
**Status:** Ready for Integration  
**Last Updated:** 2024
