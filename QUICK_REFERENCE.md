# SZM Forge — Quick Reference Card

## 🚀 Quick Start (30 seconds)

```bash
cd szm_forge
./quick_start.sh          # Linux/macOS
# or
quick_start.bat           # Windows
```

Done! The app will build and launch.

---

## 📋 Main Menu Options

| Option | What It Does |
|--------|-------------|
| **1** | Run all scenarios automatically (5 min) |
| **2** | Pick one scenario to explore |
| **3** | Adjust speed, colors, verbosity |
| **4** | Learn about the app and results |
| **5** | Exit the app |

---

## 🎯 Scenario Selection

| Scenario | What Happens |
|----------|-------------|
| **Table** | 200 kg load on a steel table |
| **Normal** | Safe operation (both parts SAFE) |
| **Overload** | Excessive force (bracket approaches failure) |
| **Thermal** | Excessive heat (beam temperature rises) |

---

## 📊 Understanding Output

```
t= 0.50 s | Steel Beam | Stress 150.25 MPa | Temp 45.32 C | Util 60.10% SAFE
```

| Column | Meaning |
|--------|---------|
| **t=** | Simulation time (seconds) |
| **Name** | Component being tested |
| **Stress** | Mechanical load (MPa) |
| **Temp** | Temperature (°C) |
| **Util** | How much capacity is used (%) |
| **Status** | SAFE / WARN / FAIL |

---

## 🟢 Status Codes

| Status | Meaning | Action |
|--------|---------|--------|
| **SAFE** | < 70% utilization | Continue |
| **WARN** | 70–99% utilization | Monitor |
| **FAIL** | ≥ 100% utilization | Stop |

---

## ⚙️ Settings Menu

| Option | What It Does |
|--------|-------------|
| **1** | Change simulation speed (0.5x to 2.0x) |
| **2** | Toggle verbose output |
| **3** | Toggle color output |
| **4** | Reset to defaults |

---

## 📚 Help Menu

| Option | What It Does |
|--------|-------------|
| **1** | About SZM Forge |
| **2** | How to read results |
| **3** | Material properties |
| **4** | Keyboard controls |

---

## 🔧 Customization

### Change Simulation Parameters
Edit `config/simulation_config.json`:
```json
"force_N": 150000.0,    // Change force
"heat_W": 0.8           // Change heat
```

### Add New Materials
```json
"materials": {
  "MyMaterial": {
    "yield_strength_Pa": 250000000,
    "density_kg_m3": 7850,
    "thermal_limit_C": 400
  }
}
```

---

## 🐛 Troubleshooting

| Problem | Solution |
|---------|----------|
| CMake not found | Install CMake 3.20+ |
| Compiler not found | Install GCC 11+ or Clang 14+ |
| Permission denied | `chmod +x quick_start.sh` |
| Build fails | `rm -rf build && ./quick_start.sh` |
| Output garbled | Resize terminal to 80+ columns |

---

## 📖 Documentation

| File | Purpose |
|------|---------|
| **GETTING_STARTED.md** | Quick start guide |
| **USER_MANUAL.md** | Complete reference |
| **USABILITY_IMPROVEMENTS.md** | Technical details |
| **config/simulation_config.json** | Configuration |

---

## 💡 Tips

- Start with **Quick Demo** to see all scenarios
- Use **Settings** to slow down if too fast
- Enable **Color Output** for easier reading
- Read **Help Menu** to understand materials
- Edit **config.json** for custom parameters

---

## 🎮 Keyboard Input

When prompted for a choice:
1. Type a number (1–5)
2. Press Enter
3. Menu updates

---

## 📊 Material Properties

### Steel (Structural)
- Yield: 250 MPa
- Density: 7,850 kg/m³
- Thermal Limit: 400°C

### Aluminum (6061-T6)
- Yield: 95 MPa
- Density: 2,700 kg/m³
- Thermal Limit: 150°C

---

## 🚀 Next Steps

1. Run `./quick_start.sh`
2. Select "Run Quick Demo"
3. Explore individual scenarios
4. Adjust settings
5. Read help menu
6. Edit config.json for custom parameters

---

## 📞 Need Help?

1. Check **GETTING_STARTED.md**
2. Read **USER_MANUAL.md**
3. Use in-app **Help Menu**
4. Edit **config/simulation_config.json**

---

**SZM Forge v0.2.0 — Physics-Driven Engineering Platform**

*Print this card and keep it handy!*
