#include "LegendaryCore/TrainingPipeline.hpp"
#include "LegendaryCore/MemoryEngine.hpp"
#include "LegendaryCore/SkillSystem.hpp"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace SZM::AI::Legendary {

namespace {

std::string ReadFileBounded(const std::filesystem::path& path, size_t maxBytes) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return {};
    }
    std::string buffer(maxBytes, '\0');
    in.read(buffer.data(), static_cast<std::streamsize>(maxBytes));
    buffer.resize(static_cast<size_t>(in.gcount()));
    return buffer;
}

bool IsTextTrainingExtension(const std::filesystem::path& ext) {
    std::string e = ext.string();
    std::transform(e.begin(), e.end(), e.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return e == ".hpp" || e == ".h" || e == ".cpp" || e == ".cc" ||
           e == ".py" || e == ".md" || e == ".txt" || e == ".json";
}

TrainingData LoadTrainingSource(const std::string& prefix,
                                DataSourceType type,
                                const std::string& sourcePath,
                                size_t index) {
    TrainingData data;
    data.id = prefix + "_" + std::to_string(index);
    data.source_type = type;
    data.source_path = sourcePath;

    std::error_code ec;
    const std::filesystem::path fsPath(sourcePath);
    if (std::filesystem::is_regular_file(fsPath, ec)) {
        data.content = ReadFileBounded(fsPath, 131072);
    } else if (std::filesystem::is_directory(fsPath, ec)) {
        std::ostringstream combined;
        size_t totalBytes = 0;
        size_t fileCount = 0;
        constexpr size_t kMaxTotalBytes = 262144;
        constexpr size_t kMaxFiles = 24;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 fsPath, std::filesystem::directory_options::skip_permission_denied, ec)) {
            if (ec || fileCount >= kMaxFiles || totalBytes >= kMaxTotalBytes) {
                break;
            }
            if (!entry.is_regular_file(ec) || !IsTextTrainingExtension(entry.path().extension())) {
                continue;
            }
            std::string chunk = ReadFileBounded(entry.path(), 16384);
            if (!chunk.empty()) {
                combined << "// FILE: " << entry.path().filename().string() << "\n" << chunk << "\n";
                totalBytes += chunk.size();
                fileCount++;
            }
        }
        data.content = combined.str();
    }

    if (data.content.empty()) {
        data.content = sourcePath;
    }
    return data;
}

std::string ToLowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

} // namespace

TrainingPipeline& TrainingPipeline::GetInstance() {
    static TrainingPipeline instance;
    return instance;
}

void TrainingPipeline::ImportRepository(const std::string& repo_path) {
    imported_data.push_back(LoadTrainingSource("repo", DataSourceType::GITHUB_REPO, repo_path, imported_data.size()));
}

void TrainingPipeline::ImportDocument(const std::string& doc_path, DataSourceType type) {
    imported_data.push_back(LoadTrainingSource("doc", type, doc_path, imported_data.size()));
}

void TrainingPipeline::ImportZipProject(const std::string& zip_path) {
    imported_data.push_back(LoadTrainingSource("zip", DataSourceType::ZIP_PROJECT, zip_path, imported_data.size()));
}

TrainingSession TrainingPipeline::StartTrainingSession(TrainingMode mode) {
    TrainingSession session;
    session.id = "session_" + std::to_string(training_history.size());
    session.mode = mode;
    session.training_data = imported_data;
    session.progress = 0.0f;
    session.completed = false;
    return session;
}

void TrainingPipeline::ProcessTrainingData(TrainingSession& session) {
    if (session.training_data.empty()) {
        session.progress = 1.0f;
        return;
    }

    const size_t total = session.training_data.size();
    for (size_t i = 0; i < total; ++i) {
        auto& data = session.training_data[i];
        auto concepts = ExtractConcepts(data.content);
        data.extracted_concepts.clear();
        for (const auto& c : concepts) {
            data.extracted_concepts.push_back(c.concept_name);
        }

        data.extracted_patterns = ExtractPatterns(data.content);
        IntegrateKnowledge(data);
        UpdateSkillsFromTraining(data.extracted_concepts);

        session.progress = static_cast<float>(i + 1) / static_cast<float>(total);
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
    auto& memory = MemoryEngine::GetInstance();
    MemoryEntry entry;
    entry.id = "training_" + data.id;
    entry.type = "training_data";
    entry.content = data.content.size() > 4096 ? data.content.substr(0, 4096) : data.content;
    entry.tags = data.extracted_concepts;
    entry.relevance_score = 5 + static_cast<int>(data.extracted_concepts.size());
    entry.created_at = std::chrono::system_clock::now();
    entry.last_accessed = entry.created_at;
    memory.StoreLongTermMemory(entry);

    for (size_t i = 0; i < data.extracted_concepts.size(); ++i) {
        SemanticRelation rel;
        rel.source_id = entry.id;
        rel.target_id = data.extracted_concepts[i];
        rel.relation_type = "teaches";
        rel.strength = 0.8f;
        memory.CreateSemanticRelation(rel);

        if (i > 0) {
            SemanticRelation chain;
            chain.source_id = data.extracted_concepts[i - 1];
            chain.target_id = data.extracted_concepts[i];
            chain.relation_type = "related_to";
            chain.strength = 0.6f;
            memory.CreateSemanticRelation(chain);
        }
    }
}

void TrainingPipeline::UpdateSkillsFromTraining(const std::vector<std::string>& concepts) {
    if (concepts.empty()) {
        return;
    }

    auto& skillSystem = SkillSystem::GetInstance();
    const auto capabilities = skillSystem.GetAllCapabilities();
    for (const auto& conceptName : concepts) {
        const std::string lowerConcept = ToLowerCopy(conceptName);
        bool matchedAny = false;
        for (const auto& cap : capabilities) {
            const std::string lowerSkill = ToLowerCopy(cap.skill_id);
            if (lowerSkill.find(lowerConcept) != std::string::npos ||
                lowerConcept.find(lowerSkill) != std::string::npos) {
                skillSystem.AwardXP(cap.skill_id, 15.0f);
                skillSystem.TryLevelUp(cap.skill_id);
                matchedAny = true;
            }
        }
        if (!matchedAny) {
            for (const auto& cap : capabilities) {
                if (cap.skill_id.find("code") != std::string::npos ||
                    cap.skill_id.find("foundation") != std::string::npos ||
                    cap.skill_id.find("engineering") != std::string::npos) {
                    skillSystem.AwardXP(cap.skill_id, 5.0f);
                    skillSystem.TryLevelUp(cap.skill_id);
                }
            }
        }
    }
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
