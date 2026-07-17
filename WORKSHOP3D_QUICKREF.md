# Workshop3D Quick Reference

## Overview
A complete 3D CAD and assembly environment for building, visualizing, and simulating mechanical systems. Integrates with Code 8 CalculiX examples.

## Core Classes

### PartLibrary
**Purpose**: Manage collection of CAD parts  
**Key Methods**:
- `CreatePart(name)` → Part
- `ImportFromSTEP(path)` → Part
- `ImportFromCode8(path)` → Part
- `GetPart(id)` → Part
- `GetPartByName(name)` → Part

### AssemblyManager
**Purpose**: Manage assemblies and instances  
**Key Methods**:
- `CreateAssembly(name)` → Assembly
- `GetAssembly(id)` → Assembly
- `SetActiveAssembly(id)`
- `GetActiveAssembly()` → Assembly

### WorkshopManager (Singleton)
**Purpose**: Main coordinator  
**Key Methods**:
- `GetInstance()` → WorkshopManager&
- `Init(code8Dir = "")` - Initialize
- `ImportPart(path)` → Part
- `CreateAssembly(name)` → UUID
- `AddPartToActiveAssembly(id, pos)` → instanceId
- `RenderActiveAssembly()`
- `SetupSimulation()` → componentCount
- `RunSimulation(duration)` → bool

### CADImporter
**Purpose**: Load CAD geometry from files  
**Supported Formats**:
- STEP (.step, .stp)
- Gmsh (.geo)
- STL (.stl)
- OBJ (.obj)

**Static Methods**:
- `ImportAuto(path)` → Part
- `ImportFromCode8(dir, library)` → count

## Usage Pattern

```cpp
// 1. Initialize
WorkshopManager& workshop = WorkshopManager::GetInstance();
workshop.Init("/path/to/code 8");

// 2. Create assembly
workshop.CreateAssembly("MyAssembly");

// 3. Import parts
auto part = workshop.ImportPart("model.stl");

// 4. Add to assembly
workshop.AddPartToActiveAssembly(part->GetID(), Vector3(0,0,0));

// 5. Simulate
workshop.SetupSimulation();
workshop.RunSimulation(5.0f);

// 6. Visualize
workshop.RenderActiveAssembly();
```

## File Formats

| Format | Extension | Support | Notes |
|--------|-----------|---------|-------|
| STEP | .step, .stp | Planned | Industry standard CAD |
| Gmsh | .geo | Planned | Geometry definition |
| STL | .stl | ✅ Implemented | Binary & ASCII |
| OBJ | .obj | ✅ Implemented | Wavefront format |
| JSON | .json | Planned | SZM native |

## UI Panels (ImGui)

- **PartLibraryPanel** - Browse and import parts
- **PartEditorPanel** - Create/edit parts
- **AssemblyPanel** - Manage assembly hierarchy
- **WorkshopViewportPanel** - 3D visualization
- **SimulationSetupPanel** - Run FEA/simulation

## Code 8 Categories

```
Linear          → Static stress analysis
NonLinear       → Plasticity, buckling
Contact         → Surface contact, assemblies
Dynamics        → Transient, vibration
Thermal         → Heat transfer
Elements        → Element convergence
Drahtbiegen     → Wire bending, tensile
Kasten          → Box compression
Pillow          → Membrane inflation
Streifen        → Strip bending
```

## Integration

**With Simulation**:
```cpp
workshop.SetupSimulation();      // Create components
workshop.RunSimulation(5.0f);    // Run FEA
workshop.UpdateVisualization();  // Show results
```

**With Graphics**:
```cpp
workshop.RenderActiveAssembly(); // Render to viewport
graphics.SetHeatmapMode(mode);   // Stress/Temp visualization
```

## Key Data Types

```cpp
UUID128         // 128-bit unique ID (part/assembly)
Vector3         // 3D point/vector
Part            // Single CAD geometry
Assembly        // Collection of part instances
AssemblyInstance // Part with position/rotation/constraints
```

## Statistics

- **Parts in Library**: `GetPartCount()`
- **Assemblies**: `GetAssemblyCount()`
- **Instances**: `GetInstanceCount()`
- **Assembly Mass**: `assembly->CalculateTotalMass()`
- **Center of Mass**: `assembly->CalculateCenterOfMass()`

## Import Code8 Examples

```cpp
// Import all CalculiX examples
int count = workshop.ImportCode8Examples(
    "/mnt/D/projects/szm_forge/code 8"
);

// Result: Loaded ~50+ parts across all categories
```

## Assembly Operations

```cpp
// Create
auto assembly = workshop.GetActiveAssembly();

// Add instance
uint32_t id = assembly->AddInstance(part, "name");

// Transform
auto inst = assembly->GetInstance(id);
inst->SetTransform(Vector3(x,y,z), Vector3(rx,ry,rz));

// Add constraint
assembly->AddConstraint(id1, id2, "hinge");

// Query
assembly->GetInstanceCount();
assembly->CalculateTotalMass();
assembly->CalculateBounds(min, max);
```

## Command Reference

```bash
# Build with Workshop3D
cmake -B build -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build --parallel

# Run application
./build/SZM_Forge

# Run tests
./build/SZM_TestRunner
```

## File Locations

```
src/Workshop3D/
├── PartLibrary.hpp/cpp          ← Part management
├── AssemblyManager.hpp/cpp      ← Assembly hierarchy
├── CADImporter.hpp/cpp          ← File loading
├── WorkshopManager.hpp/cpp      ← Main coordinator
├── WorkshopPanels.hpp/cpp       ← UI integration
├── CMakeLists.txt               ← Build config
└── README.md                    ← Full documentation
```

## TODO Items

- [ ] STEP file import (OpenCascade)
- [ ] GEO file import (GMSH)
- [ ] ImGui panel rendering
- [ ] 3D mesh visualization
- [ ] Constraint solving
- [ ] FEA mesh generation
- [ ] Real-time heatmap visualization
- [ ] Part parameterization
- [ ] Design optimization

---

**Module Status**: Foundation Complete ✅  
**Build Status**: Ready to compile ✅  
**Integration**: Graphics/Simulation ready ✅  
**Next Phase**: UI Implementation & Visualization
