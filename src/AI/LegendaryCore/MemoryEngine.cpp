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

    json ltm = json::array();
    for (const auto& entry : long_term_memory) {
        ltm.push_back({
            {"id", entry.id},
            {"type", entry.type},
            {"content", entry.content},
            {"tags", entry.tags},
            {"embedding", entry.embedding},
            {"relevance_score", entry.relevance_score},
            {"created_at", std::chrono::duration_cast<std::chrono::seconds>(
                               entry.created_at.time_since_epoch()).count()},
            {"last_accessed", std::chrono::duration_cast<std::chrono::seconds>(
                                  entry.last_accessed.time_since_epoch()).count()},
            {"access_count", entry.access_count}
        });
    }
    memory_data["long_term_memory"] = std::move(ltm);

    json graph = json::array();
    for (const auto& rel : semantic_graph) {
        graph.push_back({
            {"source_id", rel.source_id},
            {"target_id", rel.target_id},
            {"relation_type", rel.relation_type},
            {"strength", rel.strength}
        });
    }
    memory_data["semantic_graph"] = std::move(graph);

    json skillMap = json::object();
    for (const auto& [name, s] : skills) {
        skillMap[name] = {
            {"name", s.name},
            {"level", s.level},
            {"confidence", s.confidence},
            {"xp", s.xp},
            {"dependency_skills", s.dependency_skills},
            {"related_skills", s.related_skills},
            {"last_used_timestamp", s.last_used_timestamp},
            {"improvement_history", s.improvement_history}
        };
    }
    memory_data["skills"] = std::move(skillMap);

    json failures = json::array();
    for (const auto& f : failure_records) {
        failures.push_back({
            {"id", f.id},
            {"description", f.description},
            {"root_cause", f.root_cause},
            {"context", f.context},
            {"related_skills", f.related_skills},
            {"severity", f.severity},
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(
                              f.timestamp.time_since_epoch()).count()},
            {"resolved", f.resolved}
        });
    }
    memory_data["failure_records"] = std::move(failures);

    std::ofstream file(filepath);
    if (file.is_open()) {
        file << memory_data.dump(2);
    }
}

void MemoryEngine::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;

    json memory_data;
    try {
        file >> memory_data;
    } catch (...) {
        return;
    }
    if (!memory_data.is_object()) return;

    if (memory_data.contains("session_memory") && memory_data["session_memory"].is_object()) {
        session_memory = memory_data["session_memory"].get<std::map<std::string, json>>();
    }
    if (memory_data.contains("evolution_history") && memory_data["evolution_history"].is_array()) {
        evolution_history = memory_data["evolution_history"].get<std::vector<json>>();
    }
    if (memory_data.contains("long_term_memory") && memory_data["long_term_memory"].is_array()) {
        long_term_memory.clear();
        for (const auto& item : memory_data["long_term_memory"]) {
            if (!item.is_object()) continue;
            MemoryEntry entry;
            entry.id = item.value("id", "");
            entry.type = item.value("type", "");
            entry.content = item.value("content", "");
            if (item.contains("tags") && item["tags"].is_array()) {
                entry.tags = item["tags"].get<std::vector<std::string>>();
            }
            if (item.contains("embedding") && item["embedding"].is_array()) {
                entry.embedding = item["embedding"].get<std::vector<float>>();
            }
            entry.relevance_score = item.value("relevance_score", 0);
            entry.created_at = std::chrono::system_clock::time_point(
                std::chrono::seconds(item.value("created_at", int64_t{0})));
            entry.last_accessed = std::chrono::system_clock::time_point(
                std::chrono::seconds(item.value("last_accessed", int64_t{0})));
            entry.access_count = item.value("access_count", 0);
            long_term_memory.push_back(std::move(entry));
        }
    }
    if (memory_data.contains("semantic_graph") && memory_data["semantic_graph"].is_array()) {
        semantic_graph.clear();
        for (const auto& item : memory_data["semantic_graph"]) {
            if (!item.is_object()) continue;
            SemanticRelation rel;
            rel.source_id = item.value("source_id", "");
            rel.target_id = item.value("target_id", "");
            rel.relation_type = item.value("relation_type", "");
            rel.strength = item.value("strength", 0.5f);
            semantic_graph.push_back(std::move(rel));
        }
    }
    if (memory_data.contains("skills") && memory_data["skills"].is_object()) {
        skills.clear();
        for (const auto& [name, item] : memory_data["skills"].items()) {
            if (!item.is_object()) continue;
            SkillEntry s;
            s.name = item.value("name", name);
            s.level = item.value("level", 1);
            s.confidence = item.value("confidence", 0.0f);
            s.xp = item.value("xp", 0.0f);
            if (item.contains("dependency_skills") && item["dependency_skills"].is_array()) {
                s.dependency_skills = item["dependency_skills"].get<std::vector<std::string>>();
            }
            if (item.contains("related_skills") && item["related_skills"].is_array()) {
                s.related_skills = item["related_skills"].get<std::vector<std::string>>();
            }
            s.last_used_timestamp = item.value("last_used_timestamp", 0);
            if (item.contains("improvement_history") && item["improvement_history"].is_array()) {
                s.improvement_history = item["improvement_history"].get<std::vector<float>>();
            }
            skills[name] = std::move(s);
        }
    }
    if (memory_data.contains("failure_records") && memory_data["failure_records"].is_array()) {
        failure_records.clear();
        for (const auto& item : memory_data["failure_records"]) {
            if (!item.is_object()) continue;
            FailureRecord f;
            f.id = item.value("id", "");
            f.description = item.value("description", "");
            f.root_cause = item.value("root_cause", "");
            f.context = item.value("context", "");
            if (item.contains("related_skills") && item["related_skills"].is_array()) {
                f.related_skills = item["related_skills"].get<std::vector<std::string>>();
            }
            f.severity = item.value("severity", 0);
            f.timestamp = std::chrono::system_clock::time_point(
                std::chrono::seconds(item.value("timestamp", int64_t{0})));
            f.resolved = item.value("resolved", false);
            failure_records.push_back(std::move(f));
        }
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
