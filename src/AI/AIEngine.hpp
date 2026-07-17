#pragma once

#include "AIDataTypes.hpp"
#include <vector>
#include <memory>

namespace SZM::AI {

/**
 * @class AIEngine
 * @brief AI assistant for design optimization and failure prediction
 *
 * Features:
 *  - Design analysis and recommendations
 *  - Failure prediction and risk assessment
 *  - Multi-objective optimization (cost, weight, reliability)
 *  - Sensitivity analysis
 *  - Explainable AI (reasoning for recommendations)
 */
class AIEngine {
public:
    static AIEngine& GetInstance() {
        static AIEngine instance;
        return instance;
    }

    AIEngine(const AIEngine&) = delete;
    AIEngine& operator=(const AIEngine&) = delete;

    // Lifecycle
    void Init();
    void Shutdown();

    // Training
    void TrainOnHistoricalData(const std::vector<DesignMetrics>& data);
    void UpdateFailureDatabase(const std::vector<FailurePrediction>& failures);

    // Analysis
    std::vector<DesignRecommendation> AnalyzeDesign(const DesignMetrics& metrics);
    FailurePrediction PredictFailure(uint32_t componentId, const DesignMetrics& metrics);
    ReliabilityAnalysis AnalyzeReliability(const DesignMetrics& metrics);

    // Optimization
    OptimizationResult OptimizeForCost(const DesignMetrics& current);
    OptimizationResult OptimizeForReliability(const DesignMetrics& current);
    OptimizationResult OptimizeForWeight(const DesignMetrics& current);
    OptimizationResult OptimizeMultiObjective(const DesignMetrics& current);

    // Explanations
    std::string ExplainRecommendation(const DesignRecommendation& rec);
    std::string ExplainFailurePrediction(const FailurePrediction& pred);
    std::string ExplainReliability(const ReliabilityAnalysis& analysis);

    // Accessors
    float GetReliabilityScore(const DesignMetrics& metrics) const;
    float GetFailureProbability(const DesignMetrics& metrics) const;

private:
    AIEngine() = default;
    ~AIEngine() = default;

    float CalculateStressMargin(const DesignMetrics& metrics) const;
    float CalculateThermalMargin(const DesignMetrics& metrics) const;
    float CalculateReliabilityScore(const DesignMetrics& metrics) const;
    std::vector<DesignRecommendation> GenerateRecommendations(const DesignMetrics& metrics);

private:
    std::vector<DesignMetrics> m_HistoricalData;
    std::vector<FailurePrediction> m_FailureDatabase;
};

} // namespace SZM::AI
