#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

namespace SZM::AI {

/**
 * @enum TrainingDataType
 * @brief Types of training data that can be imported
 */
enum class TrainingDataType {
    GITHUB_REPOSITORY,
    ZIP_PROJECT,
    PDF_DOCUMENT,
    MARKDOWN_FILE,
    CODEBASE,
    RESEARCH_PAPER,
    CAD_DOCUMENTATION,
    MECHANICAL_ENGINEERING_DOCS,
    PHYSICS_PAPERS,
    SIMULATION_DATASET,
    WIKIPEDIA_DUMP,
    CUSTOM_KNOWLEDGE
};

/**
 * @enum TrainingMode
 * @brief Different training modes for the AI
 */
enum class TrainingMode {
    PASSIVE_LEARNING,           // Learn from imported data
    REPOSITORY_LEARNING,        // Learn from codebase
    ARCHITECTURE_LEARNING,      // Learn system architecture
    CODE_UNDERSTANDING,         // Understand code patterns
    SIMULATION_LEARNING,        // Learn physics/simulation
    MECHANICAL_ENGINEERING,     // Learn mechanical concepts
    PHYSICS_LEARNING,           // Learn physics principles
    OPTIMIZATION_LEARNING       // Learn optimization techniques
};

/**
 * @struct TrainingData
 * @brief Imported training data
 */
struct TrainingData {
    uint64_t id;
    TrainingDataType type;
    std::string sourcePath;
    std::string name;
    uint64_t sizeBytes;
    uint64_t importedAt;
    std::vector<std::string> tags;
    float processingProgress;
    bool isProcessed;
};

/**
 * @struct TrainingCheckpoint
 * @brief Saved training state
 */
struct TrainingCheckpoint {
    uint64_t id;
    std::string name;
    uint64_t timestamp;
    TrainingMode mode;
    float accuracy;
    std::string modelPath;
    std::map<std::string, float> metrics;
};

/**
 * @struct TrainingConfig
 * @brief Training configuration
 */
struct TrainingConfig {
    uint32_t chunkSize = 512;
    uint32_t contextSize = 2048;
    std::string embeddingModel = "local";
    float gpuUsagePercent = 0.8f;
    float cpuUsagePercent = 0.5f;
    uint64_t vramLimitMB = 8192;
    uint64_t memoryLimitMB = 16384;
    uint32_t trainingDepth = 3;
    uint32_t retrievalK = 5;
};

/**
 * @class TrainingStudio
 * @brief AI training and knowledge management system
 */
class TrainingStudio {
public:
    static TrainingStudio& GetInstance();

    /**
     * @brief Initialize training studio
     */
    void Initialize();

    /**
     * @brief Shutdown training studio
     */
    void Shutdown();

    // ===== DATA IMPORT =====

    /**
     * @brief Import training data
     */
    uint64_t ImportTrainingData(const std::string& sourcePath, TrainingDataType type);

    /**
     * @brief Get imported data
     */
    TrainingData* GetTrainingData(uint64_t dataId);

    /**
     * @brief List all imported data
     */
    std::vector<TrainingData> ListImportedData();

    /**
     * @brief Remove training data
     */
    bool RemoveTrainingData(uint64_t dataId);

    /**
     * @brief Get import progress
     */
    float GetImportProgress(uint64_t dataId);

    // ===== TRAINING CONTROL =====

    /**
     * @brief Start training
     */
    void StartTraining(TrainingMode mode);

    /**
     * @brief Pause training
     */
    void PauseTraining();

    /**
     * @brief Resume training
     */
    void ResumeTraining();

    /**
     * @brief Stop training
     */
    void StopTraining();

    /**
     * @brief Get training progress
     */
    float GetTrainingProgress();

    /**
     * @brief Get training status
     */
    std::string GetTrainingStatus();

    /**
     * @brief Get current training mode
     */
    TrainingMode GetCurrentTrainingMode();

    // ===== CONFIGURATION =====

    /**
     * @brief Set training configuration
     */
    void SetTrainingConfig(const TrainingConfig& config);

    /**
     * @brief Get training configuration
     */
    TrainingConfig GetTrainingConfig() const;

    /**
     * @brief Update config parameter
     */
    void UpdateConfigParameter(const std::string& param, const std::string& value);

    // ===== CHECKPOINTS =====

    /**
     * @brief Save training checkpoint
     */
    uint64_t SaveCheckpoint(const std::string& name);

    /**
     * @brief Load training checkpoint
     */
    bool LoadCheckpoint(uint64_t checkpointId);

    /**
     * @brief List checkpoints
     */
    std::vector<TrainingCheckpoint> ListCheckpoints();

    /**
     * @brief Delete checkpoint
     */
    bool DeleteCheckpoint(uint64_t checkpointId);

    /**
     * @brief Export model
     */
    bool ExportModel(const std::string& outputPath);

    /**
     * @brief Import model
     */
    bool ImportModel(const std::string& modelPath);

    // ===== MONITORING =====

    /**
     * @brief Get training metrics
     */
    std::map<std::string, float> GetTrainingMetrics();

    /**
     * @brief Get resource usage
     */
    std::map<std::string, float> GetResourceUsage();

    /**
     * @brief Get training history
     */
    std::vector<std::string> GetTrainingHistory();

    // ===== KNOWLEDGE BASE =====

    /**
     * @brief Query knowledge base
     */
    std::vector<std::string> QueryKnowledgeBase(const std::string& query);

    /**
     * @brief Get knowledge statistics
     */
    std::map<std::string, uint64_t> GetKnowledgeStatistics();

    /**
     * @brief Rebuild knowledge index
     */
    void RebuildKnowledgeIndex();

private:
    TrainingStudio() = default;
    ~TrainingStudio() = default;

    TrainingStudio(const TrainingStudio&) = delete;
    TrainingStudio& operator=(const TrainingStudio&) = delete;

    std::map<uint64_t, TrainingData> m_ImportedData;
    std::map<uint64_t, TrainingCheckpoint> m_Checkpoints;
    TrainingConfig m_Config;
    TrainingMode m_CurrentMode = TrainingMode::PASSIVE_LEARNING;
    bool m_IsTraining = false;
    float m_TrainingProgress = 0.0f;
    uint64_t m_NextDataId = 1;
    uint64_t m_NextCheckpointId = 1;
};

} // namespace SZM::AI
