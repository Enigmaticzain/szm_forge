# SZM Forge Code Modules Inventory

## Overview
Complete list of all code modules available for integration into SZM Forge. These are organized by source file and contain thousands of lines of production-quality code.

---

## **CODE (code file) - C++ Core Modules**
**Status:** Ready for integration | **Language:** C++20 | **Lines:** ~8,000+

### Modules Included:

#### 1. **Application & Window Management (010.1)**
- `CMakeLists.txt` - Build configuration
- `Application.hpp/cpp` - Core application singleton
- `WindowManager.ts` - Window lifecycle management
- **Features:**
  - GLFW window creation and management
  - Delta-time calculation
  - Main event loop
  - V-Sync configuration
  - DPI awareness

#### 2. **Input & Event System (010.3)**
- `InputManager.hpp/cpp` - Keyboard/mouse input handling
- `EventBus.hpp` - Observer pattern event system
- `InputEvents.hpp` - Event type definitions
- **Features:**
  - Key state polling
  - Mouse position tracking
  - Event dispatching
  - Focus loss handling (sticky keys fix)
  - SpaceMouseDriver support

#### 3. **Graphics Context (010.4)**
- `GraphicsContext.hpp` - Abstract graphics API
- `OpenGLContext.hpp/cpp` - OpenGL 4.6 implementation
- `VulkanContext.hpp/cpp` - Vulkan implementation (stub)
- `GraphicsContextFactory.cpp` - Factory pattern
- **Features:**
  - Multi-API support (OpenGL/Vulkan)
  - MSAA configuration
  - Depth buffering
  - Device lost handling
  - GPU info querying

#### 4. **UI/ImGui Integration (020.1)**
- `UIManager.hpp/cpp` - ImGui docking manager
- `IPanel.hpp` - Panel interface
- `ConsolePanel.cpp` - Debug console
- `SceneOutliner.cpp` - Hierarchy viewer
- `PropertiesPanel.cpp` - Inspector
- `SimulationPanel.cpp` - Simulation controls
- `ToolbarPanel.cpp` - Toolbar
- **Features:**
  - Docking space layout
  - Multi-panel support
  - Menu bar integration
  - Panel registration system

#### 5. **Scene Graph & Data (020.2)**
- `SceneGraph.hpp` - Entity hierarchy
- `SceneGraphManager.hpp/cpp` - Graph management
- `UUID.hpp` - 128-bit UUID generation
- **Features:**
  - Circular dependency detection
  - Hierarchy depth validation
  - Transform management
  - Serialization/deserialization
  - DFS traversal

#### 6. **2D Constraint Solver (060)**
- `ConstraintSolver.hpp/cpp` - Newton-Raphson solver
- `DistanceConstraint.hpp` - Distance constraint implementation
- **Features:**
  - Eigen-based linear algebra
  - SVD decomposition
  - Rank deficiency detection
  - Over-constraint failure handling
  - Locked variable support

#### 7. **Geometry Operations (070)**
- `SolidBody.hpp/cpp` - B-Rep solid manager
- `ExtrudeOp.hpp/cpp` - Linear extrusion
- `RevolveOp.hpp/cpp` - Revolution operation
- `BooleanOp.hpp/cpp` - CSG operations (Union/Difference/Intersect)
- **Features:**
  - Half-edge data structure
  - Manifold verification (Euler characteristic)
  - Volume calculation
  - Micro-edge healing
  - BVH broad-phase culling
  - Ray-cast classification

#### 8. **Materials Database (110)**
- `IMaterial.hpp` - Material interface
- `MaterialDatabase.hpp/cpp` - Thread-safe singleton
- **Features:**
  - Isotropic material properties
  - Shear modulus derivation
  - Thread-safe reads (shared_mutex)
  - Standard library (Steel, Al-6061, Ti-6Al-4V)
  - Custom material registration

#### 9. **Assembly & Kinematics (120)**
- `AssemblyGraph.hpp/cpp` - Instance-based assembly
- `KinematicSolver.cpp` - 3D mate solver
- `KinematicTypes.hpp` - Constraint types
- `Mates.hpp` - Coincident planar mate
- **Features:**
  - Lightweight instance nodes
  - BOM generation
  - Mass property calculation
  - Instanced draw data
  - 6-DOF Newton-Raphson solver
  - Conflict detection

#### 10. **Physics Engine (130)**
- `RigidBody.hpp/cpp` - Rigid body dynamics
- `CollisionDetection.hpp/cpp` - GJK+EPA narrow phase
- **Features:**
  - Inertia tensor from mesh (Mirtich/Eberly)
  - Parallel axis theorem
  - GJK simplex algorithm
  - EPA penetration resolution
  - Contact manifold caching (warm starting)
  - CCD (continuous collision detection)
  - Kinematic/Static/Dynamic states

#### 11. **FEA Linear Static Solver (320)**
- `FEADataTypes.hpp` - FEA data structures
- `LinearSolver.hpp/cpp` - PCG solver
- `VolumetricMesher.hpp/cpp` - Gmsh integration
- **Features:**
  - Sparse matrix assembly
  - Preconditioned Conjugate Gradient
  - Boundary condition application
  - Von Mises stress calculation
  - Tet4/Tet10 element support
  - Constitutive matrix (isotropic linear elastic)

#### 12. **Graphics & Rendering (150)**
- `Camera.hpp/cpp` - Quaternion trackball camera
- `MeshGenerator.hpp/cpp` - B-Rep to GPU tessellation
- `PBR.vert/frag` - Cook-Torrance BRDF shaders
- **Features:**
  - Gimbal-lock-free orbit/pan/zoom
  - Dynamic LOD (chordal error)
  - Perspective/Orthographic modes
  - PBR material system
  - HDR tonemapping (Reinhard)
  - Gamma correction

---

## **CODE2 (code2 file) - Advanced C++ Implementations**
**Status:** Ready for integration | **Language:** C++20 | **Lines:** ~6,000+

### Modules Included:

#### 1. **Advanced Graphics Context**
- Multi-API abstraction (OpenGL/Vulkan/DirectX12)
- MSAA and depth buffer configuration
- GPU device lost recovery
- Hardware capability querying

#### 2. **Enhanced Physics**
- Tetrahedron integral computation
- Mesh-based inertia calculation
- World-space inertia transformation
- Kinematic state management

#### 3. **Advanced Geometry**
- Half-edge topology management
- Manifold verification (water-tight check)
- Volume calculation via divergence theorem
- Center of mass computation

#### 4. **Boolean CSG Operations**
- BVH broad-phase intersection
- Face partitioning and splitting
- Ray-cast inside/outside classification
- Material inheritance for cuts
- Normal flipping for difference operations

#### 5. **Volumetric Meshing**
- Gmsh integration for tetrahedral meshing
- Micro-edge healing
- Element quality optimization
- Tet4 and Tet10 support
- GPU memory mapping

#### 6. **Advanced Assembly**
- Material override system
- Instanced rendering data
- Mass property traversal
- BOM generation with quantities

#### 7. **Constraint Solving**
- Eigen-based matrix operations
- SVD rank detection
- Over-constraint failure handling
- Spatial update application

---

## **CODE 3 (code 3 file) - TypeScript UI Modules**
**Status:** Ready for integration | **Language:** TypeScript | **Lines:** ~4,000+

### Modules Included:

#### 1. **Window Manager (010.1)**
- Window creation/destruction
- Resize/move operations
- Z-order management
- Focus handling
- State snapshots

#### 2. **Input Manager (010.3)**
- Virtual input regions
- Region lifecycle
- Priority-based input routing
- Bounds validation
- State immutability

#### 3. **Graphics Manager (010.4)**
- Swapchain management
- Resolution handling
- V-Sync control
- Frame presentation
- Hardware acceleration flags

#### 4. **Scene Graph Manager (020.1)**
- Node creation/removal
- Reparenting with cycle detection
- Hierarchy depth validation
- Transform updates
- Serialization/deserialization
- State snapshots

#### 5. **History Manager (020.2)**
- Command-based undo/redo
- Future branch dropping
- Max history limit enforcement
- Atomic operations
- Timeline management

#### 6. **Topology Manager (110.4)**
- Vertex/Edge/Face/Solid creation
- Cascade removal
- Reference validation
- Manifold constraints
- Entity ordering

#### 7. **Solver Manager (130.1)**
- Constraint creation/destruction
- Value updates
- Constraint violation detection
- Execution ordering
- Rule validation

#### 8. **Extrusion Manager (120.1)**
- Extrusion operation lifecycle
- Distance validation
- Direction vector normalization
- Status tracking
- Execution queue

#### 9. **Tessellation Manager (150.2)**
- Mesh generation
- Material updates
- Geometry validation
- PBR bounds checking
- Render queue management

#### 10. **Camera Controller (150.1)**
- Camera creation/destruction
- Position/target updates
- FOV management
- Distance validation
- State snapshots

---

## **CODE 4 - Bullet Physics SDK**
**Status:** External library | **Language:** C++ | **Type:** Physics engine

### What it is:
Complete Bullet Physics SDK v3 - industry-standard rigid body dynamics and collision detection library.

### Key Components:
- **Collision Detection:** GJK, SAT, sphere-sweep
- **Rigid Body Dynamics:** Constraint solver, contact generation
- **Soft Body Dynamics:** Cloth, rope, deformable objects
- **Vehicle Dynamics:** Raycast vehicle controller
- **Character Controller:** Capsule-based character physics
- **Constraints:** Hinge, slider, 6-DOF, point-to-point
- **PyBullet:** Python bindings for ML/robotics

### Integration Value:
- Can replace/augment SZM's custom physics engine
- Proven in production (games, VR, robotics)
- Extensive constraint library
- Soft body support
- Python integration for ML workflows

---

## **CODE 5 - SOFA Framework**
**Status:** External library | **Language:** C++ | **Type:** FEM simulation framework

### What it is:
SOFA (Simulation Open-Framework Architecture) - specialized FEM framework for medical simulation and robotics.

### Key Components:
- **FEM Solvers:** Linear/nonlinear, implicit/explicit
- **Deformable Objects:** Cloth, soft tissue, organs
- **Collision Detection:** Advanced narrow-phase algorithms
- **Constraints:** Bilateral, unilateral, contact
- **Plugins:** Extensible architecture
- **Visualization:** Real-time rendering
- **Robotics:** Kinematics, dynamics, control

### Integration Value:
- Specialized for medical/soft-body simulation
- Advanced FEM capabilities beyond SZM's current solver
- Plugin ecosystem
- Research-grade algorithms
- Robotics-focused features

---

## Integration Priority Matrix

| Module | Priority | Complexity | Dependencies | Est. Time |
|--------|----------|-----------|--------------|-----------|
| **C++ Core (010-020)** | 🔴 CRITICAL | Low | None | 2-3 days |
| **Graphics (150)** | 🔴 CRITICAL | Medium | GLFW, OpenGL | 2-3 days |
| **Physics (130)** | 🟠 HIGH | High | Eigen | 3-4 days |
| **FEA (320)** | 🟠 HIGH | High | Gmsh, Eigen | 3-4 days |
| **Geometry (070)** | 🟠 HIGH | High | None | 2-3 days |
| **Assembly (120)** | 🟡 MEDIUM | Medium | Physics | 2 days |
| **TypeScript UI (020)** | 🟡 MEDIUM | Low | Node.js | 1-2 days |
| **Bullet Physics (code 4)** | 🟢 OPTIONAL | High | None | 2-3 days |
| **SOFA Framework (code 5)** | 🟢 OPTIONAL | Very High | CMake, plugins | 4-5 days |

---

## Recommended Integration Order

### Phase 1: Foundation (Days 1-3)
1. C++ Core modules (010-020)
2. Graphics modules (150)
3. Input/Event system (010.3)

### Phase 2: Physics & Simulation (Days 4-7)
4. Physics engine (130)
5. Geometry operations (070)
6. Assembly system (120)

### Phase 3: Advanced Features (Days 8-10)
7. FEA solver (320)
8. TypeScript UI modules (020)
9. Material database (110)

### Phase 4: Optional Enhancements (Days 11+)
10. Bullet Physics integration (code 4)
11. SOFA Framework integration (code 5)

---

## Total Code Statistics

| Source | Files | Lines | Language | Status |
|--------|-------|-------|----------|--------|
| code | ~50 | 8,000+ | C++20 | Ready |
| code2 | ~40 | 6,000+ | C++20 | Ready |
| code 3 | ~60 | 4,000+ | TypeScript | Ready |
| code 4 | 1000+ | 500,000+ | C++ | External |
| code 5 | 2000+ | 1,000,000+ | C++ | External |
| **TOTAL** | **3,150+** | **1,518,000+** | Mixed | - |

---

## Next Steps

1. **Confirm integration order** - Which phase should we start with?
2. **Dependency resolution** - Ensure all external libraries are available
3. **Build system updates** - Integrate new modules into CMakeLists.txt
4. **Testing strategy** - Unit tests for each module
5. **Documentation** - API docs and integration guides

Ready to begin Phase 1 integration?
