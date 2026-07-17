# Legendary AI Assistant — Quick Start Guide

## What is This?

The **Legendary AI Assistant** is a self-improving AI operating system that will become the intelligence layer of SZM Forge. It learns, evolves, and improves itself over time.

---

## Quick Start (5 minutes)

### 1. Build the System

```bash
cd /mnt/D/projects/szm_forge
mkdir -p build-legendary
cd build-legendary
cmake -B . -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

### 2. Use in Your Code

```cpp
#include "AI/LegendaryCore/LegendaryAIAssistant.hpp"

using namespace SZM::AI::Legendary;

int main() {
    // Get the AI instance
    auto& ai = LegendaryAIAssistant::GetInstance();
    
    // Initialize
    ai.Initialize();
    
    // Process a request
    json response = ai.ProcessRequest("Analyze the codebase");
    
    // Get current state
    AIState state = ai.GetCurrentState();
    std::cout << "AI Stage: " << static_cast<int>(state.current_stage) << std::endl;
    
    // Get comprehensive report
    json report = ai.GetComprehensiveReport();
    std::cout << report.dump(2) << std::endl;
    
    return 0;
}
```

---

## Core Concepts

### Memory Engine

The AI remembers everything it learns:

```cpp
auto& memory = MemoryEngine::GetInstance();

// Store knowledge
MemoryEntry entry;
entry.id = "concept_001";
entry.type = "concept";
entry.content = "Physics simulation uses Newton's laws";
entry.tags = {"physics", "simulation"};
memory.StoreLongTermMemory(entry);

// Search knowledge
auto results = memory.SearchLongTermMemory("physics");
```

### Skill System

The AI has measurable skills that level up:

```cpp
auto& skills = SkillSystem::GetInstance();

// Initialize skills
skills.InitializeFoundationSkills();

// Award XP
skills.AwardXP("code_understanding", 10.0f);

// Try to level up
if (skills.TryLevelUp("code_understanding")) {
    std::cout << "Leveled up!" << std::endl;
}

// Get skill graph
json graph = skills.GetSkillGraph();
```

### Agent Framework

The AI has specialized agents that work together:

```cpp
auto& orchestrator = AgentOrchestrator::GetInstance();

// Register agents
orchestrator.RegisterAgent(std::make_shared<PlannerAgent>());
orchestrator.RegisterAgent(std::make_shared<ExecutionAgent>());

// Execute a task
Task task;
task.id = "task_001";
task.description = "Optimize the rendering system";
task.subtasks = {"analyze", "identify_bottlenecks", "propose_fixes"};

ExecutionResult result = orchestrator.ExecuteTask(task);
std::cout << "Success: " << result.success << std::endl;
```

### Training Pipeline

The AI learns from repositories and documents:

```cpp
auto& training = TrainingPipeline::GetInstance();

// Import a repository
training.ImportRepository("/path/to/szm_forge");

// Start a training session
TrainingSession session = training.StartTrainingSession(TrainingMode::REPOSITORY);

// Process the data
training.ProcessTrainingData(session);

// Complete the session
training.CompleteTrainingSession(session);

// Get statistics
json stats = training.GetTrainingStatistics();
```

### Modification System

The AI can safely propose code changes:

```cpp
auto& modifications = ModificationSystem::GetInstance();

// Create a proposal
std::vector<CodeDiff> diffs;
CodeDiff diff;
diff.file_path = "src/Physics/PhysicsEngine.cpp";
diff.old_content = "// old";
diff.new_content = "// new";
diffs.push_back(diff);

auto proposal = modifications.CreateProposal(
    ModificationType::OPTIMIZATION,
    "Optimize collision detection",
    diffs
);

// Analyze risk
modifications.AnalyzeRisk(proposal);
std::cout << "Risk Score: " << proposal.risk_score << std::endl;

// Submit for approval
modifications.SubmitForApproval(proposal);

// Approve and apply
modifications.ApproveProposal(proposal.id);
auto result = modifications.ApplyModification(proposal);
```

---

## Evolution Stages

The AI evolves through 4 stages:

### Stage 1: Foundation (Current)
- Learns basics: text, code, logic
- Builds knowledge graphs
- Understands architecture

### Stage 2: Engineering
- Learns debugging, optimization, refactoring
- Understands physics and simulations
- Can suggest improvements

### Stage 3: Agentic
- Agents become autonomous
- Can plan and execute tasks
- Validates changes

### Stage 4: Self-Improving
- Analyzes own performance
- Improves workflows
- Evolves strategies

---

## Key Features

✅ **Offline-First** — Works completely offline
✅ **Modular** — Each subsystem is independent
✅ **Safe** — All changes require approval
✅ **Learning** — Continuously improves
✅ **Transparent** — All decisions are explainable
✅ **Extensible** — Easy to add new capabilities

---

## Common Tasks

### Get AI Status

```cpp
auto& ai = LegendaryAIAssistant::GetInstance();
AIState state = ai.GetCurrentState();

std::cout << "Stage: " << static_cast<int>(state.current_stage) << std::endl;
std::cout << "Confidence: " << state.overall_confidence << std::endl;
std::cout << "Tasks: " << state.total_tasks_completed << std::endl;
std::cout << "Failures: " << state.total_failures << std::endl;
```

### Get Skill Graph

```cpp
json skills = ai.GetSkillGraph();
std::cout << skills.dump(2) << std::endl;
```

### Get Memory Statistics

```cpp
json stats = ai.GetMemoryStatistics();
std::cout << "Total Memories: " << stats["total_memories"] << std::endl;
std::cout << "Total Skills: " << stats["total_skills"] << std::endl;
std::cout << "Average Skill Level: " << stats["average_skill_level"] << std::endl;
```

### Get Comprehensive Report

```cpp
json report = ai.GetComprehensiveReport();
std::cout << report.dump(2) << std::endl;
```

### Save/Load State

```cpp
// Save
ai.SaveState("legendary_ai_state.json");

// Load
ai.LoadState("legendary_ai_state.json");
```

---

## Next Steps

1. **Integrate with SZM Forge** — Add to main application
2. **Build UI Panels** — Visualize AI state and capabilities
3. **Train on Codebase** — Let AI learn from SZM Forge
4. **Create Workflows** — Define engineering tasks
5. **Monitor Progress** — Track AI evolution

---

## Architecture Overview

```
┌─────────────────────────────────────────┐
│   Legendary AI Assistant (Main)         │
├─────────────────────────────────────────┤
│                                         │
│  Memory Engine      ← Stores knowledge  │
│  Skill System       ← Tracks abilities  │
│  Agent Framework    ← Executes tasks    │
│  Training Pipeline  ← Learns from data  │
│  Modification Sys   ← Applies changes   │
│                                         │
└─────────────────────────────────────────┘
```

---

## File Structure

```
src/AI/LegendaryCore/
├── MemoryEngine.hpp/cpp
├── SkillSystem.hpp/cpp
├── AgentFramework.hpp/cpp
├── TrainingPipeline.hpp/cpp
├── ModificationSystem.hpp/cpp
├── LegendaryAIAssistant.hpp/cpp
└── CMakeLists.txt
```

---

## Dependencies

- C++20
- nlohmann/json
- Standard library

---

## Performance

- **Memory:** ~50MB for foundation stage
- **CPU:** Minimal overhead
- **Disk:** JSON-based persistence
- **Scalability:** 10,000+ memory entries

---

## Support

For questions or issues:
1. Check the architecture documentation
2. Review the code comments
3. Examine the example usage
4. Check the test files

---

**Legendary AI Assistant — The future of engineering intelligence starts here.**
