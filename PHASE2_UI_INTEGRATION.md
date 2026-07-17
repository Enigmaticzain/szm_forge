# Phase 2: UI Integration Guide

## Overview

Phase 2 adds three ImGui panels to visualize and control the simulation engine:

1. **SimulationPanel** — Control panel for component management
2. **SimulationVisualizerPanel** — 3D viewport with danger coloring
3. **ComponentBuilderPanel** — Material presets and quick-add templates

## Files Created

```
src/UI/panels/
├── SimulationPanel.hpp/cpp           ← Component list + force/heat controls
├── SimulationVisualizerPanel.hpp/cpp ← 3D rendering with heatmaps
└── ComponentBuilderPanel.hpp/cpp     ← Material presets + templates
```

## Architecture

### Event Flow

```
Application::Run()
    ↓
Application::TickSystems()
    ↓
SimulationEngine::Tick(dt)
    ├─ UpdateStress()
    ├─ UpdateTemperature()
    ├─ UpdateDangerState()
    └─ EventBus::Publish("SimulationTick")
        ↓
    UIManager::RenderUI()
        ├─ SimulationPanel::Draw()
        │   └─ Display component list + controls
        ├─ SimulationVisualizerPanel::Draw()
        │   └─ Render 3D components with colors
        └─ ComponentBuilderPanel::Draw()
            └─ Material presets + quick-add
```

### Panel Responsibilities

#### SimulationPanel
- **Add Component** — text input + button
- **Component List** — table with ID, Name, Stress, Temp, Status
- **Selected Component Controls** — force/heat sliders, remove button
- **Color Coding** — red (dangerous), green (safe)

#### SimulationVisualizerPanel
- **3D Viewport** — render components as cubes
- **Heatmap Modes** — stress, temperature, or danger indicator
- **Camera Controls** — orbit, pan, zoom (via Graphics module)
- **Placeholder** — ready for Graphics integration

#### ComponentBuilderPanel
- **Material Presets** — Steel, Aluminum, Titanium, Copper
- **Geometry Input** — name, area, thickness
- **Quick Templates** — pre-configured component types
- **Create Button** — adds component with material properties

## Integration Steps

### Step 1: Enable ImGui (when ready)

In `src/UI/CMakeLists.txt`, uncomment:

```cmake
# ${IMGUI_DIR}/imgui.cpp
# ${IMGUI_DIR}/imgui_draw.cpp
# ${IMGUI_DIR}/imgui_tables.cpp
# ${IMGUI_DIR}/imgui_widgets.cpp
# ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
# ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
```

And in `src/UI/UIManager.cpp`, uncomment ImGui initialization code.

### Step 2: Register Panels in Application

In `src/Core/Application.cpp` or `src/main.cpp`:

```cpp
#include "UI/UIManager.hpp"
#include "UI/panels/SimulationPanel.hpp"
#include "UI/panels/SimulationVisualizerPanel.hpp"
#include "UI/panels/ComponentBuilderPanel.hpp"

// In Application::Init():
UIManager uiManager;
uiManager.Initialize();
uiManager.RegisterPanel(std::make_shared<SimulationPanel>());
uiManager.RegisterPanel(std::make_shared<SimulationVisualizerPanel>());
uiManager.RegisterPanel(std::make_shared<ComponentBuilderPanel>());
```

### Step 3: Call UIManager::RenderUI() in Main Loop

In `src/Core/Application.cpp`:

```cpp
void Application::Run() {
    while (!glfwWindowShouldClose(m_WindowHandle)) {
        // ... existing code ...
        
        // Render UI
        UIManager::GetInstance().RenderUI();
        
        // Swap buffers
        glfwSwapBuffers(m_WindowHandle);
    }
}
```

### Step 4: Uncomment ImGui Code in Panels

Each panel has ImGui code wrapped in `/* ... */` comments. Uncomment when ImGui is linked.

## Usage Example

### Creating a Component via UI

1. Open **Component Builder** panel
2. Select material preset (e.g., "Steel")
3. Enter component name (e.g., "Main Shaft")
4. Adjust area/thickness if needed
5. Click "Create Component"

### Applying Load

1. Open **Simulation Control** panel
2. Select component from list
3. Adjust "Force (N)" slider to 5000 N
4. Adjust "Heat Input (W)" slider to 100 W
5. Watch stress/temperature update in real-time

### Visualizing Danger

1. Open **Simulation Visualizer** panel
2. Check "Show Stress Heatmap" or "Show Temperature Heatmap"
3. Components turn red when dangerous, green when safe
4. Hover over components to see detailed stats

## Material Presets

| Material | Density | Yield Strength | Thermal Conductivity |
|----------|---------|----------------|----------------------|
| Steel | 7850 kg/m³ | 250 MPa | 50 W/(m·K) |
| Aluminum | 2700 kg/m³ | 70 MPa | 160 W/(m·K) |
| Titanium | 4500 kg/m³ | 880 MPa | 7.4 W/(m·K) |
| Copper | 8960 kg/m³ | 200 MPa | 385 W/(m·K) |

## Color Coding

### Danger Indicator Mode (default)
- **Green** — stress < 80% yield AND temp < 80% max
- **Red** — stress > 80% yield OR temp > 80% max

### Stress Heatmap
- **Blue** — 0% stress
- **Yellow** — 50% stress
- **Red** — 100% stress (yield)

### Temperature Heatmap
- **Blue** — ambient (20°C)
- **Purple** — mid-range (60°C)
- **Red** — max (100°C)

## Next Steps (Phase 3)

1. **Graphics Integration** — render components into ImGui texture
2. **Camera Controls** — orbit/pan/zoom in 3D viewport
3. **Material Database** — integrate with existing `MaterialDatabase`
4. **Physics Integration** — connect to `RigidBody` and `FEA` modules
5. **Assembly Graph** — build multi-component machines with constraints

## Testing

### Compile Check
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Runtime Check
```bash
./build/SZM_Forge
# Panels should appear in ImGui dockspace (once ImGui is enabled)
```

### Manual Test Scenario

1. Add "Steel Beam" component
2. Set force to 3000 N (should be ~60% of yield)
3. Set heat to 50 W (should be ~50% of max temp)
4. Verify component shows "OK" status
5. Increase force to 5000 N (should show "DANGER")
6. Verify color changes to red in visualizer

## Architecture Notes

- **Zero coupling** — panels communicate only via EventBus
- **ImGui-agnostic** — all code compiles without ImGui (commented out)
- **Extensible** — easy to add more panels or visualization modes
- **Thread-safe ready** — can be extended with locks if needed

---

**Status:** ✅ Code complete | ⏳ Awaiting ImGui integration | Ready for Phase 3
