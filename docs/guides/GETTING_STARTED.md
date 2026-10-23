# SZM Forge — Getting Started Guide

**Quick, easy, and interactive.**

---

## 🚀 Fastest Way to Run

### Linux / macOS
```bash
cd szm_forge
./quick_start.sh
```

### Windows
```cmd
cd szm_forge
quick_start.bat
```

That's it! The script will:
1. Check for required tools
2. Build the project
3. Launch the interactive demo

---

## 📋 What You'll See

When you run the app, you'll get an interactive menu:

```
============================================================
 SZM FORGE v0.2.0
 Physics-Driven Engineering Platform
============================================================

Welcome! This interactive demo shows mechanical and thermal
simulation of machine parts under various load conditions.

--- MAIN MENU ---
1. Run Quick Demo (all scenarios)
2. Select Individual Scenario
3. Settings
4. Help & Documentation
5. Exit

Enter choice (1-5):
```

### Menu Options

**1. Quick Demo** — Runs all scenarios automatically (5 minutes)
- Table stress analysis
- Normal operation
- Mechanical overload
- Thermal overload

**2. Individual Scenario** — Pick one scenario to explore
- Table Stress Analysis
- Normal Operation
- Mechanical Overload
- Thermal Overload
- Custom Scenario (coming soon)

**3. Settings** — Customize the experience
- Simulation speed (0.5x to 2.0x)
- Verbose output (ON/OFF)
- Color output (ON/OFF)
- Reset to defaults

**4. Help** — Learn about the app
- About SZM Forge
- How to read results
- Material database
- Keyboard controls

**5. Exit** — Close the app

---

## 🎯 Understanding the Output

Each simulation step shows:

```
t= 0.50 s | Steel Beam         | Stress   150.25 MPa | Temp   45.32 C | Util  60.10% SAFE
```

### What Each Column Means

| Column | Meaning | Example |
|--------|---------|---------|
| **t=** | Simulation time | 0.50 s |
| **Name** | Component being tested | Steel Beam |
| **Stress** | Mechanical load per unit area | 150.25 MPa |
| **Temp** | Component temperature | 45.32 °C |
| **Util** | How much of the material's capacity is used | 60.10% |
| **Status** | Safety level | SAFE / WARN / FAIL |

### Status Meanings

- **SAFE** (Green) — All metrics below 70% utilization. Safe to operate.
- **WARN** (Yellow) — Any metric 70–99% utilization. Monitor closely; reduce load.
- **FAIL** (Red) — Any metric ≥ 100% utilization. Stop immediately; redesign needed.

---

## 🔧 Manual Build (If Quick Start Doesn't Work)

### Step 1: Install Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install cmake g++-11 build-essential
```

**macOS:**
```bash
brew install cmake
xcode-select --install
```

**Windows:**
- Download CMake from https://cmake.org/download/
- Install Visual Studio 2019 or later

### Step 2: Build

```bash
cd szm_forge
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
cd ..
```

### Step 3: Run

**Interactive mode (recommended):**
```bash
./build/SZM_Forge_Interactive
```

**Automatic demo:**
```bash
./build/SZM_Forge
```

---

## 📚 Scenario Descriptions

### Table Stress Analysis
A steel table with a 200 kg load is analyzed:
- **Tabletop:** Large flat plate experiencing bending stress
- **Four Legs:** Vertical support members under compression

Shows what happens when you add weight to a table.

### Normal Operation
A steel beam and aluminum bracket under safe loads:
- **Beam:** 150 kN force, 0.8 W heat
- **Bracket:** 22 kN force, no heat

Both parts stay in the SAFE zone.

### Mechanical Overload
Same components, but with excessive force:
- **Beam:** 160 kN force, 0.8 W heat
- **Bracket:** 95 kN force (4× normal), no heat

The aluminum bracket approaches failure due to its lower yield strength.

### Thermal Overload
Same components, but with excessive heat:
- **Beam:** 160 kN force, 10 W heat (12× normal)
- **Bracket:** 30 kN force, no heat

The steel beam's temperature rises rapidly, entering the danger zone.

---

## ⚙️ Customization

### Change Simulation Speed

In the Settings menu, select option 1 and enter a multiplier:
- `0.5` = Half speed (slower, easier to read)
- `1.0` = Normal speed (default)
- `2.0` = Double speed (faster)

### Enable Color Output

In the Settings menu, toggle option 3 to enable/disable colored status indicators.

### Verbose Output

In the Settings menu, toggle option 2 for detailed logging.

---

## 🐛 Troubleshooting

### "CMake not found"
Install CMake 3.20 or later from https://cmake.org/download/

### "C++ compiler not found"
- **Linux:** `sudo apt-get install g++-11`
- **macOS:** `xcode-select --install`
- **Windows:** Install Visual Studio 2019+

### Build fails with "Permission denied"
On Linux/macOS, make the script executable:
```bash
chmod +x quick_start.sh
```

### Output is garbled or incomplete
Ensure your terminal:
- Supports UTF-8 encoding
- Is at least 80 columns wide
- Has a dark background (for better readability)

### Application crashes on startup
Try a clean rebuild:
```bash
rm -rf build
./quick_start.sh
```

---

## 📖 Next Steps

1. **Run the Quick Demo** to see all scenarios
2. **Explore individual scenarios** to understand each one
3. **Adjust settings** to customize the experience
4. **Read the Help menu** for detailed explanations
5. **Check the full User Manual** (USER_MANUAL.md) for advanced topics

---

## 💡 Tips

- Start with the Quick Demo to get an overview
- Use the Settings menu to slow down the simulation if it's too fast
- Enable color output for easier status reading
- Read the Help menu to understand material properties
- Try different scenarios to see how materials behave under different loads

---

## 📞 Support

For issues or questions:
1. Check the Help menu in the app
2. Read the full User Manual (USER_MANUAL.md)
3. Review the README.md for architecture details

---

**SZM Forge v0.2.0 — Physics-Driven Engineering Platform**
