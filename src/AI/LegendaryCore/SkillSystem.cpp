#include "LegendaryCore/SkillSystem.hpp"
#include <algorithm>

namespace SZM::AI::Legendary {

SkillSystem& SkillSystem::GetInstance() {
    static SkillSystem instance;
    return instance;
}

void SkillSystem::RegisterSkill(const SkillDefinition& skill) {
    skill_definitions[skill.id] = skill;
    SkillCapability cap;
    cap.skill_id = skill.id;
    cap.current_level = 1;
    cap.confidence = 0.0f;
    skill_capabilities[skill.id] = cap;
    skill_xp[skill.id] = 0.0f;
}

void SkillSystem::RegisterSkillTree(const std::vector<SkillDefinition>& skills) {
    for (const auto& skill : skills) {
        RegisterSkill(skill);
    }
}

void SkillSystem::AwardXP(const std::string& skill_id, float amount) {
    auto it = skill_xp.find(skill_id);
    if (it != skill_xp.end()) {
        it->second += amount;
    }
}

bool SkillSystem::TryLevelUp(const std::string& skill_id) {
    auto xp_it = skill_xp.find(skill_id);
    auto cap_it = skill_capabilities.find(skill_id);
    auto def_it = skill_definitions.find(skill_id);

    if (xp_it == skill_xp.end() || cap_it == skill_capabilities.end() || def_it == skill_definitions.end()) {
        return false;
    }

    float xp_needed = def_it->second.xp_per_level;
    if (xp_it->second >= xp_needed && cap_it->second.current_level < def_it->second.max_level) {
        cap_it->second.current_level++;
        xp_it->second -= xp_needed;
        cap_it->second.confidence = std::min(1.0f, cap_it->second.confidence + 0.1f);
        return true;
    }
    return false;
}

void SkillSystem::UnlockCapability(const std::string& skill_id, const std::string& capability) {
    auto it = skill_capabilities.find(skill_id);
    if (it != skill_capabilities.end()) {
        it->second.unlocked_capabilities.push_back(capability);
    }
}

SkillCapability SkillSystem::GetSkillCapability(const std::string& skill_id) {
    auto it = skill_capabilities.find(skill_id);
    return it != skill_capabilities.end() ? it->second : SkillCapability{};
}

std::vector<SkillCapability> SkillSystem::GetAllCapabilities() {
    std::vector<SkillCapability> result;
    for (const auto& [id, cap] : skill_capabilities) {
        result.push_back(cap);
    }
    return result;
}

std::vector<std::string> SkillSystem::GetUnlockedCapabilities() {
    std::vector<std::string> result;
    for (const auto& [id, cap] : skill_capabilities) {
        for (const auto& capability : cap.unlocked_capabilities) {
            result.push_back(capability);
        }
    }
    return result;
}

bool SkillSystem::IsSkillUnlocked(const std::string& skill_id) {
    auto it = skill_capabilities.find(skill_id);
    return it != skill_capabilities.end() && it->second.current_level > 0;
}

bool SkillSystem::CanLearnSkill(const std::string& skill_id) {
    auto def_it = skill_definitions.find(skill_id);
    if (def_it == skill_definitions.end()) return false;

    for (const auto& prereq : def_it->second.prerequisite_skills) {
        if (!IsSkillUnlocked(prereq)) return false;
    }
    return true;
}

json SkillSystem::GetSkillGraph() {
    json graph;
    for (const auto& [id, def] : skill_definitions) {
        json skill_node;
        skill_node["id"] = id;
        skill_node["name"] = def.name;
        skill_node["category"] = static_cast<int>(def.category);
        skill_node["prerequisites"] = def.prerequisite_skills;
        skill_node["unlocks"] = def.unlocks_skills;

        auto cap_it = skill_capabilities.find(id);
        if (cap_it != skill_capabilities.end()) {
            skill_node["level"] = cap_it->second.current_level;
            skill_node["confidence"] = cap_it->second.confidence;
        }

        graph[id] = skill_node;
    }
    return graph;
}

json SkillSystem::GetSkillProgressionPath(const std::string& target_skill) {
    json path;
    path["target"] = target_skill;
    path["required_prerequisites"] = json::array();

    auto def_it = skill_definitions.find(target_skill);
    if (def_it != skill_definitions.end()) {
        for (const auto& prereq : def_it->second.prerequisite_skills) {
            path["required_prerequisites"].push_back(prereq);
        }
    }

    return path;
}

float SkillSystem::GetOverallProgress() {
    if (skill_capabilities.empty()) return 0.0f;
    float total = 0.0f;
    for (const auto& [id, cap] : skill_capabilities) {
        total += cap.current_level;
    }
    return total / (skill_capabilities.size() * 5.0f);
}

json SkillSystem::GetSkillStatistics() {
    json stats;
    stats["total_skills"] = skill_definitions.size();
    stats["unlocked_skills"] = 0;
    stats["average_level"] = 0.0f;
    stats["overall_progress"] = GetOverallProgress();

    int unlocked = 0;
    float total_level = 0.0f;
    for (const auto& [id, cap] : skill_capabilities) {
        if (cap.current_level > 0) unlocked++;
        total_level += cap.current_level;
    }

    stats["unlocked_skills"] = unlocked;
    if (!skill_capabilities.empty()) {
        stats["average_level"] = total_level / skill_capabilities.size();
    }

    return stats;
}

void SkillSystem::InitializeFoundationSkills() {
    std::vector<SkillDefinition> foundation_skills = {
        {"text_understanding", "Text Understanding", "Understand natural language", SkillCategory::FOUNDATION, 5, {}, {"code_understanding"}},
        {"code_understanding", "Code Understanding", "Parse and understand code", SkillCategory::FOUNDATION, 5, {"text_understanding"}, {"syntax_parsing"}},
        {"syntax_parsing", "Syntax Parsing", "Parse programming syntax", SkillCategory::FOUNDATION, 5, {"code_understanding"}, {"logic_reasoning"}},
        {"logic_reasoning", "Logic Reasoning", "Reason about logic", SkillCategory::FOUNDATION, 5, {"syntax_parsing"}, {"semantic_search"}},
        {"semantic_search", "Semantic Search", "Search semantically", SkillCategory::FOUNDATION, 5, {"logic_reasoning"}, {}},
    };
    RegisterSkillTree(foundation_skills);
}

void SkillSystem::InitializeEngineeringSkills() {
    std::vector<SkillDefinition> engineering_skills = {
        {"debugging", "Debugging", "Debug code issues", SkillCategory::ENGINEERING, 5, {"code_understanding"}, {"optimization"}},
        {"optimization", "Optimization", "Optimize code", SkillCategory::ENGINEERING, 5, {"debugging"}, {"refactoring"}},
        {"refactoring", "Refactoring", "Refactor code", SkillCategory::ENGINEERING, 5, {"optimization"}, {}},
    };
    RegisterSkillTree(engineering_skills);
}

void SkillSystem::InitializeSimulationSkills() {
    std::vector<SkillDefinition> simulation_skills = {
        {"physics_understanding", "Physics Understanding", "Understand physics", SkillCategory::SIMULATION, 5, {"logic_reasoning"}, {}},
        {"thermal_systems", "Thermal Systems", "Understand thermal systems", SkillCategory::SIMULATION, 5, {"physics_understanding"}, {}},
        {"mechanical_systems", "Mechanical Systems", "Understand mechanical systems", SkillCategory::SIMULATION, 5, {"physics_understanding"}, {}},
    };
    RegisterSkillTree(simulation_skills);
}

} // namespace SZM::AI::Legendary
