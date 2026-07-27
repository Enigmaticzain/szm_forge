#include "LearningIntegration.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>

namespace SZM::AI::Learning {

LearningIntegration& LearningIntegration::GetInstance() {
    static LearningIntegration instance;
    return instance;
}

void LearningIntegration::Initialize() {
    if (m_IsInitialized) return;
    
    std::cout << "[LearningIntegration] Initializing..." << std::endl;
    
    // Initialize dependent systems
    m_VisionSystem.Initialize();
    m_ObservationSystem.Initialize();
    
    // Register initial skills in the skill system
    Legendary::SkillEntry observation_skill;
    observation_skill.name = "screen_observation";
    observation_skill.level = 1;
    observation_skill.confidence = 0.5f;
    m_MemoryEngine.RegisterSkill(observation_skill);
    
    Legendary::SkillEntry pattern_skill;
    pattern_skill.name = "pattern_recognition";
    pattern_skill.level = 1;
    pattern_skill.confidence = 0.5f;
    m_MemoryEngine.RegisterSkill(pattern_skill);
    
    Legendary::SkillEntry action_skill;
    action_skill.name = "action_prediction";
    action_skill.level = 1;
    action_skill.confidence = 0.5f;
    m_MemoryEngine.RegisterSkill(action_skill);
    
    m_IsInitialized = true;
    m_IsActive = true;
    
    // Start with passive learning mode
    SetLearningMode(LearningMode::PASSIVE);
    
    std::cout << "[LearningIntegration] Initialized successfully" << std::endl;
}

void LearningIntegration::Shutdown() {
    if (!m_IsInitialized) return;
    
    // Save state before shutdown
    AutoSave();
    
    // Stop observation
    StopObservation();
    
    // Shutdown dependent systems
    m_ObservationSystem.Shutdown();
    m_VisionSystem.Shutdown();
    
    m_IsInitialized = false;
    m_IsActive = false;
    
    std::cout << "[LearningIntegration] Shutdown complete" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CONFIGURATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::SetConfig(const LearningConfig& config) {
    m_Config = config;
    
    // Apply mode-specific settings
    switch (config.mode) {
        case LearningMode::PASSIVE:
            m_ObservationSystem.StartObserving();
            break;
        case LearningMode::TEACHING:
            m_ObservationSystem.StartObserving();
            break;
        default:
            break;
    }
}

void LearningIntegration::SetLearningMode(LearningMode mode) {
    m_Config.mode = mode;
    
    std::cout << "[LearningIntegration] Learning mode set to: ";
    switch (mode) {
        case LearningMode::PASSIVE:    std::cout << "PASSIVE"; break;
        case LearningMode::ACTIVE:     std::cout << "ACTIVE"; break;
        case LearningMode::SUPERVISED: std::cout << "SUPERVISED"; break;
        case LearningMode::AUTONOMOUS: std::cout << "AUTONOMOUS"; break;
        case LearningMode::TEACHING:   std::cout << "TEACHING"; break;
    }
    std::cout << std::endl;
    
    // Update observation state based on mode
    if (mode == LearningMode::PASSIVE || mode == LearningMode::TEACHING) {
        if (!m_ObservationSystem.IsObserving()) {
            m_ObservationSystem.StartObserving();
        }
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// OBSERVATION PIPELINE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::StartObservation() {
    m_ObservationSystem.StartObserving();
    std::cout << "[LearningIntegration] Started observation" << std::endl;
}

void LearningIntegration::StopObservation() {
    m_ObservationSystem.StopObserving();
    std::cout << "[LearningIntegration] Stopped observation" << std::endl;
}

void LearningIntegration::Update() {
    if (!m_IsActive || !m_IsInitialized) return;
    
    uint64_t now = std::chrono::system_clock::now().time_since_epoch().count() / 1000000; // ms
    
    // Process observation loop
    ProcessObservationLoop();
    
    // Analyze screen periodically
    if (now - m_LastScreenAnalysisTime > SCREEN_ANALYSIS_INTERVAL_MS) {
        AnalyzeScreenPeriodically();
        m_LastScreenAnalysisTime = now;
    }
    
    // Update pattern confidence
    UpdatePatternConfidence();
    
    // Auto-save periodically
    if (m_Config.auto_save && now - m_LastUpdateTime > AUTO_SAVE_INTERVAL_SECONDS * 1000) {
        AutoSave();
        m_LastUpdateTime = now;
    }
    
    // Generate suggestions based on learning mode
    if (m_Config.mode == LearningMode::ACTIVE || 
        m_Config.mode == LearningMode::AUTONOMOUS) {
        GenerateSuggestionsFromPatterns();
    }
}

void LearningIntegration::ProcessObservationLoop() {
    // This is where you'd integrate with the actual application
    // to capture real user actions. In a real implementation, this
    // would hook into the application's event system.
    
    // For now, we just ensure the observation system is running
    if (!m_ObservationSystem.IsObserving()) {
        m_ObservationSystem.StartObserving();
    }
}

void LearningIntegration::AnalyzeScreenPeriodically() {
    try {
        m_LastScreenUnderstanding = AnalyzeCurrentScreen();
        
        // Trigger callback if set
        if (m_ScreenAnalysisCallback) {
            m_ScreenAnalysisCallback(m_LastScreenUnderstanding);
        }
        
        // Store in memory
        StoreInMemory("last_screen_analysis", m_LastScreenUnderstanding.visual_analysis);
        
    } catch (const std::exception& e) {
        std::cerr << "[LearningIntegration] Screen analysis error: " << e.what() << std::endl;
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SCREEN UNDERSTANDING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ScreenUnderstanding LearningIntegration::AnalyzeCurrentScreen() {
    ScreenUnderstanding understanding;
    
    try {
        // Capture screen
        auto capture = m_VisionSystem.CaptureScreen();
        
        // Analyze content
        understanding.visual_analysis = m_VisionSystem.AnalyzeScreenContent(capture.image);
        understanding.screen_description = m_VisionSystem.DescribeScreen(capture.image);
        
        // Detect UI elements
        auto elements = m_VisionSystem.DetectElements(capture.image);
        for (const auto& elem : elements) {
            if (!elem.text_content.empty()) {
                understanding.active_elements.push_back(elem.text_content);
            }
        }
        
        // Detect application context
        understanding.detected_apps = m_VisionSystem.DetectUIFramework(capture.image);
        
        // Analyze complexity
        understanding.complexity_score = understanding.visual_analysis.value("complexity_score", 0.5f);
        
        // Get recent actions for context
        auto recent_sessions = m_ObservationSystem.GetRecentSessions(1);
        if (!recent_sessions.empty()) {
            for (const auto& action : recent_sessions.back().actions) {
                understanding.recent_actions.push_back(action.description);
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[LearningIntegration] Screen analysis failed: " << e.what() << std::endl;
    }
    
    return understanding;
}

std::string LearningIntegration::UnderstandScreenContext() {
    auto understanding = AnalyzeCurrentScreen();
    return understanding.screen_description;
}

std::vector<std::string> LearningIntegration::DetectActiveApplications() {
    std::vector<std::string> apps;
    
    try {
        auto capture = m_VisionSystem.CaptureScreen();
        apps = m_VisionSystem.DetectUIFramework(capture.image);
    } catch (const std::exception& e) {
        std::cerr << "[LearningIntegration] App detection failed: " << e.what() << std::endl;
    }
    
    return apps;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN LEARNING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::LearnFromAction(const Observation::UserAction& action) {
    if (!m_IsActive) return;
    
    // Record the action
    m_ObservationSystem.RecordAction(action);
    
    // Extract concepts from the action
    ExtractConceptsFromAction(action);
    
    // Update skill levels
    UpdateSkillLevels();
    
    std::cout << "[LearningIntegration] Learned from action: " << action.description << std::endl;
}

void LearningIntegration::LearnFromDemonstration() {
    if (m_Config.mode != LearningMode::TEACHING) {
        std::cerr << "[LearningIntegration] Can only learn from demonstration in TEACHING mode" << std::endl;
        return;
    }
    
    // Get recent actions as the demonstration
    auto recent_sessions = m_ObservationSystem.GetRecentSessions(1);
    if (!recent_sessions.empty() && !recent_sessions.back().actions.empty()) {
        m_ObservationSystem.ProcessDemonstration(recent_sessions.back().actions);
        
        std::cout << "[LearningIntegration] Processed user demonstration with " 
                  << recent_sessions.back().actions.size() << " actions" << std::endl;
    }
}

void LearningIntegration::LearnFromSuccess(const std::string& action_id) {
    m_ObservationSystem.LearnFromSuccess(action_id);
    
    // Update memory with successful action pattern
    auto actions = m_ObservationSystem.QueryActions(Observation::ActionType::MOUSE_CLICK);
    for (const auto& action : actions) {
        if (action.id == action_id) {
            Legendary::MemoryEntry memory;
            memory.id = "success_" + action_id;
            memory.type = "successful_action";
            memory.content = action.description;
            memory.tags = {action.context.window_title, action.context.application_name};
            memory.relevance_score = 10;
            memory.created_at = std::chrono::system_clock::now();
            memory.last_accessed = memory.created_at;
            
            m_MemoryEngine.StoreLongTermMemory(memory);
            break;
        }
    }
    
    // Award XP to relevant skills
    m_MemoryEngine.UpdateSkillXP("action_prediction", 5.0f);
    m_MemoryEngine.UpdateSkillXP("pattern_recognition", 3.0f);
    
    std::cout << "[LearningIntegration] Learned from success: " << action_id << std::endl;
}

void LearningIntegration::LearnFromFailure(const std::string& action_id, const std::string& error) {
    m_ObservationSystem.LearnFromFailure(action_id, error);
    
    // Record failure for future avoidance
    Legendary::FailureRecord failure;
    failure.id = "failure_" + action_id;
    failure.description = error;
    failure.context = "Action ID: " + action_id;
    failure.severity = 5;
    failure.timestamp = std::chrono::system_clock::now();
    
    m_MemoryEngine.RecordFailure(failure);
    
    std::cout << "[LearningIntegration] Learned from failure: " << error << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN APPLICATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::vector<AssistantResponse> LearningIntegration::GetSuggestions() {
    // Generate suggestions if not already generated
    if (m_CurrentSuggestions.empty()) {
        GenerateSuggestionsFromPatterns();
    }
    return m_CurrentSuggestions;
}

AssistantResponse LearningIntegration::GetBestSuggestion() {
    auto suggestions = GetSuggestions();
    if (suggestions.empty()) {
        AssistantResponse empty;
        empty.confidence = 0.0f;
        return empty;
    }
    
    // Return the highest confidence suggestion
    return *std::max_element(suggestions.begin(), suggestions.end(),
        [](const AssistantResponse& a, const AssistantResponse& b) {
            return a.confidence < b.confidence;
        });
}

bool LearningIntegration::ApplySuggestion(const std::string& suggestion_id) {
    // Find the suggestion
    for (const auto& suggestion : m_CurrentSuggestions) {
        if (suggestion.id == suggestion_id) {
            std::cout << "[LearningIntegration] Applying suggestion: " << suggestion.message << std::endl;
            
            // In a real implementation, this would trigger the action
            // For now, we just log it
            
            return true;
        }
    }
    
    return false;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CONTEXT AWARENESS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::UpdateContext() {
    m_ObservationSystem.UpdateContextSnapshot();
}

json LearningIntegration::GetCurrentContext() {
    auto context = m_ObservationSystem.CaptureCurrentContext();
    
    json ctx_json;
    ctx_json["application_name"] = context.application_name;
    ctx_json["window_title"] = context.window_title;
    ctx_json["focused_element_id"] = context.focused_element_id;
    ctx_json["focused_element_type"] = context.focused_element_type;
    ctx_json["focused_element_text"] = context.focused_element_text;
    ctx_json["ui_state"] = context.ui_state;
    
    return ctx_json;
}

std::vector<std::string> LearningIntegration::GetRecentLearnedPatterns() {
    std::vector<std::string> patterns;
    
    auto all_patterns = m_ObservationSystem.GetAllPatterns();
    for (const auto& pattern : all_patterns) {
        patterns.push_back(pattern.description);
    }
    
    return patterns;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MEMORY INTEGRATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::StoreInMemory(const std::string& key, const json& value) {
    m_MemoryEngine.StoreSessionMemory(key, value);
}

json LearningIntegration::RetrieveFromMemory(const std::string& key) {
    return m_MemoryEngine.RetrieveSessionMemory(key);
}

void LearningIntegration::SyncWithMemoryEngine() {
    // Sync learned patterns to long-term memory
    auto patterns = m_ObservationSystem.GetAllPatterns();
    
    for (const auto& pattern : patterns) {
        Legendary::MemoryEntry memory;
        memory.id = "pattern_" + pattern.id;
        memory.type = "learned_pattern";
        memory.content = pattern.description;
        memory.tags = {pattern.pattern_type, "learned"};
        memory.relevance_score = (int)(pattern.confidence * 10);
        memory.created_at = std::chrono::system_clock::from_time_t(pattern.first_demonstrated / 1000000000);
        memory.last_accessed = std::chrono::system_clock::now();
        
        m_MemoryEngine.StoreLongTermMemory(memory);
        
        // Create semantic relations
        Legendary::SemanticRelation relation;
        relation.source_id = memory.id;
        relation.relation_type = "part_of";
        relation.strength = pattern.confidence;
        
        m_MemoryEngine.CreateSemanticRelation(relation);
    }
    
    std::cout << "[LearningIntegration] Synced " << patterns.size() << " patterns to memory" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SKILL DEVELOPMENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::UpdateSkillLevels() {
    auto patterns = m_ObservationSystem.GetAllPatterns();
    
    // Calculate skill levels based on learned patterns
    int total_patterns = patterns.size();
    int successful_patterns = 0;
    
    for (const auto& pattern : patterns) {
        if (pattern.success_rate > 0.5f) {
            successful_patterns++;
        }
    }
    
    // Update skill levels
    Legendary::SkillEntry observation_skill = m_MemoryEngine.GetSkill("screen_observation");
    if (total_patterns > 10) observation_skill.level = 2;
    if (total_patterns > 50) observation_skill.level = 3;
    if (total_patterns > 100) observation_skill.level = 4;
    if (total_patterns > 200) observation_skill.level = 5;
    observation_skill.confidence = std::min(1.0f, total_patterns / 200.0f);
    m_MemoryEngine.RegisterSkill(observation_skill);
    
    Legendary::SkillEntry pattern_skill = m_MemoryEngine.GetSkill("pattern_recognition");
    if (successful_patterns > 5) pattern_skill.level = 2;
    if (successful_patterns > 20) pattern_skill.level = 3;
    if (successful_patterns > 50) pattern_skill.level = 4;
    if (successful_patterns > 100) pattern_skill.level = 5;
    pattern_skill.confidence = std::min(1.0f, successful_patterns / 100.0f);
    m_MemoryEngine.RegisterSkill(pattern_skill);
    
    // Update action prediction skill based on suggestions acceptance
    Legendary::SkillEntry action_skill = m_MemoryEngine.GetSkill("action_prediction");
    action_skill.confidence = m_ObservationSystem.GetAverageSuccessRate();
    m_MemoryEngine.RegisterSkill(action_skill);
}

std::vector<Observation::LearningPattern> LearningIntegration::GetLearnedSkills() {
    return m_ObservationSystem.GetAllPatterns();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PERSISTENCE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::SaveState(const std::string& filepath) {
    // Save observations
    m_ObservationSystem.SaveObservations(filepath + "_observations.json");
    
    // Save patterns
    m_ObservationSystem.ExportPatterns(filepath + "_patterns.json");
    
    // Save memory engine state
    m_MemoryEngine.SaveToFile(filepath + "_memory.json");
    
    std::cout << "[LearningIntegration] Saved state to: " << filepath << std::endl;
}

void LearningIntegration::LoadState(const std::string& filepath) {
    // Load observations
    m_ObservationSystem.LoadObservations(filepath + "_observations.json");
    
    // Load patterns
    m_ObservationSystem.ImportPatterns(filepath + "_patterns.json");
    
    // Load memory engine state
    m_MemoryEngine.LoadFromFile(filepath + "_memory.json");
    
    std::cout << "[LearningIntegration] Loaded state from: " << filepath << std::endl;
}

void LearningIntegration::AutoSave() {
    SaveState(m_Config.save_directory + "/ai_state");
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// STATISTICS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

json LearningIntegration::GetStatistics() {
    json stats;
    
    stats["is_active"] = m_IsActive;
    stats["learning_mode"] = std::to_string((int)m_Config.mode);
    stats["total_learned_patterns"] = GetTotalLearnedPatterns();
    stats["overall_confidence"] = GetOverallConfidence();
    
    // Observation stats
    auto obs_stats = m_ObservationSystem.GetStatistics();
    stats["observation"] = obs_stats;
    
    // Memory stats
    stats["memory"] = m_MemoryEngine.GetMemoryStatistics();
    
    // Screen understanding
    stats["screen_complexity"] = m_LastScreenUnderstanding.complexity_score;
    stats["active_elements"] = m_LastScreenUnderstanding.active_elements.size();
    
    return stats;
}

int LearningIntegration::GetTotalLearnedPatterns() {
    return m_ObservationSystem.GetTotalPatternsLearned();
}

float LearningIntegration::GetOverallConfidence() {
    auto patterns = m_ObservationSystem.GetAllPatterns();
    if (patterns.empty()) return 0.0f;
    
    float total = 0.0f;
    for (const auto& pattern : patterns) {
        total += pattern.confidence;
    }
    
    return total / patterns.size();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CALLBACKS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::SetSuggestionCallback(
    std::function<void(const AssistantResponse&)> callback) {
    m_SuggestionCallback = callback;
}

void LearningIntegration::SetScreenAnalysisCallback(
    std::function<void(const ScreenUnderstanding&)> callback) {
    m_ScreenAnalysisCallback = callback;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// INTERNAL METHODS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void LearningIntegration::GenerateSuggestionsFromPatterns() {
    m_CurrentSuggestions.clear();
    
    if (m_Config.mode != LearningMode::ACTIVE && 
        m_Config.mode != LearningMode::AUTONOMOUS) {
        return;
    }
    
    // Get current context
    auto context = m_ObservationSystem.CaptureCurrentContext();
    auto recent_sessions = m_ObservationSystem.GetRecentSessions(1);
    
    std::vector<Observation::UserAction> recent_actions;
    if (!recent_sessions.empty()) {
        recent_actions = recent_sessions.back().actions;
    }
    
    // Find matching patterns
    auto matching_patterns = m_ObservationSystem.FindMatchingPatterns(recent_actions);
    
    int suggestion_count = 0;
    for (const auto& pattern : matching_patterns) {
        if (suggestion_count >= MAX_SUGGESTIONS) break;
        if (pattern.confidence < m_Config.confidence_threshold) continue;
        
        AssistantResponse suggestion = ResponseBuilder::CreateSuggestion(
            "Based on your pattern: " + pattern.description,
            pattern.trigger_condition,
            pattern.confidence
        );
        
        suggestion.pattern_id = pattern.id;
        for (const auto& act : pattern.action_sequence) {
            suggestion.suggested_actions.push_back(act.description);
        }
        
        m_CurrentSuggestions.push_back(suggestion);
        suggestion_count++;
        
        // Trigger callback
        if (m_SuggestionCallback) {
            m_SuggestionCallback(suggestion);
        }
    }
    
    // Add context-based suggestions
    if (m_CurrentSuggestions.empty()) {
        // No specific pattern match, provide general observation
        auto screen_desc = m_LastScreenUnderstanding.screen_description;
        if (!screen_desc.empty()) {
            AssistantResponse observation = ResponseBuilder::CreateObservation(
                "I see: " + screen_desc,
                m_LastScreenUnderstanding.visual_analysis
            );
            m_CurrentSuggestions.push_back(observation);
        }
    }
}

void LearningIntegration::UpdatePatternConfidence() {
    auto patterns = m_ObservationSystem.GetAllPatterns();
    
    for (auto& pattern : patterns) {
        // Decrease confidence for unused patterns over time
        uint64_t now = std::chrono::system_clock::now().time_since_epoch().count() / 1000000000;
        uint64_t time_since_use = now - pattern.last_used;
        
        if (time_since_use > 86400) { // 24 hours
            pattern.confidence *= 0.99f; // Slowly decay
        }
        
        // Update in observation system
        m_ObservationSystem.UpdatePattern(pattern);
    }
}

void LearningIntegration::PruneOldPatterns() {
    auto patterns = m_ObservationSystem.GetAllPatterns();
    
    // Remove low-confidence patterns if we have too many
    while ((int)patterns.size() > m_Config.max_patterns) {
        // Find the lowest confidence pattern
        auto lowest = std::min_element(patterns.begin(), patterns.end(),
            [](const Observation::LearningPattern& a, const Observation::LearningPattern& b) {
                return a.confidence < b.confidence;
            });
        
        if (lowest != patterns.end()) {
            m_ObservationSystem.RemovePattern(lowest->id);
            patterns.erase(lowest);
        }
    }
}

bool LearningIntegration::DoesCurrentContextMatchPattern(
    const Observation::LearningPattern& pattern) {
    
    auto current_context = m_ObservationSystem.CaptureCurrentContext();
    
    // Check if context matches required contexts
    for (const auto& required : pattern.required_contexts) {
        if (current_context.window_title.find(required) != std::string::npos ||
            current_context.application_name.find(required) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

float LearningIntegration::CalculateContextSimilarity(
    const Observation::ActionContext& a, 
    const Observation::ActionContext& b) {
    
    float similarity = 0.0f;
    int comparisons = 0;
    
    if (a.window_title == b.window_title) {
        similarity += 1.0f;
    }
    comparisons++;
    
    if (a.application_name == b.application_name) {
        similarity += 1.0f;
    }
    comparisons++;
    
    if (a.focused_element_type == b.focused_element_type) {
        similarity += 0.5f;
    }
    comparisons++;
    
    return comparisons > 0 ? similarity / comparisons : 0.0f;
}

void LearningIntegration::ExtractConceptsFromAction(
    const Observation::UserAction& action) {
    
    // Create memory entry for the action
    Legendary::MemoryEntry entry;
    entry.id = "action_" + action.id;
    entry.type = "observed_action";
    entry.content = action.description;
    entry.tags = {
        std::to_string((int)action.type),
        action.context.window_title,
        action.context.application_name
    };
    entry.relevance_score = 5;
    entry.created_at = std::chrono::system_clock::now();
    entry.last_accessed = entry.created_at;
    
    m_MemoryEngine.StoreLongTermMemory(entry);
    
    // Create semantic relations
    for (const auto& tag : entry.tags) {
        Legendary::SemanticRelation relation;
        relation.source_id = entry.id;
        relation.target_id = tag;
        relation.relation_type = "related_to";
        relation.strength = 0.5f;
        
        m_MemoryEngine.CreateSemanticRelation(relation);
    }
}

void LearningIntegration::UpdateSkillFromPattern(
    const Observation::LearningPattern& pattern) {
    
    // Award XP based on pattern confidence
    float xp_gain = pattern.confidence * 10.0f;
    
    m_MemoryEngine.UpdateSkillXP("pattern_recognition", xp_gain);
    m_MemoryEngine.UpdateSkillXP("screen_observation", xp_gain * 0.5f);
    
    // Level up skills if XP threshold reached
    if (m_MemoryEngine.GetSkill("pattern_recognition").xp >= 100.0f) {
        m_MemoryEngine.LevelUpSkill("pattern_recognition");
    }
    if (m_MemoryEngine.GetSkill("screen_observation").xp >= 100.0f) {
        m_MemoryEngine.LevelUpSkill("screen_observation");
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// RESPONSE BUILDER IMPLEMENTATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

AssistantResponse ResponseBuilder::CreateSuggestion(const std::string& message,
                                                    const std::string& action,
                                                    float confidence) {
    AssistantResponse response;
    response.id = "suggestion_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    response.type = ResponseType::SUGGESTION;
    response.message = message;
    response.action_description = action;
    response.confidence = confidence;
    response.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return response;
}

AssistantResponse ResponseBuilder::CreateExplanation(const std::string& message) {
    AssistantResponse response;
    response.id = "explanation_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    response.type = ResponseType::EXPLANATION;
    response.message = message;
    response.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return response;
}

AssistantResponse ResponseBuilder::CreateWarning(const std::string& message) {
    AssistantResponse response;
    response.id = "warning_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    response.type = ResponseType::WARNING;
    response.message = message;
    response.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return response;
}

AssistantResponse ResponseBuilder::CreateObservation(const std::string& message,
                                                      const json& context) {
    AssistantResponse response;
    response.id = "observation_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    response.type = ResponseType::OBSERVATION;
    response.message = message;
    response.context_data = context;
    response.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return response;
}

AssistantResponse ResponseBuilder::CreateTip(const std::string& tip) {
    AssistantResponse response;
    response.id = "tip_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    response.type = ResponseType::TIP;
    response.message = tip;
    response.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return response;
}

AssistantResponse ResponseBuilder::CreateConfirmation(const std::string& question) {
    AssistantResponse response;
    response.id = "confirmation_" + std::to_string(
        std::chrono::system_clock::now().time_since_epoch().count()
    );
    response.type = ResponseType::CONFIRMATION;
    response.message = question;
    response.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    return response;
}

} // namespace SZM::AI::Learning
