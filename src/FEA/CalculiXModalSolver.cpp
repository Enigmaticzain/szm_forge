#include "CalculiXModalSolver.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <filesystem>

namespace SZM::FEA {

// Static initialization of mode coefficients for cantilever beam
const std::vector<double> CalculiXModalSolver::CANTILEVER_MODE_COEFFICIENTS = {
    1.875, 4.694, 7.855, 10.996, 14.137, 17.276, 20.420, 23.562, 26.703, 29.845
};

std::string CalculiXModalSolver::GenerateModalInputFile(const CalculiXModalConfig& config) {
    std::ostringstream inp;

    // Define geometry: 0.1m cantilever beam (valid 8-node C3D8 hex element)
    inp << "*NODE\n";
    inp << " 1, 0.0, 0.0, 0.0\n";
    inp << " 2, 0.1, 0.0, 0.0\n";
    inp << " 3, 0.1, 0.05, 0.0\n";
    inp << " 4, 0.0, 0.05, 0.0\n";
    inp << " 5, 0.0, 0.0, 0.05\n";
    inp << " 6, 0.1, 0.0, 0.05\n";
    inp << " 7, 0.1, 0.05, 0.05\n";
    inp << " 8, 0.0, 0.05, 0.05\n";

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

    // Boundary conditions: clamp root face at x = 0 (nodes 1, 4, 5, 8) or configured nodes
    inp << "*BOUNDARY\n";
    if (!config.fixedNodes.empty()) {
        for (int nid : config.fixedNodes) {
            inp << " " << nid << ", 1, 3\n";
        }
    } else {
        for (int nid : {1, 4, 5, 8}) {
            inp << " " << nid << ", 1, 3\n";
        }
    }

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
    const std::filesystem::path tempDirPath =
        std::filesystem::temp_directory_path() / "szm_modal_analysis";
    std::error_code ec;
    std::filesystem::create_directories(tempDirPath, ec);
    const std::string tempDir = tempDirPath.string();

    std::string inputPath = (tempDirPath / "modal_job.inp").string();
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
        result = SolveAnalytical(config);
        result.engineUsed = ccxBinary + " (fallback, CalculiX run failed)";
        return result;
    }

    // Parse eigenvalue table from CalculiX .dat output (fall back to .frd if present)
    result = ParseModalResults((tempDirPath / "modal_job.dat").string(), config.numModes);
    if (!result.success) {
        result = ParseModalResults((tempDirPath / "modal_job.frd").string(), config.numModes);
    }
    if (!result.success) {
        ModalAnalysisResult fallback = SolveAnalytical(config);
        fallback.engineUsed = ccxBinary + " (fallback, result parse failed)";
        fallback.message = result.message;
        return fallback;
    }
    result.engineUsed = ccxBinary;
    
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
    bool inEigenvalueTable = false;

    while (std::getline(file, line)) {
        if (static_cast<int>(frequencies.size()) >= numModes) {
            break;
        }
        if (line.find("EIGENVALUE OUTPUT") != std::string::npos ||
            line.find("MODE NO") != std::string::npos) {
            inEigenvalueTable = true;
            continue;
        }
        if (inEigenvalueTable) {
            std::istringstream iss(line);
            int modeNo = 0;
            double eigenvalue = 0.0;
            double realFreqRad = 0.0;
            double freqHz = 0.0;
            if (iss >> modeNo >> eigenvalue >> realFreqRad >> freqHz) {
                if (freqHz > 0.0) {
                    frequencies.push_back(freqHz);
                } else if (eigenvalue > 0.0) {
                    frequencies.push_back(std::sqrt(eigenvalue) / (2.0 * M_PI));
                }
                continue;
            }
            if (!frequencies.empty() && line.find_first_not_of(" \t\r\n") != std::string::npos) {
                inEigenvalueTable = false;
            }
        }
        // Fallback for summary lines mentioning eigenvalue/FREQ
        if (line.find("eigenvalue") != std::string::npos || 
            line.find("frequency") != std::string::npos ||
            line.find("FREQ") != std::string::npos) {
            std::istringstream iss(line);
            std::string token;
            double eigenvalue = 0.0;
            bool foundVal = false;
            while (iss >> token) {
                try {
                    eigenvalue = std::stod(token);
                    foundVal = true;
                } catch (...) {
                }
            }
            if (foundVal && eigenvalue > 0.0) {
                frequencies.push_back(std::sqrt(eigenvalue) / (2.0 * M_PI));
            }
        }
    }

    file.close();

    if (!frequencies.empty()) {
        result.numModesComputed = static_cast<int>(frequencies.size());
        result.naturalFrequencies = frequencies;
        result.firstNaturalFrequency = frequencies[0];
        result.lowestFrequency = *std::min_element(frequencies.begin(), frequencies.end());
        result.highestFrequency = *std::max_element(frequencies.begin(), frequencies.end());
        
        for (size_t i = 0; i < frequencies.size(); ++i) {
            ModeShape mode;
            mode.modeNumber = static_cast<int>(i) + 1;
            mode.frequency = frequencies[i];
            mode.wavelength = (frequencies[i] > 0.0) ? (1.0 / frequencies[i]) : 0.0;
            mode.dampingRatio = 0.05 * (i + 1);
            result.modeShapes.push_back(mode);
        }
        
        result.success = true;
    } else {
        result.success = false;
        result.message = "No eigenvalues found in CalculiX output";
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
        const double lambda = CANTILEVER_MODE_COEFFICIENTS[i];
        double freq = (lambda * lambda) * baseFreq;
        
        // Filter by frequency range
        if (freq >= config.minFrequency && freq <= config.maxFrequency) {
            result.naturalFrequencies.push_back(freq);
            
            // Create mode shape
            ModeShape mode;
            mode.modeNumber = i + 1;
            mode.frequency = freq;
            mode.wavelength = (freq > 0.0) ? (1.0 / freq) : 0.0;
            mode.dampingRatio = 0.02 * (i + 1);  // Estimated damping
            mode.nodalDisplacements.resize(9);   // 9 nodes
            
            double amplitude = 1.0 / (1.0 + 0.5 * i);
            for (int j = 0; j < 9; ++j) {
                double x = j / 8.0;
                mode.nodalDisplacements[j] = amplitude * std::sin(M_PI * x * (i + 1));
            }
            
            result.modeShapes.push_back(mode);
        }
    }

    if (!result.naturalFrequencies.empty()) {
        result.numModesComputed = static_cast<int>(result.naturalFrequencies.size());
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
    // Returns sqrt(E*I / (ρ*A*L⁴)) / (2*π); multiply by λ_n² for mode n.
    if (youngsModulus <= 0.0 || density <= 0.0 || momentOfInertia <= 0.0 ||
        area <= 0.0 || length <= 0.0) {
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
