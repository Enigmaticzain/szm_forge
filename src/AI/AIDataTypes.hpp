#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace SZM::AI {

/**
 * @struct DesignMetrics
 * @brief Metrics for design evaluation
 */
struct DesignMetrics {
    float stress = 0.0f;            ///< MPa (stress ratio 0-1)
    float temperature = 293.15f;    ///< K
    float power = 0.0f;             ///< W
    float cost = 0.0f;              ///< $ (relative)
    float weight = 0.0f;            ///< kg
    float reliability = 0.95f;      ///< 0-1 (probability of success)
};

/**
 * @struct DesignRecommendation
 * @brief Recommendation for design improvement
 */
struct DesignRecommendation {
    std::string title;
    std::string description;
    float expectedImprovement = 0.0f;   ///< % improvement
    std::vector<std::string> actions;
    float confidence = 0.8f;            ///< 0-1
};

/**
 * @struct FailurePrediction
 * @brief Prediction of component failure
 */
struct FailurePrediction {
    uint32_t componentId;
    std::string failureMode;
    float probability = 0.0f;           ///< 0-1
    float timeToFailure = 0.0f;         ///< hours
    std::vector<std::string> preventiveMeasures;
};

/**
 * @struct OptimizationResult
 * @brief Result of design optimization
 */
struct OptimizationResult {
    DesignMetrics optimizedMetrics;
    float improvementRatio = 0.0f;      ///< % improvement
    std::vector<std::string> changes;
    bool isValid = false;
};

/**
 * @struct ReliabilityAnalysis
 * @brief Reliability analysis results
 */
struct ReliabilityAnalysis {
    float stressMargin = 0.0f;          ///< % margin to yield
    float thermalMargin = 0.0f;         ///< % margin to max temp
    float reliabilityScore = 0.0f;      ///< 0-1
    float failureProbability = 0.0f;    ///< 0-1
    std::string riskLevel;              ///< "Low", "Medium", "High"
};

// Thresholds for recommendations
constexpr float STRESS_WARNING_THRESHOLD = 0.7f;    ///< 70% of yield
constexpr float TEMP_WARNING_THRESHOLD = 0.7f;      ///< 70% of max
constexpr float RELIABILITY_TARGET = 0.99f;         ///< 99% reliability
constexpr float COST_OPTIMIZATION_FACTOR = 0.9f;    ///< 10% cost reduction target
constexpr float WEIGHT_OPTIMIZATION_FACTOR = 0.85f; ///< 15% weight reduction target

} // namespace SZM::AI
