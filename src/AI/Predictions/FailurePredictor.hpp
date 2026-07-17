#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace SZM::AI::Predictions {

    /**
     * @brief Predicts stress hotspots using neural networks
     * Trained on FEA simulation data
     */
    class StressPredictor {
    public:
        struct PredictionResult {
            bool success = false;
            std::vector<float> stressMap;      // Per-vertex stress values
            std::vector<uint32_t> hotspots;    // Vertex indices of high-stress areas
            float maxStress = 0.0f;
            float confidenceScore = 0.0f;      // 0.0-1.0
            float inferenceTimeMs = 0.0f;
        };

        StressPredictor() = default;
        ~StressPredictor() = default;

        /**
         * @brief Initialize predictor with trained model
         * @param modelPath Path to PyTorch model (.pt file)
         * @return true if successful
         */
        bool LoadModel(const std::string& modelPath);

        /**
         * @brief Predict stress distribution for component
         * @param componentId Component UUID
         * @param loadVector Applied load [Fx, Fy, Fz]
         * @return Prediction result with stress map
         */
        PredictionResult PredictStress(uint32_t componentId, const float* loadVector);

        /**
         * @brief Batch predict for multiple load cases
         */
        std::vector<PredictionResult> PredictBatch(
            uint32_t componentId,
            const std::vector<std::vector<float>>& loadCases
        );

        bool IsModelLoaded() const { return m_ModelLoaded; }

    private:
        bool m_ModelLoaded = false;
        void* m_TorchModel = nullptr;

        std::vector<float> ExtractFeatures(uint32_t componentId);
    };

    /**
     * @brief Predicts thermal failures
     */
    class ThermalPredictor {
    public:
        struct ThermalResult {
            bool success = false;
            std::vector<float> temperatureMap;
            float maxTemperature = 0.0f;
            float timeToFailureHours = 0.0f;
            float confidenceScore = 0.0f;
        };

        bool LoadModel(const std::string& modelPath);
        ThermalResult PredictThermal(uint32_t componentId, float ambientTemp, float powerDissipation);

    private:
        bool m_ModelLoaded = false;
        void* m_TorchModel = nullptr;
    };

    /**
     * @brief Suggests design optimizations
     */
    class DesignOptimizer {
    public:
        struct OptimizationSuggestion {
            std::string description;
            float expectedStressReduction = 0.0f;  // Percentage
            float expectedWeightReduction = 0.0f;
            float feasibilityScore = 0.0f;         // 0.0-1.0
            std::vector<uint32_t> affectedRegions;
        };

        bool LoadModel(const std::string& modelPath);

        /**
         * @brief Suggest optimizations for component
         * @param componentId Component to optimize
         * @param maxSuggestions Maximum number of suggestions
         * @return List of optimization suggestions
         */
        std::vector<OptimizationSuggestion> SuggestOptimizations(
            uint32_t componentId,
            uint32_t maxSuggestions = 5
        );

    private:
        bool m_ModelLoaded = false;
        void* m_TorchModel = nullptr;
    };

    /**
     * @brief Manages all AI prediction models
     */
    class PredictionManager {
    public:
        static PredictionManager& GetInstance();

        bool Initialize(const std::string& modelDirectory);
        void Shutdown();

        StressPredictor& GetStressPredictor() { return m_StressPredictor; }
        ThermalPredictor& GetThermalPredictor() { return m_ThermalPredictor; }
        DesignOptimizer& GetOptimizer() { return m_Optimizer; }

        bool IsReady() const { return m_IsReady; }

    private:
        PredictionManager() = default;
        ~PredictionManager() = default;

        StressPredictor m_StressPredictor;
        ThermalPredictor m_ThermalPredictor;
        DesignOptimizer m_Optimizer;
        bool m_IsReady = false;
    };

} // namespace SZM::AI::Predictions
