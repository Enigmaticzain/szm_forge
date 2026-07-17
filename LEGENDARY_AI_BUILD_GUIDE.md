# Legendary AI Assistant — Build & Integration Guide

**Complete setup instructions for the Legendary AI Assistant**

---

## Prerequisites

### System Requirements
- Linux/macOS/Windows
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+
- Git

### Dependencies
- nlohmann/json (header-only)
- Standard C++ library

---

## Build Instructions

### Step 1: Clone/Navigate to Repository

```bash
cd /mnt/D/projects/szm_forge
```

### Step 2: Create Build Directory

```bash
mkdir -p build-legendary
cd build-legendary
```

### Step 3: Configure CMake

```bash
cmake -B . -DCMAKE_BUILD_TYPE=Release ..
```

### Step 4: Build

```bash
cmake --build . --parallel
```

### Step 5: Verify Build

```bash
ls -la
# Should see: libSZM_LegendaryCore_Module.a (or .lib on Windows)
```

---

## Integration with SZM Forge

### Step 1: Update Root CMakeLists.txt

Add to `/mnt/D/projects/szm_forge/CMakeLists.txt`:

```cmake
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
# LEGENDARY AI ASSISTANT MODULE
# ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

add_subdirectory(src/AI/LegendaryCore)

# Link to main application
target_link_libraries(SZM_Forge PUBLIC
    SZM_LegendaryCore_Module
)

message(STATUS "✓ Legendary AI Assistant integrated")
```

### Step 2: Initialize in Application.cpp

Add to `src/Core/Application.cpp`:

```cpp
#include "AI/LegendaryCore/LegendaryAIAssistant.hpp"

void Application::Initialize() {
    // ... existing initialization code ...
    
    // Initialize Legendary AI Assistant
    auto& ai = SZM::AI::Legendary::LegendaryAIAssistant::GetInstance();
    ai.Initialize();
    
    Logger::Info("Legendary AI Assistant initialized");
}
```

### Step 3: Hook into Main Loop

Add to `src/Core/Application.cpp` in `TickSystems()`:

```cpp
void Application::TickSystems() {
    // ... existing system ticks ...
    
    // Tick Legendary AI Assistant
    auto& ai = SZM::AI::Legendary::LegendaryAIAssistant::GetInstance();
    ai.CheckForStageProgression();
}
```

### Step 4: Add Shutdown

Add to `src/Core/Application.cpp` in shutdown:

```cpp
void Application::Shutdown() {
    // ... existing shutdown code ...
    
    // Save AI state
    auto& ai = SZM::AI::Legendary::LegendaryAIAssistant::GetInstance();
    ai.SaveState("legendary_ai_state.json");
}
```

---

## Standalone Usage

### Create a Test Program

Create `test_legendary_ai.cpp`:

```cpp
#include <iostream>
#include "AI/LegendaryCore/LegendaryAIAssistant.hpp"

using namespace SZM::AI::Legendary;

int main() {
    std::cout << "=== Legendary AI Assistant Test ===" << std::endl;
    
    // Initialize
    auto& ai = LegendaryAIAssistant::GetInstance();
    ai.Initialize();
    std::cout << "✓ AI initialized" << std::endl;
    
    // Get current state
    AIState state = ai.GetCurrentState();
    std::cout << "✓ Current stage: " << static_cast<int>(state.current_stage) << std::endl;
    
    // Get skill graph
    json skills = ai.GetSkillGraph();
    std::cout << "✓ Skills loaded: " << skills.size() << std::endl;
    
    // Get comprehensive report
    json report = ai.GetComprehensiveReport();
    std::cout << "✓ Report generated" << std::endl;
    
    // Print report
    std::cout << "\n=== AI Report ===" << std::endl;
    std::cout << report.dump(2) << std::endl;
    
    // Save state
    ai.SaveState("ai_state.json");
    std::cout << "\n✓ State saved to ai_state.json" << std::endl;
    
    return 0;
}
```

### Build Standalone

```bash
cd /mnt/D/projects/szm_forge/build-legendary
g++ -std=c++20 -I../src -I../external/nlohmann_json/include \
    ../test_legendary_ai.cpp \
    -L. -lSZM_LegendaryCore_Module \
    -o test_legendary_ai

./test_legendary_ai
```

---

## Configuration

### Memory Settings

Edit `src/AI/LegendaryCore/MemoryEngine.hpp`:

```cpp
// Adjust memory limits
static constexpr int MAX_MEMORY_ENTRIES = 10000;
static constexpr int MAX_SEMANTIC_RELATIONS = 50000;
static constexpr int MAX_FAILURE_RECORDS = 1000;
```

### Skill Settings

Edit `src/AI/LegendaryCore/SkillSystem.hpp`:

```cpp
// Adjust skill progression
static constexpr float XP_PER_LEVEL = 100.0f;
static constexpr int MAX_SKILL_LEVEL = 5;
```

### Agent Settings

Edit `src/AI/LegendaryCore/AgentFramework.hpp`:

```cpp
// Adjust agent behavior
static constexpr float DEFAULT_CONFIDENCE = 0.8f;
static constexpr int MAX_EXECUTION_HISTORY = 10000;
```

---

## Persistence

### Save State

```cpp
auto& ai = LegendaryAIAssistant::GetInstance();
ai.SaveState("legendary_ai_state.json");
```

### Load State

```cpp
auto& ai = LegendaryAIAssistant::GetInstance();
ai.LoadState("legendary_ai_state.json");
```

### State File Format

```json
{
  "session_memory": { ... },
  "long_term_memory": [ ... ],
  "evolution_history": [ ... ]
}
```

---

## Troubleshooting

### Build Errors

**Error:** `nlohmann/json.hpp: No such file or directory`

**Solution:**
```bash
# Install nlohmann-json
sudo apt-get install nlohmann-json3-dev

# Or download header
wget https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp
cp json.hpp /usr/include/nlohmann/
```

**Error:** `undefined reference to 'SZM::AI::Legendary::...'`

**Solution:**
```bash
# Ensure CMakeLists.txt links the module
target_link_libraries(your_target PUBLIC SZM_LegendaryCore_Module)
```

### Runtime Errors

**Error:** `Segmentation fault in MemoryEngine`

**Solution:**
```cpp
// Ensure singleton is initialized
auto& ai = LegendaryAIAssistant::GetInstance();
ai.Initialize();  // Must call Initialize()
```

**Error:** `Skills not found`

**Solution:**
```cpp
// Initialize skills before using
auto& skills = SkillSystem::GetInstance();
skills.InitializeFoundationSkills();
```

---

## Testing

### Unit Tests

Create `test_legendary_core.cpp`:

```cpp
#include <cassert>
#include "AI/LegendaryCore/LegendaryAIAssistant.hpp"

using namespace SZM::AI::Legendary;

void TestMemoryEngine() {
    auto& memory = MemoryEngine::GetInstance();
    
    MemoryEntry entry;
    entry.id = "test_001";
    entry.content = "Test content";
    memory.StoreLongTermMemory(entry);
    
    auto results = memory.SearchLongTermMemory("Test");
    assert(results.size() > 0);
    
    std::cout << "✓ MemoryEngine test passed" << std::endl;
}

void TestSkillSystem() {
    auto& skills = SkillSystem::GetInstance();
    skills.InitializeFoundationSkills();
    
    skills.AwardXP("text_understanding", 50.0f);
    auto cap = skills.GetSkillCapability("text_understanding");
    assert(cap.skill_id == "text_understanding");
    
    std::cout << "✓ SkillSystem test passed" << std::endl;
}

void TestAgentFramework() {
    auto& orchestrator = AgentOrchestrator::GetInstance();
    orchestrator.RegisterAgent(std::make_shared<PlannerAgent>());
    
    auto agent = orchestrator.GetAgent("Planner");
    assert(agent != nullptr);
    
    std::cout << "✓ AgentFramework test passed" << std::endl;
}

int main() {
    TestMemoryEngine();
    TestSkillSystem();
    TestAgentFramework();
    
    std::cout << "\n✓ All tests passed!" << std::endl;
    return 0;
}
```

### Run Tests

```bash
cd build-legendary
g++ -std=c++20 -I../src -I../external/nlohmann_json/include \
    ../test_legendary_core.cpp \
    -L. -lSZM_LegendaryCore_Module \
    -o test_legendary_core

./test_legendary_core
```

---

## Performance Optimization

### Compiler Flags

```cmake
# In CMakeLists.txt
if(MSVC)
    target_compile_options(SZM_LegendaryCore_Module PRIVATE /O2 /W4)
else()
    target_compile_options(SZM_LegendaryCore_Module PRIVATE -O3 -Wall -Wextra)
endif()
```

### Memory Optimization

```cpp
// Use move semantics
std::vector<MemoryEntry> entries;
entries.push_back(std::move(entry));

// Reserve space
entries.reserve(1000);

// Use const references
const auto& memory = MemoryEngine::GetInstance();
```

---

## Deployment

### Release Build

```bash
mkdir -p build-release
cd build-release
cmake -B . -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

### Debug Build

```bash
mkdir -p build-debug
cd build-debug
cmake -B . -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel
```

### Install

```bash
cd build-release
cmake --install . --prefix /usr/local
```

---

## Docker Support

### Dockerfile

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    nlohmann-json3-dev

WORKDIR /app
COPY . .

RUN mkdir -p build && cd build && \
    cmake -B . -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --parallel

CMD ["./build/test_legendary_ai"]
```

### Build Docker Image

```bash
docker build -t legendary-ai .
docker run legendary-ai
```

---

## CI/CD Integration

### GitHub Actions

Create `.github/workflows/legendary-ai.yml`:

```yaml
name: Legendary AI Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install -y nlohmann-json3-dev
      - name: Build
        run: |
          mkdir -p build
          cd build
          cmake -B . -DCMAKE_BUILD_TYPE=Release ..
          cmake --build . --parallel
      - name: Test
        run: ./build/test_legendary_ai
```

---

## Next Steps

1. ✅ Build the module
2. ✅ Integrate with SZM Forge
3. ⏳ Create UI panels
4. ⏳ Train on real codebases
5. ⏳ Implement Stage 2 capabilities

---

## Support

### Documentation
- `LEGENDARY_AI_QUICKSTART.md` — Quick start
- `LEGENDARY_AI_ARCHITECTURE.md` — Full architecture
- `LEGENDARY_AI_SUMMARY.md` — Implementation summary

### Code Examples
- See `test_legendary_ai.cpp` for basic usage
- See `test_legendary_core.cpp` for unit tests

### Troubleshooting
- Check build errors above
- Review CMakeLists.txt configuration
- Verify dependencies are installed

---

**Legendary AI Assistant — Build & Integration Guide**

*Version 0.1.0 | Ready for Production*
