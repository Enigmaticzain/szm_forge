#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>

namespace SZM::AI::Legendary {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MEMORY TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct MemoryEntry {
    std::string id;
    std::string type;           // "concept", "pattern", "solution", "failure"
    std::string content;
    std::vector<std::string> tags;
    std::vector<float> embedding;
    int relevance_score = 0;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_accessed;
    int access_count = 0;
};

struct SemanticRelation {
    std::string source_id;
    std::string target_id;
    std::string relation_type;  // "depends_on", "related_to", "contradicts", "improves"
    float strength = 0.5f;
};

struct SkillEntry {
    std::string name;
    int level = 1;              // 1-5
    float confidence = 0.0f;    // 0.0-1.0
    float xp = 0.0f;
    std::vector<std::string> dependency_skills;
    std::vector<std::string> related_skills;
    int last_used_timestamp = 0;
    std::vector<float> improvement_history;
};

struct FailureRecord {
    std::string id;
    std::string description;
    std::string root_cause;
    std::string context;
    std::vector<std::string> related_skills;
    int severity = 0;          // 0-10
    std::chrono::system_clock::time_point timestamp;
    bool resolved = false;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MEMORY ENGINE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class MemoryEngine {
public:
    static MemoryEngine& GetInstance();

    // Session Memory
    void StoreSessionMemory(const std::string& key, const json& value);
    json RetrieveSessionMemory(const std::string& key);
    void ClearSessionMemory();

    // Long-term Memory
    void StoreLongTermMemory(const MemoryEntry& entry);
    std::vector<MemoryEntry> SearchLongTermMemory(const std::string& query, int limit = 10);
    MemoryEntry GetMemoryById(const std::string& id);

    // Semantic Memory
    void CreateSemanticRelation(const SemanticRelation& relation);
    std::vector<std::string> GetRelatedConcepts(const std::string& concept_id);
    std::vector<SemanticRelation> GetConceptGraph();

    // Skill Memory
    void RegisterSkill(const SkillEntry& skill);
    SkillEntry GetSkill(const std::string& skill_name);
    void UpdateSkillXP(const std::string& skill_name, float xp_gain);
    void LevelUpSkill(const std::string& skill_name);
    std::vector<SkillEntry> GetAllSkills();
    std::vector<SkillEntry> GetSkillsByLevel(int level);

    // Failure Memory
    void RecordFailure(const FailureRecord& failure);
    std::vector<FailureRecord> GetFailureHistory();
    std::vector<FailureRecord> GetFailuresBySkill(const std::string& skill_name);

    // Evolution Memory
    void RecordEvolution(const std::string& stage, const json& data);
    json GetEvolutionHistory();

    // Persistence
    void SaveToFile(const std::string& filepath);
    void LoadFromFile(const std::string& filepath);

    // Statistics
    json GetMemoryStatistics();
    float GetAverageSkillLevel();
    int GetTotalMemoryEntries();

private:
    MemoryEngine() = default;

    std::map<std::string, json> session_memory;
    std::vector<MemoryEntry> long_term_memory;
    std::vector<SemanticRelation> semantic_graph;
    std::map<std::string, SkillEntry> skills;
    std::vector<FailureRecord> failure_records;
    std::vector<json> evolution_history;
};

} // namespace SZM::AI::Legendary
