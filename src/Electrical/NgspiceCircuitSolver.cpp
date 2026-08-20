#include "NgspiceCircuitSolver.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <complex>

namespace SZM::Electrical {

std::string NgspiceCircuitSolver::BuildNetlist(const CircuitNetlist& circuit, const NgspiceConfig& config) {
    std::ostringstream netlist;

    // Title line
    netlist << circuit.title << "\n";
    netlist << "* SZM Circuit Analysis - " << static_cast<int>(config.analysisType) << "\n";

    // Add global parameters
    for (const auto& [param_name, param_value] : circuit.parameters) {
        netlist << ".param " << param_name << "=" << param_value << "\n";
    }

    // Add component definitions
    for (const auto& comp : circuit.components) {
        switch (comp.type) {
            case ComponentType::RESISTOR:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " " << comp.value << "\n";
                break;

            case ComponentType::CAPACITOR:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " " << comp.value << "\n";
                break;

            case ComponentType::INDUCTOR:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " " << comp.value << "\n";
                break;

            case ComponentType::VOLTAGE_SOURCE:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " DC " << comp.value << " AC " << comp.value << "\n";
                break;

            case ComponentType::CURRENT_SOURCE:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " DC " << comp.value << " AC " << comp.value << "\n";
                break;

            case ComponentType::DIODE:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " " << comp.model << "\n";
                break;

            case ComponentType::TRANSISTOR_BJT:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " " << comp.node2 << " " << comp.model << "\n";
                break;

            case ComponentType::TRANSISTOR_FET:
                netlist << comp.name << " " << comp.node1 << " " << comp.node2 
                       << " " << comp.node2 << " " << comp.model << "\n";
                break;

            default:
                break;
        }
    }

    // Add model definitions
    for (const auto& [model_name, model_def] : circuit.models) {
        netlist << ".model " << model_name << " " << model_def << "\n";
    }

    // Add analysis commands based on type
    switch (config.analysisType) {
        case AnalysisType::DC:
            netlist << ".op\n";
            break;

        case AnalysisType::AC:
            netlist << ".ac dec " << config.pointsPerDecade << " " 
                   << config.startFrequency_Hz << " " << config.stopFrequency_Hz << "\n";
            break;

        case AnalysisType::TRANSIENT:
            {
                double time_step = config.duration_s / std::max(1, config.timeSteps);
                netlist << ".tran " << time_step << " " << config.duration_s << "\n";
            }
            break;

        case AnalysisType::SWEEP:
            netlist << ".dc " << config.sweepParameter << " " << config.sweepStart 
                   << " " << config.sweepStop << " " << 
                   ((config.sweepStop - config.sweepStart) / std::max(1, config.sweepPoints)) << "\n";
            break;

        case AnalysisType::NOISE:
            netlist << ".noise V(1) " << config.frequency_Hz << " dec 100\n";
            break;
    }

    // Control section
    netlist << ".control\n";
    netlist << "run\n";
    netlist << "print all\n";
    netlist << "quit\n";
    netlist << ".endc\n";
    netlist << ".end\n";

    return netlist.str();
}

CircuitAnalysisResult NgspiceCircuitSolver::RunSimulation(const CircuitNetlist& circuit, 
                                                        const NgspiceConfig& config) {
    CircuitAnalysisResult result;
    result.analysisType = config.analysisType;

    // Check if ngspice is available
    if (!ProbeNgspice()) {
        // Use analytical fallback
        result = SolveAnalytical(circuit, config);
        result.engineUsed = "Analytical fallback (ngspice not found)";
        return result;
    }

    result.engineUsed = m_NgspiceBinary;

    // Generate netlist
    std::string netlist_str = BuildNetlist(circuit, config);

    // Create temporary files
    #include <cstdlib>
    char temp_template[] = "/tmp/szm_circuit_XXXXXX";
    int temp_fd = mkstemp(temp_template);
    if (temp_fd == -1) {
        result.success = false;
        result.message = "Failed to create temporary directory";
        return result;
    }
    close(temp_fd);

    std::string temp_dir(temp_template);
    std::string cir_path = temp_dir + "/circuit.cir";
    std::string out_path = temp_dir + "/circuit.out";

    // Write netlist file
    std::ofstream cir_file(cir_path);
    if (!cir_file.is_open()) {
        result.success = false;
        result.message = "Failed to write circuit file";
        return result;
    }
    cir_file << netlist_str;
    cir_file.close();

    // Execute ngspice
    std::string cmd = m_NgspiceBinary + " -b " + cir_path + " -o " + out_path;
    int ret = system(cmd.c_str());

    if (ret != 0) {
        // Fallback to analytical
        result = SolveAnalytical(circuit, config);
        result.engineUsed += " (fallback, ngspice execution failed)";
        std::remove(cir_path.c_str());
        std::remove(temp_dir.c_str());
        return result;
    }

    // Parse results
    result = ParseResults(out_path, config);
    result.success = true;
    result.engineUsed = m_NgspiceBinary;

    // Cleanup
    std::remove(cir_path.c_str());
    std::remove(out_path.c_str());
    std::remove(temp_dir.c_str());

    return result;
}

CircuitAnalysisResult NgspiceCircuitSolver::ParseResults(const std::string& outputFile, 
                                                       const NgspiceConfig& config) {
    CircuitAnalysisResult result;
    result.analysisType = config.analysisType;

    std::ifstream file(outputFile);
    if (!file.is_open()) {
        result.success = false;
        result.message = "Could not open results file";
        return result;
    }

    std::string line;
    
    // Parse based on analysis type
    if (config.analysisType == AnalysisType::DC) {
        while (std::getline(file, line)) {
            // Look for "V(1) = ..." or "Node 1: ..." patterns
            if (line.find("V(") != std::string::npos || line.find("Node") != std::string::npos) {
                // Extract node voltage
                size_t eq_pos = line.find('=');
                if (eq_pos != std::string::npos) {
                    try {
                        double voltage = std::stod(line.substr(eq_pos + 1));
                        // Extract node number (simplified parsing)
                        int node_num = 1;
                        result.nodeVoltages[node_num] = voltage;
                    } catch (...) {
                        // Skip invalid lines
                    }
                }
            }
        }
    } else if (config.analysisType == AnalysisType::AC) {
        // Parse AC impedance and phase response
        while (std::getline(file, line)) {
            if (line.find("frequency") != std::string::npos || 
                line.find("impedance") != std::string::npos) {
                // Parse frequency response data
            }
        }
    } else if (config.analysisType == AnalysisType::TRANSIENT) {
        // Parse transient waveforms
        while (std::getline(file, line)) {
            if (line.find("time") != std::string::npos || 
                line.find("voltage") != std::string::npos) {
                // Parse time-domain data
            }
        }
    }

    file.close();

    result.success = true;
    return result;
}

CircuitAnalysisResult NgspiceCircuitSolver::SolveAnalytical(const CircuitNetlist& circuit, 
                                                          const NgspiceConfig& config) {
    switch (config.analysisType) {
        case AnalysisType::DC:
            return ComputeDC_Analytical(circuit);
        case AnalysisType::AC:
            return ComputeAC_Analytical(circuit, config);
        case AnalysisType::TRANSIENT:
            return ComputeTransient_Analytical(circuit, config);
        default:
            return CircuitAnalysisResult();
    }
}

CircuitAnalysisResult NgspiceCircuitSolver::ComputeDC_Analytical(const CircuitNetlist& circuit) {
    CircuitAnalysisResult result;
    result.analysisType = AnalysisType::DC;
    result.engineUsed = "Analytical (DC Ohm's Law)";

    // Simple DC analysis: find voltage sources and compute currents via Ohm's law
    double total_voltage = 0.0;
    double total_resistance = 0.0;

    for (const auto& comp : circuit.components) {
        if (comp.type == ComponentType::VOLTAGE_SOURCE) {
            total_voltage += comp.value;
        } else if (comp.type == ComponentType::RESISTOR) {
            total_resistance += comp.value;
        }
    }

    total_resistance = std::max(total_resistance, 0.1);  // Avoid division by zero
    double circuit_current = total_voltage / total_resistance;

    // Store results
    result.nodeVoltages[0] = 0.0;  // Ground reference
    result.nodeVoltages[1] = total_voltage;  // Supply voltage
    result.nodeVoltages[2] = total_voltage / 2.0;  // Midpoint (simplified)

    for (const auto& comp : circuit.components) {
        if (comp.type == ComponentType::RESISTOR) {
            result.componentCurrents[comp.name] = circuit_current;
            result.componentPowers[comp.name] = circuit_current * circuit_current * comp.value;
        }
    }

    result.success = true;
    return result;
}

CircuitAnalysisResult NgspiceCircuitSolver::ComputeAC_Analytical(const CircuitNetlist& circuit, 
                                                               const NgspiceConfig& config) {
    CircuitAnalysisResult result;
    result.analysisType = AnalysisType::AC;
    result.engineUsed = "Analytical (AC Impedance)";
    result.frequency_Hz = config.frequency_Hz;

    double R = 0.0, L = 0.0, C = 0.0;

    for (const auto& comp : circuit.components) {
        if (comp.type == ComponentType::RESISTOR) {
            R += comp.value;
        } else if (comp.type == ComponentType::INDUCTOR) {
            L += comp.value;
        } else if (comp.type == ComponentType::CAPACITOR) {
            C += comp.value;
        }
    }

    // Compute AC impedance
    double XL = ComputeReactance_L(config.frequency_Hz, L);
    double XC = ComputeReactance_C(config.frequency_Hz, C);
    double Z = ComputeImpedance(R, XL - XC);
    double phase = std::atan2(XL - XC, R) * 180.0 / M_PI;

    result.phaseShift_degrees = phase;
    std::complex<double> impedance(R, XL - XC);
    result.acImpedances[1] = impedance;

    // Compute resonant frequency: f = 1/(2π√LC)
    if (L > 0.0 && C > 0.0) {
        result.resonantFrequency_Hz = 1.0 / (2.0 * M_PI * std::sqrt(L * C));
    }

    result.success = true;
    return result;
}

CircuitAnalysisResult NgspiceCircuitSolver::ComputeTransient_Analytical(const CircuitNetlist& circuit, 
                                                                       const NgspiceConfig& config) {
    CircuitAnalysisResult result;
    result.analysisType = AnalysisType::TRANSIENT;
    result.engineUsed = "Analytical (RC Time Constant)";

    // Simple RC charging curve: V(t) = V_s * (1 - exp(-t/RC))
    double V_source = 0.0;
    double R = 0.0, C = 0.0;

    for (const auto& comp : circuit.components) {
        if (comp.type == ComponentType::VOLTAGE_SOURCE) {
            V_source = comp.value;
        } else if (comp.type == ComponentType::RESISTOR) {
            R += comp.value;
        } else if (comp.type == ComponentType::CAPACITOR) {
            C += comp.value;
        }
    }

    R = std::max(R, 1.0);
    C = std::max(C, 1e-9);
    double tau = R * C;  // Time constant

    // Generate transient response
    for (int i = 0; i <= config.timeSteps; ++i) {
        double t = config.duration_s * i / std::max(1, config.timeSteps);
        double V_t = V_source * (1.0 - std::exp(-t / tau));
        
        result.timePoints.push_back(t);
        result.voltageWaveforms[1].push_back(V_t);
    }

    result.settlingTime_s = 5.0 * tau;  // Time to reach 99.3% final value
    result.success = true;

    return result;
}

std::vector<std::string> NgspiceCircuitSolver::DetectFaults(const CircuitNetlist& circuit, 
                                                          const CircuitAnalysisResult& results) {
    std::vector<std::string> faults;

    // Check for component faults
    for (const auto& comp : circuit.components) {
        if (comp.isFaulty) {
            faults.push_back(comp.name + " is marked as faulty");
        }
    }

    // Check for overcurrent/overvoltage conditions
    for (const auto& [node, voltage] : results.nodeVoltages) {
        if (std::abs(voltage) > 1000.0) {
            faults.push_back("Overvoltage detected at node " + std::to_string(node));
        }
    }

    for (const auto& [comp_name, current] : results.componentCurrents) {
        if (std::abs(current) > 10.0) {
            faults.push_back("Overcurrent detected in " + comp_name);
        }
    }

    return faults;
}

bool NgspiceCircuitSolver::ProbeNgspice() {
    std::vector<std::string> ngspice_bins = {"ngspice", "ngspice-32", "ngspice-31"};
    
    for (const auto& bin : ngspice_bins) {
        int ret = system(("which " + bin + " > /dev/null 2>&1").c_str());
        if (ret == 0) {
            m_NgspiceBinary = bin;
            return true;
        }
    }
    
    return false;
}

double NgspiceCircuitSolver::ComputeReactance_L(double frequency_Hz, double inductance_H) {
    return 2.0 * M_PI * frequency_Hz * inductance_H;
}

double NgspiceCircuitSolver::ComputeReactance_C(double frequency_Hz, double capacitance_F) {
    if (frequency_Hz == 0.0 || capacitance_F == 0.0) return 1e10;
    return 1.0 / (2.0 * M_PI * frequency_Hz * capacitance_F);
}

double NgspiceCircuitSolver::ComputeImpedance(double resistance, double reactance) {
    return std::sqrt(resistance * resistance + reactance * reactance);
}

} // namespace SZM::Electrical
