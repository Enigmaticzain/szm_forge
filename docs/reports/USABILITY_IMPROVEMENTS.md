# SZM Forge — Usability Improvements

**Making the app easy to use for everyone.**

---

## 🎯 What's New

### 1. Interactive CLI Menu System

Instead of running all scenarios automatically, users can now:
- **Select individual scenarios** to explore
- **Adjust settings** (speed, verbosity, colors)
- **Access help** directly from the app
- **Exit gracefully** at any time

**Run with:**
```bash
./build/SZM_Forge_Interactive
```

### 2. Quick-Start Scripts

One-command build and run:

**Linux/macOS:**
```bash
./quick_start.sh
```

**Windows:**
```cmd
quick_start.bat
```

The scripts:
- Check for required tools
- Build the project automatically
- Launch the interactive demo
- Handle errors gracefully

### 3. Getting Started Guide

New file: `GETTING_STARTED.md`

Covers:
- Fastest way to run the app
- What you'll see in the menu
- Understanding the output
- Manual build instructions
- Scenario descriptions
- Customization options
- Troubleshooting

### 4. Configuration File

New file: `config/simulation_config.json`

Users can now customize:
- Simulation parameters (forces, heat, duration)
- Material properties
- UI settings (colors, verbosity)
- Thresholds (danger, failure)

**Without recompiling!**

### 5. Color-Coded Output

Status indicators now use colors:
- 🟢 **SAFE** (Green) — All metrics below 70%
- 🟡 **WARN** (Yellow) — Any metric 70–99%
- 🔴 **FAIL** (Red) — Any metric ≥ 100%

Can be toggled in Settings menu.

### 6. Better Documentation

- **USER_MANUAL.md** — Comprehensive reference
- **GETTING_STARTED.md** — Quick start guide
- **In-app Help** — Accessible from the menu

---

## 📁 New Files Created

```
szm_forge/
├── src/CLI/
│   ├── CMakeLists.txt              ← Build configuration
│   ├── InteractiveMenu.hpp         ← Menu system header
│   └── InteractiveMenu.cpp         ← Menu system implementation
├── src/main_interactive.cpp        ← Interactive entry point
├── quick_start.sh                  ← Linux/macOS quick start
├── quick_start.bat                 ← Windows quick start
├── GETTING_STARTED.md              ← Quick start guide
├── config/simulation_config.json   ← Configuration file
└── USER_MANUAL.md                  ← Full user manual
```

---

## 🚀 Usage Comparison

### Before (Automatic Demo)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
# Runs all scenarios automatically, no user control
```

### After (Interactive Menu)
```bash
./quick_start.sh
# Builds and launches interactive menu
# User selects scenarios, adjusts settings, reads help
```

---

## 🎮 Interactive Menu Features

### Main Menu
```
1. Run Quick Demo (all scenarios)
2. Select Individual Scenario
3. Settings
4. Help & Documentation
5. Exit
```

### Scenario Selection
```
1. Table Stress Analysis
2. Normal Operation (Beam + Bracket)
3. Mechanical Overload
4. Thermal Overload
5. Custom Scenario (coming soon)
6. Back to Main Menu
```

### Settings
```
1. Simulation Speed (0.5x to 2.0x)
2. Verbose Output (ON/OFF)
3. Color Output (ON/OFF)
4. Reset to Defaults
5. Back to Main Menu
```

### Help Menu
```
1. About SZM Forge
2. How to Read Results
3. Material Database
4. Keyboard Controls
5. Back to Main Menu
```

---

## 🔧 Configuration File Usage

Edit `config/simulation_config.json` to customize:

### Change Simulation Parameters
```json
"normal_operation": {
  "parts": [
    {
      "name": "Steel Beam",
      "force_N": 150000.0,    // Change force
      "heat_W": 0.8           // Change heat input
    }
  ]
}
```

### Add New Materials
```json
"materials": {
  "Titanium-6Al-4V": {
    "yield_strength_Pa": 880000000,
    "density_kg_m3": 4500,
    "thermal_limit_C": 600
  }
}
```

### Adjust Thresholds
```json
"simulation": {
  "danger_threshold_percent": 70,
  "failure_threshold_percent": 100
}
```

---

## 📊 Output Improvements

### Before
```
t= 0.50 s | Steel Beam         | Stress   150.25 MPa | Temp   45.32 C | Utilization  60.10% stress,  15.20% temp | SAFE
```

### After (With Colors)
```
t= 0.50 s | Steel Beam         | Stress   150.25 MPa | Temp   45.32 C | Util  60.10% [GREEN]SAFE[RESET]
```

Plus:
- Cleaner formatting
- Configurable precision
- Optional verbose mode
- Color-coded status

---

## 🛠️ Technical Details

### CLI Module Architecture

```
InteractiveMenu
├── DisplayMainMenu()
├── DisplayScenarioMenu()
├── DisplaySettingsMenu()
├── DisplayHelpMenu()
├── RunQuickDemo()
├── RunTableDemo()
├── RunNormalOperation()
├── RunMechanicalOverload()
├── RunThermalOverload()
└── Config (settings storage)
```

### Integration

The CLI module:
- Uses the existing `SimulationEngine` singleton
- Requires no changes to core simulation code
- Builds as a separate library (`SZM_CLI_Module`)
- Links to `SZM_Simulation_Module`

### Build System

Updated `CMakeLists.txt`:
- Adds `src/CLI` subdirectory
- Creates `SZM_Forge_Interactive` executable
- Keeps original `SZM_Forge` executable for backward compatibility

---

## 🎓 User Experience Flow

```
User runs quick_start.sh
        ↓
Script checks prerequisites
        ↓
Script builds project
        ↓
Interactive menu launches
        ↓
User selects scenario
        ↓
Simulation runs with real-time output
        ↓
User can:
  • View results
  • Adjust settings
  • Run another scenario
  • Access help
  • Exit gracefully
```

---

## ✅ Usability Checklist

- ✅ One-command build and run
- ✅ Interactive menu system
- ✅ Scenario selection
- ✅ Settings customization
- ✅ In-app help system
- ✅ Color-coded output
- ✅ Configuration file support
- ✅ Error handling
- ✅ Cross-platform (Linux, macOS, Windows)
- ✅ Comprehensive documentation

---

## 🚀 Future Enhancements

Potential improvements for v0.3.0:

1. **Custom Scenario Builder** — Let users create scenarios from scratch
2. **Results Export** — Save simulation results to CSV/JSON
3. **Visualization** — ASCII graphs of stress/temperature over time
4. **Batch Mode** — Run multiple scenarios automatically
5. **Web Dashboard** — Browser-based UI
6. **Parameter Sweep** — Automated sensitivity analysis

---

## 📝 Migration Guide

### For Existing Users

The original `SZM_Forge` executable still works:
```bash
./build/SZM_Forge
```

New interactive mode is optional:
```bash
./build/SZM_Forge_Interactive
```

### For Developers

No breaking changes to the simulation engine. The CLI module is:
- Completely separate
- Non-invasive
- Easy to extend
- Well-documented

---

## 🎯 Design Principles

1. **User-First** — Prioritize ease of use
2. **Non-Invasive** — Don't break existing code
3. **Configurable** — Let users customize without recompiling
4. **Accessible** — Clear documentation and help
5. **Cross-Platform** — Works on Linux, macOS, Windows
6. **Extensible** — Easy to add new features

---

## 📞 Support

For questions or issues:

1. **Quick Start** — Read `GETTING_STARTED.md`
2. **Full Manual** — Read `USER_MANUAL.md`
3. **In-App Help** — Use the Help menu
4. **Configuration** — Edit `config/simulation_config.json`

---

**SZM Forge v0.2.0 — Now Easy to Use!**
