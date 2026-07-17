#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace SZM::AI::Legendary {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// TRAINING TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class TrainingMode {
    PASSIVE,            // Observe and store
    SUPERVISED,         // Creator teaches
    REPOSITORY,         // Study code
    ENGINEERING,        // Study architecture
    SIMULATION,         // Study physics/mechanics
    REFLECTION,         // Analyze past actions
    FAILURE             // Learn from mistakes
};

enum class DataSourceType {
    GITHUB_REPO,
    LOCAL_REPO,
    ZIP_PROJECT,
    PDF_DOCUMENT,
    MARKDOWN_DOC,
    ARCHITECTURE_DIAGRAM,
    SOURCE_CODE,
    RESEARCH_PAPER,
    ENGINEERING_SPEC
};

struct TrainingData {
    std::string id;
    DataSourceType source_type;
    std::string source_path;
    std::string content;
    std::vector<std::string> extracted_concepts;
    std::vector<std::string> extracted_patterns;
    std::vector<std::string> related_skills;
    float relevance_score = 0.0f;
};

struct TrainingSession {
    std::string id;
    TrainingMode mode;
    std::vector<TrainingData> training_data;
    std::vector<std::string> skills_trained;
    float progress = 0.0f;
    bool completed = false;
    json results;
};

struct ConceptExtraction {
    std::string concept_name;
    std::string category;
    std::vector<std::string> related_concepts;
    std::string description;
    float confidence = 0.0f;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// TRAINING PIPELINE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class TrainingPipeline {
public:
    static TrainingPipeline& GetInstance();

    // Data Import
    void ImportRepository(const std::string& repo_path);
    void ImportDocument(const std::string& doc_path, DataSourceType type);
    void ImportZipProject(const std::string& zip_path);

    // Training Execution
    TrainingSession StartTrainingSession(TrainingMode mode);
    void ProcessTrainingData(TrainingSession& session);
    void CompleteTrainingSession(TrainingSession& session);

    // Concept Extraction
    std::vector<ConceptExtraction> ExtractConcepts(const std::string& content);
    std::vector<std::string> ExtractPatterns(const std::string& content);
    std::vector<std::string> ExtractArchitectureInsights(const std::string& code);

    // Knowledge Integration
    void IntegrateKnowledge(const TrainingData& data);
    void UpdateSkillsFromTraining(const std::vector<std::string>& concepts);

    // Training History
    std::vector<TrainingSession> GetTrainingHistory();
    json GetTrainingStatistics();

    // Passive Learning
    void ObserveAction(const std::string& action, const json& context);
    void ObserveFailure(const std::string& failure_description);
    void ObserveSuccess(const std::string& success_description);

private:
    TrainingPipeline() = default;

    std::vector<TrainingSession> training_history;
    std::vector<TrainingData> imported_data;
};

} // namespace SZM::AI::Legendary
