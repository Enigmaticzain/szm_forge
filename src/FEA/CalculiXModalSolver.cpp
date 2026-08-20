#include "CalculiXModalSolver.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace SZM::FEA {

// Static initialization of mode coefficients for cantilever beam
const std::vector<double> CalculiXModalSolver::CANTILEVER_MODE_COEFFICIENTS = {
    1.875, 4.694, 7.855, 10.996, 14.137, 17.276, 20.420, 23.562, 26.703, 29.845
};

std::string CalculiXModalSolver::GenerateModalInputFile(const CalculiXModalConfig& config) {
    std::ostringstream inp;

    // Define geometry: 0.1m cantilever beam
    inp << "*NODE\n";
    inp << " 1, 0.0, 0.0, 0.0\n";
    inp << " 2, 0.05, 0.0, 0.0\n";
    inp << " 3, 0.1, 0.0, 0.0\n";
    inp << " 4, 0.0, 0.05, 0.0\n";
    inp << " 5, 0.05, 0.05, 0.0\n";
    inp << " 6, 0.1, 0.05, 0.0\n";
    inp << " 7, 0.0, 0.0, 0.05\n";
    inp << " 8, 0.05, 0.0, 0.05\n";
    inp << " 9, 0.1, 0.0, 0.05\n";

    // Define 8-node structural elements (C3D8)
    inp << "*ELEMENT, TYPE=C3D8, ELSET=EALL\n";
    inp << " 1, 1,2,3,4,5,6,7,8\n";  // First element

    // Material definition
    inp << "*MATERIAL, NAME=MODAL_MAT\n";
    inp << "*ELASTIC\n";
    inp << " " << config.youngsModulus << ", " << config.poissonRatio << "\n";
    inp << "*DENSITY\n";
    inp << " " << config.density << "\n";

    inp << "*SOLID SECTION, ELSET=EALL, MATERIAL=MODAL_MAT\n";

    // Boundary conditions: fix all DOFs at node 1 (cantilever)
    inp << "*BOUNDARY\n";
    inp << " 1, 1, 3\n";  // Fixed: U1, U2, U3 at node 1

    // Frequency step
    inp << "*STEP\n";
    inp << "*FREQUENCY, NMIN=1, NMAX=" << config.numModes << "\n";
    inp << "*END STEP\n";

    return inp.str();
}

ModalAnalysisResult CalculiXModalSolver::RunModalAnalysis(const CalculiXModalConfig& config) {
    ModalAnalysisResult result;
    
    // Try to find CalculiX
    std::vector<std::string> ccxBinaries = {"ccx", "ccx_2.17", "ccx_2.20", "calculix"};
    bool ccxFound = false;
    std::string ccxBinary;

    for (const auto& bin : ccxBinaries) {
        int ret = system(("which " + bin + " > /dev/null 2>&1").c_str());
        if (ret == 0) {
            ccxBinary = bin;
            ccxFound = true;
            break;
        }
    }

    if (!ccxFound) {
        // Fall back to analytical solution
        result = SolveAnalytical(config);
        result.engineUsed = "Analytical (CalculiX not found)";
        return result;
    }

    result.engineUsed = ccxBinary;
    
    // Generate input file
    std::string inputContent = GenerateModalInputFile(config);

    // Create temporary directory and write input file
    std::string tempDir = "/tmp/szm_modal_analysis";
    system("mkdir -p " + tempDir);
    
    std::string inputPath = tempDir + "/modal_job.inp";
    std::ofstream inputFile(inputPath);
    if (!inputFile.is_open()) {
        result.success = false;
        result.message = "Failed to create input file";
        return result;
    }
    
    inputFile << inputContent;
    inputFile.close();

    // Run CalculiX
    std::string cmd = "cd " + tempDir + " && " + ccxBinary + " -i modal_job > modal_job.log 2>&1";
    int ret = system(cmd.c_str());

    if (ret != 0) {
        // Fall back to analytical
        result = SolveAnalytical(config);
        result.engineUsed += " (fallback, CalculiX run failed)";
        return result;
    }

    // Parse results
    result = ParseModalResults(tempDir + "/modal_job.frd", config.numModes);
    result.engineUsed = ccxBinary;
    result.success = true;
    
    return result;
}

ModalAnalysisResult CalculiXModalSolver::ParseModalResults(const std::string& outputPath, int numModes) {
    ModalAnalysisResult result;
    
    std::ifstream file(outputPath);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open result file";
        return result;
    }

    std::vector<double> frequencies;
    std::string line;

    // Parse .frd file for eigenvalues (frequencies)
    while (std::getline(file, line)) {
        // Look for eigenvalue lines in output
        if (line.find("eigenvalue") != std::string::npos || 
            line.find("frequency") != std::string::npos ||
            line.find("FREQ") != std::string::npos) {
            try {
                size_t pos = line.find_last_of("0123456789.eE+-");
                if (pos != std::string::npos) {
                    std::string numStr = line.substr(pos);
                    double eigenvalue = std::stod(numStr);
                    // Convert eigenvalue to frequency (Hz)
                    double freq = std::sqrt(eigenvalue) / (2.0 * M_PI);
                    frequencies.push_back(freq);
                }
            } catch (...) {
                // Skip invalid lines
            }
        }
    }

    file.close();

    if (!frequencies.empty()) {
        result.numModesComputed = frequencies.size();
        result.naturalFrequencies = frequencies;
        result.firstNaturalFrequency = frequencies[0];
        result.lowestFrequency = *std::min_element(frequencies.begin(), frequencies.end());
        result.highestFrequency = *std::max_element(frequencies.begin(), frequencies.end());
        
        // Create mode shapes (simplified)
        for (size_t i = 0; i < frequencies.size(); ++i) {
            ModeShape mode;
            mode.modeNumber = i + 1;
            mode.frequency = frequencies[i];
            mode.wavelength = 1.0 / frequencies[i];
            mode.dampingRatio = 0.05 * (i + 1);  // Estimated damping
            result.modeShapes.push_back(mode);
        }
        
        result.success = true;
    }

    return result;
}

ModalAnalysisResult CalculiXModalSolver::SolveAnalytical(const CalculiXModalConfig& config) {
    ModalAnalysisResult result;

    // Analytical solution using cantilever beam theory
    // f_n = (λ_n² / (2*π)) * sqrt(E*I / (ρ*A*L⁴))
    
    double L = 0.1;      // m (beam length)
    double A = 0.01;     // m² (cross-section area)
    double I = (0.1 * 0.1 * 0.1 * 0.1) / 12.0;  // m⁴ (second moment of inertia)
    
    double baseFreq = ComputeBeamFrequency(config.youngsModulus, config.density, L, A, I);

    // Compute natural frequencies for each mode
    int numModes = std::min(static_cast<int>(CANTILEVER_MODE_COEFFICIENTS.size()), 
                           config.numModes);
    
    for (int i = 0; i < numModes; ++i) {
        double freq = CANTILEVER_MODE_COEFFICIENTS[i] * baseFreq;
        
        // Filter by frequency range
        if (freq >= config.minFrequency && freq <= config.maxFrequency) {
            result.naturalFrequencies.push_back(freq);
            
            // Create mode shape
            ModeShape mode;
            mode.modeNumber = i + 1;
            mode.frequency = freq;
            mode.wavelength = 1.0 / freq;
            mode.dampingRatio = 0.02 * (i + 1);  // Estimated damping
            mode.nodalDisplacements.resize(9);   // 9 nodes
            
            // Simple amplitude estimation (decreases with mode number)
            double amplitude = 1.0 / (1.0 + 0.5 * i);
            for (int j = 0; j < 9; ++j) {
                // Simple sine distribution (0 at node 1, max at node 9)
                double x = j / 8.0;
                mode.nodalDisplacements[j] = amplitude * std::sin(M_PI * x * (i + 1));
            }
            
            result.modeShapes.push_back(mode);
        }
    }

    if (!result.naturalFrequencies.empty()) {
        result.numModesComputed = result.naturalFrequencies.size();
        result.firstNaturalFrequency = result.naturalFrequencies[0];
        result.lowestFrequency = result.naturalFrequencies[0];
        result.highestFrequency = result.naturalFrequencies.back();
    }

    result.success = true;
    result.engineUsed = "Analytical (Cantilever Beam Theory)";
    
    return result;
}

double CalculiXModalSolver::ComputeBeamFrequency(double youngsModulus, double density,
                                                double length, double area, double momentOfInertia) {
    // f = (λ² / (2*π)) * sqrt(E*I / (ρ*A*L⁴))
    
    if (momentOfInertia <= 0 || area <= 0 || length <= 0) {
        return 0.0;
    }

    double stiffness_term = (youngsModulus * momentOfInertia) / (density * area);
    double length_term = std::pow(length, 4);
    
    return (1.0 / (2.0 * M_PI)) * std::sqrt(stiffness_term / length_term);
}

bool CalculiXModalSolver::ProbeCalculiX() {
    std::vector<std::string> ccxBinaries = {"ccx", "ccx_2.17", "ccx_2.20", "calculix"};
    
    for (const auto& bin : ccxBinaries) {
        int ret = system(("which " + bin + " > /dev/null 2>&1").c_str());
        if (ret == 0) {
            m_CalculiXBinary = bin;
            return true;
        }
    }
    
    return false;
}

} // namespace SZM::FEA
