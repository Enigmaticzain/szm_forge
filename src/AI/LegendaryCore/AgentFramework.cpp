#include "LegendaryCore/AgentFramework.hpp"
#include <algorithm>

namespace SZM::AI::Legendary {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PLANNER AGENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ExecutionPlan PlannerAgent::CreatePlan(const Task& task) {
    ExecutionPlan plan;
    plan.task_id = task.id;
    plan.steps = task.subtasks;
    plan.estimated_success_rate = 0.8f;
    plan.reasoning["strategy"] = "decompose_and_conquer";
    return plan;
}

ExecutionResult PlannerAgent::Execute(const ExecutionPlan& plan) {
    ExecutionResult result;
    result.task_id = plan.task_id;
    result.success = true;
    result.confidence = 0.9f;
    result.skills_used = {"planning", "decomposition"};
    return result;
}

json PlannerAgent::Reflect(const ExecutionResult& result) {
    json reflection;
    reflection["success"] = result.success;
    reflection["confidence"] = result.confidence;
    reflection["improvements"] = json::array();
    return reflection;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// EXECUTION AGENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ExecutionPlan ExecutionAgent::CreatePlan(const Task& task) {
    ExecutionPlan plan;
    plan.task_id = task.id;
    plan.steps = task.subtasks;
    return plan;
}

ExecutionResult ExecutionAgent::Execute(const ExecutionPlan& plan) {
    ExecutionResult result;
    result.task_id = plan.task_id;
    result.success = true;
    result.confidence = 0.85f;
    result.skills_used = {"execution", "task_management"};
    return result;
}

json ExecutionAgent::Reflect(const ExecutionResult& result) {
    json reflection;
    reflection["execution_quality"] = result.confidence;
    reflection["errors"] = json::array();
    return reflection;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// RESEARCH AGENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ExecutionPlan ResearchAgent::CreatePlan(const Task& task) {
    ExecutionPlan plan;
    plan.task_id = task.id;
    plan.steps = {"analyze", "extract_concepts", "build_knowledge_graph"};
    return plan;
}

ExecutionResult ResearchAgent::Execute(const ExecutionPlan& plan) {
    ExecutionResult result;
    result.task_id = plan.task_id;
    result.success = true;
    result.confidence = 0.8f;
    result.skills_used = {"research", "analysis", "knowledge_extraction"};
    return result;
}

json ResearchAgent::Reflect(const ExecutionResult& result) {
    json reflection;
    reflection["knowledge_gained"] = json::array();
    reflection["confidence"] = result.confidence;
    return reflection;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// REVIEW AGENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ExecutionPlan ReviewAgent::CreatePlan(const Task& task) {
    ExecutionPlan plan;
    plan.task_id = task.id;
    plan.steps = {"validate", "check_safety", "assess_performance"};
    return plan;
}

ExecutionResult ReviewAgent::Execute(const ExecutionPlan& plan) {
    ExecutionResult result;
    result.task_id = plan.task_id;
    result.success = true;
    result.confidence = 0.9f;
    result.skills_used = {"review", "validation", "quality_assurance"};
    return result;
}

json ReviewAgent::Reflect(const ExecutionResult& result) {
    json reflection;
    reflection["issues_found"] = json::array();
    reflection["quality_score"] = result.confidence;
    return reflection;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// REFLECTION AGENT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ExecutionPlan ReflectionAgent::CreatePlan(const Task& task) {
    ExecutionPlan plan;
    plan.task_id = task.id;
    plan.steps = {"analyze_performance", "identify_improvements", "update_strategies"};
    return plan;
}

ExecutionResult ReflectionAgent::Execute(const ExecutionPlan& plan) {
    ExecutionResult result;
    result.task_id = plan.task_id;
    result.success = true;
    result.confidence = 0.85f;
    result.skills_used = {"reflection", "self_analysis", "improvement_planning"};
    return result;
}

json ReflectionAgent::Reflect(const ExecutionResult& result) {
    json reflection;
    reflection["insights"] = json::array();
    reflection["improvements_identified"] = json::array();
    return reflection;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AGENT ORCHESTRATOR
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

AgentOrchestrator& AgentOrchestrator::GetInstance() {
    static AgentOrchestrator instance;
    return instance;
}

void AgentOrchestrator::RegisterAgent(std::shared_ptr<IAgent> agent) {
    agents[agent->GetName()] = agent;
}

ExecutionResult AgentOrchestrator::ExecuteTask(const Task& task) {
    if (agents.empty()) {
        ExecutionResult result;
        result.task_id = task.id;
        result.success = false;
        result.error = "No agents registered";
        return result;
    }

    auto planner = agents["Planner"];
    auto executor = agents["Executor"];

    ExecutionPlan plan = planner->CreatePlan(task);
    ExecutionResult result = executor->Execute(plan);

    execution_history.push_back(result);
    return result;
}

std::vector<ExecutionResult> AgentOrchestrator::ExecuteTaskSequence(const std::vector<Task>& tasks) {
    std::vector<ExecutionResult> results;
    for (const auto& task : tasks) {
        results.push_back(ExecuteTask(task));
    }
    return results;
}

std::shared_ptr<IAgent> AgentOrchestrator::GetAgent(const std::string& name) {
    auto it = agents.find(name);
    return it != agents.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<IAgent>> AgentOrchestrator::GetAllAgents() {
    std::vector<std::shared_ptr<IAgent>> result;
    for (const auto& [name, agent] : agents) {
        result.push_back(agent);
    }
    return result;
}

std::vector<ExecutionResult> AgentOrchestrator::GetExecutionHistory() {
    return execution_history;
}

json AgentOrchestrator::GetExecutionStatistics() {
    json stats;
    stats["total_executions"] = execution_history.size();
    stats["successful"] = 0;
    stats["failed"] = 0;
    stats["average_confidence"] = 0.0f;

    float total_confidence = 0.0f;
    int successful = 0;
    int failed = 0;
    for (const auto& result : execution_history) {
        if (result.success) successful++;
        else failed++;
        total_confidence += result.confidence;
    }
    stats["successful"] = successful;
    stats["failed"] = failed;

    if (!execution_history.empty()) {
        stats["average_confidence"] = total_confidence / execution_history.size();
    }

    return stats;
}

} // namespace SZM::AI::Legendary
