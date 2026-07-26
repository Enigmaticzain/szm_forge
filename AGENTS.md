<!-- code-review-graph MCP tools -->
## MCP Tools: code-review-graph

**IMPORTANT: This project has a knowledge graph. ALWAYS use the
code-review-graph MCP tools BEFORE using Grep/Glob/Read to explore
the codebase.** The graph is faster, cheaper (fewer tokens), and gives
you structural context (callers, dependents, test coverage) that file
scanning cannot.

### When to use graph tools FIRST

- **Exploring code**: `semantic_search_nodes` or `query_graph` instead of Grep
- **Understanding impact**: `get_impact_radius` instead of manually tracing imports
- **Code review**: `detect_changes` + `get_review_context` instead of reading entire files
- **Finding relationships**: `query_graph` with callers_of/callees_of/imports_of/tests_for
- **Architecture questions**: `get_architecture_overview` + `list_communities`

Fall back to Grep/Glob/Read **only** when the graph doesn't cover what you need.

### Key Tools

| Tool | Use when |
| ------ | ---------- |
| `detect_changes` | Reviewing code changes — gives risk-scored analysis |
| `get_review_context` | Need source snippets for review — token-efficient |
| `get_impact_radius` | Understanding blast radius of a change |
| `get_affected_flows` | Finding which execution paths are impacted |
| `query_graph` | Tracing callers, callees, imports, tests, dependencies |
| `semantic_search_nodes` | Finding functions/classes by name or keyword |
| `get_architecture_overview` | Understanding high-level codebase structure |
| `refactor_tool` | Planning renames, finding dead code |

### Workflow

1. The graph auto-updates on file changes (via hooks).
2. Use `detect_changes` for code review.
3. Use `get_affected_flows` to understand impact.
4. Use `query_graph` pattern="tests_for" to check coverage.

---

<!-- AI System -->
## AI System (Vision & Learning)

This project includes a comprehensive AI system for screen understanding, action learning, and **machine learning from data**.

### Quick Reference

| Component | Location | Purpose |
| --------- | -------- | ------- |
| Vision System | `src/AI/VisionSystem/` | Screen capture & element detection |
| Observation System | `src/AI/ObservationSystem/` | User action capture & pattern learning |
| Learning Integration | `src/AI/LearningIntegration/` | Wires all systems together |
| **Machine Learning** | `src/AI/MachineLearningSystem/` | **Learn from machines, generate new capabilities** |
| Assistant Interface | `src/AI/AssistantInterface/` | User-facing AI panel |
| Python Training | `ai_service/custom_ai_training.py` | Train your custom AI model |
| Machine ML Training | `ai_service/machine_learning_training.py` | **Train machine learning from data** |

### Key Headers

- `src/AI/SZM_AI.hpp` - Main unified header (include this)
- `src/AI/VisionSystem/VisionSystem.hpp` - Screen capture API
- `src/AI/ObservationSystem/ObservationSystem.hpp` - Action recording
- `src/AI/LearningIntegration/LearningIntegration.hpp` - Main AI interface
- **`src/AI/MachineLearningSystem/MachineLearningSystem.hpp`** - **Machine learning & code generation**

### Learning Modes

```
PASSIVE    - Watch and learn without interfering
ACTIVE     - Suggest actions based on learned patterns
SUPERVISED - Learn from explicit user guidance
AUTONOMOUS - Take actions and learn from outcomes
TEACHING   - User demonstrates, AI learns
```

### Machine Learning System (NEW!)

Your AI **CAN** learn from machine data and generate new app capabilities!

```cpp
#include <AI/SZM_AI.hpp>

// Initialize machine learning
auto& ml = SZM::AI::Machines::MachineLearningSystem::GetInstance();
ml.Initialize();

// Ingest machine data
ml.IngestMachineDataset({"./machine_data/cnc_mill.json"});

// Learn patterns
ml.LearnFromMachines();

// Generate new machine
auto generated = ml.GenerateMachine(
    SZM::AI::Machines::MachineType::CNC_MILL,
    {{"power", 7.5f}, {"max_speed", 6000.0f}},
    {"kinematics"}
);

// Generate C++ code
auto code = ml.GenerateMachineCode(generated);

// Integrate into app
ml.IntegrateMachineIntoApp(generated);
```

### Python Machine Training

```bash
# Ingest machine data and learn patterns
python ai_service/machine_learning_training.py --data_dir ./machine_data --learn

# Generate new machines
python ai_service/machine_learning_training.py --generate --type cnc_mill --count 5

# With custom parameters
python ai_service/machine_learning_training.py --generate --type robot_arm \
    --params '{"payload": 15.0, "reach": 1800}'
```

### Basic Usage (Screen Learning)

```cpp
#include <AI/SZM_AI.hpp>

// Initialize
auto& assistant = SZM::AI::UI::AssistantInterface::GetInstance();
assistant.Initialize();

// Set mode
auto& learning = SZM::AI::Learning::LearningIntegration::GetInstance();
learning.SetLearningMode(SZM::AI::Learning::LearningMode::TEACHING);

// Main loop
while (running) {
    assistant.Update();
    assistant.Render();
}

// Shutdown
assistant.Shutdown();
```

### Documentation

- `AI_INTEGRATION_GUIDE.md` - Complete integration guide
- `AI_OPERATING_SYSTEM.md` - System architecture documentation
- **`MACHINE_LEARNING_SYSTEM.md`** - **Machine learning from data guide**
- `machine_data/` - Sample machine data for training
