# SZM Forge — User Manual v0.2.0

**Physics-Driven Engineering Platform**

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Running the Application](#running-the-application)
3. [Understanding the Demo](#understanding-the-demo)
4. [Simulation Scenarios](#simulation-scenarios)
5. [Interpreting Results](#interpreting-results)
6. [Troubleshooting](#troubleshooting)

---

## Getting Started

### System Requirements

- **OS:** Linux, macOS, or Windows (with WSL2)
- **Compiler:** GCC 11+, Clang 14+, or MSVC 2019+
- **CMake:** 3.20 or later
- **RAM:** 2 GB minimum (4 GB recommended)

### Installation

1. Clone or extract the SZM Forge repository:
   ```bash
   cd szm_forge
   ```

2. Create a build directory and configure:
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```

3. Build the project:
   ```bash
   cmake --build build --parallel
   ```

---

## Running the Application

### Lightweight Demo Mode (Default)

The default build runs a **terminal-based mechanical simulation** with no GUI dependencies.

```bash
./build/SZM_Forge
```

**Output:** Three sequential simulation scenarios displayed in the terminal with real-time stress and thermal analysis.

**Duration:** ~30 seconds

### Running Tests

To verify the simulation engine:

```bash
./build/SZM_TestRunner
```

---

## Understanding the Demo

### What the Demo Shows

The application demonstrates a **physics-driven engineering platform** by simulating real mechanical and thermal behavior of machine parts under load.

#### Part 1: Table Stress Analysis

A steel table with a 200 kg load is analyzed:

- **Tabletop:** Large flat plate experiencing bending stress
- **Four Legs:** Vertical support members under compression

The demo shows:
1. **Normal load** (200 kg) — all parts remain safe
2. **Overload scenario** (+500 kg extra) — stress utilization increases, some parts approach failure

#### Part 2: Beam & Bracket Simulation

A steel beam and aluminum bracket are subjected to combined mechanical and thermal loads across three scenarios.

---

## Simulation Scenarios

### Scenario 1: Normal Operation

**Conditions:**
- Steel beam: 150 kN force, 0.8 W heat input
- Aluminum bracket: 22 kN force, no heat
- Duration: 3 seconds (6 steps × 0.5 s each)

**Expected Result:** Both parts remain in the **SAFE** zone with low stress and temperature utilization.

**What to Look For:**
- Stress utilization < 50%
- Temperature utilization < 30%
- Status: **SAFE**

---

### Scenario 2: Mechanical Overload

**Conditions:**
- Steel beam: 160 kN force, 0.8 W heat input
- Aluminum bracket: 95 kN force (4× normal), no heat
- Duration: 3 seconds

**Expected Result:** The aluminum bracket approaches its yield strength, entering the **WARN** zone.

**What to Look For:**
- Bracket stress utilization rises to 80–95%
- Status transitions from **SAFE** → **WARN**
- Beam remains stable

**Engineering Insight:** Aluminum has lower yield strength (95 MPa) than steel (250 MPa), so it fails first under overload.

---

### Scenario 3: Thermal Overload

**Conditions:**
- Steel beam: 160 kN force, 10 W heat input (12× normal)
- Aluminum bracket: 30 kN force, no heat
- Duration: 5 seconds

**Expected Result:** The steel beam's temperature rises rapidly, entering the **WARN** zone.

**What to Look For:**
- Beam temperature rises from ~20°C to 100–150°C
- Temperature utilization climbs to 80–95%
- Stress remains moderate
- Status: **WARN** (thermal danger)

**Engineering Insight:** Thermal stress can degrade material properties and cause failure independent of mechanical load.

---

## Interpreting Results

### Output Format

Each simulation step prints:

```
t= 0.50 s | Steel Beam         | Stress   150.25 MPa | Temp   45.32 C | Utilization  60.10% stress,  15.20% temp | SAFE
```

### Key Metrics

| Metric | Meaning | Safe Range |
|--------|---------|-----------|
| **Stress (MPa)** | Mechanical load per unit area | < Yield Strength |
| **Temp (°C)** | Component temperature | < Material limit |
| **Utilization %** | Ratio of current to maximum safe value | < 100% |
| **Status** | Overall safety state | SAFE |

### Status Codes

| Status | Meaning | Action |
|--------|---------|--------|
| **SAFE** | All metrics below 70% utilization | Continue operation |
| **WARN** | Any metric 70–99% utilization | Monitor closely; reduce load |
| **FAIL** | Any metric ≥ 100% utilization | Stop immediately; redesign required |

---

## Material Properties

The simulation uses real material data:

### Steel (Structural)
- **Yield Strength:** 250 MPa
- **Density:** 7,850 kg/m³
- **Thermal Limit:** ~400°C

### Aluminum (6061-T6)
- **Yield Strength:** 95 MPa
- **Density:** 2,700 kg/m³
- **Thermal Limit:** ~150°C

---

## Troubleshooting

### Build Fails with "CMake not found"

**Solution:** Install CMake 3.20+
```bash
# Ubuntu/Debian
sudo apt-get install cmake

# macOS
brew install cmake

# Windows
Download from https://cmake.org/download/
```

### Build Fails with "C++ compiler not found"

**Solution:** Install a C++20 compatible compiler
```bash
# Ubuntu/Debian
sudo apt-get install g++-11

# macOS
xcode-select --install

# Windows
Install Visual Studio 2019 or later
```

### Application Crashes on Startup

**Solution:** Ensure the build directory is clean
```bash
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
```

### Output is Garbled or Incomplete

**Solution:** Ensure terminal supports UTF-8 and has sufficient width (80+ columns)
```bash
export LANG=en_US.UTF-8
# Resize terminal window to at least 80 columns wide
./build/SZM_Forge
```

### Simulation Runs Too Fast or Too Slow

**Solution:** This is normal. The simulation runs at wall-clock speed. Output speed depends on terminal I/O performance.

---

## Advanced Usage

### Full Desktop Mode (Optional)

To build with GUI, graphics, and UI modules:

```bash
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel
./build-full/SZM_Forge
```

**Requirements:**
- GLFW 3.3.8
- Eigen3 >= 3.4
- Dear ImGui (docking branch) cloned to `external/imgui/`

### Modifying Simulation Parameters

Edit `src/main.cpp` to change:
- Load magnitudes (kLoadKg, kLegForce, etc.)
- Heat input (beamHeatW, bracketHeatW)
- Simulation duration (steps, dt)
- Material properties (area, density, yieldStrength)

Rebuild with:
```bash
cmake --build build --parallel
./build/SZM_Forge
```

---

## Architecture Overview

### Core Components

| Component | Purpose |
|-----------|---------|
| **SimulationEngine** | Singleton managing all simulation state |
| **SimulationComponent** | Individual part with stress/thermal properties |
| **EventBus** | Zero-coupling inter-module communication |
| **MaterialDatabase** | Thread-safe material property registry |

### Data Flow

```
User Input
    ↓
SimulationEngine::Tick()
    ↓
Compute Stress & Temperature
    ↓
Update Component State
    ↓
Publish Results via EventBus
    ↓
Terminal Output
```

---

## Performance Notes

- **Simulation Speed:** ~1,000 components per frame on modern hardware
- **Memory Usage:** ~1 MB per 100 components
- **Precision:** Double-precision floating-point (IEEE 754)

---

## Support & Documentation

- **Project README:** See `README.md` for architecture and module status
- **Configuration:** See `config/szm_forge.json` for project settings
- **Source Code:** All modules in `src/` with inline documentation

---

## Version History

### v0.2.0 (Current)
- ✅ Stress analysis (Von Mises equivalent stress)
- ✅ Thermal analysis (transient heat diffusion)
- ✅ Material database with real properties
- ✅ Multi-scenario demonstration
- ✅ Terminal-based output

### v0.1.0
- Initial release with basic simulation framework

---

## License & Attribution

SZM Forge — Physics-Driven Engineering Platform  
© 2024 — All Rights Reserved

---

**Last Updated:** 2024  
**Manual Version:** 1.0
