# SZM Forge — Quick Start (30 seconds)

## 🚀 Launch Now

```bash
cd /home/szm7226/Downloads/szm_forge
./launch.sh
```

**That's it.** The demo runs immediately.

---

## 📊 What You'll See

```
============================================================
 SZM Forge - Simple Mechanical Demo
 Topic: stress + thermal safety of two machine parts
============================================================

--- Scenario 1: Normal Operation ---
Safe load and mild heating. Both parts should stay stable.
t= 0.50 s | Steel Beam         | Stress   30.00 MPa | Temp   21.02 C | SAFE
t= 1.00 s | Steel Beam         | Stress   30.00 MPa | Temp   21.91 C | SAFE
...

--- Scenario 2: Mechanical Overload ---
Force spike on aluminum bracket to demonstrate stress failure risk.
t= 3.50 s | Al Bracket         | Stress  118.75 MPa | Temp   20.00 C | FAIL
...

--- Scenario 3: Thermal Overload ---
Heat input spike on steel beam to show thermal danger.
t=10.00 s | Steel Beam         | Stress   32.00 MPa | Temp   88.55 C | WARN
...
```

---

## 🎯 What This Demonstrates

✅ **Stress Calculation** — force / area = stress (MPa)  
✅ **Temperature Simulation** — heat input + dissipation  
✅ **Danger Detection** — SAFE / WARN / FAIL states  
✅ **Real Physics** — Not just pretty graphics  

---

## 📚 Next Steps

### Option 1: Understand the Code (10 min)
```bash
cat QUICK_REFERENCE.md      # API reference
cat SIMULATION_MVP.md        # How it works
```

### Option 2: Integrate New Modules (2-3 hours)
```bash
cat INTEGRATION_CHECKLIST.md # Step-by-step guide
```

### Option 3: Run Full Desktop Mode (requires setup)
```bash
# Clone ImGui docking branch
git clone -b docking https://github.com/ocornut/imgui.git external/imgui

# Build full mode
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel
./build-full/SZM_Forge
```

---

## 🔧 Build Options

### Lightweight (Default)
```bash
./launch.sh
# or manually:
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/SZM_Forge
```

### Full Desktop
```bash
./launch.sh --desktop
# or manually:
cmake -B build-full -DCMAKE_BUILD_TYPE=Release -DSZM_BUILD_FULL_DESKTOP=ON
cmake --build build-full --parallel
./build-full/SZM_Forge
```

### Debug Mode
```bash
./launch.sh --debug
```

### Clean Build
```bash
./launch.sh --clean
```

---

## 📖 Documentation

| File | Purpose |
|------|---------|
| `LAUNCH_GUIDE.md` | Complete launch guide |
| `QUICK_REFERENCE.md` | API reference card |
| `SIMULATION_MVP.md` | Phase 1 details |
| `PHASE2_UI_INTEGRATION.md` | Phase 2 details |
| `INTEGRATION_CHECKLIST.md` | Integration steps |
| `PROJECT_SUMMARY.md` | Complete overview |

---

## ✅ Verification

After running `./launch.sh`, you should see:

- [x] Three scenarios execute
- [x] Stress values in MPa
- [x] Temperature values in °C
- [x] Status: SAFE / WARN / FAIL
- [x] "Demo takeaway" message at end

---

## 🎓 Key Concepts

**Stress** = Force / Area  
**Temperature** = Heat Input - Dissipation  
**Danger** = (Stress > 80% yield) OR (Temp > 80% max)

---

## 🚀 You're Ready!

Run: `./launch.sh`

Then explore the documentation to understand what's happening.

---

**Version:** 0.3.0  
**Status:** Ready to Launch  
**Time to First Success:** 30 seconds
