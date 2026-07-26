#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>
#include "../VisionSystem/VisionSystem.hpp"
#include "../ObservationSystem/ObservationSystem.hpp"
#include "../LegendaryCore/MemoryEngine.hpp"
#include "../LegendaryCore/TrainingPipeline.hpp"
#include "../LegendaryCore/SkillSystem.hpp"

namespace SZM::AI::Learning {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// LEARNING MODES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class LearningMode {
    PASSIVE,           // Watch and learn without interfering
    ACTIVE,            // Suggest actions based on learned patterns
    SUPERVISED,        // Learn from explicit user guidance
    AUTONOMOUS,        // Take actions and learn from outcomes
    TEACHING           // User demonstrates, AI learns
};

enum class LearningPriority {
    LOW,
    NORMAL,
    HIGH,
    CRITICAL
};

struct LearningConfig {
    LearningMode mode = LearningMode::PASSIVE;
    bool auto_save = true;
    bool continuous_learning = true;
    float confidence_threshold = 0.7f;
    int min_demonstrations = 3;
    int max_patterns = 1000;
    float learning_rate = 0.1f;
    std::string save_directory = "./ai_memory";
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT RESPONSE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class ResponseType {
    SUGGESTION,
    EXPLANATION,
    WARNING,
    ERROR,
    QUESTION,
    CONFIRMATION,
    OBSERVATION,
    TIP
};

struct AssistantResponse {
    std::string id;
    ResponseType type;
    std::string message;
    std::string action_description;
    std::string pattern_id;
    float confidence;
    std::vector<std::string> suggested_actions;
    json context_data;
    uint64_t timestamp;
};

struct ScreenUnderstanding {
    std::string screen_description;
    std::vector<std::string> detected_apps;
    std::vector<std::string> active_elements;
    std::vector<std::string> recent_actions;
    std::vector<std::string> potential_goals;
    float complexity_score;
    json visual_analysis;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// LEARNING INTEGRATION SYSTEM
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class LearningIntegration {
public:
    static LearningIntegration& GetInstance();

    // Initialization
    void Initialize();
    void Shutdown();
    bool IsActive() const { return m_IsActive; }

    // Configuration
    void SetConfig(const LearningConfig& config);
    LearningConfig GetConfig() const { return m_Config; }
    void SetLearningMode(LearningMode mode);
    LearningMode GetLearningMode() const { return m_Config.mode; }

    // Observation Pipeline
    void StartObservation();
    void StopObservation();
    void Update();  // Called every frame/tick
    
    // Screen Understanding
    ScreenUnderstanding AnalyzeCurrentScreen();
    std::string UnderstandScreenContext();
    std::vector<std::string> DetectActiveApplications();
    
    // Pattern Learning
    void LearnFromAction(const Observation::UserAction& action);
    void LearnFromDemonstration();
    void LearnFromSuccess(const std::string& action_id);
    void LearnFromFailure(const std::string& action_id, const std::string& error);
    
    // Pattern Application
    std::vector<AssistantResponse> GetSuggestions();
    AssistantResponse GetBestSuggestion();
    bool ApplySuggestion(const std::string& suggestion_id);
    
    // Context Awareness
    void UpdateContext();
    json GetCurrentContext();
    std::vector<std::string> GetRecentLearnedPatterns();
    
    // Memory Integration
    void StoreInMemory(const std::string& key, const json& value);
    json RetrieveFromMemory(const std::string& key);
    void SyncWithMemoryEngine();
    
    // Skill Development
    void UpdateSkillLevels();
    std::vector<Observation::LearningPattern> GetLearnedSkills();
    
    // Persistence
    void SaveState(const std::string& filepath);
    void LoadState(const std::string& filepath);
    void AutoSave();
    
    // Statistics
    json GetStatistics();
    int GetTotalLearnedPatterns();
    float GetOverallConfidence();
    
    // Callbacks for UI integration
    void SetSuggestionCallback(std::function<void(const AssistantResponse&)> callback);
    void SetScreenAnalysisCallback(std::function<void(const ScreenUnderstanding&)> callback);

private:
    LearningIntegration() = default;

    LearningIntegration(const LearningIntegration&) = delete;
    LearningIntegration& operator=(const LearningIntegration&) = delete;

    bool m_IsInitialized = false;
    bool m_IsActive = false;
    LearningConfig m_Config;
    
    // Core systems
    Vision::VisionSystem& m_VisionSystem = Vision::VisionSystem::GetInstance();
    Observation::ObservationSystem& m_ObservationSystem = Observation::ObservationSystem::GetInstance();
    Legendary::MemoryEngine& m_MemoryEngine = Legendary::MemoryEngine::GetInstance();
    Legendary::TrainingPipeline& m_TrainingPipeline = Legendary::TrainingPipeline::GetInstance();
    Legendary::SkillSystem& m_SkillSystem = Legendary::SkillSystem::GetInstance();
    
    // State
    uint64_t m_LastUpdateTime = 0;
    uint64_t m_LastScreenAnalysisTime = 0;
    ScreenUnderstanding m_LastScreenUnderstanding;
    std::vector<AssistantResponse> m_CurrentSuggestions;
    
    // Callbacks
    std::function<void(const AssistantResponse&)> m_SuggestionCallback;
    std::function<void(const ScreenUnderstanding&)> m_ScreenAnalysisCallback;
    
    // Internal methods
    void AnalyzeScreenPeriodically();
    void GenerateSuggestionsFromPatterns();
    void UpdatePatternConfidence();
    void PruneOldPatterns();
    void ProcessObservationLoop();
    
    // Pattern matching helpers
    bool DoesCurrentContextMatchPattern(const Observation::LearningPattern& pattern);
    float CalculateContextSimilarity(const Observation::ActionContext& a, 
                                      const Observation::ActionContext& b);
    
    // Learning helpers
    void ExtractConceptsFromAction(const Observation::UserAction& action);
    void UpdateSkillFromPattern(const Observation::LearningPattern& pattern);
    
    // Constants
    static constexpr uint64_t SCREEN_ANALYSIS_INTERVAL_MS = 2000;  // Every 2 seconds
    static constexpr int MAX_SUGGESTIONS = 5;
    static constexpr int AUTO_SAVE_INTERVAL_SECONDS = 300;  // Every 5 minutes
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ASSISTANT RESPONSE BUILDER (Helper)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class ResponseBuilder {
public:
    static AssistantResponse CreateSuggestion(const std::string& message,
                                              const std::string& action,
                                              float confidence);
    static AssistantResponse CreateExplanation(const std::string& message);
    static AssistantResponse CreateWarning(const std::string& message);
    static AssistantResponse CreateObservation(const std::string& message,
                                               const json& context);
    static AssistantResponse CreateTip(const std::string& tip);
    static AssistantResponse CreateConfirmation(const std::string& question);
};

} // namespace SZM::AI::Learning
