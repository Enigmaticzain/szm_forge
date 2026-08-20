#pragma once

#include <string>
#include <vector>
#include <complex>
#include <memory>

namespace SZM::FEA {

    /**
     * @struct CalculiXModalConfig
     * @brief Configuration for modal/eigenvalue analysis in CalculiX
     */
    struct CalculiXModalConfig {
        int numModes = 10;                  // Number of mode shapes
        double minFrequency = 0.0;          // Hz
        double maxFrequency = 10000.0;      // Hz
        
        // Material properties
        double density = 7850.0;            // kg/m³
        double youngsModulus = 210e9;       // Pa
        double poissonRatio = 0.3;
        
        // Constraints (fixed DOFs)
        std::vector<int> fixedNodes;
    };

    /**
     * @struct ModeShape
     * @brief Single mode shape result
     */
    struct ModeShape {
        int modeNumber = 0;
        double frequency = 0.0;             // Hz
        double wavelength = 0.0;            // m (inverse of frequency)
        double dampingRatio = 0.0;          // Percentage
        std::vector<double> nodalDisplacements; // Displacement at each node
    };

    /**
     * @struct ModalAnalysisResult
     * @brief Results from modal/eigenvalue analysis
     */
    struct ModalAnalysisResult {
        bool success = false;
        std::string engineUsed;
        int numModesComputed = 0;
        std::vector<double> naturalFrequencies;  // Hz
        std::vector<ModeShape> modeShapes;
        double firstNaturalFrequency = 0.0;
        double lowestFrequency = 0.0;
        double highestFrequency = 0.0;
        std::string message;
    };

    /**
     * @class CalculiXModalSolver
     * @brief Generates and runs modal analysis using CalculiX for eigenvalue computation
     */
    class CalculiXModalSolver {
    public:
        CalculiXModalSolver() = default;
        ~CalculiXModalSolver() = default;

        /**
         * Generate CalculiX input file for modal analysis
         */
        std::string GenerateModalInputFile(const CalculiXModalConfig& config);

        /**
         * Run modal analysis (must have CalculiX installed)
         */
        ModalAnalysisResult RunModalAnalysis(const CalculiXModalConfig& config);

        /**
         * Parse modal results from CalculiX output
         */
        ModalAnalysisResult ParseModalResults(const std::string& outputPath, int numModes);

        /**
         * Analytical modal solver (fallback when CalculiX unavailable)
         * Uses beam theory for cantilever estimation
         */
        ModalAnalysisResult SolveAnalytical(const CalculiXModalConfig& config);

        /**
         * Compute fundamental frequency using beam theory
         * f = (λ² / (2*π)) * sqrt(E*I / (ρ*A*L⁴))
         */
        double ComputeBeamFrequency(double youngsModulus, double density,
                                   double length, double area, double momentOfInertia);

    private:
        bool ProbeCalculiX();
        std::string m_CalculiXBinary;
        static const std::vector<double> CANTILEVER_MODE_COEFFICIENTS; // 1.875, 4.694, ...
    };

} // namespace SZM::FEA
