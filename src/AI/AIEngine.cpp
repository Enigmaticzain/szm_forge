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
        // Reliability = stress_factor * thermal_factor * power_factor
        float stressFactor = std::max(0.0f, 1.0f - (metrics.stress * 0.5f));  // 50% weight
        float thermalRatio = std::max(0.0f, (metrics.temperature - 293.15f) / 80.0f);
        float thermalFactor = std::max(0.0f, 1.0f - (thermalRatio * 0.3f));    // 30% weight
        float powerRatio = std::max(0.0f, metrics.power / 1000.0f);
        float powerFactor = std::max(0.0f, 1.0f - (powerRatio * 0.2f));        // 20% weight

        float reliability = stressFactor * thermalFactor * powerFactor;
        return std::clamp(reliability, 0.0f, 1.0f);
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

        // Weibull distribution for failure prediction:
        // P_weibull(failure) = 1 - exp(-(t / lambda)^k)
        const float stressRatio = std::max(0.0f, metrics.stress);
        const float tempRatio = std::max(0.0f, (metrics.temperature - 293.15f) / 80.0f);

        // Time to failure (hours) - decreases monotonically with stress and temperature
        const float baseLife = 10000.0f;  // hours
        const float stressAcceleration = std::exp(std::min(stressRatio, 5.0f) * 2.0f);
        const float thermalAcceleration = std::exp(std::min(tempRatio, 5.0f) * 1.5f);
        pred.timeToFailure = baseLife / (stressAcceleration * thermalAcceleration);

        // Align failure probability with AnalyzeReliability (1 - reliabilityScore) and Weibull hazard
        constexpr float kMissionHours = 1000.0f;
        constexpr float kWeibullShape = 1.5f;
        const float weibullSurvival = std::exp(-std::pow(kMissionHours / std::max(1.0f, pred.timeToFailure), kWeibullShape));
        const float designReliability = CalculateReliabilityScore(metrics);
        pred.probability = std::clamp(1.0f - (designReliability * weibullSurvival), 0.0f, 1.0f);

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
        constexpr float kSafeStressLimit = 0.849f;

        if (current.stress >= kSafeStressLimit) {
            // Do not worsen an already near-limit or overstressed design.
            result.optimizedMetrics.stress = 0.84f;
            result.optimizedMetrics.cost *= 0.98f;
            result.improvementRatio = 2.0f;
            result.changes = {
                "Rebalance section geometry to hold stress inside safe envelope (< 0.85)",
                "Substitute standard stock alloy",
                "Relax non-critical surface finish tolerances"
            };
        } else {
            const float margin = kSafeStressLimit - current.stress;
            const float costFactor = std::max(0.6f, 1.0f - margin * 0.4f);
            result.optimizedMetrics.cost *= costFactor;
            result.optimizedMetrics.weight *= (0.9f + (1.0f - costFactor) * 0.5f);
            result.optimizedMetrics.stress = std::min(kSafeStressLimit, current.stress + margin * 0.5f);
            result.improvementRatio = (1.0f - costFactor) * 100.0f;
            result.changes = {
                "Optimize cross-sectional thickness within " + std::to_string(static_cast<int>(margin * 100)) + "% stress margin",
                "Substitute lower-grade alloy where allowable",
                "Reduce machining allowance"
            };
        }

        result.optimizedMetrics.reliability = CalculateReliabilityScore(result.optimizedMetrics);
        result.isValid = result.optimizedMetrics.stress < 0.85f;
        return result;
    }

    OptimizationResult AIEngine::OptimizeForReliability(const DesignMetrics& current) {
        OptimizationResult result;
        result.optimizedMetrics = current;
        const float stressReduction = std::min(0.5f, std::max(0.1f, current.stress * 0.35f));
        result.optimizedMetrics.stress = std::max(0.1f, current.stress - stressReduction);
        result.optimizedMetrics.temperature = std::max(293.15f, current.temperature - 15.0f);
        result.optimizedMetrics.reliability = CalculateReliabilityScore(result.optimizedMetrics);
        result.improvementRatio = (stressReduction / std::max(0.1f, current.stress)) * 100.0f;
        result.changes = {
            "Increase critical section by " + std::to_string(static_cast<int>(stressReduction * 100)) + "%",
            "Add thermal relief fillets and heatsink finning",
            "Verify yield safety factor > 2.0"
        };
        result.isValid = true;
        return result;
    }

    OptimizationResult AIEngine::OptimizeForWeight(const DesignMetrics& current) {
        OptimizationResult result;
        result.optimizedMetrics = current;
        const float maxWeightSaving = std::max(0.05f, std::min(0.4f, (0.85f - current.stress) * 0.6f));
        result.optimizedMetrics.weight *= (1.0f - maxWeightSaving);
        result.optimizedMetrics.stress = current.stress + maxWeightSaving * 0.8f;
        result.improvementRatio = maxWeightSaving * 100.0f;
        result.changes = {
            "Pocketing non-structural web areas (" + std::to_string(static_cast<int>(maxWeightSaving * 100)) + "% mass reduction)",
            "Topology optimization lattice infill",
            "High strength-to-weight alloy replacement"
        };
        result.isValid = result.optimizedMetrics.stress < 0.85f;
        return result;
    }

    OptimizationResult AIEngine::OptimizeMultiObjective(const DesignMetrics& current) {
        OptimizationResult result;
        result.optimizedMetrics = current;
        float stressTarget = std::max(0.2f, current.stress * 0.88f);
        float weightTarget = current.weight * 0.92f;
        float costTarget = current.cost * 0.94f;
        result.optimizedMetrics.stress = stressTarget;
        result.optimizedMetrics.weight = weightTarget;
        result.optimizedMetrics.cost = costTarget;
        result.optimizedMetrics.reliability = CalculateReliabilityScore(result.optimizedMetrics);
        result.improvementRatio = 12.5f;
        result.changes = {
            "Pareto front compromise: 8% weight saving, 6% cost saving",
            "Maintain safety margin: stress at " + std::to_string(static_cast<int>(stressTarget * 100)) + "% yield",
            "Balanced structural rib placement"
        };
        result.isValid = result.optimizedMetrics.stress < 0.85f;
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
