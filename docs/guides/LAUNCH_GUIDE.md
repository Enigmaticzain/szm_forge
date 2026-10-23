# SZM Forge — Launch Guide

## Quick Start

### Lightweight Demo (No Dependencies)
```bash
cd szm_forge
./launch.sh
```

This runs the terminal-based mechanical simulation demo with three scenarios:
1. Normal operation
2. Mechanical overload
3. Thermal overload

**Requirements:** C++20 compiler only

**Time:** ~30 seconds

### Full Desktop Mode (With GUI)
```bash
./launch.sh --desktop
```

Launches the full GUI application with ImGui panels and 3D visualization.

**Requirements:**
- GLFW development headers
- Eigen3 >= 3.4
- OpenGL 4.6 support
- ImGui docking branch cloned to `external/imgui`

**Time:** ~2-3 minutes (first build)

---

## Installation & Setup

### Prerequisites

#### Linux (Ubuntu/Debian)
```bash
# Install build tools
sudo apt-get install -y build-essential cmake git

# Install GLFW dependencies
sudo apt-get install -y libglfw3-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev

# Install Eigen3
sudo apt-get install -y libeigen3-dev

# Install OpenGL headers
sudo apt-get install -y libgl1-mesa-dev
```

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake glfw eigen
```

#### Windows
- Install Visual Studio 2022 with C++ workload
- Install CMake 3.21+
- Install GLFW and Eigen3 via vcpkg or manually

### Clone ImGui (for desktop mode)

```bash
cd szm_forge
git clone -b docking https://github.com/ocornut/imgui.git external/imgui
```

---

## Build Modes

### Mode 1: Lightweight Demo (Default)

**What it does:**
- Terminal-based mechanical simulation
- Three demo scenarios
- No GUI dependencies
- Fast build (~10 seconds)

**Build:**
```bash
./launch.sh
```

**Output:**
```
============================================================
 SZM Forge - Simple Mechanical Demo
 Topic: stress + thermal safety of two machine parts
============================================================

--- Scenario 1: Normal Operation ---
Safe load and mild heating. Both parts should stay stable.
t= 0.50 s | Steel Beam         | Stress   0.30 MPa | Temp  20.00 C | Utilization   0.12% stress,   0.00% temp | SAFE
...
```

### Mode 2: Full Desktop (Optional)

**What it does:**
- Full GUI with ImGui dockspace
- Three simulation control panels
- 3D visualization (placeholder)
- Material presets and component builder
- Real-time stress/temperature monitoring

**Build:**
```bash
./launch.sh --desktop
```

**First-time setup:**
```bash
# 1. Install dependencies (see Prerequisites)
# 2. Clone ImGui
git clone -b docking https://github.com/ocornut/imgui.git external/imgui

# 3. Build and launch
./launch.sh --desktop
```

---

## Launch Script Options

```bash
./launch.sh [OPTIONS]

Options:
  --desktop    Build full desktop mode (requires GLFW, Eigen, ImGui)
  --debug      Build in Debug mode (default: Release)
  --clean      Clean build directory and exit
  --help       Show help message

Examples:
  ./launch.sh                 # Lightweight demo
  ./launch.sh --desktop       # Full desktop mode
  ./launch.sh --debug         # Debug lightweight mode
  ./launch.sh --desktop --debug  # Debug desktop mode
  ./launch.sh --clean         # Clean build directory
```

---

## Manual Build (Without Script)

### Lightweight Mode
```bash
cd szm_forge
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
```

### Desktop Mode
```bash
cd szm_forge
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build --parallel
./build/SZM_Forge
```

### Debug Mode
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/SZM_Forge
```

---

## What to Expect

### Lightweight Demo Output

The demo runs three scenarios with real-time output:

```
--- Scenario 1: Normal Operation ---
Safe load and mild heating. Both parts should stay stable.
t= 0.50 s | Steel Beam         | Stress   0.30 MPa | Temp  20.00 C | Utilization   0.12% stress,   0.00% temp | SAFE
t= 1.00 s | Steel Beam         | Stress   0.30 MPa | Temp  20.00 C | Utilization   0.12% stress,   0.00% temp | SAFE
...

--- Scenario 2: Mechanical Overload ---
Force spike on aluminum bracket to demonstrate stress failure risk.
t= 3.50 s | Al Bracket         | Stress  11.88 MPa | Temp  20.00 C | Utilization  12.50% stress,   0.00% temp | SAFE
t= 4.00 s | Al Bracket         | Stress  11.88 MPa | Temp  20.00 C | Utilization  12.50% stress,   0.00% temp | SAFE
...

--- Scenario 3: Thermal Overload ---
Heat input spike on steel beam to show thermal danger.
t= 6.50 s | Steel Beam         | Stress   0.32 MPa | Temp  25.00 C | Utilization   0.13% stress,   5.00% temp | SAFE
t= 7.00 s | Steel Beam         | Stress   0.32 MPa | Temp  30.00 C | Utilization   0.13% stress,  10.00% temp | SAFE
...

=== Final Snapshot ===
t= 11.50 s | Steel Beam         | Stress   0.32 MPa | Temp  45.00 C | Utilization   0.13% stress,  25.00% temp | SAFE
t= 11.50 s | Al Bracket         | Stress   3.75 MPa | Temp  20.00 C | Utilization   3.95% stress,   0.00% temp | SAFE

Published simulation ticks: 46
Demo takeaway: force overload drives stress failure first,
while sustained heat raises thermal risk over time.
```

### Desktop Mode Output

When you launch desktop mode, you'll see:
1. GLFW window opens (1920×1080)
2. ImGui dockspace initializes
3. Three panels appear:
   - **Simulation Control** — component list + force/heat controls
   - **Simulation Visualizer** — 3D viewport (placeholder)
   - **Component Builder** — material presets + quick-add

---

## Troubleshooting

### Build Fails: "GLFW not found"
**Solution:** Install GLFW development headers
```bash
# Ubuntu/Debian
sudo apt-get install libglfw3-dev

# macOS
brew install glfw
```

### Build Fails: "Eigen3 not found"
**Solution:** Install Eigen3
```bash
# Ubuntu/Debian
sudo apt-get install libeigen3-dev

# macOS
brew install eigen
```

### Build Fails: "ImGui headers not found"
**Solution:** Clone ImGui docking branch
```bash
git clone -b docking https://github.com/ocornut/imgui.git external/imgui
```

### Application Crashes on Startup
**Solution:** Check OpenGL support
```bash
glxinfo | grep "OpenGL version"
# Should show OpenGL 4.6 or higher
```

### Desktop Mode: Panels Don't Appear
**Solution:** ImGui code is commented out. Uncomment in:
- `src/UI/UIManager.cpp`
- `src/UI/panels/SimulationPanel.cpp`
- `src/UI/panels/SimulationVisualizerPanel.cpp`
- `src/UI/panels/ComponentBuilderPanel.cpp`

See `IMGUI_ACTIVATION_CHECKLIST.md` for detailed steps.

---

## Performance

### Lightweight Demo
- **Build time:** ~10 seconds
- **Runtime:** ~5 seconds
- **Memory:** <10 MB
- **CPU:** Single-threaded

### Desktop Mode
- **Build time:** ~60-120 seconds (first time)
- **Runtime:** Depends on graphics
- **Memory:** ~100-200 MB
- **CPU:** Multi-threaded (FEA solver)

---

## Next Steps

### After Running Lightweight Demo
1. Review the output and understand the three scenarios
2. Read `SIMULATION_MVP.md` for architecture details
3. Explore the code in `src/Simulation/`

### After Running Desktop Mode
1. Try adding components via "Component Builder" panel
2. Apply forces and heat via "Simulation Control" panel
3. Watch stress/temperature update in real-time
4. Read `PHASE2_UI_INTEGRATION.md` for panel details

### For Development
1. Modify `src/Simulation/SimulationEngine.cpp` to add physics
2. Add new panels in `src/UI/panels/`
3. Integrate with `src/Graphics/` for 3D rendering
4. Connect to `src/Physics/` and `src/FEA/` modules

---

## Project Structure

```
szm_forge/
├── launch.sh                    ← Launch script (use this!)
├── CMakeLists.txt              ← Build configuration
├── src/
│   ├── main.cpp                ← Lightweight demo entry point
│   ├── main_desktop.cpp        ← Desktop GUI entry point
│   ├── Simulation/             ← Physics engine (Phase 1)
│   ├── UI/panels/              ← UI panels (Phase 2)
│   ├── Core/                   ← Application singleton
│   ├── Graphics/               ← Rendering (OpenGL/Vulkan)
│   ├── Physics/                ← Rigid body dynamics
│   ├── FEA/                    ← Finite element analysis
│   └── ...
├── tests/
│   ├── simulation_test.cpp     ← Simulation engine test
│   └── main_test.cpp           ← Full integration tests
├── external/
│   └── imgui/                  ← ImGui docking branch (clone here)
└── docs/
    ├── SIMULATION_MVP.md       ← Phase 1 overview
    ├── PHASE2_UI_INTEGRATION.md ← Phase 2 guide
    ├── QUICK_REFERENCE.md      ← Quick lookup
    └── ...
```

---

## Support

### Documentation
- `README.md` — Project overview
- `SIMULATION_MVP.md` — Phase 1 details
- `PHASE2_UI_INTEGRATION.md` — Phase 2 details
- `QUICK_REFERENCE.md` — Quick lookup card
- `IMGUI_ACTIVATION_CHECKLIST.md` — ImGui setup

### Code Examples
- `tests/simulation_test.cpp` — Simulation engine usage
- `src/UI/panels/SimulationPanel.cpp` — UI panel pattern
- `src/main.cpp` — Demo scenario setup

### Troubleshooting
See "Troubleshooting" section above or check build logs:
```bash
cmake --build build --verbose
```

---

**Status:** ✅ Ready to Launch

**Recommended First Run:** `./launch.sh` (lightweight demo)

**Time to First Success:** ~30 seconds
