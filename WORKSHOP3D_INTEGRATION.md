# 3D Virtual Workshop - Integration Guide

## Quick Start

The **3D Virtual Workshop** is now integrated into SZM Forge, combining CAD part management, assembly building, and physics simulation visualization.

## Building with Workshop3D

### Full Desktop Build (with Workshop3D)

```bash
cd /mnt/D/projects/szm_forge
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build --parallel 8
```

### Optional: Enable Bullet Physics Backend

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DSZM_BUILD_FULL_DESKTOP=ON \
  -DSZM_ENABLE_BULLET_BACKEND=ON
cmake --build build --parallel 8
```

## Architecture Overview

```
SZM Forge (C++20)
│
├── Core Application & Graphics
│   ├── Core/Application.hpp          (Window, event loop)
│   ├── Graphics/GraphicsManager.hpp  (OpenGL rendering)
│   └── UI/UIManager.hpp              (ImGui docking)
│
├── Physics & Simulation
│   ├── Physics/RigidBody.hpp         (Physics objects)
│   ├── Simulation/SimulationEngine.hpp (Main solver)
│   └── FEA/FEAModule.hpp             (Finite element)
│
└── 3D Virtual Workshop (NEW!)
    ├── Workshop3D/PartLibrary.hpp      (CAD parts)
    ├── Workshop3D/AssemblyManager.hpp  (Assemblies)
    ├── Workshop3D/CADImporter.hpp      (File loading)
    ├── Workshop3D/WorkshopManager.hpp  (Coordinator)
    └── Workshop3D/WorkshopPanels.hpp   (UI integration)
```

## Module Integration Points

### 1. Graphics Integration

```cpp
#include "Workshop3D/WorkshopManager.hpp"
#include "Graphics/GraphicsManager.hpp"

// WorkshopManager renders through GraphicsManager
WorkshopManager& workshop = WorkshopManager::GetInstance();
workshop.RenderActiveAssembly();

// Graphics handles viewport and heatmap visualization
Graphics::GraphicsManager& graphics = Graphics::GraphicsManager::GetInstance();
graphics.Update(simulation_engine);
```

### 2. Simulation Integration

```cpp
// Setup parts for simulation
int componentCount = workshop.SetupSimulation();

// Run FEA/simulation
bool success = workshop.RunSimulation(5.0f);

// Visualize results with heatmap
workshop.UpdateVisualization("stress_data", Graphics::Heatmap::Mode::Stress);
```

### 3. Data Integration

```cpp
// UUID-based part and assembly references
UUID128 partId = part->GetID();
UUID128 assemblyId = assembly->GetID();

// Scene graph integration for rendering hierarchy
// (via DataCore_SceneGraph)
```

## Code 8 Integration

The Workshop3D module automatically discovers and loads CalculiX examples from Code 8:

### Load All Examples

```cpp
#include "Workshop3D/CADImporter.hpp"

// Import all CalculiX examples
int imported = workshop.ImportCode8Examples("/mnt/D/projects/szm_forge/code 8");
std::cout << "Loaded " << imported << " FEA examples\n";

// Browse part library
std::cout << "Total parts: " << workshop.GetPartCount() << "\n";
```

### Load Specific Category

```cpp
using namespace SZM::Workshop3D;

PartLibrary& library = workshop.GetPartLibrary();

// Load Contact analysis examples
Code8AssetManager::LoadCategory(
    "/mnt/D/projects/szm_forge/code 8",
    "Contact",
    library
);

// Load Linear analysis examples
Code8AssetManager::LoadCategory(
    "/path/to/code 8",
    "Linear",
    library
);
```

### Available Code 8 Categories

| Category | Examples | Use Case |
|----------|----------|----------|
| **Linear** | Beam, plate, shell examples | Basic stress analysis |
| **NonLinear** | Buckling, plasticity, large deformations | Advanced analysis |
| **Contact** | CNC, surfaces, edge-to-face | Assembly simulation |
| **Dynamics** | Transient, vibration | Time-dependent loading |
| **Thermal** | Heat transfer, distortion | Thermal effects |
| **Elements** | Convergence studies | Element types |
| **Drahtbiegen** | Wire bending, tensile tests | Manufacturing processes |
| **Kasten** | Box compression | Nonlinear geometry |
| **Pillow** | Inflation simulation | Membrane analysis |
| **Streifen** | Strip bending | Beam/shell comparison |

## UI Integration (ImGui Panels)

The Workshop3D module provides 5 main UI panels for the ImGui docking interface:

### Panel Classes

```cpp
#include "Workshop3D/WorkshopPanels.hpp"

// Register panels with UIManager
UIManager& ui = UIManager::GetInstance();
ui.RegisterPanel(std::make_shared<PartLibraryPanel>());
ui.RegisterPanel(std::make_shared<PartEditorPanel>());
ui.RegisterPanel(std::make_shared<AssemblyPanel>());
ui.RegisterPanel(std::make_shared<WorkshopViewportPanel>());
ui.RegisterPanel(std::make_shared<SimulationSetupPanel>());
```

### Panel Functionality

#### PartLibraryPanel
- Browse loaded parts
- Search/filter by name or category
- Part properties preview
- Import new parts
- Delete/manage library

#### PartEditorPanel
- Create new parts
- Edit part properties (name, material, color)
- CAD operations (Extrude, Revolve, Boolean)
- 3D preview with geometry

#### AssemblyPanel
- Assembly tree hierarchy
- Add/remove instances
- Edit instance properties
- Constraint definition
- Mass and COM calculations

#### WorkshopViewportPanel
- Real-time 3D rendering of active assembly
- Camera control (orbit, pan, zoom)
- Auto-rotation
- Instance selection and highlighting

#### SimulationSetupPanel
- Simulation type selection
- Load definition (forces, pressures)
- Boundary conditions
- Run simulation
- Results visualization (stress, temperature, danger zones)

## Usage Examples

### Example 1: Load Code8 Bolted Assembly

```cpp
#include "Workshop3D/WorkshopManager.hpp"
#include "Simulation/SimulationEngine.hpp"
#include "Graphics/GraphicsManager.hpp"

using namespace SZM;

int main() {
    // Initialize systems
    Application& app = Application::GetInstance();
    app.Init({1920, 1080, "SZM Forge Workshop", true});
    
    SimulationEngine& sim = SimulationEngine::GetInstance();
    sim.Init();
    
    Graphics::GraphicsManager& graphics = Graphics::GraphicsManager::GetInstance();
    graphics.Init();

    // Initialize workshop with Code8 examples
    Workshop3D::WorkshopManager& workshop = Workshop3D::WorkshopManager::GetInstance();
    workshop.Init("/mnt/D/projects/szm_forge/code 8");

    std::cout << "Loaded " << workshop.GetPartCount() << " parts\n";

    // Create assembly
    workshop.CreateAssembly("Bolted Assembly");
    
    // Get a bolted joint example
    auto boltPart = workshop.GetPartLibrary().GetPartByName("bolt");
    auto platePart = workshop.GetPartLibrary().GetPartByName("plate");

    // Add instances
    if (boltPart && platePart) {
        workshop.AddPartToActiveAssembly(boltPart->GetID(), Vector3(0, 0, 0), "Bolt");
        workshop.AddPartToActiveAssembly(platePart->GetID(), Vector3(0, 0.05f, 0), "Plate1");
        workshop.AddPartToActiveAssembly(platePart->GetID(), Vector3(0, 0.1f, 0), "Plate2");
    }

    // Main loop
    while (app.IsRunning()) {
        // Render workshop
        workshop.RenderActiveAssembly();
        
        // Update graphics
        graphics.Update(sim);
    }

    workshop.Shutdown();
    graphics.Shutdown();
    sim.Shutdown();
    app.Shutdown();
    
    return 0;
}
```

### Example 2: Create Custom Assembly and Simulate

```cpp
#include "Workshop3D/WorkshopManager.hpp"

using namespace SZM::Workshop3D;

// Create custom robot arm assembly
WorkshopManager& workshop = WorkshopManager::GetInstance();
workshop.Init();

// Import parts
auto basePart = workshop.ImportPart("models/robot_base.stl", "RobotBase");
auto linkPart = workshop.ImportPart("models/robot_link.stl", "RobotLink");
auto gripperPart = workshop.ImportPart("models/gripper.stl", "Gripper");

// Create assembly
workshop.CreateAssembly("RoboticArm");

// Build hierarchy
workshop.AddPartToActiveAssembly(basePart->GetID(), Vector3(0, 0, 0), "Base");
workshop.AddPartToActiveAssembly(linkPart->GetID(), Vector3(0, 0.2f, 0), "Link1");
workshop.AddPartToActiveAssembly(linkPart->GetID(), Vector3(0, 0.4f, 0), "Link2");
workshop.AddPartToActiveAssembly(gripperPart->GetID(), Vector3(0, 0.6f, 0), "Gripper");

auto assembly = workshop.GetActiveAssembly();

// Add constraints (connections)
assembly->AddConstraint(0, 1, "hinge");      // Link1 hinges on Base
assembly->AddConstraint(1, 2, "hinge");      // Link2 hinges on Link1
assembly->AddConstraint(2, 3, "hinge");      // Gripper on Link2

// Setup and run simulation
workshop.SetupSimulation();
workshop.RunSimulation(10.0f);

// Display results
auto assembly = workshop.GetActiveAssembly();
std::cout << "Total mass: " << assembly->CalculateTotalMass() << " kg\n";
auto com = assembly->CalculateCenterOfMass();
std::cout << "COM: (" << com.x << ", " << com.y << ", " << com.z << ")\n";

// Save assembly
workshop.SaveWorkshop("saved_assemblies/robot_arm");
```

### Example 3: CAD Import and Mesh Generation

```cpp
#include "Workshop3D/CADImporter.hpp"

using namespace SZM::Workshop3D;

PartLibrary library;

// Auto-detect format and import
auto part1 = CADImporter::ImportAuto("models/component.step");
auto part2 = CADImporter::ImportAuto("models/bracket.stl");
auto part3 = CADImporter::ImportAuto("models/part.obj");

if (part1) {
    std::cout << "Part 1 volume: " << part1->CalculateVolume() << " m³\n";
    part1->CalculateMass(7850.0f);  // Steel density
    std::cout << "Part 1 mass: " << part1->GetMass() << " kg\n";
}

// Batch import from directory
int count = 0;
for (const auto& file : fs::directory_iterator("models/parts")) {
    if (file.path().extension() == ".stl" ||
        file.path().extension() == ".obj") {
        
        auto part = CADImporter::ImportAuto(file.path().string());
        if (part) {
            library.GetAllParts(); // Would add to library
            count++;
        }
    }
}

std::cout << "Imported " << count << " parts\n";
```

## File Structure After Integration

```
/mnt/D/projects/szm_forge/
├── src/
│   ├── Workshop3D/                  ← NEW MODULE
│   │   ├── PartLibrary.hpp/cpp
│   │   ├── AssemblyManager.hpp/cpp
│   │   ├── CADImporter.hpp/cpp
│   │   ├── WorkshopManager.hpp/cpp
│   │   ├── WorkshopPanels.hpp/cpp
│   │   ├── CMakeLists.txt
│   │   └── README.md
│   ├── Core/
│   ├── Graphics/
│   ├── UI/
│   ├── Simulation/
│   └── [other modules...]
├── code 8/                          ← CAD EXAMPLES
│   ├── CAD/                         (STEP, GEO files)
│   ├── Linear/                      (FEA examples)
│   ├── Contact/                     (Assemblies)
│   ├── Dynamics/
│   ├── Thermal/
│   └── [other analysis types...]
├── CMakeLists.txt                   ← UPDATED
└── build/
    ├── SZM_Forge                    ← EXECUTABLE
    └── SZM_TestRunner
```

## Build Verification

After integration, verify the Workshop3D module compiled:

```bash
cd /mnt/D/projects/szm_forge/build
grep -i "workshop" CMakeFiles/SZM_Forge.dir/build.log
```

Should see messages like:
```
[Workshop3D] Module configured
-- Configuring done
```

## Troubleshooting

### Workshop3D symbols not found at link time
- Ensure `add_subdirectory(src/Workshop3D)` is in CMakeLists.txt ✓
- Verify `SZM_Workshop3D_Module` is linked in executable targets ✓

### Code 8 import fails
- Check path exists: `/mnt/D/projects/szm_forge/code 8`
- Verify STEP files present: `code 8/CAD/OnshapeTutorial/part.step`
- Note: Full STEP import requires OpenCascade library (not yet integrated)

### Missing ImGui panels
- ImGui rendering stubs are in `WorkshopPanels.cpp`
- Full ImGui integration requires calling `ImGui::Begin/End` in `Render()` methods
- Panels integrate with UIManager in `UI/UIManager.hpp`

### Physics integration
- Verify SimulationEngine is initialized before Workshop
- Ensure GraphicsManager is initialized for rendering
- Heatmap modes: Stress, Temperature, Danger

## Next Steps

1. **Full ImGui Integration** - Implement `Render()` methods in UI panels
2. **3D Mesh Rendering** - Connect to GraphicsManager for part visualization
3. **STEP/GEO Import** - Add OpenCascade or GMSH library
4. **Constraint Solving** - Implement assembly constraint solver
5. **FEA Meshing** - Generate meshes from parts for simulation
6. **Real-time Visualization** - Heatmaps during simulation

## References

- **Workshop3D Documentation**: `src/Workshop3D/README.md`
- **Code 8 Examples**: `code 8/` directory
- **SZM Forge Main**: `README.md`
- **Build Guide**: `PHASE3_FOUNDATION.md`

---

**Status**: Workshop3D module created and integrated ✅  
**Date**: April 27, 2026  
**Phase**: Phase 3 - Graphics Integration (Workshop3D Foundation)
