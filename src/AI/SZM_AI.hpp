/**
 * SZM AI System - Main Header
 * 
 * This is the unified entry point for the AI subsystem. It integrates:
 * - Vision System: Screen capture and visual analysis
 * - Observation System: User action capture and pattern learning
 * - Learning Integration: Wires all systems together
 * - Assistant Interface: User-facing AI panel
 * - Legendary AI Core: Advanced reasoning and memory
 */

#pragma once

// =============================================================================
// CORE AI COMPONENTS
// =============================================================================

#include "CreatorMode.hpp"
#include "LegendaryCore/LegendaryAIAssistant.hpp"
#include "LegendaryCore/MemoryEngine.hpp"
#include "LegendaryCore/SkillSystem.hpp"
#include "LegendaryCore/AgentFramework.hpp"
#include "LegendaryCore/TrainingPipeline.hpp"
#include "LegendaryCore/ModificationSystem.hpp"
#include "AIOperatingSystem.hpp"
#include "TrainingStudio.hpp"
#include "AgentSystem.hpp"
#include "AIEngine.hpp"

// =============================================================================
// VISION & OBSERVATION SYSTEM
// =============================================================================

#include "VisionSystem/VisionSystem.hpp"
#include "ObservationSystem/ObservationSystem.hpp"
#include "LearningIntegration/LearningIntegration.hpp"
#include "AssistantInterface/AssistantInterface.hpp"

// =============================================================================
// MACHINE LEARNING SYSTEM
// =============================================================================

#include "MachineLearningSystem/MachineLearningSystem.hpp"

// =============================================================================
// NAMESPACE DEFINITIONS
// =============================================================================

namespace SZM::AI {
    // Core AI types
    using namespace Legendary;
    
    // Vision types
    namespace Vision = ::SZM::AI::Vision;
    
    // Observation types
    namespace Observation = ::SZM::AI::Observation;
    
    // Learning integration types
    namespace Learning = ::SZM::AI::Learning;
    
    // Machine learning types
    namespace Machines = ::SZM::AI::Machines;
    
    // UI types
    namespace UI = ::SZM::AI::UI;
}

// =============================================================================
// QUICK START GUIDE
// =============================================================================

/**
 * USAGE EXAMPLE:
 * 
 * 1. Initialize the AI System:
 * 
 *    // In your main.cpp or application initialization:
 *    SZM::AI::UI::AssistantInterface::GetInstance().Initialize();
 * 
 * 2. Add the UI panel to your render loop:
 * 
 *    // In your render loop:
 *    SZM::AI::UI::AssistantInterface::GetInstance().Update();
 *    SZM::AI::UI::AssistantInterface::GetInstance().Render();
 * 
 * 3. Use the Learning System directly:
 * 
 *    auto& learning = SZM::AI::Learning::LearningIntegration::GetInstance();
 *    learning.SetLearningMode(SZM::AI::Learning::LearningMode::TEACHING);
 * 
 *    // The AI will now learn from user demonstrations
 * 
 * 4. Access Vision System for screen capture:
 * 
 *    auto& vision = SZM::AI::Vision::VisionSystem::GetInstance();
 *    auto capture = vision.CaptureScreen();
 *    auto elements = vision.DetectElements(capture.image);
 * 
 * 5. Record user actions:
 * 
 *    auto& observation = SZM::AI::Observation::ObservationSystem::GetInstance();
 *    SZM::AI::Observation::UserAction action;
 *    action.type = SZM::AI::Observation::ActionType::MOUSE_CLICK;
 *    action.position = {x, y, 1920, 1080};
 *    observation.RecordAction(action);
 * 
 * 6. Get AI suggestions:
 * 
 *    auto& learning = SZM::AI::Learning::LearningIntegration::GetInstance();
 *    auto suggestions = learning.GetSuggestions();
 *    if (!suggestions.empty()) {
 *        auto best = learning.GetBestSuggestion();
 *        // Present to user or auto-apply
 *    }
 */

// =============================================================================
// SYSTEM ARCHITECTURE
// =============================================================================

/**
 * SYSTEM OVERVIEW:
 * 
 *                    ┌─────────────────────────────────────────┐
 *                    │         Assistant Interface            │
 *                    │   (User-facing AI panel & controls)    │
 *                    └─────────────────┬───────────────────────┘
 *                                      │
 *                    ┌─────────────────▼───────────────────────┐
 *                    │       Learning Integration               │
 *                    │  (Wires all systems, generates context) │
 *                    └─────────────────┬───────────────────────┘
 *                                      │
 *          ┌───────────────────────────┼───────────────────────────┐
 *          │                           │                           │
 * ┌────────▼────────┐        ┌─────────▼─────────┐       ┌────────▼────────┐
 * │  Vision System  │        │ Observation System │       │  Legendary AI   │
 * │ (Screen Capture)│        │  (Action Capture)  │       │  (Reasoning)    │
 * └─────────────────┘        └───────────────────┘       └─────────────────┘
 *          │                           │                           │
 *          └───────────────────────────┼───────────────────────────┘
 *                                      │
 *                    ┌─────────────────▼───────────────────────┐
 *                    │         Memory Engine                   │
 *                    │   (Long-term memory & skills)           │
 *                    └─────────────────────────────────────────┘
 * 
 * LEARNING MODES:
 * 
 * - PASSIVE:    Watch and learn without interfering
 * - ACTIVE:     Suggest actions based on learned patterns  
 * - SUPERVISED: Learn from explicit user guidance
 * - AUTONOMOUS: Take actions and learn from outcomes
 * - TEACHING:   User demonstrates, AI learns
 */

// =============================================================================
// FEATURE SUMMARY
// =============================================================================

/**
 * VisionSystem Features:
 * - Cross-platform screen capture (Windows, Linux, macOS)
 * - Real-time UI element detection (buttons, text fields, menus, panels)
 * - Screen analysis and description generation
 * - Frame comparison and change detection
 * - Video recording of screen activity
 * - OCR text extraction (placeholder for Tesseract integration)
 * 
 * ObservationSystem Features:
 * - Comprehensive action recording (mouse, keyboard, UI interactions)
 * - Context capture (window title, focused element, application state)
 * - Automatic pattern detection (sequences, shortcuts, workflows)
 * - Pattern matching and similarity calculation
 * - Session management for organized learning
 * - Export/import of learned patterns
 * 
 * LearningIntegration Features:
 * - Unified interface for all AI subsystems
 * - Multiple learning modes for different use cases
 * - Automatic suggestion generation based on patterns
 * - Memory integration for persistent learning
 * - Skill development tracking
 * - Continuous learning with confidence-based filtering
 * 
 * AssistantInterface Features:
 * - Real-time status display (listening, thinking, idle)
 * - Learning mode selector with descriptions
 * - Conversation history
 * - Suggestion cards with accept/dismiss
 * - Statistics dashboard
 * - Configurable settings
 * - Collapsible/minimized mode
 */

// =============================================================================
// COMPATIBILITY NOTES
// =============================================================================

/**
 * DEPENDENCIES:
 * - nlohmann_json (JSON handling)
 * - OpenCV (Vision system)
 * - ImGui (UI components)
 * - C++20 compatible compiler
 * 
 * PLATFORM SUPPORT:
 * - Windows: Full support with GDI+ screen capture
 * - Linux: X11 screen capture (Wayland support planned)
 * - macOS: CoreGraphics screen capture
 * 
 * BUILD REQUIREMENTS:
 * - CMake 3.16+
 * - C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
 * - OpenCV 4.0+
 */

// =============================================================================
// VERSION INFO
// =============================================================================

#define SZM_AI_VERSION_MAJOR 1
#define SZM_AI_VERSION_MINOR 0
#define SZM_AI_VERSION_PATCH 0

#define SZM_AI_VERSION_STRING "1.0.0"

#endif // SZM_AI_HPP
