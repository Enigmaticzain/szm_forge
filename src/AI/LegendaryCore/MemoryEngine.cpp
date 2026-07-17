#include "LegendaryCore/MemoryEngine.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <ctime>

namespace SZM::AI::Legendary {

MemoryEngine& MemoryEngine::GetInstance() {
    static MemoryEngine instance;
    return instance;
}

void MemoryEngine::StoreSessionMemory(const std::string& key, const json& value) {
    session_memory[key] = value;
}

json MemoryEngine::RetrieveSessionMemory(const std::string& key) {
    auto it = session_memory.find(key);
    return it != session_memory.end() ? it->second : json::object();
}

void MemoryEngine::ClearSessionMemory() {
    session_memory.clear();
}

void MemoryEngine::StoreLongTermMemory(const MemoryEntry& entry) {
    long_term_memory.push_back(entry);
}

std::vector<MemoryEntry> MemoryEngine::SearchLongTermMemory(const std::string& query, int limit) {
    std::vector<MemoryEntry> results;
    for (const auto& entry : long_term_memory) {
        if (entry.content.find(query) != std::string::npos ||
            std::any_of(entry.tags.begin(), entry.tags.end(),
                       [&query](const std::string& tag) { return tag.find(query) != std::string::npos; })) {
            results.push_back(entry);
            if (results.size() >= limit) break;
        }
    }
    return results;
}

MemoryEntry MemoryEngine::GetMemoryById(const std::string& id) {
    auto it = std::find_if(long_term_memory.begin(), long_term_memory.end(),
                          [&id](const MemoryEntry& e) { return e.id == id; });
    return it != long_term_memory.end() ? *it : MemoryEntry{};
}

void MemoryEngine::CreateSemanticRelation(const SemanticRelation& relation) {
    semantic_graph.push_back(relation);
}

std::vector<std::string> MemoryEngine::GetRelatedConcepts(const std::string& concept_id) {
    std::vector<std::string> related;
    for (const auto& rel : semantic_graph) {
        if (rel.source_id == concept_id) {
            related.push_back(rel.target_id);
        } else if (rel.target_id == concept_id) {
            related.push_back(rel.source_id);
        }
    }
    return related;
}

std::vector<SemanticRelation> MemoryEngine::GetConceptGraph() {
    return semantic_graph;
}

void MemoryEngine::RegisterSkill(const SkillEntry& skill) {
    skills[skill.name] = skill;
}

SkillEntry MemoryEngine::GetSkill(const std::string& skill_name) {
    auto it = skills.find(skill_name);
    return it != skills.end() ? it->second : SkillEntry{};
}

void MemoryEngine::UpdateSkillXP(const std::string& skill_name, float xp_gain) {
    auto it = skills.find(skill_name);
    if (it != skills.end()) {
        it->second.xp += xp_gain;
        it->second.improvement_history.push_back(it->second.xp);
    }
}

void MemoryEngine::LevelUpSkill(const std::string& skill_name) {
    auto it = skills.find(skill_name);
    if (it != skills.end() && it->second.level < 5) {
        it->second.level++;
        it->second.xp = 0.0f;
    }
}

std::vector<SkillEntry> MemoryEngine::GetAllSkills() {
    std::vector<SkillEntry> result;
    for (const auto& [name, skill] : skills) {
        result.push_back(skill);
    }
    return result;
}

std::vector<SkillEntry> MemoryEngine::GetSkillsByLevel(int level) {
    std::vector<SkillEntry> result;
    for (const auto& [name, skill] : skills) {
        if (skill.level == level) {
            result.push_back(skill);
        }
    }
    return result;
}

void MemoryEngine::RecordFailure(const FailureRecord& failure) {
    failure_records.push_back(failure);
}

std::vector<FailureRecord> MemoryEngine::GetFailureHistory() {
    return failure_records;
}

std::vector<FailureRecord> MemoryEngine::GetFailuresBySkill(const std::string& skill_name) {
    std::vector<FailureRecord> result;
    for (const auto& failure : failure_records) {
        auto it = std::find(failure.related_skills.begin(), failure.related_skills.end(), skill_name);
        if (it != failure.related_skills.end()) {
            result.push_back(failure);
        }
    }
    return result;
}

void MemoryEngine::RecordEvolution(const std::string& stage, const json& data) {
    json evolution_entry = json::object();
    evolution_entry["stage"] = stage;
    evolution_entry["timestamp"] = std::time(nullptr);
    evolution_entry["data"] = data;
    evolution_history.push_back(evolution_entry);
}

json MemoryEngine::GetEvolutionHistory() {
    return json(evolution_history);
}

void MemoryEngine::SaveToFile(const std::string& filepath) {
    json memory_data = json::object();
    memory_data["session_memory"] = session_memory;
    memory_data["evolution_history"] = evolution_history;

    std::ofstream file(filepath);
    file << memory_data.dump(2);
}

void MemoryEngine::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    json memory_data;
    file >> memory_data;

    if (memory_data.contains("session_memory")) {
        session_memory = memory_data["session_memory"].get<std::map<std::string, json>>();
    }
    if (memory_data.contains("evolution_history")) {
        evolution_history = memory_data["evolution_history"].get<std::vector<json>>();
    }
}

json MemoryEngine::GetMemoryStatistics() {
    json stats = json::object();
    stats["total_memories"] = long_term_memory.size();
    stats["total_skills"] = skills.size();
    stats["total_failures"] = failure_records.size();
    stats["semantic_relations"] = semantic_graph.size();
    stats["average_skill_level"] = GetAverageSkillLevel();
    return stats;
}

float MemoryEngine::GetAverageSkillLevel() {
    if (skills.empty()) return 0.0f;
    float total = 0.0f;
    for (const auto& [name, skill] : skills) {
        total += skill.level;
    }
    return total / skills.size();
}

int MemoryEngine::GetTotalMemoryEntries() {
    return long_term_memory.size();
}

} // namespace SZM::AI::Legendary
