#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace SZM::AI::Legendary {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AGENT TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class TaskStatus {
    PENDING,
    PLANNING,
    EXECUTING,
    VALIDATING,
    REFLECTING,
    COMPLETED,
    FAILED
};

struct Task {
    std::string id;
    std::string description;
    std::vector<std::string> subtasks;
    TaskStatus status = TaskStatus::PENDING;
    float priority = 0.5f;
    json context;
    json result;
    std::string error_message;
};

struct ExecutionPlan {
    std::string task_id;
    std::vector<std::string> steps;
    std::vector<std::string> required_skills;
    float estimated_success_rate = 0.0f;
    json reasoning;
};

struct ExecutionResult {
    std::string task_id;
    bool success = false;
    json output;
    std::string error;
    float confidence = 0.0f;
    std::vector<std::string> skills_used;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AGENT INTERFACE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class IAgent {
public:
    virtual ~IAgent() = default;
    virtual std::string GetName() const = 0;
    virtual ExecutionPlan CreatePlan(const Task& task) = 0;
    virtual ExecutionResult Execute(const ExecutionPlan& plan) = 0;
    virtual json Reflect(const ExecutionResult& result) = 0;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SPECIALIZED AGENTS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class PlannerAgent : public IAgent {
public:
    std::string GetName() const override { return "Planner"; }
    ExecutionPlan CreatePlan(const Task& task) override;
    ExecutionResult Execute(const ExecutionPlan& plan) override;
    json Reflect(const ExecutionResult& result) override;
};

class ExecutionAgent : public IAgent {
public:
    std::string GetName() const override { return "Executor"; }
    ExecutionPlan CreatePlan(const Task& task) override;
    ExecutionResult Execute(const ExecutionPlan& plan) override;
    json Reflect(const ExecutionResult& result) override;
};

class ResearchAgent : public IAgent {
public:
    std::string GetName() const override { return "Researcher"; }
    ExecutionPlan CreatePlan(const Task& task) override;
    ExecutionResult Execute(const ExecutionPlan& plan) override;
    json Reflect(const ExecutionResult& result) override;
};

class ReviewAgent : public IAgent {
public:
    std::string GetName() const override { return "Reviewer"; }
    ExecutionPlan CreatePlan(const Task& task) override;
    ExecutionResult Execute(const ExecutionPlan& plan) override;
    json Reflect(const ExecutionResult& result) override;
};

class ReflectionAgent : public IAgent {
public:
    std::string GetName() const override { return "Reflector"; }
    ExecutionPlan CreatePlan(const Task& task) override;
    ExecutionResult Execute(const ExecutionPlan& plan) override;
    json Reflect(const ExecutionResult& result) override;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AGENT ORCHESTRATOR
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class AgentOrchestrator {
public:
    static AgentOrchestrator& GetInstance();

    void RegisterAgent(std::shared_ptr<IAgent> agent);
    ExecutionResult ExecuteTask(const Task& task);
    std::vector<ExecutionResult> ExecuteTaskSequence(const std::vector<Task>& tasks);

    // Agent Management
    std::shared_ptr<IAgent> GetAgent(const std::string& name);
    std::vector<std::shared_ptr<IAgent>> GetAllAgents();

    // Execution History
    std::vector<ExecutionResult> GetExecutionHistory();
    json GetExecutionStatistics();

private:
    AgentOrchestrator() = default;

    std::map<std::string, std::shared_ptr<IAgent>> agents;
    std::vector<ExecutionResult> execution_history;
};

} // namespace SZM::AI::Legendary
