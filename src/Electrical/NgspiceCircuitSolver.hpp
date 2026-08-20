#pragma once

#include <string>
#include <vector>
#include <map>

namespace SZM::Electrical {

    /**
     * @enum AnalysisType
     * @brief Circuit analysis types supported by ngspice
     */
    enum class AnalysisType {
        DC,           // DC operating point analysis
        AC,           // AC frequency response
        TRANSIENT,    // Time-domain transient analysis
        SWEEP,        // Parameter sweep
        NOISE         // Noise analysis
    };

    /**
     * @enum ComponentType
     * @brief Electronic component types
     */
    enum class ComponentType {
        RESISTOR,
        CAPACITOR,
        INDUCTOR,
        VOLTAGE_SOURCE,
        CURRENT_SOURCE,
        DIODE,
        TRANSISTOR_BJT,
        TRANSISTOR_FET,
        OPAMP
    };

    /**
     * @struct CircuitComponent
     * @brief Represents an electronic circuit component
     */
    struct CircuitComponent {
        std::string name;           // e.g., "R1", "C1", "V1"
        ComponentType type;
        double value = 0.0;         // Resistance (Ω), Capacitance (F), Inductance (H), Voltage (V), Current (A)
        double tolerance = 0.05;    // 5% default tolerance
        double temperature = 27.0;  // °C
        int node1 = 1;              // Positive/input node
        int node2 = 0;              // Negative/output node (0=ground)
        bool isFaulty = false;
        std::string model;          // Model name for complex components (diodes, transistors)
    };

    /**
     * @struct CircuitNetlist
     * @brief Complete circuit netlist for simulation
     */
    struct CircuitNetlist {
        std::string title;
        std::vector<CircuitComponent> components;
        std::map<std::string, std::string> models;  // Model definitions
        std::map<std::string, double> parameters;   // Global parameters
    };

    /**
     * @struct CircuitAnalysisResult
     * @brief Results from circuit analysis
     */
    struct CircuitAnalysisResult {
        bool success = false;
        std::string engineUsed;
        AnalysisType analysisType;
        
        // DC Analysis Results
        std::map<int, double> nodeVoltages;      // Node number -> voltage (V)
        std::map<std::string, double> componentCurrents;  // Component name -> current (A)
        std::map<std::string, double> componentPowers;    // Component name -> power (W)
        
        // AC Analysis Results
        double frequency_Hz = 0.0;
        std::map<int, std::complex<double>> acImpedances;  // Node impedances
        double phaseShift_degrees = 0.0;
        double resonantFrequency_Hz = 0.0;
        
        // Transient Analysis Results
        std::vector<double> timePoints;          // Time values (seconds)
        std::map<int, std::vector<double>> voltageWaveforms;  // Node voltage over time
        std::map<std::string, std::vector<double>> currentWaveforms;  // Branch currents over time
        double settlingTime_s = 0.0;
        
        // General results
        std::string message;
        std::vector<std::string> warnings;
    };

    /**
     * @struct NgspiceConfig
     * @brief Configuration for ngspice simulation
     */
    struct NgspiceConfig {
        AnalysisType analysisType = AnalysisType::DC;
        double temperature_C = 27.0;
        
        // AC analysis parameters
        double frequency_Hz = 1000.0;
        double startFrequency_Hz = 10.0;
        double stopFrequency_Hz = 1e6;
        int pointsPerDecade = 100;
        
        // Transient analysis parameters
        double duration_s = 1.0;
        int timeSteps = 100;
        double timeStep_s = 0.01;
        
        // Parameter sweep
        std::string sweepParameter;
        double sweepStart = 0.0;
        double sweepStop = 100.0;
        int sweepPoints = 100;
    };

    /**
     * @class NgspiceCircuitSolver
     * @brief Bridge to ngspice circuit simulator with analytical fallback
     */
    class NgspiceCircuitSolver {
    public:
        NgspiceCircuitSolver() = default;
        ~NgspiceCircuitSolver() = default;

        /**
         * Build SPICE netlist from components
         */
        std::string BuildNetlist(const CircuitNetlist& circuit, const NgspiceConfig& config);

        /**
         * Run circuit simulation
         */
        CircuitAnalysisResult RunSimulation(const CircuitNetlist& circuit, const NgspiceConfig& config);

        /**
         * Parse ngspice output file
         */
        CircuitAnalysisResult ParseResults(const std::string& outputFile, const NgspiceConfig& config);

        /**
         * Analytical solver (fallback when ngspice unavailable)
         */
        CircuitAnalysisResult SolveAnalytical(const CircuitNetlist& circuit, const NgspiceConfig& config);

        /**
         * Detect circuit faults (open/short circuits, component failure)
         */
        std::vector<std::string> DetectFaults(const CircuitNetlist& circuit, 
                                             const CircuitAnalysisResult& results);

        /**
         * Check if ngspice is installed
         */
        bool ProbeNgspice();

    private:
        std::string m_NgspiceBinary;
        
        // Analytical computation methods
        CircuitAnalysisResult ComputeDC_Analytical(const CircuitNetlist& circuit);
        CircuitAnalysisResult ComputeAC_Analytical(const CircuitNetlist& circuit, const NgspiceConfig& config);
        CircuitAnalysisResult ComputeTransient_Analytical(const CircuitNetlist& circuit, const NgspiceConfig& config);
        
        // Utility methods
        double ComputeReactance_L(double frequency_Hz, double inductance_H);
        double ComputeReactance_C(double frequency_Hz, double capacitance_F);
        double ComputeImpedance(double resistance, double reactance);
    };

} // namespace SZM::Electrical
