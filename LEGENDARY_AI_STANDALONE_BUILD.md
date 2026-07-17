# Legendary AI Assistant — Standalone Build Guide

**Build the Legendary AI Assistant independently from SZM Forge**

---

## Prerequisites

```bash
# Install nlohmann-json
sudo apt-get install nlohmann-json3-dev

# Or download headers manually
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
mkdir -p /usr/include/nlohmann
sudo cp json.hpp /usr/include/nlohmann/
```

---

## Build Steps

### 1. Navigate to Project

```bash
cd /mnt/D/projects/szm_forge
```

### 2. Create Build Directory

```bash
mkdir -p build-legendary
cd build-legendary
```

### 3. Configure CMake

```bash
cmake -B . -DCMAKE_BUILD_TYPE=Release -f ../CMakeLists_LegendaryAI.txt ..
```

Or use the standalone CMakeLists:

```bash
cp ../CMakeLists_LegendaryAI.txt CMakeLists.txt
cmake -B . -DCMAKE_BUILD_TYPE=Release ..
```

### 4. Build

```bash
cmake --build . --parallel
```

### 5. Run Tests

```bash
./test_legendary_ai
```

---

## Expected Output

```
================================================================================
  LEGENDARY AI ASSISTANT — TEST PROGRAM
================================================================================

[1/6] Initializing Legendary AI Assistant...
✓ AI initialized successfully

[2/6] Getting AI state...
✓ Current stage: 0
✓ Overall confidence: 0.5
✓ Tasks completed: 0

[3/6] Loading skill graph...
✓ Skills loaded: 5 skills

[4/6] Getting memory statistics...
✓ Total memories: 0
✓ Total skills: 5
✓ Average skill level: 1

[5/6] Getting learning progress...
✓ Stage progress: 0

[6/6] Generating comprehensive report...
✓ Report generated successfully

================================================================================
  COMPREHENSIVE AI REPORT
================================================================================

{
  "state": {
    "current_stage": 0,
    "stage_progress": 0.0,
    "total_tasks_completed": 0,
    "total_failures": 0,
    "overall_confidence": 0.5
  },
  ...
}

================================================================================
  TEST RESULTS
================================================================================

✓ All tests passed successfully!

Legendary AI Assistant is ready for integration with SZM Forge.
```

---

## Troubleshooting

### Error: `nlohmann/json.hpp: No such file or directory`

**Solution:**
```bash
sudo apt-get install nlohmann-json3-dev
```

Or manually:
```bash
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
sudo mkdir -p /usr/include/nlohmann
sudo cp json.hpp /usr/include/nlohmann/
```

### Error: `CMakeLists.txt not found`

**Solution:**
```bash
cd build-legendary
cp ../CMakeLists_LegendaryAI.txt CMakeLists.txt
cmake -B . -DCMAKE_BUILD_TYPE=Release ..
```

### Build Errors

**Check:**
1. C++20 compiler available: `g++ --version` (need GCC 10+)
2. CMake 3.20+: `cmake --version`
3. nlohmann-json installed: `dpkg -l | grep nlohmann`

---

## Integration with SZM Forge

Once built successfully, integrate with main SZM Forge:

### 1. Update Root CMakeLists.txt

```cmake
add_subdirectory(src/AI/LegendaryCore)
target_link_libraries(SZM_Forge PUBLIC SZM_LegendaryCore_Module)
```

### 2. Initialize in Application

```cpp
#include "AI/LegendaryCore/LegendaryAIAssistant.hpp"

void Application::Initialize() {
    auto& ai = SZM::AI::Legendary::LegendaryAIAssistant::GetInstance();
    ai.Initialize();
}
```

### 3. Hook into Main Loop

```cpp
void Application::TickSystems() {
    auto& ai = SZM::AI::Legendary::LegendaryAIAssistant::GetInstance();
    ai.CheckForStageProgression();
}
```

---

## Files

### Source Code
```
src/AI/LegendaryCore/
├── MemoryEngine.hpp/cpp
├── SkillSystem.hpp/cpp
├── AgentFramework.hpp/cpp
├── TrainingPipeline.hpp/cpp
├── ModificationSystem.hpp/cpp
├── LegendaryAIAssistant.hpp/cpp
├── test_legendary_ai.cpp
└── CMakeLists.txt
```

### Build Configuration
```
CMakeLists_LegendaryAI.txt  (Standalone build)
```

---

## Next Steps

1. ✅ Build standalone
2. ✅ Run tests
3. ⏳ Integrate with SZM Forge
4. ⏳ Create UI panels
5. ⏳ Train on codebase

---

**Legendary AI Assistant — Building the future of engineering intelligence.**
