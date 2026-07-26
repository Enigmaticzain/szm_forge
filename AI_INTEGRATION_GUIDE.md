# SZM Forge AI Integration Guide

## Overview

This guide explains how to integrate your custom-trained AI model into the SZM Forge application. The AI system enables:

- **Screen Understanding**: See what's on screen and analyze UI elements
- **Action Learning**: Learn from user demonstrations and actions
- **Pattern Recognition**: Detect and remember common action sequences
- **Smart Suggestions**: Suggest next actions based on learned patterns

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Your Custom AI Model                      │
│                  (PyTorch/TensorFlow/etc.)                   │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────────┐
│                custom_ai_training.py                         │
│  - Data collection (screen + actions)                        │
│  - Training pipeline                                         │
│  - Model management                                          │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────────┐
│               C++ AI System (src/AI/)                        │
│  ┌─────────────┐  ┌──────────────┐  ┌────────────────┐     │
│  │VisionSystem│  │Observation   │  │Learning        │     │
│  │             │  │System        │  │Integration     │     │
│  │• Capture    │  │• Record      │  │• Wires all     │     │
│  │• Detect     │  │  actions     │  │  systems       │     │
│  │  elements   │  │• Detect      │  │• Generates     │     │
│  │• Analyze    │  │  patterns    │  │  suggestions   │     │
│  └─────────────┘  └──────────────┘  └────────────────┘     │
│                          │                                   │
│  ┌───────────────────────────────────────────────────────┐ │
│  │              MemoryEngine + LegendaryAI                │ │
│  │  • Long-term memory                                    │ │
│  │  • Skills & progression                                 │ │
│  │  • Reasoning engine                                     │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────────┐
│                AssistantInterface (ImGui)                   │
│  • Real-time status display                                 │
│  • Learning mode selector                                    │
│  • Conversation history                                      │
│  • Suggestion cards                                          │
└─────────────────────────────────────────────────────────────┘
```

## Quick Start

### 1. Setup Your AI Model

Create your custom AI model that follows this interface:

```python
# my_model.py
from ai_service.custom_ai_training import CustomAIModel

class MyCustomAI(CustomAIModel):
    def __init__(self, model_path):
        super().__init__(model_path)
        
    def load_model(self):
        # Load your trained weights
        self.model = load_my_model(self.model_path)
        self.is_loaded = True
        return True
        
    def preprocess_input(self, image_data, actions):
        # Convert to model input format
        return prepare_for_my_model(image_data, actions)
        
    def predict(self, screen_capture, context):
        # Run inference
        prediction = self.model(self.preprocess_input(...))
        return {
            "suggested_action": parse_prediction(prediction),
            "confidence": calculate_confidence(prediction),
            "reasoning": explain_prediction(prediction),
            "detected_intents": detect_intents(prediction)
        }
        
    def train(self, training_batch, epochs, learning_rate):
        # Fine-tune on new data
        return train_my_model(self.model, training_batch, epochs, learning_rate)
```

### 2. Collect Training Data

```bash
# Capture 5 minutes of your workflow
python -m ai_service.custom_ai_training --mode capture --duration 300 --data_dir ./my_training_data
```

This will capture your screen and record all your actions, saving them to `./my_training_data/training_data.json`.

### 3. Train Your Model

```bash
# Train on the collected data
python -m ai_service.custom_ai_training --mode train --model ./my_model.pt --data_dir ./my_training_data --epochs 20
```

### 4. Integrate with C++ Application

```cpp
// In your main.cpp
#include <AI/SZM_AI.hpp>

int main() {
    // Initialize the AI system
    auto& assistant = SZM::AI::UI::AssistantInterface::GetInstance();
    assistant.Initialize();
    
    // Set learning mode
    auto& learning = SZM::AI::Learning::LearningIntegration::GetInstance();
    learning.SetLearningMode(SZM::AI::Learning::LearningMode::TEACHING);
    
    // Main loop
    while (running) {
        // Update AI systems
        assistant.Update();
        
        // Render AI interface
        assistant.Render();
        
        // Your application logic...
    }
    
    assistant.Shutdown();
    return 0;
}
```

## Learning Modes

The AI system supports different learning modes:

| Mode | Description | Use Case |
|------|-------------|----------|
| **PASSIVE** | Watch and learn without interfering | Background learning |
| **ACTIVE** | Suggest actions based on patterns | User-guided automation |
| **SUPERVISED** | Learn from explicit corrections | Training new behaviors |
| **AUTONOMOUS** | Take actions and learn from outcomes | Automated workflows (caution!) |
| **TEACHING** | User demonstrates, AI learns | Initial training |

### Teaching Mode Example

```cpp
// Start teaching mode
auto& learning = SZM::AI::Learning::LearningIntegration::GetInstance();
learning.SetLearningMode(SZM::AI::Learning::LearningMode::TEACHING);

// User demonstrates a task...
// AI watches and learns patterns

// End teaching session
learning.SetLearningMode(SZM::AI::Learning::LearningMode::PASSIVE);

// AI now knows the demonstrated task
auto patterns = learning.GetLearnedSkills();
```

## Capturing User Actions

To make the AI learn from user actions, you need to record them:

```cpp
// When user performs an action
auto& observation = SZM::AI::Observation::ObservationSystem::GetInstance();

SZM::AI::Observation::UserAction action;
action.type = SZM::AI::Observation::ActionType::MOUSE_CLICK;
action.position = {x, y, screen_width, screen_height};
action.context = observation.CaptureCurrentContext();

observation.RecordAction(action);

// Later, record the outcome
SZM::AI::Observation::ActionResult result;
result.outcome = SZM::AI::Observation::ActionOutcome::SUCCESS;
observation.RecordActionOutcome(action.id, result);

// The AI learns from this success
learning.LearnFromSuccess(action.id);
```

## Getting Suggestions

```cpp
auto& learning = SZM::AI::Learning::LearningIntegration::GetInstance();

// Get current suggestions
auto suggestions = learning.GetSuggestions();

for (const auto& suggestion : suggestions) {
    if (suggestion.confidence > 0.8f) {
        // High confidence - consider auto-applying
        std::cout << "AI suggests: " << suggestion.message << std::endl;
        
        // Apply the suggestion
        learning.ApplySuggestion(suggestion.id);
    }
}

// Or get just the best suggestion
auto best = learning.GetBestSuggestion();
if (best.confidence > 0.9f) {
    // Very confident - might auto-apply
}
```

## Screen Analysis

```cpp
auto& vision = SZM::AI::Vision::VisionSystem::GetInstance();

// Capture current screen
auto capture = vision.CaptureScreen();

// Analyze what's on screen
auto elements = vision.DetectElements(capture.image);
std::cout << "Found " << elements.size() << " UI elements" << std::endl;

// Get description
std::string description = vision.DescribeScreen(capture.image);
std::cout << "Screen: " << description << std::endl;

// Find specific element
auto* button = vision.FindElementByLabel("Submit", capture.image);
if (button) {
    std::cout << "Found Submit button at (" 
              << button->bounding_box.x << ", " 
              << button->bounding_box.y << ")" << std::endl;
}
```

## Pattern Detection

The system automatically detects patterns from recorded actions:

```cpp
auto& observation = SZM::AI::Observation::ObservationSystem::GetInstance();

// Detect patterns
auto sequences = observation.DetectActionSequences();
auto shortcuts = observation.DetectShortcuts();
auto workflows = observation.DetectWorkflows();

// Get all learned patterns
auto patterns = observation.GetAllPatterns();

for (const auto& pattern : patterns) {
    std::cout << "Pattern: " << pattern.description << std::endl;
    std::cout << "  Type: " << pattern.pattern_type << std::endl;
    std::cout << "  Confidence: " << (pattern.confidence * 100) << "%" << std::endl;
    std::cout << "  Success rate: " << (pattern.success_rate * 100) << "%" << std::endl;
}
```

## Memory and Skills

The AI maintains long-term memory and develops skills:

```cpp
auto& memory = SZM::AI::Legendary::MemoryEngine::GetInstance();

// Store information
memory.StoreSessionMemory("current_goal", {{"task", "Create button"}});

// Retrieve information
auto goal = memory.RetrieveSessionMemory("current_goal");

// View skills
auto skills = memory.GetAllSkills();
for (const auto& skill : skills) {
    std::cout << skill.name << " - Level " << skill.level 
              << " (XP: " << skill.xp << ")" << std::endl;
}

// Skill XP increases with successful actions
memory.UpdateSkillXP("pattern_recognition", 5.0f);
memory.LevelUpSkill("pattern_recognition");
```

## Persistence

Save and load the AI state:

```cpp
// Auto-save
learning.AutoSave();

// Manual save
learning.SaveState("./ai_state");

// Manual load
learning.LoadState("./ai_state");

// Export patterns for sharing
observation.ExportPatterns("./my_patterns.json");
observation.ImportPatterns("./my_patterns.json");
```

## UI Integration

Add the AI panel to your ImGui interface:

```cpp
// Create the panel
SZM::AI::UI::AssistantPanel panel;
panel.Initialize();

// In your render loop
panel.Render();

// Configure position and size
panel.SetPosition({20, 20});
panel.SetSize({400, 600});
panel.SetAlwaysVisible(true);

// Handle suggestion callbacks
panel.SetOnSuggestionCallback([](const std::string& suggestion_id) {
    std::cout << "User accepted suggestion: " << suggestion_id << std::endl;
});
```

## API Reference

### VisionSystem

| Method | Description |
|--------|-------------|
| `CaptureScreen()` | Capture current screen |
| `DetectElements()` | Find UI elements |
| `FindElementByLabel()` | Find element by text |
| `DescribeScreen()` | Generate natural language description |
| `AnalyzeScreenContent()` | Get detailed analysis JSON |

### ObservationSystem

| Method | Description |
|--------|-------------|
| `RecordAction()` | Record a user action |
| `GetAllPatterns()` | Get learned patterns |
| `DetectActionSequences()` | Find action sequences |
| `GetSuggestions()` | Get action suggestions |
| `SaveObservations()` | Save to file |

### LearningIntegration

| Method | Description |
|--------|-------------|
| `SetLearningMode()` | Change learning mode |
| `Update()` | Main update loop |
| `GetSuggestions()` | Get AI suggestions |
| `ApplySuggestion()` | Apply a suggestion |
| `AnalyzeCurrentScreen()` | Analyze screen |

### MemoryEngine

| Method | Description |
|--------|-------------|
| `StoreSessionMemory()` | Store temporary data |
| `StoreLongTermMemory()` | Store persistent data |
| `GetSkill()` | Get skill info |
| `UpdateSkillXP()` | Award XP to skill |
| `RecordFailure()` | Record a failure |

## Troubleshooting

### Vision not working
- Ensure OpenCV is installed: `pip install opencv-python`
- Check platform-specific screen capture dependencies

### Patterns not detected
- Need at least 2-3 demonstrations of the same action sequence
- Ensure actions have proper context (window title, element info)
- Check that action outcomes are being recorded

### Low confidence suggestions
- More training data improves confidence
- Consistent demonstrations are better than varied ones
- Adjust confidence threshold: `learning.GetConfig().confidence_threshold`

### Memory growing too large
- Patterns automatically decay if unused
- Call `PruneOldPatterns()` periodically
- Export and clear old data with `ExportPatterns()`

## Next Steps

1. **Train your model** on your specific use case
2. **Test in TEACHING mode** to verify pattern detection
3. **Switch to ACTIVE mode** once patterns are learned
4. **Monitor performance** and adjust thresholds
5. **Iterate** by collecting more data and retraining

## Support

For issues or questions:
- Check the documentation in `AI_OPERATING_SYSTEM.md`
- Review example code in `ai_service/demo.py`
- Examine the full API in `src/AI/SZM_AI.hpp`
