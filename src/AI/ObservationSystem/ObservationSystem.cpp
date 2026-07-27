#include "ObservationSystem.hpp"
#include <iostream>
#include <algorithm>
#include <set>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>

namespace SZM::AI::Observation {

ObservationSystem& ObservationSystem::GetInstance() {
    static ObservationSystem instance;
    return instance;
}

void ObservationSystem::Initialize() {
    if (m_IsInitialized) return;
    
    m_IsInitialized = true;
    m_IsObserving = false;
    m_IsPaused = false;
    
    std::cout << "[ObservationSystem] Initialized successfully" << std::endl;
}

void ObservationSystem::Shutdown() {
    if (!m_IsInitialized) return;
    
    StopObserving();
    m_IsInitialized = false;
    
    std::cout << "[ObservationSystem] Shutdown complete" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// OBSERVATION CONTROL
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void ObservationSystem::StartObserving() {
    if (!m_IsInitialized || m_IsObserving) return;
    
    m_IsObserving = true;
    m_IsPaused = false;
    
    std::cout << "[ObservationSystem] Started observing" << std::endl;
}

void ObservationSystem::StopObserving() {
    if (!m_IsObserving) return;
    
    if (!m_CurrentSessionId.empty()) {
        EndSession(m_CurrentSessionId);
    }
    
    m_IsObserving = false;
    m_IsPaused = false;
    
    std::cout << "[ObservationSystem] Stopped observing" << std::endl;
}

void ObservationSystem::PauseObserving() {
    if (!m_IsObserving) return;
    m_IsPaused = true;
    std::cout << "[ObservationSystem] Paused observing" << std::endl;
}

void ObservationSystem::ResumeObserving() {
    if (!m_IsObserving) return;
    m_IsPaused = false;
    std::cout << "[ObservationSystem] Resumed observing" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ACTION RECORDING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void ObservationSystem::RecordAction(const UserAction& action) {
    if (!m_IsObserving || m_IsPaused) return;
    
    UserAction mutable_action = action;
    mutable_action.id = GenerateActionId();
    mutable_action.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    
    // Capture screenshots if VisionSystem is available
    try {
        auto& vision = Vision::VisionSystem::GetInstance();
        if (vision.IsInitialized()) {
            mutable_action.screenshot_before = vision.CaptureScreen().image;
        }
    } catch (...) {
        // VisionSystem might not be initialized
    }
    
    m_RecordedActions.push_back(mutable_action);
    
    // Update current session
    if (!m_CurrentSessionId.empty()) {
        for (auto& session : m_Sessions) {
            if (session.id == m_CurrentSessionId) {
                session.actions.push_back(mutable_action);
                break;
            }
        }
    }
    
    std::cout << "[ObservationSystem] Recorded action: " << action.description << std::endl;
}

void ObservationSystem::RecordMouseClick(int x, int y, int button, int click_count,
                                         const ActionContext& context) {
    UserAction action;
    action.type = (click_count == 2) ? ActionType::MOUSE_DOUBLE_CLICK : ActionType::MOUSE_CLICK;
    
    if (button == 2) {
        action.type = ActionType::MOUSE_RIGHT_CLICK;
    }
    
    action.position = {x, y, 1920, 1080}; // Would get actual screen dimensions
    action.click_count = click_count;
    action.context = context;
    
    // Find what element was clicked
    try {
        auto& vision = Vision::VisionSystem::GetInstance();
        if (vision.IsInitialized()) {
            auto elements = vision.DetectElements(vision.CaptureScreen().image);
            for (const auto& elem : elements) {
                if (elem.bounding_box.contains(cv::Point(x, y))) {
                    action.description = "Clicked " + elem.text_content + " (" + 
                                       std::to_string((int)elem.type) + ")";
                    action.context.focused_element_id = elem.id;
                    action.context.focused_element_type = std::to_string((int)elem.type);
                    break;
                }
            }
        }
    } catch (...) {}
    
    if (action.description.empty()) {
        action.description = "Mouse click at (" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
    
    RecordAction(action);
}

void ObservationSystem::RecordMouseDrag(int start_x, int start_y, int end_x, int end_y,
                                        const ActionContext& context) {
    UserAction action;
    action.type = ActionType::MOUSE_DRAG;
    action.start_position = {start_x, start_y, 1920, 1080};
    action.position = {end_x, end_y, 1920, 1080};
    action.context = context;
    action.description = "Dragged from (" + std::to_string(start_x) + ", " + std::to_string(start_y) + 
                        ") to (" + std::to_string(end_x) + ", " + std::to_string(end_y) + ")";
    
    RecordAction(action);
}

void ObservationSystem::RecordKeyboardInput(const std::string& text, const ActionContext& context) {
    UserAction action;
    action.type = ActionType::KEYBOARD_TYPING;
    action.text_typed = text;
    action.context = context;
    action.description = "Typed: \"" + text + "\"";
    
    RecordAction(action);
}

void ObservationSystem::RecordKeyboardShortcut(const KeyCombination& keys, 
                                              const ActionContext& context) {
    UserAction action;
    action.type = ActionType::KEYBOARD_SHORTCUT;
    action.key_combination = keys;
    action.context = context;
    action.description = "Pressed: " + keys.to_string();
    
    RecordAction(action);
}

void ObservationSystem::RecordActionOutcome(const std::string& action_id, 
                                             const ActionResult& result) {
    for (auto& action : m_RecordedActions) {
        if (action.id == action_id) {
            action.result = result;
            
            // Learn from outcome
            if (result.outcome == ActionOutcome::SUCCESS) {
                LearnFromSuccess(action_id);
            } else if (result.outcome == ActionOutcome::FAILURE) {
                LearnFromFailure(action_id, result.error_type);
            }
            
            // Capture after screenshot
            try {
                auto& vision = Vision::VisionSystem::GetInstance();
                if (vision.IsInitialized()) {
                    action.screenshot_after = vision.CaptureScreen().image;
                }
            } catch (...) {}
            
            break;
        }
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// OBSERVATION SESSIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ObservationSession ObservationSystem::StartSession(const std::string& goal) {
    ObservationSession session;
    session.id = GenerateSessionId();
    session.start_time = std::chrono::system_clock::now().time_since_epoch().count();
    session.goal = goal;
    
    m_CurrentSessionId = session.id;
    m_SessionStartTime = session.start_time;
    m_CurrentGoal = goal;
    
    m_Sessions.push_back(session);
    
    std::cout << "[ObservationSystem] Started session: " << goal << std::endl;
    
    return session;
}

void ObservationSystem::EndSession(const std::string& session_id) {
    for (auto& session : m_Sessions) {
        if (session.id == session_id) {
            session.end_time = std::chrono::system_clock::now().time_since_epoch().count();
            
            // Analyze patterns from this session
            AnalyzePatterns();
            
            // Calculate completion rate
            int successful = 0;
            for (const auto& action : session.actions) {
                if (action.result.outcome == ActionOutcome::SUCCESS) {
                    successful++;
                }
            }
            if (!session.actions.empty()) {
                session.completion_rate = (float)successful / session.actions.size();
            }
            
            std::cout << "[ObservationSystem] Ended session: " << session_id 
                      << " (completion: " << (session.completion_rate * 100) << "%)" << std::endl;
            
            break;
        }
    }
    
    if (m_CurrentSessionId == session_id) {
        m_CurrentSessionId.clear();
        m_CurrentGoal.clear();
    }
}

ObservationSession ObservationSystem::GetSession(const std::string& session_id) {
    for (const auto& session : m_Sessions) {
        if (session.id == session_id) {
            return session;
        }
    }
    return ObservationSession{};
}

std::vector<ObservationSession> ObservationSystem::GetRecentSessions(int count) {
    std::vector<ObservationSession> recent;
    
    int start = std::max(0, (int)m_Sessions.size() - count);
    for (int i = start; i < (int)m_Sessions.size(); i++) {
        recent.push_back(m_Sessions[i]);
    }
    
    return recent;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN LEARNING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void ObservationSystem::AnalyzePatterns() {
    std::cout << "[ObservationSystem] Analyzing patterns..." << std::endl;
    
    // Detect various pattern types
    auto sequences = DetectActionSequences();
    auto shortcuts = DetectShortcuts();
    auto workflows = DetectWorkflows();
    
    // Add new patterns
    for (const auto& pattern : sequences) {
        bool exists = false;
        for (const auto& existing : m_LearnedPatterns) {
            if (existing.id == pattern.id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_LearnedPatterns.push_back(pattern);
        }
    }
    
    for (const auto& pattern : shortcuts) {
        bool exists = false;
        for (const auto& existing : m_LearnedPatterns) {
            if (existing.id == pattern.id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_LearnedPatterns.push_back(pattern);
        }
    }
    
    for (const auto& pattern : workflows) {
        bool exists = false;
        for (const auto& existing : m_LearnedPatterns) {
            if (existing.id == pattern.id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_LearnedPatterns.push_back(pattern);
        }
    }
    
    std::cout << "[ObservationSystem] Found " << m_LearnedPatterns.size() << " patterns" << std::endl;
}

std::vector<LearningPattern> ObservationSystem::DetectActionSequences() {
    return DetectNGramPatterns(3); // Look for 3-action sequences
}

std::vector<LearningPattern> ObservationSystem::DetectShortcuts() {
    std::vector<LearningPattern> shortcuts;
    
    // Find keyboard shortcuts that appear frequently
    std::map<std::string, std::vector<UserAction>> shortcut_map;
    
    for (const auto& action : m_RecordedActions) {
        if (action.type == ActionType::KEYBOARD_SHORTCUT) {
            std::string key_str = action.key_combination.to_string();
            shortcut_map[key_str].push_back(action);
        }
    }
    
    int pattern_id = 0;
    for (const auto& [key_str, actions] : shortcut_map) {
        if (actions.size() >= 2) { // Appeared at least twice
            LearningPattern pattern;
            pattern.id = "shortcut_" + std::to_string(pattern_id++);
            pattern.pattern_type = "shortcut";
            pattern.description = "Keyboard shortcut: " + key_str;
            pattern.trigger_condition = key_str;
            pattern.action_sequence = actions;
            pattern.times_demonstrated = actions.size();
            
            // Calculate success rate
            int successful = 0;
            for (const auto& action : actions) {
                if (action.result.outcome == ActionOutcome::SUCCESS) {
                    successful++;
                }
            }
            pattern.success_rate = (float)successful / actions.size();
            pattern.confidence = std::min(1.0f, actions.size() * 0.1f);
            pattern.first_demonstrated = actions.front().timestamp;
            pattern.last_used = actions.back().timestamp;
            
            shortcuts.push_back(pattern);
        }
    }
    
    return shortcuts;
}

std::vector<LearningPattern> ObservationSystem::DetectWorkflows() {
    std::vector<LearningPattern> workflows;
    
    // Detect common workflow patterns (longer sequences)
    auto ngram_patterns = DetectNGramPatterns(5);
    
    for (const auto& pattern : ngram_patterns) {
        if (pattern.action_sequence.size() >= 4) {
            LearningPattern workflow = pattern;
            workflow.pattern_type = "workflow";
            workflow.description = "Workflow: " + pattern.description;
            workflows.push_back(workflow);
        }
    }
    
    return workflows;
}

LearningPattern ObservationSystem::MergePatterns(const std::vector<LearningPattern>& patterns) {
    if (patterns.empty()) return LearningPattern{};
    
    LearningPattern merged = patterns[0];
    merged.id = "merged_" + std::to_string(m_LearnedPatterns.size());
    
    std::set<std::string> generalized_contexts;
    for (const auto& pattern : patterns) {
        for (const auto& context : pattern.required_contexts) {
            generalized_contexts.insert(context);
        }
        merged.generalizations.insert(merged.generalizations.end(),
                                        pattern.generalizations.begin(),
                                        pattern.generalizations.end());
    }
    
    merged.required_contexts.assign(generalized_contexts.begin(), generalized_contexts.end());
    merged.confidence = 0.0f;
    for (const auto& pattern : patterns) {
        merged.confidence += pattern.confidence;
    }
    merged.confidence /= patterns.size();
    
    return merged;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN DETECTION ALGORITHMS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::vector<LearningPattern> ObservationSystem::DetectNGramPatterns(int n) {
    std::vector<LearningPattern> patterns;
    
    if (m_RecordedActions.size() < (size_t)n) return patterns;
    
    // Create n-grams
    std::map<std::vector<std::string>, std::vector<std::vector<UserAction>>> ngram_map;
    
    for (size_t i = 0; i <= m_RecordedActions.size() - n; i++) {
        std::vector<std::string> ngram_key;
        std::vector<UserAction> ngram_actions;
        
        for (int j = 0; j < n; j++) {
            ngram_key.push_back(std::to_string((int)m_RecordedActions[i + j].type));
            ngram_actions.push_back(m_RecordedActions[i + j]);
        }
        
        ngram_map[ngram_key].push_back(ngram_actions);
    }
    
    // Find frequent n-grams
    int pattern_id = 0;
    for (const auto& [ngram_key, occurrences] : ngram_map) {
        if (occurrences.size() >= 2) { // Appeared at least twice
            LearningPattern pattern;
            pattern.id = "sequence_" + std::to_string(pattern_id++);
            pattern.pattern_type = "sequence";
            
            // Build description
            std::ostringstream desc;
            for (size_t i = 0; i < ngram_key.size(); i++) {
                if (i > 0) desc << " -> ";
                desc << ngram_key[i];
            }
            pattern.description = desc.str();
            
            pattern.action_sequence = occurrences[0];
            pattern.times_demonstrated = occurrences.size();
            
            // Calculate success rate
            int successful = 0;
            for (const auto& action : occurrences[0]) {
                if (action.result.outcome == ActionOutcome::SUCCESS) {
                    successful++;
                }
            }
            pattern.success_rate = (float)successful / occurrences[0].size();
            pattern.confidence = std::min(1.0f, occurrences.size() * 0.2f);
            
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<LearningPattern> ObservationSystem::DetectRepetitivePatterns() {
    std::vector<LearningPattern> patterns;
    
    // Find repeated action sequences
    std::map<std::string, int> repetition_count;
    std::map<std::string, std::vector<UserAction>> repetition_actions;
    
    for (size_t i = 1; i < m_RecordedActions.size(); i++) {
        const auto& current = m_RecordedActions[i];
        const auto& previous = m_RecordedActions[i - 1];
        
        std::string key = std::to_string((int)previous.type) + "_" + 
                         std::to_string((int)current.type);
        
        repetition_count[key]++;
        if (repetition_count[key] == 1) {
            repetition_actions[key] = {previous, current};
        }
    }
    
    int pattern_id = 0;
    for (const auto& [key, count] : repetition_count) {
        if (count >= 3) { // Repeated at least 3 times
            LearningPattern pattern;
            pattern.id = "repetitive_" + std::to_string(pattern_id++);
            pattern.pattern_type = "sequence";
            pattern.description = "Repeated pattern: " + key;
            pattern.action_sequence = repetition_actions[key];
            pattern.times_demonstrated = count;
            pattern.confidence = std::min(1.0f, count * 0.15f);
            
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<LearningPattern> ObservationSystem::DetectConditionalPatterns() {
    // Detect patterns that occur in specific contexts
    std::vector<LearningPattern> patterns;
    
    // Group actions by context
    std::map<std::string, std::vector<UserAction>> context_groups;
    
    for (const auto& action : m_RecordedActions) {
        std::string context_key = action.context.window_title + "_" + 
                                  action.context.application_name;
        context_groups[context_key].push_back(action);
    }
    
    // Look for patterns within each context
    int pattern_id = 0;
    for (const auto& [context, actions] : context_groups) {
        if (actions.size() >= 4) {
            auto ngram_patterns = DetectNGramPatterns(3);
            for (auto& pattern : ngram_patterns) {
                pattern.id = "conditional_" + std::to_string(pattern_id++);
                pattern.pattern_type = "workflow";
                pattern.trigger_condition = context;
                pattern.required_contexts = {context};
                patterns.push_back(pattern);
            }
        }
    }
    
    return patterns;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN MANAGEMENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void ObservationSystem::AddLearnedPattern(const LearningPattern& pattern) {
    m_LearnedPatterns.push_back(pattern);
}

void ObservationSystem::UpdatePattern(const LearningPattern& pattern) {
    for (auto& existing : m_LearnedPatterns) {
        if (existing.id == pattern.id) {
            existing = pattern;
            return;
        }
    }
}

void ObservationSystem::RemovePattern(const std::string& pattern_id) {
    m_LearnedPatterns.erase(
        std::remove_if(m_LearnedPatterns.begin(), m_LearnedPatterns.end(),
            [&pattern_id](const LearningPattern& p) { return p.id == pattern_id; }),
        m_LearnedPatterns.end()
    );
}

LearningPattern ObservationSystem::GetPattern(const std::string& pattern_id) {
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern.id == pattern_id) {
            return pattern;
        }
    }
    return LearningPattern{};
}

std::vector<LearningPattern> ObservationSystem::GetAllPatterns() {
    return m_LearnedPatterns;
}

std::vector<LearningPattern> ObservationSystem::GetPatternsByType(const std::string& type) {
    std::vector<LearningPattern> filtered;
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern.pattern_type == type) {
            filtered.push_back(pattern);
        }
    }
    return filtered;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN MATCHING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::vector<LearningPattern> ObservationSystem::FindMatchingPatterns(
    const std::vector<UserAction>& recent_actions) {
    
    std::vector<LearningPattern> matches;
    
    for (const auto& pattern : m_LearnedPatterns) {
        float match = CalculatePatternMatch(pattern, recent_actions);
        if (match >= PATTERN_MATCH_THRESHOLD) {
            matches.push_back(pattern);
        }
    }
    
    // Sort by match score
    std::sort(matches.begin(), matches.end(),
        [&recent_actions, this](const LearningPattern& a, const LearningPattern& b) {
            return CalculatePatternMatch(a, recent_actions) > 
                   CalculatePatternMatch(b, recent_actions);
        });
    
    return matches;
}

LearningPattern* ObservationSystem::FindBestMatchingPattern(
    const std::vector<UserAction>& recent_actions) {
    
    float best_match = 0.0f;
    LearningPattern* best_pattern = nullptr;
    
    for (auto& pattern : m_LearnedPatterns) {
        float match = CalculatePatternMatch(pattern, recent_actions);
        if (match > best_match) {
            best_match = match;
            best_pattern = &pattern;
        }
    }
    
    if (best_match < PATTERN_MATCH_THRESHOLD) {
        return nullptr;
    }
    
    return best_pattern;
}

float ObservationSystem::CalculatePatternMatch(const LearningPattern& pattern,
                                                const std::vector<UserAction>& actions) {
    
    if (pattern.action_sequence.empty() || actions.empty()) {
        return 0.0f;
    }
    
    // Simple matching: count how many actions in sequence match
    int matches = 0;
    size_t max_compare = std::min(pattern.action_sequence.size(), actions.size());
    
    for (size_t i = 0; i < max_compare; i++) {
        if (pattern.action_sequence[i].type == actions[i].type) {
            matches++;
        }
    }
    
    float sequence_match = (float)matches / pattern.action_sequence.size();
    float recency_match = 1.0f;
    
    // Check if pattern was recently used
    uint64_t now = std::chrono::system_clock::now().time_since_epoch().count();
    uint64_t time_diff = now - pattern.last_used;
    if (time_diff < 3600000000000ULL) { // Within 1 hour
        recency_match = 1.0f - (time_diff / 3600000000000.0f);
    }
    
    return (sequence_match * 0.7f + recency_match * 0.3f) * pattern.confidence;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CONTEXT CAPTURE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ActionContext ObservationSystem::CaptureCurrentContext() {
    ActionContext context;
    
    // Get active window info
#ifdef _WIN32
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        char title[256];
        char class_name[256];
        GetWindowText(hwnd, title, sizeof(title));
        GetClassName(hwnd, class_name, sizeof(class_name));
        context.window_title = title;
        context.application_name = class_name;
    }
#endif
    
    // Capture screenshot for context
    try {
        auto& vision = Vision::VisionSystem::GetInstance();
        if (vision.IsInitialized()) {
            auto capture = vision.CaptureScreen();
            context.ui_state = vision.AnalyzeScreenContent(capture.image);
            
            // Detect focused element
            auto elements = vision.DetectElements(capture.image);
#ifdef _WIN32
            POINT cursor_pos;
            GetCursorPos(&cursor_pos);
            int cursor_x = cursor_pos.x;
            int cursor_y = cursor_pos.y;
#else
            int cursor_x = 0;
            int cursor_y = 0;
#endif
            
            for (const auto& elem : elements) {
                if (elem.bounding_box.contains(cv::Point(cursor_x, cursor_y))) {
                    context.focused_element_id = elem.id;
                    context.focused_element_type = std::to_string((int)elem.type);
                    context.focused_element_text = elem.text_content;
                    context.element_bounds = elem.bounding_box;
                    break;
                }
            }
        }
    } catch (...) {}
    
    m_LastContext = context;
    return context;
}

void ObservationSystem::UpdateContextSnapshot() {
    m_LastContext = CaptureCurrentContext();
    
    try {
        auto& vision = Vision::VisionSystem::GetInstance();
        if (vision.IsInitialized()) {
            m_LastScreenshot = vision.CaptureScreen().image;
        }
    } catch (...) {}
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// LEARNING FROM OBSERVATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void ObservationSystem::LearnFromSuccess(const std::string& action_id) {
    for (auto& action : m_RecordedActions) {
        if (action.id == action_id) {
            action.is_learned = true;
            
            // Update pattern statistics
            for (auto& pattern : m_LearnedPatterns) {
                bool found = false;
                for (const auto& pattern_action : pattern.action_sequence) {
                    if (pattern_action.id == action_id) {
                        pattern.times_successful++;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    UpdatePatternStatistics(pattern);
                }
            }
            
            break;
        }
    }
}

void ObservationSystem::LearnFromFailure(const std::string& action_id, const std::string& error) {
    for (auto& action : m_RecordedActions) {
        if (action.id == action_id) {
            action.result.outcome = ActionOutcome::FAILURE;
            action.result.error_type = error;
            
            // Update pattern statistics
            for (auto& pattern : m_LearnedPatterns) {
                bool found = false;
                for (const auto& pattern_action : pattern.action_sequence) {
                    if (pattern_action.id == action_id) {
                        pattern.times_failed++;
                        found = true;
                        break;
                    }
                }
                if (found) {
                    UpdatePatternStatistics(pattern);
                }
            }
            
            break;
        }
    }
}

void ObservationSystem::ProcessDemonstration(const std::vector<UserAction>& demonstration) {
    // Mark all actions as demonstrated
    for (auto& action : m_RecordedActions) {
        for (const auto& demo_action : demonstration) {
            if (action.timestamp == demo_action.timestamp &&
                action.type == demo_action.type) {
                action.is_demonstrated = true;
                action.confidence = 1.0f;
                break;
            }
        }
    }
    
    // Analyze demonstration for patterns
    ObservationSession demo_session;
    demo_session.id = "demo_" + std::to_string(m_Sessions.size());
    demo_session.start_time = demonstration.front().timestamp;
    demo_session.end_time = demonstration.back().timestamp;
    demo_session.actions = demonstration;
    demo_session.key_learning_points = {"User demonstration"};
    
    m_Sessions.push_back(demo_session);
    
    // Trigger pattern analysis
    AnalyzePatterns();
    
    std::cout << "[ObservationSystem] Processed demonstration with " 
              << demonstration.size() << " actions" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// QUERY AND SEARCH
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::vector<UserAction> ObservationSystem::QueryActions(ActionType type,
                                                          uint64_t start_time,
                                                          uint64_t end_time) {
    std::vector<UserAction> results;
    
    for (const auto& action : m_RecordedActions) {
        if (action.type == type) {
            if ((start_time == 0 || action.timestamp >= start_time) &&
                (end_time == 0 || action.timestamp <= end_time)) {
                results.push_back(action);
            }
        }
    }
    
    return results;
}

std::vector<UserAction> ObservationSystem::GetActionsInRegion(const cv::Rect& region) {
    std::vector<UserAction> results;
    
    for (const auto& action : m_RecordedActions) {
        if (region.contains(cv::Point(action.position.x, action.position.y))) {
            results.push_back(action);
        }
    }
    
    return results;
}

std::map<ActionType, int> ObservationSystem::GetActionStatistics() {
    std::map<ActionType, int> stats;
    
    for (const auto& action : m_RecordedActions) {
        stats[action.type]++;
    }
    
    return stats;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PERSISTENCE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void ObservationSystem::SaveObservations(const std::string& filepath) {
    json data;
    
    // Save patterns
    json patterns = json::array();
    for (const auto& pattern : m_LearnedPatterns) {
        json p;
        p["id"] = pattern.id;
        p["pattern_type"] = pattern.pattern_type;
        p["description"] = pattern.description;
        p["times_demonstrated"] = pattern.times_demonstrated;
        p["times_successful"] = pattern.times_successful;
        p["times_failed"] = pattern.times_failed;
        p["success_rate"] = pattern.success_rate;
        p["confidence"] = pattern.confidence;
        p["last_used"] = pattern.last_used;
        p["first_demonstrated"] = pattern.first_demonstrated;
        patterns.push_back(p);
    }
    data["patterns"] = patterns;
    
    // Save sessions (without screenshots to save space)
    json sessions = json::array();
    for (const auto& session : m_Sessions) {
        json s;
        s["id"] = session.id;
        s["start_time"] = session.start_time;
        s["end_time"] = session.end_time;
        s["goal"] = session.goal;
        s["completion_rate"] = session.completion_rate;
        s["action_count"] = session.actions.size();
        sessions.push_back(s);
    }
    data["sessions"] = sessions;
    
    // Save to file
    std::ofstream file(filepath);
    file << data.dump(2);
    
    std::cout << "[ObservationSystem] Saved observations to: " << filepath << std::endl;
}

void ObservationSystem::LoadObservations(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[ObservationSystem] Could not load observations from: " << filepath << std::endl;
        return;
    }
    
    json data;
    file >> data;
    
    // Load patterns
    if (data.contains("patterns")) {
        for (const auto& p : data["patterns"]) {
            LearningPattern pattern;
            pattern.id = p["id"];
            pattern.pattern_type = p["pattern_type"];
            pattern.description = p["description"];
            pattern.times_demonstrated = p["times_demonstrated"];
            pattern.times_successful = p["times_successful"];
            pattern.times_failed = p["times_failed"];
            pattern.success_rate = p["success_rate"];
            pattern.confidence = p["confidence"];
            pattern.last_used = p["last_used"];
            pattern.first_demonstrated = p["first_demonstrated"];
            m_LearnedPatterns.push_back(pattern);
        }
    }
    
    std::cout << "[ObservationSystem] Loaded " << m_LearnedPatterns.size() 
              << " patterns from: " << filepath << std::endl;
}

void ObservationSystem::ExportPatterns(const std::string& filepath) {
    json data = json::array();
    
    for (const auto& pattern : m_LearnedPatterns) {
        json p;
        p["id"] = pattern.id;
        p["pattern_type"] = pattern.pattern_type;
        p["description"] = pattern.description;
        p["trigger_condition"] = pattern.trigger_condition;
        p["required_contexts"] = pattern.required_contexts;
        p["times_demonstrated"] = pattern.times_demonstrated;
        p["times_successful"] = pattern.times_successful;
        p["times_failed"] = pattern.times_failed;
        p["success_rate"] = pattern.success_rate;
        p["confidence"] = pattern.confidence;
        p["last_used"] = pattern.last_used;
        p["first_demonstrated"] = pattern.first_demonstrated;
        p["generalizations"] = pattern.generalizations;
        
        data.push_back(p);
    }
    
    std::ofstream file(filepath);
    file << data.dump(2);
    
    std::cout << "[ObservationSystem] Exported " << data.size() 
              << " patterns to: " << filepath << std::endl;
}

void ObservationSystem::ImportPatterns(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[ObservationSystem] Could not import patterns from: " << filepath << std::endl;
        return;
    }
    
    json data;
    file >> data;
    
    for (const auto& p : data) {
        LearningPattern pattern;
        pattern.id = p["id"];
        pattern.pattern_type = p["pattern_type"];
        pattern.description = p["description"];
        pattern.trigger_condition = p.value("trigger_condition", "");
        pattern.required_contexts = p.value("required_contexts", std::vector<std::string>{});
        pattern.times_demonstrated = p["times_demonstrated"];
        pattern.times_successful = p["times_successful"];
        pattern.times_failed = p["times_failed"];
        pattern.success_rate = p["success_rate"];
        pattern.confidence = p["confidence"];
        pattern.last_used = p["last_used"];
        pattern.first_demonstrated = p["first_demonstrated"];
        pattern.generalizations = p.value("generalizations", std::vector<std::string>{});
        
        m_LearnedPatterns.push_back(pattern);
    }
    
    std::cout << "[ObservationSystem] Imported " << data.size() 
              << " patterns from: " << filepath << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// STATISTICS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

json ObservationSystem::GetStatistics() {
    json stats;
    
    stats["total_actions"] = m_RecordedActions.size();
    stats["total_sessions"] = m_Sessions.size();
    stats["total_patterns"] = m_LearnedPatterns.size();
    stats["is_observing"] = m_IsObserving;
    stats["is_paused"] = m_IsPaused;
    
    // Action type breakdown
    json action_breakdown = json::object();
    auto action_stats = GetActionStatistics();
    for (const auto& [type, count] : action_stats) {
        action_breakdown[std::to_string((int)type)] = count;
    }
    stats["action_breakdown"] = action_breakdown;
    
    // Pattern statistics
    int total_success = 0, total_fail = 0;
    float total_confidence = 0.0f;
    for (const auto& pattern : m_LearnedPatterns) {
        total_success += pattern.times_successful;
        total_fail += pattern.times_failed;
        total_confidence += pattern.confidence;
    }
    stats["pattern_successes"] = total_success;
    stats["pattern_failures"] = total_fail;
    stats["average_confidence"] = m_LearnedPatterns.empty() ? 0.0f : total_confidence / m_LearnedPatterns.size();
    
    return stats;
}

int ObservationSystem::GetTotalActionsObserved() {
    return m_RecordedActions.size();
}

int ObservationSystem::GetTotalPatternsLearned() {
    return m_LearnedPatterns.size();
}

float ObservationSystem::GetAverageSuccessRate() {
    if (m_LearnedPatterns.empty()) return 0.0f;
    
    float total = 0.0f;
    for (const auto& pattern : m_LearnedPatterns) {
        total += pattern.success_rate;
    }
    
    return total / m_LearnedPatterns.size();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// HELPER METHODS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::string ObservationSystem::GenerateActionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    std::ostringstream oss;
    oss << "action_" << dis(gen) << "_" << 
           std::chrono::system_clock::now().time_since_epoch().count();
    return oss.str();
}

std::string ObservationSystem::GenerateSessionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    std::ostringstream oss;
    oss << "session_" << dis(gen) << "_" << 
           std::chrono::system_clock::now().time_since_epoch().count();
    return oss.str();
}

ActionType ObservationSystem::InferActionType(const UserAction& action) {
    return action.type; // Already set during recording
}

float ObservationSystem::CalculateActionSimilarity(const UserAction& a1, const UserAction& a2) {
    if (a1.type != a2.type) return 0.0f;
    
    float similarity = 0.5f;
    
    // Same action type
    similarity += 0.3f;
    
    // Similar position (for mouse actions)
    if (a1.type >= ActionType::MOUSE_CLICK && a1.type <= ActionType::MOUSE_SCROLL) {
        float dx = (float)(a1.position.x - a2.position.x) / a1.position.screen_width;
        float dy = (float)(a1.position.y - a2.position.y) / a1.position.screen_height;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < 0.1f) similarity += 0.2f;
    }
    
    // Same context
    if (a1.context.window_title == a2.context.window_title) {
        similarity += 0.2f;
    }
    
    return similarity;
}

void ObservationSystem::UpdatePatternStatistics(LearningPattern& pattern) {
    int total = pattern.times_successful + pattern.times_failed;
    if (total > 0) {
        pattern.success_rate = (float)pattern.times_successful / total;
    }
    
    // Update confidence based on usage
    pattern.confidence = std::min(1.0f, (float)pattern.times_demonstrated * 0.1f);
    
    // Update last used time
    pattern.last_used = std::chrono::system_clock::now().time_since_epoch().count();
}

} // namespace SZM::AI::Observation
