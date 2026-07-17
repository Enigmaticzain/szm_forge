#include "AIEngine.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace SZM::AI {

    void AIEngine::Init() {
        m_HistoricalData.clear();
        m_FailureDatabase.clear();
    }

    void AIEngine::Shutdown() {
        m_HistoricalData.clear();
        m_FailureDatabase.clear();
    }

    void AIEngine::TrainOnHistoricalData(const std::vector<DesignMetrics>& data) {
        m_HistoricalData = data;
    }

    void AIEngine::UpdateFailureDatabase(const std::vector<FailurePrediction>& failures) {
        m_FailureDatabase = failures;
    }

    float AIEngine::CalculateStressMargin(const DesignMetrics& metrics) const {
        // Margin = (1 - stress) * 100%
        return (1.0f - metrics.stress) * 100.0f;
    }

    float AIEngine::CalculateThermalMargin(const DesignMetrics& metrics) const {
        // Margin = (1 - (temp - 293.15) / (373.15 - 293.15)) * 100%
        float tempRatio = (metrics.temperature - 293.15f) / (373.15f - 293.15f);
        tempRatio = std::max(0.0f, std::min(1.0f, tempRatio));
        return (1.0f - tempRatio) * 100.0f;
    }

    float AIEngine::CalculateReliabilityScore(const DesignMetrics& metrics) const {
        // Reliability = base * stress_factor * thermal_factor
        float stressMargin = CalculateStressMargin(metrics);
        float thermalMargin = CalculateThermalMargin(metrics);

        float stressFactor = 1.0f - (metrics.stress * 0.5f);  // 50% weight
        float thermalFactor = 1.0f - ((metrics.temperature - 293.15f) / 80.0f * 0.3f);  // 30% weight
        float powerFactor = 1.0f - (metrics.power / 1000.0f * 0.2f);  // 20% weight

        float reliability = stressFactor * thermalFactor * powerFactor;
        return std::max(0.0f, std::min(1.0f, reliability));
    }

    std::vector<DesignRecommendation> AIEngine::GenerateRecommendations(const DesignMetrics& metrics) {
        std::vector<DesignRecommendation> recommendations;

        // Stress recommendation
        if (metrics.stress > STRESS_WARNING_THRESHOLD) {
            DesignRecommendation rec;
            rec.title = "Reduce Stress";
            rec.description = "Component stress is high. Consider increasing cross-sectional area or using higher-strength material.";
            rec.expectedImprovement = (metrics.stress - STRESS_WARNING_THRESHOLD) * 50.0f;
            rec.actions = {"Increase area by 20%", "Switch to higher-strength material", "Reduce applied load"};
            rec.confidence = 0.9f;
            recommendations.push_back(rec);
        }

        // Temperature recommendation
        if (metrics.temperature > 293.15f + 40.0f) {
            DesignRecommendation rec;
            rec.title = "Improve Cooling";
            rec.description = "Component temperature is elevated. Add heat dissipation or improve convection.";
            rec.expectedImprovement = 15.0f;
            rec.actions = {"Add cooling fins", "Increase convection coefficient", "Reduce heat input"};
            rec.confidence = 0.85f;
            recommendations.push_back(rec);
        }

        // Power recommendation
        if (metrics.power > 100.0f) {
            DesignRecommendation rec;
            rec.title = "Reduce Power Dissipation";
            rec.description = "High power dissipation detected. Consider efficiency improvements.";
            rec.expectedImprovement = 10.0f;
            rec.actions = {"Improve electrical efficiency", "Reduce resistance", "Optimize circuit design"};
            rec.confidence = 0.8f;
            recommendations.push_back(rec);
        }

        // Cost recommendation
        if (metrics.cost > 100.0f) {
            DesignRecommendation rec;
            rec.title = "Optimize Cost";
            rec.description = "Design cost is high. Consider material substitution or design simplification.";
            rec.expectedImprovement = 20.0f;
            rec.actions = {"Use cheaper material", "Simplify geometry", "Reduce material volume"};
            rec.confidence = 0.75f;
            recommendations.push_back(rec);
        }

        return recommendations;
    }

    std::vector<DesignRecommendation> AIEngine::AnalyzeDesign(const DesignMetrics& metrics) {
        return GenerateRecommendations(metrics);
    }

    FailurePrediction AIEngine::PredictFailure(uint32_t componentId, const DesignMetrics& metrics) {
        FailurePrediction pred;
        pred.componentId = componentId;

        // Weibull distribution for failure prediction
        // P(failure) = 1 - exp(-(t/λ)^k)
        float stressRatio = metrics.stress;
        float tempRatio = (metrics.temperature - 293.15f) / 80.0f;

        // Failure probability increases with stress and temperature
        pred.probability = stressRatio * 0.6f + tempRatio * 0.4f;
        pred.probability = std::max(0.0f, std::min(1.0f, pred.probability));

        // Time to failure (hours) - decreases with stress/temperature
        float baseLife = 10000.0f;  // hours
        float stressAcceleration = std::exp(stressRatio * 2.0f);
        float thermalAcceleration = std::exp(tempRatio * 1.5f);
        pred.timeToFailure = baseLife / (stressAcceleration * thermalAcceleration);

        // Failure modes
        if (stressRatio > 0.8f) {
            pred.failureMode = "Yield/Plastic deformation";
            pred.preventiveMeasures = {"Reduce load", "Use stronger material", "Increase cross-section"};
        } else if (tempRatio > 0.8f) {
            pred.failureMode = "Thermal degradation";
            pred.preventiveMeasures = {"Improve cooling", "Reduce heat input", "Use heat-resistant material"};
        } else {
            pred.failureMode = "Fatigue";
            pred.preventiveMeasures = {"Reduce cyclic loading", "Improve surface finish", "Add stress relief"};
        }

        return pred;
    }

    ReliabilityAnalysis AIEngine::AnalyzeReliability(const DesignMetrics& metrics) {
        ReliabilityAnalysis analysis;
        analysis.stressMargin = CalculateStressMargin(metrics);
        analysis.thermalMargin = CalculateThermalMargin(metrics);
        analysis.reliabilityScore = CalculateReliabilityScore(metrics);
        analysis.failureProbability = 1.0f - analysis.reliabilityScore;

        if (analysis.reliabilityScore > 0.95f) {
            analysis.riskLevel = "Low";
        } else if (analysis.reliabilityScore > 0.80f) {
            analysis.riskLevel = "Medium";
        } else {
            analysis.riskLevel = "High";
        }

        return analysis;
    }

    OptimizationResult AIEngine::OptimizeForCost(const DesignMetrics& current) {
        OptimizationResult result;
        result.optimizedMetrics = current;
        result.optimizedMetrics.cost *= COST_OPTIMIZATION_FACTOR;
        result.optimizedMetrics.weight *= 0.95f;  // Slight weight reduction
        result.improvementRatio = (1.0f - COST_OPTIMIZATION_FACTOR) * 100.0f;
        result.changes = {"Use cheaper material", "Reduce material volume", "Simplify design"};
        result.isValid = result.optimizedMetrics.stress < 0.9f;  // Must stay safe
        return result;
    }

    OptimizationResult AIEngine::OptimizeForReliability(const DesignMetrics& current) {
        OptimizationResult result;
        result.optimizedMetrics = current;
        result.optimizedMetrics.stress *= 0.7f;      // 30% stress reduction
        result.optimizedMetrics.temperature -= 10.0f; // 10K cooling
        result.optimizedMetrics.reliability = 0.99f;
        result.improvementRatio = 15.0f;
        result.changes = {"Increase cross-section", "Improve cooling", "Use higher-strength material"};
        result.isValid = true;
        return result;
    }

    OptimizationResult AIEngine::OptimizeForWeight(const DesignMetrics& current) {
        OptimizationResult result;
        result.optimizedMetrics = current;
        result.optimizedMetrics.weight *= WEIGHT_OPTIMIZATION_FACTOR;
        result.optimizedMetrics.stress *= 1.1f;  // Slight stress increase
        result.improvementRatio = (1.0f - WEIGHT_OPTIMIZATION_FACTOR) * 100.0f;
        result.changes = {"Use lighter material", "Optimize geometry", "Remove unnecessary features"};
        result.isValid = result.optimizedMetrics.stress < 0.85f;  // Must stay safe
        return result;
    }

    OptimizationResult AIEngine::OptimizeMultiObjective(const DesignMetrics& current) {
        // Pareto optimization: balance cost, weight, and reliability
        OptimizationResult result;
        result.optimizedMetrics = current;
        result.optimizedMetrics.cost *= 0.95f;
        result.optimizedMetrics.weight *= 0.90f;
        result.optimizedMetrics.stress *= 0.85f;
        result.improvementRatio = 10.0f;
        result.changes = {"Optimize material selection", "Improve design efficiency", "Balance all objectives"};
        result.isValid = result.optimizedMetrics.stress < 0.80f;
        return result;
    }

    std::string AIEngine::ExplainRecommendation(const DesignRecommendation& rec) {
        std::stringstream ss;
        ss << rec.title << " (Confidence: " << (rec.confidence * 100.0f) << "%)\n";
        ss << rec.description << "\n";
        ss << "Expected improvement: " << rec.expectedImprovement << "%\n";
        ss << "Actions:\n";
        for (const auto& action : rec.actions) {
            ss << "  - " << action << "\n";
        }
        return ss.str();
    }

    std::string AIEngine::ExplainFailurePrediction(const FailurePrediction& pred) {
        std::stringstream ss;
        ss << "Failure Mode: " << pred.failureMode << "\n";
        ss << "Probability: " << (pred.probability * 100.0f) << "%\n";
        ss << "Time to Failure: " << pred.timeToFailure << " hours\n";
        ss << "Preventive Measures:\n";
        for (const auto& measure : pred.preventiveMeasures) {
            ss << "  - " << measure << "\n";
        }
        return ss.str();
    }

    std::string AIEngine::ExplainReliability(const ReliabilityAnalysis& analysis) {
        std::stringstream ss;
        ss << "Reliability Score: " << (analysis.reliabilityScore * 100.0f) << "%\n";
        ss << "Risk Level: " << analysis.riskLevel << "\n";
        ss << "Stress Margin: " << analysis.stressMargin << "%\n";
        ss << "Thermal Margin: " << analysis.thermalMargin << "%\n";
        ss << "Failure Probability: " << (analysis.failureProbability * 100.0f) << "%\n";
        return ss.str();
    }

    float AIEngine::GetReliabilityScore(const DesignMetrics& metrics) const {
        return CalculateReliabilityScore(metrics);
    }

    float AIEngine::GetFailureProbability(const DesignMetrics& metrics) const {
        return 1.0f - CalculateReliabilityScore(metrics);
    }

} // namespace SZM::AI
