#include "LegendaryCore/LegendaryAIAssistant.hpp"
#include <iostream>

namespace SZM::AI::Legendary {

LegendaryAIAssistant& LegendaryAIAssistant::GetInstance() {
    static LegendaryAIAssistant instance;
    return instance;
}

void LegendaryAIAssistant::Initialize() {
    current_state.current_stage = EvolutionStage::STAGE_1_FOUNDATION;
    current_state.stage_progress = 0.0f;
    current_state.total_tasks_completed = 0;
    current_state.total_failures = 0;
    current_state.overall_confidence = 0.5f;

    InitializeFoundationStage();
}

void LegendaryAIAssistant::InitializeStage(EvolutionStage stage) {
    switch (stage) {
        case EvolutionStage::STAGE_1_FOUNDATION:
            InitializeFoundationStage();
            break;
        case EvolutionStage::STAGE_2_ENGINEERING:
            InitializeEngineeringStage();
            break;
        case EvolutionStage::STAGE_3_AGENTIC:
            InitializeAgenticStage();
            break;
        case EvolutionStage::STAGE_4_SELF_IMPROVING:
            InitializeSelfImprovingStage();
            break;
    }
}

void LegendaryAIAssistant::InitializeFoundationStage() {
    skill_system.InitializeFoundationSkills();
    memory_engine.RecordEvolution("STAGE_1_FOUNDATION", {
        {"timestamp", std::time(nullptr)},
        {"skills_initialized", 5}
    });
}

void LegendaryAIAssistant::InitializeEngineeringStage() {
    skill_system.InitializeEngineeringSkills();
    skill_system.InitializeSimulationSkills();
    memory_engine.RecordEvolution("STAGE_2_ENGINEERING", {
        {"timestamp", std::time(nullptr)},
        {"skills_initialized", 8}
    });
}

void LegendaryAIAssistant::InitializeAgenticStage() {
    auto planner = std::make_shared<PlannerAgent>();
    auto executor = std::make_shared<ExecutionAgent>();
    auto researcher = std::make_shared<ResearchAgent>();
    auto reviewer = std::make_shared<ReviewAgent>();

    agent_orchestrator.RegisterAgent(planner);
    agent_orchestrator.RegisterAgent(executor);
    agent_orchestrator.RegisterAgent(researcher);
    agent_orchestrator.RegisterAgent(reviewer);

    memory_engine.RecordEvolution("STAGE_3_AGENTIC", {
        {"timestamp", std::time(nullptr)},
        {"agents_initialized", 4}
    });
}

void LegendaryAIAssistant::InitializeSelfImprovingStage() {
    auto reflector = std::make_shared<ReflectionAgent>();
    agent_orchestrator.RegisterAgent(reflector);

    memory_engine.RecordEvolution("STAGE_4_SELF_IMPROVING", {
        {"timestamp", std::time(nullptr)},
        {"reflection_enabled", true}
    });
}

json LegendaryAIAssistant::ProcessRequest(const std::string& request, const json& context) {
    json response = json::object();
    response["request"] = request;
    response["status"] = "processing";

    Task task;
    task.id = "task_" + std::to_string(current_state.total_tasks_completed);
    task.description = request;
    task.context = context;

    ExecutionResult result = ExecuteTask(task);
    response["result"] = result.success;
    response["confidence"] = result.confidence;

    return response;
}

ExecutionResult LegendaryAIAssistant::ExecuteTask(const Task& task) {
    return agent_orchestrator.ExecuteTask(task);
}

std::vector<ExecutionResult> LegendaryAIAssistant::ExecuteWorkflow(const std::vector<Task>& tasks) {
    return agent_orchestrator.ExecuteTaskSequence(tasks);
}

void LegendaryAIAssistant::LearnFromRepository(const std::string& repo_path) {
    training_pipeline.ImportRepository(repo_path);
    TrainingSession session = training_pipeline.StartTrainingSession(TrainingMode::REPOSITORY);
    training_pipeline.ProcessTrainingData(session);
    training_pipeline.CompleteTrainingSession(session);
}

void LegendaryAIAssistant::LearnFromDocument(const std::string& doc_path) {
    training_pipeline.ImportDocument(doc_path, DataSourceType::MARKDOWN_DOC);
    TrainingSession session = training_pipeline.StartTrainingSession(TrainingMode::SUPERVISED);
    training_pipeline.ProcessTrainingData(session);
    training_pipeline.CompleteTrainingSession(session);
}

void LegendaryAIAssistant::LearnFromExperience(const std::string& experience_description) {
    training_pipeline.ObserveAction(experience_description, json::object());
}

void LegendaryAIAssistant::LearnFromFailure(const std::string& failure_description) {
    training_pipeline.ObserveFailure(failure_description);
    current_state.total_failures++;
}

ModificationProposal LegendaryAIAssistant::ProposeModification(
    ModificationType type,
    const std::string& description,
    const std::vector<CodeDiff>& diffs) {

    return modification_system.CreateProposal(type, description, diffs);
}

ModificationResult LegendaryAIAssistant::ApplyModification(const ModificationProposal& proposal) {
    return modification_system.ApplyModification(proposal);
}

AIState LegendaryAIAssistant::GetCurrentState() {
    UpdateStateMetrics();
    return current_state;
}

json LegendaryAIAssistant::GetCapabilityMatrix() {
    json matrix = json::object();
    matrix["stage"] = static_cast<int>(current_state.current_stage);
    matrix["skills"] = skill_system.GetSkillGraph();
    matrix["agents"] = json::array();

    for (const auto& agent : agent_orchestrator.GetAllAgents()) {
        matrix["agents"].push_back(agent->GetName());
    }

    return matrix;
}

json LegendaryAIAssistant::GetSkillGraph() {
    return skill_system.GetSkillGraph();
}

json LegendaryAIAssistant::GetMemoryStatistics() {
    return memory_engine.GetMemoryStatistics();
}

void LegendaryAIAssistant::CheckForStageProgression() {
    if (ShouldProgressStage()) {
        ProgressToNextStage();
    }
}

void LegendaryAIAssistant::ProgressToNextStage() {
    int next_stage = static_cast<int>(current_state.current_stage) + 1;
    if (next_stage <= static_cast<int>(EvolutionStage::STAGE_4_SELF_IMPROVING)) {
        current_state.current_stage = static_cast<EvolutionStage>(next_stage);
        InitializeStage(current_state.current_stage);
        current_state.stage_progress = 0.0f;
    }
}

json LegendaryAIAssistant::ReflectOnPerformance() {
    json reflection = json::object();
    reflection["total_tasks"] = current_state.total_tasks_completed;
    reflection["total_failures"] = current_state.total_failures;
    reflection["success_rate"] = current_state.total_tasks_completed > 0 ?
        (float)(current_state.total_tasks_completed - current_state.total_failures) / current_state.total_tasks_completed : 0.0f;
    reflection["overall_confidence"] = current_state.overall_confidence;
    return reflection;
}

json LegendaryAIAssistant::IdentifyWeaknesses() {
    json weaknesses = json::object();
    auto skills = skill_system.GetAllCapabilities();
    for (const auto& skill : skills) {
        if (skill.current_level < 3) {
            weaknesses[skill.skill_id] = skill.current_level;
        }
    }
    return weaknesses;
}

json LegendaryAIAssistant::SuggestImprovements() {
    json suggestions = json::object();
    suggestions["focus_areas"] = json::array();
    suggestions["recommended_training"] = json::array();
    suggestions["next_milestone"] = "Reach Level 3 in all foundation skills";
    return suggestions;
}

json LegendaryAIAssistant::GetComprehensiveReport() {
    json report = json::object();
    
    json state_obj = json::object();
    state_obj["stage"] = static_cast<int>(current_state.current_stage);
    state_obj["progress"] = current_state.stage_progress;
    state_obj["tasks"] = current_state.total_tasks_completed;
    state_obj["failures"] = current_state.total_failures;
    state_obj["confidence"] = current_state.overall_confidence;
    
    report["state"] = state_obj;
    report["capabilities"] = GetCapabilityMatrix();
    report["memory_stats"] = GetMemoryStatistics();
    report["performance"] = ReflectOnPerformance();
    report["weaknesses"] = IdentifyWeaknesses();
    report["suggestions"] = SuggestImprovements();
    return report;
}

json LegendaryAIAssistant::GetPerformanceMetrics() {
    json metrics = json::object();
    metrics["tasks_completed"] = current_state.total_tasks_completed;
    metrics["failures"] = current_state.total_failures;
    metrics["confidence"] = current_state.overall_confidence;
    metrics["stage"] = static_cast<int>(current_state.current_stage);
    return metrics;
}

json LegendaryAIAssistant::GetLearningProgress() {
    json progress = json::object();
    progress["stage_progress"] = current_state.stage_progress;
    progress["skill_statistics"] = skill_system.GetSkillStatistics();
    progress["training_history"] = training_pipeline.GetTrainingStatistics();
    return progress;
}

void LegendaryAIAssistant::SaveState(const std::string& filepath) {
    memory_engine.SaveToFile(filepath);
}

void LegendaryAIAssistant::LoadState(const std::string& filepath) {
    memory_engine.LoadFromFile(filepath);
}

void LegendaryAIAssistant::UpdateStateMetrics() {
    auto stats = agent_orchestrator.GetExecutionStatistics();
    if (stats.contains("average_confidence")) {
        current_state.overall_confidence = stats["average_confidence"];
    }
}

bool LegendaryAIAssistant::ShouldProgressStage() {
    return current_state.stage_progress >= 1.0f;
}

} // namespace SZM::AI::Legendary
