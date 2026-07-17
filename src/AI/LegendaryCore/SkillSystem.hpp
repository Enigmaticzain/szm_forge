#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

namespace SZM::AI::Legendary {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SKILL DEFINITIONS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class SkillCategory {
    FOUNDATION,         // Text, code, logic understanding
    SYSTEM,             // Linux, terminal, processes
    ENGINEERING,        // Debugging, optimization, refactoring
    AI_OPERATION,       // Planning, reflection, validation
    SIMULATION,         // Physics, thermal, electrical
    ADVANCED_ENGINEERING,
    ADVANCED_AI,
    FUTURE
};

struct SkillDefinition {
    std::string id;
    std::string name;
    std::string description;
    SkillCategory category;
    int max_level = 5;
    std::vector<std::string> prerequisite_skills;
    std::vector<std::string> unlocks_skills;
    float xp_per_level = 100.0f;
};

struct SkillCapability {
    std::string skill_id;
    int current_level;
    float confidence;
    std::vector<std::string> unlocked_capabilities;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SKILL SYSTEM
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class SkillSystem {
public:
    static SkillSystem& GetInstance();

    // Skill Registration
    void RegisterSkill(const SkillDefinition& skill);
    void RegisterSkillTree(const std::vector<SkillDefinition>& skills);

    // Skill Progression
    void AwardXP(const std::string& skill_id, float amount);
    bool TryLevelUp(const std::string& skill_id);
    void UnlockCapability(const std::string& skill_id, const std::string& capability);

    // Skill Queries
    SkillCapability GetSkillCapability(const std::string& skill_id);
    std::vector<SkillCapability> GetAllCapabilities();
    std::vector<std::string> GetUnlockedCapabilities();
    bool IsSkillUnlocked(const std::string& skill_id);
    bool CanLearnSkill(const std::string& skill_id);

    // Skill Graph
    json GetSkillGraph();
    json GetSkillProgressionPath(const std::string& target_skill);

    // Statistics
    float GetOverallProgress();
    json GetSkillStatistics();

    // Initialization
    void InitializeFoundationSkills();
    void InitializeEngineeringSkills();
    void InitializeSimulationSkills();

private:
    SkillSystem() = default;

    std::map<std::string, SkillDefinition> skill_definitions;
    std::map<std::string, SkillCapability> skill_capabilities;
    std::map<std::string, float> skill_xp;
};

} // namespace SZM::AI::Legendary
