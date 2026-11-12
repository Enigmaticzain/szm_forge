#include "CalculiXThermalSolver.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <numeric>

namespace SZM::FEA {

std::string CalculiXThermalSolver::GenerateThermalInputFile(const CalculiXThermalConfig& config) {
    std::ostringstream inp;

    // Define geometry: 0.1m x 0.1m x 0.1m cube
    inp << "*NODE\n";
    inp << " 1, 0.0, 0.0, 0.0\n";
    inp << " 2, 0.1, 0.0, 0.0\n";
    inp << " 3, 0.1, 0.1, 0.0\n";
    inp << " 4, 0.0, 0.1, 0.0\n";
    inp << " 5, 0.0, 0.0, 0.1\n";
    inp << " 6, 0.1, 0.0, 0.1\n";
    inp << " 7, 0.1, 0.1, 0.1\n";
    inp << " 8, 0.0, 0.1, 0.1\n";

    // Define 8-node thermal element
    inp << "*ELEMENT, TYPE=DC3D8, ELSET=EALL\n";
    inp << " 1, 1,2,3,4,5,6,7,8\n";

    // Material definition
    inp << "*MATERIAL, NAME=THERMAL_MAT\n";
    inp << "*DENSITY\n";
    inp << " " << config.density << "\n";
    inp << "*CONDUCTIVITY\n";
    inp << " " << config.thermalConductivity << "\n";
    
    if (config.mode == CalculiXThermalConfig::TRANSIENT) {
        inp << "*SPECIFIC HEAT\n";
        inp << " " << config.specificHeat << "\n";
    }

    inp << "*SOLID SECTION, ELSET=EALL, MATERIAL=THERMAL_MAT\n";

    // Initial conditions
    inp << "*INITIAL CONDITIONS, TYPE=TEMPERATURE\n";
    for (int i = 1; i <= 8; ++i) {
        inp << " " << i << ", " << config.ambientTemperature << "\n";
    }

    // Boundary conditions (fixed temperature on bottom face or configured nodes)
    inp << "*BOUNDARY\n";
    if (!config.fixedTemperatures.empty()) {
        for (const auto& [nodeId, tempVal] : config.fixedTemperatures) {
            inp << " " << nodeId << ", 11, 11, " << tempVal << "\n";
        }
    } else {
        inp << " 1, 11, 11, " << config.ambientTemperature << "\n";
    }

    // Step and loading
    double heatPerNode = config.heatInput / 4.0;  // Distribute over 4 top nodes
    
    if (config.mode == CalculiXThermalConfig::TRANSIENT) {
        const int steps = std::max(1, config.timeSteps);
        inp << "*STEP, INC=" << steps << "\n";
        inp << "*HEAT TRANSFER\n";
        inp << " " << (config.duration / steps) << ", " << config.duration << "\n";
    } else {
        inp << "*STEP\n";
        inp << "*HEAT TRANSFER, STEADY STATE\n";
    }

    // Convection boundary condition (CalculiX uses *FILM)
    inp << "*FILM\n";
    inp << " 1, F1, " << config.ambientTemperature << ", " << config.convectionCoefficient << "\n";

    // Heat source (applied to top nodes)
    inp << "*CFLUX\n";
    inp << " 5, 11, " << heatPerNode << "\n";
    inp << " 6, 11, " << heatPerNode << "\n";
    inp << " 7, 11, " << heatPerNode << "\n";
    inp << " 8, 11, " << heatPerNode << "\n";

    // Output requests
    inp << "*NODE PRINT, NSET=NALL\n";
    inp << " NT\n";
    inp << "*EL PRINT, ELSET=EALL\n";
    inp << " HFL\n";  // Heat flux
    inp << "*END STEP\n";

    return inp.str();
}

ThermalAnalysisResult CalculiXThermalSolver::RunThermalAnalysis(const CalculiXThermalConfig& config) {
    ThermalAnalysisResult result;
    
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
        result = SolveAnalytical(config);
        result.engineUsed = "Analytical (CalculiX not found)";
        return result;
    }

    result.engineUsed = ccxBinary;
    
    // Generate input file
    std::string inputContent = GenerateThermalInputFile(config);

    // Create temporary directory and write input file
    const std::filesystem::path tempDirPath =
        std::filesystem::temp_directory_path() / "szm_thermal_analysis";
    std::error_code ec;
    std::filesystem::create_directories(tempDirPath, ec);
    const std::string tempDir = tempDirPath.string();

    std::string inputPath = (tempDirPath / "thermal_job.inp").string();
    std::ofstream inputFile(inputPath);
    if (!inputFile.is_open()) {
        result.success = false;
        result.message = "Failed to create input file";
        return result;
    }
    
    inputFile << inputContent;
    inputFile.close();

    // Run CalculiX
    std::string cmd = "cd " + tempDir + " && " + ccxBinary + " -i thermal_job > thermal_job.log 2>&1";
    int ret = system(cmd.c_str());

    if (ret != 0) {
        result = SolveAnalytical(config);
        result.engineUsed = ccxBinary + " (fallback, CalculiX run failed)";
        return result;
    }

    // Parse results
    result = ParseThermalResults((tempDirPath / "thermal_job.dat").string());
    if (!result.success) {
        ThermalAnalysisResult fallback = SolveAnalytical(config);
        fallback.engineUsed = ccxBinary + " (fallback, result parse failed)";
        fallback.message = result.message;
        return fallback;
    }
    result.engineUsed = ccxBinary;
    
    return result;
}

ThermalAnalysisResult CalculiXThermalSolver::ParseThermalResults(const std::string& outputPath) {
    ThermalAnalysisResult result;
    
    std::ifstream file(outputPath);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open result file";
        return result;
    }

    std::vector<double> temperatures;
    std::string line;
    bool inTempSection = false;

    while (std::getline(file, line)) {
        if (line.find("temperatures") != std::string::npos ||
            line.find("NT") != std::string::npos ||
            line.find("TEMP") != std::string::npos) {
            inTempSection = true;
            // Also accept inline synthetic values such as "NT11 45.2"
            std::istringstream iss(line);
            std::string label;
            double inlineVal = 0.0;
            while (iss >> label) {
                try {
                    size_t pos = 0;
                    double v = std::stod(label, &pos);
                    if (pos == label.size()) {
                        inlineVal = v;
                        temperatures.push_back(inlineVal);
                    }
                } catch (...) {}
            }
            continue;
        }
        if (inTempSection) {
            std::istringstream iss(line);
            int nodeId = 0;
            double temp = 0.0;
            if (iss >> nodeId >> temp) {
                temperatures.push_back(temp);
            } else if (!temperatures.empty() && line.find_first_not_of(" \t\r\n") != std::string::npos) {
                inTempSection = false;
            }
        }
    }

    file.close();

    if (!temperatures.empty()) {
        result.maxTemperature = *std::max_element(temperatures.begin(), temperatures.end());
        result.minTemperature = *std::min_element(temperatures.begin(), temperatures.end());
        result.averageTemperature = 
            std::accumulate(temperatures.begin(), temperatures.end(), 0.0) / temperatures.size();
        result.temperatureRise = result.maxTemperature - result.minTemperature;
        result.temperatureField = temperatures;
        result.isDangerous = result.maxTemperature > 100.0;
        result.success = true;
    } else {
        result.success = false;
        result.message = "No nodal temperatures found in CalculiX output";
    }

    return result;
}

ThermalAnalysisResult CalculiXThermalSolver::SolveAnalytical(const CalculiXThermalConfig& config) {
    ThermalAnalysisResult result;

    // Analytical solution: simple 1D heat conduction
    // Q = k*A*dT/dx  =>  dT = Q*dx / (k*A)
    
    double thickness = 0.1;  // m
    double area = 0.01;      // m² (0.1 x 0.1)
    double k = std::max(0.1, config.thermalConductivity);
    
    double deltaT = (config.heatInput * thickness) / (k * area);
    double maxTemp = config.ambientTemperature + deltaT;

    result.maxTemperature = maxTemp;
    result.minTemperature = config.ambientTemperature;
    result.averageTemperature = (maxTemp + config.ambientTemperature) / 2.0;
    result.temperatureRise = deltaT;
    result.isDangerous = maxTemp > 100.0;
    result.success = true;
    result.engineUsed = "Analytical";
    
    // Generate synthetic temperature field (8 nodes)
    for (int i = 0; i < 8; ++i) {
        // Simple distribution: nodes 1-4 at ambient, nodes 5-8 at max
        if (i < 4) {
            result.temperatureField.push_back(config.ambientTemperature);
        } else {
            result.temperatureField.push_back(maxTemp);
        }
    }

    return result;
}

bool CalculiXThermalSolver::ProbeCalculiX() {
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
