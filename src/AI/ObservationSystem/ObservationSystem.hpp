#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include "../VisionSystem/VisionSystem.hpp"

namespace SZM::AI::Observation {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ACTION TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class ActionType {
    // Mouse actions
    MOUSE_CLICK,
    MOUSE_DOUBLE_CLICK,
    MOUSE_RIGHT_CLICK,
    MOUSE_DRAG,
    MOUSE_MOVE,
    MOUSE_HOVER,
    MOUSE_SCROLL,
    
    // Keyboard actions
    KEYBOARD_TYPING,
    KEYBOARD_SHORTCUT,
    KEYBOARD_PRESS,
    KEYBOARD_RELEASE,
    
    // UI interactions
    BUTTON_CLICK,
    TEXT_INPUT,
    CHECKBOX_TOGGLE,
    DROPDOWN_SELECT,
    MENU_SELECT,
    TAB_SWITCH,
    SLIDER_ADJUST,
    TOGGLE_SWITCH,
    
    // Application actions
    WINDOW_OPEN,
    WINDOW_CLOSE,
    WINDOW_MINIMIZE,
    WINDOW_MAXIMIZE,
    FILE_OPEN,
    FILE_SAVE,
    SCREEN_CAPTURE,
    
    // Custom actions
    CUSTOM,
    
    // Unknown
    UNKNOWN
};

enum class ActionOutcome {
    SUCCESS,
    PARTIAL_SUCCESS,
    FAILURE,
    ERROR,
    CANCELLED,
    UNKNOWN
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ACTION DATA STRUCTURES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct MousePosition {
    int x;
    int y;
    int screen_width;
    int screen_height;
    
    float normalized_x() const { return (float)x / screen_width; }
    float normalized_y() const { return (float)y / screen_height; }
};

struct KeyCombination {
    std::vector<std::string> keys;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool meta = false;
    
    std::string to_string() const;
};

struct ActionContext {
    std::string application_name;
    std::string window_title;
    std::string focused_element_id;
    std::string focused_element_type;
    std::string focused_element_text;
    cv::Rect element_bounds;
    json ui_state;
    json application_state;
};

struct ActionResult {
    ActionOutcome outcome = ActionOutcome::UNKNOWN;
    std::string message;
    std::string error_type;
    json result_data;
};

struct UserAction {
    std::string id;
    ActionType type;
    std::string description;
    uint64_t timestamp;
    uint64_t duration_ms;
    
    // Mouse data
    MousePosition position;
    MousePosition start_position;  // For drag operations
    int click_count = 1;
    
    // Keyboard data
    std::string text_typed;
    KeyCombination key_combination;
    
    // Context
    ActionContext context;
    cv::Mat screenshot_before;
    cv::Mat screenshot_after;
    cv::Mat screenshot_during;  // For drag operations
    
    // Result
    ActionResult result;
    
    // Learning metadata
    float confidence = 0.0f;
    bool is_demonstrated = false;  // True if this was a user demonstration
    bool is_learned = false;       // True if AI learned from this
    std::vector<std::string> related_concepts;
};

struct ObservationSession {
    std::string id;
    uint64_t start_time;
    uint64_t end_time;
    std::string goal;
    std::vector<UserAction> actions;
    std::vector<std::string> key_learning_points;
    float completion_rate = 0.0f;
};

struct LearningPattern {
    std::string id;
    std::string pattern_type;  // "sequence", "workflow", "shortcut", "technique"
    std::string description;
    
    // Pattern data
    std::vector<UserAction> action_sequence;
    std::string trigger_condition;
    std::vector<std::string> required_contexts;
    
    // Usage statistics
    int times_demonstrated = 0;
    int times_successful = 0;
    int times_failed = 0;
    float success_rate = 0.0f;
    
    // Learning metadata
    float confidence = 0.0f;
    uint64_t last_used;
    uint64_t first_demonstrated;
    std::vector<std::string> generalizations;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// OBSERVATION SYSTEM
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class ObservationSystem {
public:
    static ObservationSystem& GetInstance();

    // Initialization
    void Initialize();
    void Shutdown();
    bool IsObserving() const { return m_IsObserving; }

    // Observation Control
    void StartObserving();
    void StopObserving();
    void PauseObserving();
    void ResumeObserving();
    
    // Action Recording
    void RecordAction(const UserAction& action);
    void RecordMouseClick(int x, int y, int button, int click_count, 
                          const ActionContext& context);
    void RecordMouseDrag(int start_x, int start_y, int end_x, int end_y,
                         const ActionContext& context);
    void RecordKeyboardInput(const std::string& text, const ActionContext& context);
    void RecordKeyboardShortcut(const KeyCombination& keys, const ActionContext& context);
    void RecordActionOutcome(const std::string& action_id, const ActionResult& result);

    // Observation Sessions
    ObservationSession StartSession(const std::string& goal);
    void EndSession(const std::string& session_id);
    ObservationSession GetSession(const std::string& session_id);
    std::vector<ObservationSession> GetRecentSessions(int count = 10);

    // Pattern Learning
    void AnalyzePatterns();
    std::vector<LearningPattern> DetectActionSequences();
    std::vector<LearningPattern> DetectShortcuts();
    std::vector<LearningPattern> DetectWorkflows();
    LearningPattern MergePatterns(const std::vector<LearningPattern>& patterns);
    
    // Pattern Management
    void AddLearnedPattern(const LearningPattern& pattern);
    void UpdatePattern(const LearningPattern& pattern);
    void RemovePattern(const std::string& pattern_id);
    LearningPattern GetPattern(const std::string& pattern_id);
    std::vector<LearningPattern> GetAllPatterns();
    std::vector<LearningPattern> GetPatternsByType(const std::string& type);
    
    // Pattern Matching
    std::vector<LearningPattern> FindMatchingPatterns(const std::vector<UserAction>& recent_actions);
    LearningPattern* FindBestMatchingPattern(const std::vector<UserAction>& recent_actions);
    float CalculatePatternMatch(const LearningPattern& pattern, 
                                 const std::vector<UserAction>& actions);

    // Context Capture
    ActionContext CaptureCurrentContext();
    void UpdateContextSnapshot();
    
    // Learning from Observation
    void LearnFromSuccess(const std::string& action_id);
    void LearnFromFailure(const std::string& action_id, const std::string& error);
    void ProcessDemonstration(const std::vector<UserAction>& demonstration);
    
    // Query and Search
    std::vector<UserAction> QueryActions(ActionType type, 
                                          uint64_t start_time = 0, 
                                          uint64_t end_time = 0);
    std::vector<UserAction> GetActionsInRegion(const cv::Rect& region);
    std::map<ActionType, int> GetActionStatistics();

    // Persistence
    void SaveObservations(const std::string& filepath);
    void LoadObservations(const std::string& filepath);
    void ExportPatterns(const std::string& filepath);
    void ImportPatterns(const std::string& filepath);

    // Statistics
    json GetStatistics();
    int GetTotalActionsObserved();
    int GetTotalPatternsLearned();
    float GetAverageSuccessRate();

private:
    ObservationSystem() = default;

    ObservationSystem(const ObservationSystem&) = delete;
    ObservationSystem& operator=(const ObservationSystem&) = delete;

    bool m_IsInitialized = false;
    bool m_IsObserving = false;
    bool m_IsPaused = false;
    
    std::string m_CurrentSessionId;
    uint64_t m_SessionStartTime;
    std::string m_CurrentGoal;
    
    std::vector<UserAction> m_RecordedActions;
    std::vector<ObservationSession> m_Sessions;
    std::vector<LearningPattern> m_LearnedPatterns;
    
    ActionContext m_LastContext;
    cv::Mat m_LastScreenshot;
    
    // Helper methods
    std::string GenerateActionId();
    std::string GenerateSessionId();
    ActionType InferActionType(const UserAction& action);
    float CalculateActionSimilarity(const UserAction& a1, const UserAction& a2);
    void UpdatePatternStatistics(LearningPattern& pattern);
    
    // Pattern detection algorithms
    std::vector<LearningPattern> DetectNGramPatterns(int n);
    std::vector<LearningPattern> DetectRepetitivePatterns();
    std::vector<LearningPattern> DetectConditionalPatterns();
    
    // Constants
    static constexpr int MAX_CACHED_SCREENSHOTS = 50;
    static constexpr float PATTERN_MATCH_THRESHOLD = 0.75f;
    static constexpr int MIN_PATTERN_LENGTH = 2;
    static constexpr int MAX_PATTERN_LENGTH = 10;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// INLINE HELPER IMPLEMENTATIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

inline std::string KeyCombination::to_string() const {
    std::ostringstream oss;
    if (ctrl) oss << "Ctrl+";
    if (shift) oss << "Shift+";
    if (alt) oss << "Alt+";
    if (meta) oss << "Meta+";
    for (size_t i = 0; i < keys.size(); i++) {
        if (i > 0) oss << "+";
        oss << keys[i];
    }
    return oss.str();
}

} // namespace SZM::AI::Observation
