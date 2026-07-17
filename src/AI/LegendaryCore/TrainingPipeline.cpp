#include "LegendaryCore/TrainingPipeline.hpp"
#include <algorithm>

namespace SZM::AI::Legendary {

TrainingPipeline& TrainingPipeline::GetInstance() {
    static TrainingPipeline instance;
    return instance;
}

void TrainingPipeline::ImportRepository(const std::string& repo_path) {
    TrainingData data;
    data.id = "repo_" + std::to_string(imported_data.size());
    data.source_type = DataSourceType::GITHUB_REPO;
    data.source_path = repo_path;
    imported_data.push_back(data);
}

void TrainingPipeline::ImportDocument(const std::string& doc_path, DataSourceType type) {
    TrainingData data;
    data.id = "doc_" + std::to_string(imported_data.size());
    data.source_type = type;
    data.source_path = doc_path;
    imported_data.push_back(data);
}

void TrainingPipeline::ImportZipProject(const std::string& zip_path) {
    TrainingData data;
    data.id = "zip_" + std::to_string(imported_data.size());
    data.source_type = DataSourceType::ZIP_PROJECT;
    data.source_path = zip_path;
    imported_data.push_back(data);
}

TrainingSession TrainingPipeline::StartTrainingSession(TrainingMode mode) {
    TrainingSession session;
    session.id = "session_" + std::to_string(training_history.size());
    session.mode = mode;
    session.progress = 0.0f;
    session.completed = false;
    return session;
}

void TrainingPipeline::ProcessTrainingData(TrainingSession& session) {
    for (auto& data : session.training_data) {
        auto concepts = ExtractConcepts(data.content);
        for (const auto& c : concepts) {
            data.extracted_concepts.push_back(c.concept_name);
        }

        auto patterns = ExtractPatterns(data.content);
        data.extracted_patterns = patterns;

        session.progress += 0.1f;
    }
}

void TrainingPipeline::CompleteTrainingSession(TrainingSession& session) {
    session.completed = true;
    session.progress = 1.0f;
    training_history.push_back(session);
}

std::vector<ConceptExtraction> TrainingPipeline::ExtractConcepts(const std::string& content) {
    std::vector<ConceptExtraction> concepts;

    std::vector<std::string> keywords = {
        "architecture", "design", "pattern", "algorithm", "optimization",
        "physics", "simulation", "rendering", "memory", "performance"
    };

    for (const auto& keyword : keywords) {
        if (content.find(keyword) != std::string::npos) {
            ConceptExtraction c;
            c.concept_name = keyword;
            c.category = "engineering";
            c.confidence = 0.7f;
            concepts.push_back(c);
        }
    }

    return concepts;
}

std::vector<std::string> TrainingPipeline::ExtractPatterns(const std::string& content) {
    std::vector<std::string> patterns;

    std::vector<std::string> pattern_keywords = {
        "singleton", "factory", "observer", "strategy", "decorator",
        "adapter", "bridge", "composite", "facade", "proxy"
    };

    for (const auto& pattern : pattern_keywords) {
        if (content.find(pattern) != std::string::npos) {
            patterns.push_back(pattern);
        }
    }

    return patterns;
}

std::vector<std::string> TrainingPipeline::ExtractArchitectureInsights(const std::string& code) {
    std::vector<std::string> insights;

    if (code.find("class") != std::string::npos) {
        insights.push_back("object_oriented_design");
    }
    if (code.find("template") != std::string::npos) {
        insights.push_back("generic_programming");
    }
    if (code.find("async") != std::string::npos || code.find("thread") != std::string::npos) {
        insights.push_back("concurrent_design");
    }
    if (code.find("virtual") != std::string::npos) {
        insights.push_back("polymorphism");
    }

    return insights;
}

void TrainingPipeline::IntegrateKnowledge(const TrainingData& data) {
    // Store in memory engine
    // Update skill graph
    // Create semantic relations
}

void TrainingPipeline::UpdateSkillsFromTraining(const std::vector<std::string>& concepts) {
    // Award XP to relevant skills based on concepts
}

std::vector<TrainingSession> TrainingPipeline::GetTrainingHistory() {
    return training_history;
}

json TrainingPipeline::GetTrainingStatistics() {
    json stats;
    stats["total_sessions"] = training_history.size();
    stats["total_data_imported"] = imported_data.size();
    stats["completed_sessions"] = 0;

    int completed = 0;
    for (const auto& session : training_history) {
        if (session.completed) completed++;
    }
    stats["completed_sessions"] = completed;

    return stats;
}

void TrainingPipeline::ObserveAction(const std::string& action, const json& context) {
    // Passive learning: store action and context
}

void TrainingPipeline::ObserveFailure(const std::string& failure_description) {
    // Learn from failures
}

void TrainingPipeline::ObserveSuccess(const std::string& success_description) {
    // Learn from successes
}

} // namespace SZM::AI::Legendary
