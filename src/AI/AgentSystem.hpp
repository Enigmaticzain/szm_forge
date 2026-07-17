#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <cstdint>

namespace SZM::AI {

/**
 * @enum AgentType
 * @brief Types of specialized agents
 */
enum class AgentType {
    PLANNER,                // Breaks tasks into subtasks
    ENGINEERING,            // Writes and modifies code
    REVIEW,                 // Checks quality/security/performance
    RESEARCH,               // Processes knowledge
    MEMORY,                 // Stores patterns
    SIMULATION,             // Understands physics
    OPTIMIZATION            // Improves performance
};

/**
 * @enum AgentState
 * @brief Agent execution state
 */
enum class AgentState {
    IDLE,
    PLANNING,
    EXECUTING,
    REVIEWING,
    COMPLETED,
    FAILED,
    RETRY
};

/**
 * @struct AgentResult
 * @brief Result from agent execution
 */
struct AgentResult {
    uint64_t agentId;
    AgentType type;
    AgentState state;
    std::string output;
    std::vector<std::string> artifacts;
    float confidence;
    uint32_t executionTimeMs;
    std::string errorMessage;
};

/**
 * @struct AgentCapability
 * @brief Agent capability descriptor
 */
struct AgentCapability {
    std::string name;
    std::string description;
    std::vector<std::string> tools;
    float proficiency;
};

/**
 * @class Agent
 * @brief Base agent class
 */
class Agent {
public:
    virtual ~Agent() = default;

    /**
     * @brief Execute agent task
     */
    virtual AgentResult Execute(const std::string& task) = 0;

    /**
     * @brief Get agent type
     */
    virtual AgentType GetType() const = 0;

    /**
     * @brief Get agent capabilities
     */
    virtual std::vector<AgentCapability> GetCapabilities() const = 0;

    /**
     * @brief Get agent state
     */
    virtual AgentState GetState() const = 0;

    /**
     * @brief Get agent ID
     */
    uint64_t GetId() const { return m_Id; }

protected:
    uint64_t m_Id;
    AgentState m_State = AgentState::IDLE;
};

/**
 * @class PlannerAgent
 * @brief Breaks complex tasks into subtasks
 */
class PlannerAgent : public Agent {
public:
    PlannerAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::PLANNER; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class EngineeringAgent
 * @brief Writes and modifies code
 */
class EngineeringAgent : public Agent {
public:
    EngineeringAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::ENGINEERING; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class ReviewAgent
 * @brief Reviews code quality, security, performance
 */
class ReviewAgent : public Agent {
public:
    ReviewAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::REVIEW; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class ResearchAgent
 * @brief Processes imported knowledge and datasets
 */
class ResearchAgent : public Agent {
public:
    ResearchAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::RESEARCH; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class MemoryAgent
 * @brief Stores and retrieves successful patterns
 */
class MemoryAgent : public Agent {
public:
    MemoryAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::MEMORY; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class SimulationAgent
 * @brief Understands physics and simulation systems
 */
class SimulationAgent : public Agent {
public:
    SimulationAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::SIMULATION; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class OptimizationAgent
 * @brief Optimizes performance and efficiency
 */
class OptimizationAgent : public Agent {
public:
    OptimizationAgent(uint64_t id) { m_Id = id; }
    AgentResult Execute(const std::string& task) override;
    AgentType GetType() const override { return AgentType::OPTIMIZATION; }
    std::vector<AgentCapability> GetCapabilities() const override;
    AgentState GetState() const override { return m_State; }
};

/**
 * @class AgentOrchestrator
 * @brief Manages and coordinates multiple agents
 */
class AgentOrchestrator {
public:
    static AgentOrchestrator& GetInstance();

    /**
     * @brief Initialize orchestrator
     */
    void Initialize();

    /**
     * @brief Shutdown orchestrator
     */
    void Shutdown();

    /**
     * @brief Create agent
     */
    std::shared_ptr<Agent> CreateAgent(AgentType type);

    /**
     * @brief Execute agent task
     */
    AgentResult ExecuteAgent(std::shared_ptr<Agent> agent, const std::string& task);

    /**
     * @brief Execute multiple agents in sequence
     */
    std::vector<AgentResult> ExecuteAgentSequence(const std::vector<std::pair<AgentType, std::string>>& tasks);

    /**
     * @brief Execute agents in parallel
     */
    std::vector<AgentResult> ExecuteAgentsParallel(const std::vector<std::pair<AgentType, std::string>>& tasks);

    /**
     * @brief Get agent by type
     */
    std::shared_ptr<Agent> GetAgent(AgentType type);

    /**
     * @brief List all agents
     */
    std::vector<std::shared_ptr<Agent>> ListAgents();

    /**
     * @brief Get orchestrator status
     */
    std::string GetStatus();

private:
    AgentOrchestrator() = default;
    ~AgentOrchestrator() = default;

    AgentOrchestrator(const AgentOrchestrator&) = delete;
    AgentOrchestrator& operator=(const AgentOrchestrator&) = delete;

    std::map<AgentType, std::shared_ptr<Agent>> m_Agents;
    uint64_t m_NextAgentId = 1;
    bool m_IsInitialized = false;
};

} // namespace SZM::AI
