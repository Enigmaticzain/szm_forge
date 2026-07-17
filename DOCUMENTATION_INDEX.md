# SZM Forge — Documentation Index

**Everything you need to know about using SZM Forge.**

---

## 🚀 Start Here

### For First-Time Users
1. Read **[GETTING_STARTED.md](GETTING_STARTED.md)** (5 min read)
2. Run `./quick_start.sh` or `quick_start.bat`
3. Select "Run Quick Demo" from the menu
4. Explore individual scenarios

### For Experienced Users
1. Run `./quick_start.sh`
2. Select individual scenarios
3. Customize settings in the menu
4. Edit `config/simulation_config.json` for advanced customization

---

## 📚 Documentation Files

### Quick References
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** — One-page cheat sheet
  - Menu options
  - Status codes
  - Troubleshooting
  - Material properties

- **[GETTING_STARTED.md](GETTING_STARTED.md)** — Quick start guide
  - Fastest way to run
  - What you'll see
  - Understanding output
  - Manual build instructions
  - Scenario descriptions

### Comprehensive Guides
- **[USER_MANUAL.md](USER_MANUAL.md)** — Complete reference
  - System requirements
  - Installation
  - Running the application
  - Understanding the demo
  - Simulation scenarios
  - Interpreting results
  - Material properties
  - Troubleshooting
  - Advanced usage

- **[USABILITY_IMPROVEMENTS.md](USABILITY_IMPROVEMENTS.md)** — Technical details
  - What's new
  - New files created
  - Usage comparison
  - Interactive menu features
  - Configuration file usage
  - Output improvements
  - Architecture details
  - Future enhancements

### Summaries
- **[IMPROVEMENTS_SUMMARY.md](IMPROVEMENTS_SUMMARY.md)** — Overview of changes
  - Key improvements
  - New files
  - Usage comparison
  - Interactive menu
  - Configuration
  - Output improvements
  - Usability checklist

- **[IMPROVEMENTS_CHECKLIST.txt](IMPROVEMENTS_CHECKLIST.txt)** — Visual checklist
  - All improvements at a glance
  - Before/after comparison
  - New executables
  - New documentation
  - Quick start steps

### Project Documentation
- **[README.md](README.md)** — Project overview
  - Vision and status
  - Quick start
  - Project structure
  - Module status
  - Build modes
  - Architecture principles

---

## 🎮 Interactive Menu

The app provides built-in help through the interactive menu:

```
--- MAIN MENU ---
1. Run Quick Demo (all scenarios)
2. Select Individual Scenario
3. Settings
4. Help & Documentation
5. Exit
```

### Help Menu (Option 4)
- About SZM Forge
- How to Read Results
- Material Database
- Keyboard Controls

---

## 🔧 Configuration

Edit `config/simulation_config.json` to customize:
- Simulation parameters (forces, heat, duration)
- Material properties
- UI settings (colors, verbosity)
- Thresholds (danger, failure)

No recompilation needed!

---

## 📊 Quick Reference

### Status Codes
- **SAFE** (Green) — < 70% utilization
- **WARN** (Yellow) — 70–99% utilization
- **FAIL** (Red) — ≥ 100% utilization

### Material Properties
- **Steel (Structural):** 250 MPa yield, 7,850 kg/m³ density
- **Aluminum (6061-T6):** 95 MPa yield, 2,700 kg/m³ density

### Scenarios
- **Table:** 200 kg load on steel table
- **Normal:** Safe operation (both parts SAFE)
- **Overload:** Excessive force (bracket approaches failure)
- **Thermal:** Excessive heat (beam temperature rises)

---

## 🚀 Quick Start Commands

### Linux/macOS
```bash
cd szm_forge
./quick_start.sh
```

### Windows
```cmd
cd szm_forge
quick_start.bat
```

### Manual Build
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
cd ..
./build/SZM_Forge_Interactive
```

---

## 📁 File Structure

```
szm_forge/
├── README.md                        Project overview
├── GETTING_STARTED.md               Quick start guide
├── USER_MANUAL.md                   Complete reference
├── QUICK_REFERENCE.md               One-page cheat sheet
├── USABILITY_IMPROVEMENTS.md        Technical details
├── IMPROVEMENTS_SUMMARY.md          Overview of changes
├── IMPROVEMENTS_CHECKLIST.txt       Visual checklist
├── quick_start.sh                   Linux/macOS quick start
├── quick_start.bat                  Windows quick start
├── CMakeLists.txt                   Build configuration
├── config/
│   ├── szm_forge.json               Project config
│   └── simulation_config.json       Simulation parameters
├── src/
│   ├── main.cpp                     Original demo
│   ├── main_interactive.cpp         Interactive entry point
│   ├── CLI/
│   │   ├── CMakeLists.txt
│   │   ├── InteractiveMenu.hpp
│   │   └── InteractiveMenu.cpp
│   └── Simulation/
│       ├── SimulationEngine.hpp
│       └── SimulationEngine.cpp
└── tests/
    └── simulation_test.cpp
```

---

## 🎯 Common Tasks

### Run the App
```bash
./quick_start.sh
```

### Select a Scenario
1. Run the app
2. Choose option 2 (Select Individual Scenario)
3. Pick a scenario (1-5)

### Adjust Settings
1. Run the app
2. Choose option 3 (Settings)
3. Adjust speed, colors, or verbosity

### Read Help
1. Run the app
2. Choose option 4 (Help & Documentation)
3. Select a topic

### Customize Parameters
1. Edit `config/simulation_config.json`
2. Change forces, heat, materials, or thresholds
3. Run the app (no rebuild needed)

### Build Manually
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

---

## 🐛 Troubleshooting

### Quick Start Script Fails
1. Check prerequisites: `cmake --version`, `g++ --version`
2. Install missing tools
3. Try manual build (see above)

### Build Fails
1. Ensure C++20 compiler: GCC 11+, Clang 14+, MSVC 2019+
2. Clean build: `rm -rf build && ./quick_start.sh`
3. Check CMake version: `cmake --version` (need 3.20+)

### App Crashes
1. Ensure terminal is 80+ columns wide
2. Check UTF-8 encoding: `export LANG=en_US.UTF-8`
3. Try clean rebuild: `rm -rf build && ./quick_start.sh`

### Output is Garbled
1. Resize terminal to 80+ columns
2. Disable color output in Settings menu
3. Check terminal encoding

---

## 📞 Getting Help

1. **Quick answers:** Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
2. **Getting started:** Read [GETTING_STARTED.md](GETTING_STARTED.md)
3. **Complete guide:** Read [USER_MANUAL.md](USER_MANUAL.md)
4. **Technical details:** Read [USABILITY_IMPROVEMENTS.md](USABILITY_IMPROVEMENTS.md)
5. **In-app help:** Use the Help menu (option 4)

---

## ✨ Features

- ✅ Interactive menu system
- ✅ Scenario selection
- ✅ Settings customization
- ✅ In-app help system
- ✅ Color-coded output
- ✅ Configuration file support
- ✅ One-command quick start
- ✅ Cross-platform (Linux, macOS, Windows)
- ✅ Comprehensive documentation
- ✅ Error handling and validation

---

## 🎓 Learning Path

1. **Beginner:** Read GETTING_STARTED.md → Run quick_start.sh → Select Quick Demo
2. **Intermediate:** Explore individual scenarios → Adjust settings → Read Help menu
3. **Advanced:** Edit config/simulation_config.json → Customize parameters → Run custom scenarios

---

## 📖 Documentation Map

```
START HERE
    ↓
QUICK_REFERENCE.md (1 page)
    ↓
GETTING_STARTED.md (5 min)
    ↓
Run ./quick_start.sh
    ↓
Explore scenarios
    ↓
Read Help menu
    ↓
USER_MANUAL.md (complete reference)
    ↓
USABILITY_IMPROVEMENTS.md (technical)
    ↓
Edit config/simulation_config.json
    ↓
Advanced customization
```

---

## 🎉 You're Ready!

1. Run `./quick_start.sh`
2. Select a scenario
3. Watch the simulation
4. Explore the menu
5. Read the help
6. Customize as needed

**Enjoy SZM Forge!**

---

**SZM Forge v0.2.0 — Physics-Driven Engineering Platform**

*Last Updated: 2024*
