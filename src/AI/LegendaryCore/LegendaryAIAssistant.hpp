#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "LegendaryCore/MemoryEngine.hpp"
#include "LegendaryCore/SkillSystem.hpp"
#include "LegendaryCore/AgentFramework.hpp"
#include "LegendaryCore/TrainingPipeline.hpp"
#include "LegendaryCore/ModificationSystem.hpp"

namespace SZM::AI::Legendary {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// EVOLUTION STAGES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class EvolutionStage {
    STAGE_1_FOUNDATION,         // Learning basics
    STAGE_2_ENGINEERING,        // Learning engineering
    STAGE_3_AGENTIC,            // Learning autonomy
    STAGE_4_SELF_IMPROVING      // Learning to improve itself
};

struct AIState {
    EvolutionStage current_stage;
    float stage_progress = 0.0f;
    int total_tasks_completed = 0;
    int total_failures = 0;
    float overall_confidence = 0.0f;
    json capability_matrix;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// LEGENDARY AI ASSISTANT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class LegendaryAIAssistant {
public:
    static LegendaryAIAssistant& GetInstance();

    // Initialization
    void Initialize();
    void InitializeStage(EvolutionStage stage);

    // Core Operations
    json ProcessRequest(const std::string& request, const json& context = json::object());
    ExecutionResult ExecuteTask(const Task& task);
    std::vector<ExecutionResult> ExecuteWorkflow(const std::vector<Task>& tasks);

    // Learning
    void LearnFromRepository(const std::string& repo_path);
    void LearnFromDocument(const std::string& doc_path);
    void LearnFromExperience(const std::string& experience_description);
    void LearnFromFailure(const std::string& failure_description);

    // Modification
    ModificationProposal ProposeModification(
        ModificationType type,
        const std::string& description,
        const std::vector<CodeDiff>& diffs
    );
    ModificationResult ApplyModification(const ModificationProposal& proposal);

    // State Management
    AIState GetCurrentState();
    json GetCapabilityMatrix();
    json GetSkillGraph();
    json GetMemoryStatistics();

    // Evolution
    void CheckForStageProgression();
    void ProgressToNextStage();
    EvolutionStage GetCurrentStage() const { return current_state.current_stage; }

    // Reflection & Improvement
    json ReflectOnPerformance();
    json IdentifyWeaknesses();
    json SuggestImprovements();

    // Statistics & Reporting
    json GetComprehensiveReport();
    json GetPerformanceMetrics();
    json GetLearningProgress();

    // Persistence
    void SaveState(const std::string& filepath);
    void LoadState(const std::string& filepath);

private:
    LegendaryAIAssistant() = default;

    AIState current_state;
    MemoryEngine& memory_engine = MemoryEngine::GetInstance();
    SkillSystem& skill_system = SkillSystem::GetInstance();
    AgentOrchestrator& agent_orchestrator = AgentOrchestrator::GetInstance();
    TrainingPipeline& training_pipeline = TrainingPipeline::GetInstance();
    ModificationSystem& modification_system = ModificationSystem::GetInstance();

    void InitializeFoundationStage();
    void InitializeEngineeringStage();
    void InitializeAgenticStage();
    void InitializeSelfImprovingStage();

    void UpdateStateMetrics();
    bool ShouldProgressStage();
};

} // namespace SZM::AI::Legendary
