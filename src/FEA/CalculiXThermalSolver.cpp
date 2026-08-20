#include "CalculiXThermalSolver.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <subprocess.h> // Requires external subprocess library

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
    inp << "*INITIAL CONDITIONS\n";
    for (int i = 1; i <= 8; ++i) {
        inp << " " << i << ", " << config.ambientTemperature << "\n";
    }

    // Boundary conditions (fixed temperature at node 1)
    inp << "*BOUNDARY\n";
    inp << " 1, 11, " << config.ambientTemperature << "\n";

    // Step and loading
    double heatPerNode = config.heatInput / 4.0;  // Distribute over 4 top nodes
    
    if (config.mode == CalculiXThermalConfig::TRANSIENT) {
        inp << "*STEP, INC=" << config.timeSteps << "\n";
        inp << "*HEAT TRANSFER, TRANSIENT\n";
        inp << " " << (config.duration / config.timeSteps) << ", " << config.duration << "\n";
    } else {
        inp << "*STEP\n";
        inp << "*HEAT TRANSFER\n";
    }

    // Convection boundary condition
    inp << "*BOUNDARY CONVECTION\n";
    inp << " P1, 10, " << config.convectionCoefficient << ", " << config.ambientTemperature << "\n";

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
        // Check if binary exists in PATH
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
    std::string inputContent = GenerateThermalInputFile(config);

    // Create temporary directory and write input file
    // (In production, use boost::filesystem or std::filesystem)
    std::string tempDir = "/tmp/szm_thermal_analysis";
    system("mkdir -p " + tempDir);
    
    std::string inputPath = tempDir + "/thermal_job.inp";
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
        // Fall back to analytical
        result = SolveAnalytical(config);
        result.engineUsed += " (fallback, CalculiX run failed)";
        return result;
    }

    // Parse results
    result = ParseThermalResults(tempDir + "/thermal_job.dat");
    result.engineUsed = ccxBinary;
    result.success = true;
    
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

    // Simple parsing: look for lines with temperature values
    // (Real parsing would be more sophisticated)
    while (std::getline(file, line)) {
        // Look for temperature values in output
        if (line.find("NT") != std::string::npos || line.find("TEMP") != std::string::npos) {
            try {
                size_t pos = line.find_last_of(" \t");
                if (pos != std::string::npos) {
                    double temp = std::stod(line.substr(pos + 1));
                    temperatures.push_back(temp);
                }
            } catch (...) {
                // Skip invalid lines
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
        result.isDangerous = result.maxTemperature > 100.0;  // Arbitrary threshold
        result.success = true;
    }

    return result;
}

ThermalAnalysisResult CalculiXThermalSolver::SolveAnalytical(const CalculiXThermalConfig& config) {
    ThermalAnalysisResult result;

    // Analytical solution: simple 1D heat conduction
    // Q = k*A*dT/dx  =>  dT = Q*dx / (k*A)
    
    double thickness = 0.1;  // m
    double area = 0.01;      // m² (0.1 x 0.1)
    
    double deltaT = (config.heatInput * thickness) / (config.thermalConductivity * area);
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
