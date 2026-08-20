#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace SZM::FEA {

    /**
     * @struct CalculiXThermalConfig
     * @brief Configuration for thermal analysis in CalculiX
     */
    struct CalculiXThermalConfig {
        enum AnalysisMode {
            STEADY_STATE,
            TRANSIENT
        };

        AnalysisMode mode = STEADY_STATE;
        double ambientTemperature = 20.0;  // °C
        double heatInput = 100.0;           // Watts
        double convectionCoefficient = 10.0; // W/(m²·K)
        double duration = 3600.0;            // seconds (for transient)
        int timeSteps = 100;
        
        // Material properties
        double density = 7850.0;            // kg/m³
        double thermalConductivity = 50.0;  // W/(m·K)
        double specificHeat = 500.0;        // J/(kg·K)
        
        // Boundary conditions
        std::map<int, double> fixedTemperatures; // Node ID -> Temperature
    };

    /**
     * @struct ThermalAnalysisResult
     * @brief Results from thermal analysis
     */
    struct ThermalAnalysisResult {
        bool success = false;
        std::string engineUsed;
        double maxTemperature = 0.0;
        double minTemperature = 0.0;
        double averageTemperature = 0.0;
        double temperatureRise = 0.0;
        std::vector<double> temperatureField; // Temperature at each node
        std::vector<double> heatFluxField;    // Heat flux at each element
        bool isDangerous = false;
        std::string message;
    };

    /**
     * @class CalculiXThermalSolver
     * @brief Generates and runs thermal analysis using CalculiX
     */
    class CalculiXThermalSolver {
    public:
        CalculiXThermalSolver() = default;
        ~CalculiXThermalSolver() = default;

        /**
         * Generate CalculiX input file for thermal analysis
         */
        std::string GenerateThermalInputFile(const CalculiXThermalConfig& config);

        /**
         * Run thermal analysis (must have CalculiX installed)
         */
        ThermalAnalysisResult RunThermalAnalysis(const CalculiXThermalConfig& config);

        /**
         * Parse thermal results from CalculiX output
         */
        ThermalAnalysisResult ParseThermalResults(const std::string& outputPath);

        /**
         * Analytical thermal solver (fallback when CalculiX unavailable)
         */
        ThermalAnalysisResult SolveAnalytical(const CalculiXThermalConfig& config);

    private:
        bool ProbeCalculiX();
        std::string m_CalculiXBinary;
    };

} // namespace SZM::FEA
