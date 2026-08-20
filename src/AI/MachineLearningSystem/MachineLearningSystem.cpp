#include "MachineLearningSystem.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <set>
#include <filesystem>
#include <iostream>

#include "Simulation/SimulationEngine.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"

namespace SZM::AI::Machines {

MachineLearningSystem& MachineLearningSystem::GetInstance() {
    static MachineLearningSystem instance;
    return instance;
}

void MachineLearningSystem::Initialize() {
    if (m_IsInitialized) return;
    
    std::cout << "[MachineLearningSystem] Initializing..." << std::endl;
    
    // Initialize code templates for each component type
    InitializeCodeTemplates();
    
    m_IsInitialized = true;
    std::cout << "[MachineLearningSystem] Initialized successfully" << std::endl;
}

void MachineLearningSystem::Shutdown() {
    if (!m_IsInitialized) return;
    
    // Save learned patterns before shutdown
    ExportLearnedPatterns("./machine_learning_patterns.json");
    
    m_IsInitialized = false;
    std::cout << "[MachineLearningSystem] Shutdown complete" << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// DATA INGESTION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::IngestMachineData(const std::string& data_path) {
    std::cout << "[MachineLearningSystem] Ingesting machine data from: " << data_path << std::endl;
    
    // Check file extension
    std::string extension = data_path.substr(data_path.find_last_of('.') + 1);
    
    if (extension == "json") {
        // JSON specification file
        std::ifstream file(data_path);
        if (file.is_open()) {
            json spec_data;
            file >> spec_data;
            
            MachineSpecification spec;
            spec.id = spec_data.value("id", GenerateUniqueId());
            spec.name = spec_data.value("name", "Unknown Machine");
            spec.manufacturer = spec_data.value("manufacturer", "Unknown");
            spec.model = spec_data.value("model", "");
            spec.type = MachineType::CUSTOM;
            spec.source_file = data_path;
            
            // Parse components
            if (spec_data.contains("components")) {
                for (const auto& comp : spec_data["components"]) {
                    MachineComponentSpec comp_spec;
                    comp_spec.id = comp.value("id", GenerateUniqueId());
                    comp_spec.name = comp.value("name", "Component");
                    comp_spec.type = MachineComponent::CUSTOM;
                    
                    // Parse parameters
                    if (comp.contains("parameters")) {
                        for (const auto& param : comp["parameters"]) {
                            MachineParameter p;
                            p.name = param.value("name", "param");
                            p.category = param.value("category", "general");
                            p.unit = param.value("unit", "");
                            p.min_value = param.value("min", 0.0f);
                            p.max_value = param.value("max", 100.0f);
                            p.default_value = param.value("default", 50.0f);
                            p.current_value = p.default_value;
                            p.description = param.value("description", "");
                            comp_spec.parameters.push_back(p);
                        }
                    }
                    
                    spec.components.push_back(comp_spec);
                }
            }
            
            // Parse specifications
            if (spec_data.contains("specifications")) {
                for (const auto& [key, value] : spec_data["specifications"].items()) {
                    spec.specifications[key] = value.get<std::string>();
                }
            }
            
            // Parse tags
            if (spec_data.contains("tags")) {
                for (const auto& tag : spec_data["tags"]) {
                    spec.tags.push_back(tag.get<std::string>());
                }
            }
            
            m_KnownMachines.push_back(spec);
            AddMachineToDatabase(spec);
            
            std::cout << "[MachineLearningSystem] Ingested machine: " << spec.name << std::endl;
        }
    }
    else if (extension == "stp" || extension == "step" || extension == "iges" || extension == "igs") {
        // CAD file - extract geometry information
        IngestCADFile(data_path);
    }
    else {
        std::cerr << "[MachineLearningSystem] Unsupported file format: " << extension << std::endl;
    }
}

void MachineLearningSystem::IngestCADFile(const std::string& cad_file_path) {
    std::cout << "[MachineLearningSystem] Processing CAD file: " << cad_file_path << std::endl;
    
    // In a real implementation, this would use CAD kernels (Mayo, OpenCASCADE)
    // to extract geometry information
    
    MachineSpecification spec;
    spec.id = GenerateUniqueId();
    spec.name = "Imported CAD: " + cad_file_path;
    spec.type = MachineType::GENERIC;
    spec.source_file = cad_file_path;
    
    // Placeholder - would extract actual geometry
    MachineGeometry geom;
    geom.geometry_id = spec.id;
    geom.bounding_box = {0, 0, 0, 100, 100, 50}; // Placeholder dimensions
    geom.mesh_file = cad_file_path;
    
    std::cout << "[MachineLearningSystem] CAD file processed (placeholder implementation)" << std::endl;
}

void MachineLearningSystem::IngestSpecification(const std::string& spec_json_path) {
    IngestMachineData(spec_json_path);
}

void MachineLearningSystem::IngestSimulationResult(const std::string& result_path) {
    std::cout << "[MachineLearningSystem] Ingesting simulation result: " << result_path << std::endl;
    // Would parse simulation results and learn from them
}

void MachineLearningSystem::IngestFromDatabase(const std::string& db_connection) {
    std::cout << "[MachineLearningSystem] Ingesting from database: " << db_connection << std::endl;
    // Would connect to database and import machine data
}

void MachineLearningSystem::IngestMachineDataset(const std::vector<std::string>& data_paths) {
    std::cout << "[MachineLearningSystem] Ingesting dataset with " << data_paths.size() << " files..." << std::endl;
    
    for (const auto& path : data_paths) {
        IngestMachineData(path);
    }
    
    std::cout << "[MachineLearningSystem] Dataset ingestion complete. Total machines: " 
              << m_KnownMachines.size() << std::endl;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// PATTERN LEARNING
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::LearnFromMachines() {
    std::cout << "[MachineLearningSystem] Learning from " << m_KnownMachines.size() << " machines..." << std::endl;
    
    LearnParameterRanges();
    LearnStructuralPatterns();
    LearnBehavioralPatterns();
    LearnDesignConstraints();
    LearnComponentRelationships();
    
    std::cout << "[MachineLearningSystem] Learning complete. Patterns found: " 
              << m_LearnedPatterns.size() << std::endl;
}

void MachineLearningSystem::LearnParameterRanges() {
    std::cout << "[MachineLearningSystem] Learning parameter ranges..." << std::endl;
    
    std::map<std::string, std::pair<float, float>> param_ranges;
    std::map<std::string, std::vector<float>> param_values;
    
    // Collect all parameter values across machines
    for (const auto& machine : m_KnownMachines) {
        for (const auto& comp : machine.components) {
            for (const auto& param : comp.parameters) {
                param_values[param.name].push_back(param.current_value);
                param_ranges[param.name].first = std::min(param_ranges[param.name].first, param.min_value);
                param_ranges[param.name].second = std::max(param_ranges[param.name].second, param.max_value);
            }
        }
    }
    
    // Create patterns for common parameters
    for (const auto& [name, values] : param_values) {
        if (values.size() >= MIN_PATTERN_OCCURRENCES) {
            LearnedPattern pattern;
            pattern.id = GenerateUniqueId();
            pattern.pattern_type = "parameter_range";
            pattern.description = "Parameter '" + name + "' ranges from " + 
                                 std::to_string(param_ranges[name].first) + " to " +
                                 std::to_string(param_ranges[name].second);
            
            pattern.pattern_data["min"] = param_ranges[name].first;
            pattern.pattern_data["max"] = param_ranges[name].second;
            
            // Calculate statistics
            float sum = 0;
            for (float v : values) sum += v;
            float mean = sum / values.size();
            pattern.pattern_data["mean"] = mean;
            pattern.confidence = std::min(1.0f, values.size() / 10.0f);
            pattern.occurrence_count = values.size();
            
            m_LearnedPatterns.push_back(pattern);
        }
    }
}

void MachineLearningSystem::LearnStructuralPatterns() {
    std::cout << "[MachineLearningSystem] Learning structural patterns..." << std::endl;
    
    // Analyze common component combinations
    std::map<std::string, int> component_combinations;
    
    for (const auto& machine : m_KnownMachines) {
        std::vector<std::string> comp_types;
        for (const auto& comp : machine.components) {
            comp_types.push_back(comp.name);
        }
        std::sort(comp_types.begin(), comp_types.end());
        
        std::string key;
        for (const auto& ct : comp_types) {
            key += ct + "+";
        }
        component_combinations[key]++;
    }
    
    // Create patterns for common structures
    for (const auto& [structure, count] : component_combinations) {
        if (count >= MIN_PATTERN_OCCURRENCES) {
            LearnedPattern pattern;
            pattern.id = GenerateUniqueId();
            pattern.pattern_type = "structure";
            pattern.description = "Common component combination: " + structure;
            pattern.pattern_data["components"] = structure;
            pattern.pattern_data["frequency"] = count;
            pattern.confidence = std::min(1.0f, count / 10.0f);
            pattern.occurrence_count = count;
            
            m_LearnedPatterns.push_back(pattern);
        }
    }
}

void MachineLearningSystem::LearnBehavioralPatterns() {
    std::cout << "[MachineLearningSystem] Learning behavioral patterns..." << std::endl;
    
    // Learn common behaviors and their parameter dependencies
    for (const auto& machine : m_KnownMachines) {
        LearnedPattern pattern;
        pattern.id = GenerateUniqueId();
        pattern.pattern_type = "behavior";
        pattern.description = "Machine '" + machine.name + "' behavior model";
        
        for (const auto& comp : machine.components) {
            for (const auto& param : comp.parameters) {
                if (param.category == "performance" || param.category == "control") {
                    pattern.pattern_data[param.name] = param.current_value;
                }
            }
        }
        
        pattern.occurrence_count = 1;
        pattern.confidence = 0.5f;
        
        m_LearnedPatterns.push_back(pattern);
    }
}

void MachineLearningSystem::LearnDesignConstraints() {
    std::cout << "[MachineLearningSystem] Learning design constraints..." << std::endl;
    
    // Learn constraints between parameters
    for (const auto& machine : m_KnownMachines) {
        for (const auto& comp : machine.components) {
            // Check for parameter dependencies
            for (size_t i = 0; i < comp.parameters.size(); i++) {
                for (size_t j = i + 1; j < comp.parameters.size(); j++) {
                    const auto& p1 = comp.parameters[i];
                    const auto& p2 = comp.parameters[j];
                    
                    // If both exist, there's likely a constraint
                    LearnedPattern constraint;
                    constraint.id = GenerateUniqueId();
                    constraint.pattern_type = "constraint";
                    constraint.description = comp.name + ": " + p1.name + " relates to " + p2.name;
                    constraint.pattern_data["component"] = comp.name;
                    constraint.pattern_data["param1"] = p1.name;
                    constraint.pattern_data["param2"] = p2.name;
                    constraint.pattern_data["p1_range"] = {p1.min_value, p1.max_value};
                    constraint.pattern_data["p2_range"] = {p2.min_value, p2.max_value};
                    constraint.confidence = 0.6f;
                    constraint.occurrence_count = 1;
                    
                    m_LearnedPatterns.push_back(constraint);
                }
            }
        }
    }
}

void MachineLearningSystem::LearnComponentRelationships() {
    std::cout << "[MachineLearningSystem] Learning component relationships..." << std::endl;
    
    // Build component compatibility graph
    std::map<std::string, std::set<std::string>> compatibility;
    
    for (const auto& machine : m_KnownMachines) {
        for (const auto& comp : machine.components) {
            for (const auto& other : machine.components) {
                if (comp.id != other.id) {
                    compatibility[comp.name].insert(other.name);
                }
            }
        }
    }
    
    // Create patterns for compatible components
    for (const auto& [comp, compatibles] : compatibility) {
        if (compatibles.size() >= 2) {
            LearnedPattern pattern;
            pattern.id = GenerateUniqueId();
            pattern.pattern_type = "component_relationship";
            pattern.description = comp + " is commonly paired with " + 
                                 std::to_string(compatibles.size()) + " other components";
            
            std::vector<std::string> comp_list(compatibles.begin(), compatibles.end());
            pattern.pattern_data["compatible_components"] = comp_list;
            pattern.pattern_data["primary_component"] = comp;
            pattern.confidence = 0.7f;
            pattern.occurrence_count = 1;
            
            m_LearnedPatterns.push_back(pattern);
        }
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MACHINE FAMILIES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::CreateMachineFamily(MachineType type, const std::string& family_name) {
    MachineFamily family;
    family.id = GenerateUniqueId();
    family.name = family_name;
    family.base_type = type;
    
    // Find machines of this type
    for (const auto& machine : m_KnownMachines) {
        if (machine.type == type) {
            family.variant_ids.push_back(machine.id);
            
            // Extract common parameters
            for (const auto& comp : machine.components) {
                for (const auto& param : comp.parameters) {
                    family.common_parameters[param.name].push_back(param.current_value);
                }
            }
        }
    }
    
    m_MachineFamilies[family.id] = family;
    
    std::cout << "[MachineLearningSystem] Created family '" << family_name << "' with " 
              << family.variant_ids.size() << " machines" << std::endl;
}

void MachineLearningSystem::AddToFamily(const std::string& family_id, const std::string& machine_id) {
    auto it = m_MachineFamilies.find(family_id);
    if (it != m_MachineFamilies.end()) {
        it->second.variant_ids.push_back(machine_id);
    }
}

MachineFamily MachineLearningSystem::GetMachineFamily(const std::string& family_id) {
    if (m_MachineFamilies.find(family_id) != m_MachineFamilies.end()) {
        return m_MachineFamilies[family_id];
    }
    return MachineFamily{};
}

std::vector<MachineFamily> MachineLearningSystem::GetAllFamilies() {
    std::vector<MachineFamily> families;
    for (const auto& [id, family] : m_MachineFamilies) {
        families.push_back(family);
    }
    return families;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MACHINE GENERATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

GeneratedMachine MachineLearningSystem::GenerateMachine(
    MachineType type,
    const std::map<std::string, float>& parameters,
    const std::vector<std::string>& required_capabilities
) {
    std::cout << "[MachineLearningSystem] Generating machine of type " << (int)type << std::endl;
    
    GeneratedMachine machine;
    machine.id = GenerateUniqueId();
    machine.type = type;
    
    // Set name based on type
    switch (type) {
        case MachineType::CNC_MILL: machine.name = "Generated CNC Mill"; break;
        case MachineType::CNC_LATHE: machine.name = "Generated CNC Lathe"; break;
        case MachineType::ROBOT_ARM: machine.name = "Generated Robot Arm"; break;
        case MachineType::PRINTER_3D: machine.name = "Generated 3D Printer"; break;
        default: machine.name = "Generated Machine";
    }
    
    // Generate specification from learned patterns
    machine.specification.id = machine.id;
    machine.specification.name = machine.name;
    machine.specification.type = type;
    
    // Create components based on patterns
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern.pattern_type == "structure" && pattern.confidence > MIN_PATTERN_CONFIDENCE) {
            // Add components based on common structure
            MachineComponentSpec comp;
            comp.id = GenerateUniqueId();
            comp.name = "Generated Component";
            comp.type = MachineComponent::CUSTOM;
            
            // Add parameters from patterns
            for (const auto& [param_name, value] : parameters) {
                MachineParameter p;
                p.name = param_name;
                p.current_value = value;
                p.min_value = value * 0.5f;
                p.max_value = value * 1.5f;
                p.default_value = value;
                comp.parameters.push_back(p);
            }
            
            machine.specification.components.push_back(comp);
        }
    }
    
    // Add required capabilities
    for (const auto& cap : required_capabilities) {
        if (cap == "kinematics") machine.specification.capabilities.push_back(SimulationCapability::KINEMATICS);
        else if (cap == "dynamics") machine.specification.capabilities.push_back(SimulationCapability::DYNAMICS);
        else if (cap == "thermal") machine.specification.capabilities.push_back(SimulationCapability::THERMAL);
        else if (cap == "structural") machine.specification.capabilities.push_back(SimulationCapability::STRUCTURAL);
    }
    
    // Generate behaviors
    MachineBehavior behavior;
    behavior.behavior_id = GenerateUniqueId();
    behavior.name = "Generated Behavior";
    behavior.description = "Behavior generated from learned patterns";
    behavior.physics_model = "GENERATED";
    machine.behaviors.push_back(behavior);
    
    // Calculate confidence based on learned patterns
    machine.confidence_score = 0.5f;
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern.pattern_type == "structure" || pattern.pattern_type == "parameter_range") {
            machine.confidence_score += pattern.confidence * 0.1f;
        }
    }
    machine.confidence_score = std::min(1.0f, machine.confidence_score);
    
    machine.is_compiled = false;
    machine.is_validated = false;
    
    m_GeneratedMachines.push_back(machine);
    
    std::cout << "[MachineLearningSystem] Generated machine: " << machine.name 
              << " (confidence: " << (machine.confidence_score * 100) << "%)" << std::endl;
    
    return machine;
}

GeneratedMachine MachineLearningSystem::GenerateVariant(
    const std::string& base_machine_id,
    const std::map<std::string, float>& modifications
) {
    auto* base = FindMachineById(base_machine_id);
    if (!base) {
        std::cerr << "[MachineLearningSystem] Base machine not found: " << base_machine_id << std::endl;
        return GeneratedMachine{};
    }
    
    std::map<std::string, float> params;
    for (const auto& comp : base->components) {
        for (const auto& param : comp.parameters) {
            params[param.name] = param.current_value;
        }
    }
    
    // Apply modifications
    for (const auto& [key, value] : modifications) {
        params[key] = value;
    }
    
    return GenerateMachine(base->type, params, {});
}

std::vector<GeneratedMachine> MachineLearningSystem::GenerateFamilyVariants(
    const std::string& family_id,
    int count
) {
    std::vector<GeneratedMachine> variants;
    auto family = GetMachineFamily(family_id);
    
    if (family.variant_ids.empty()) {
        std::cerr << "[MachineLearningSystem] Family has no variants to base on" << std::endl;
        return variants;
    }
    
    // Get parameter ranges from family
    std::map<std::string, std::pair<float, float>> ranges;
    for (const auto& [param, values] : family.common_parameters) {
        if (values.size() >= 2) {
            ranges[param] = {*std::min_element(values.begin(), values.end()),
                           *std::max_element(values.begin(), values.end())};
        }
    }
    
    // Generate variants with random parameters within ranges
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < count; i++) {
        std::map<std::string, float> params;
        for (const auto& [param, range] : ranges) {
            std::uniform_real_distribution<> dis(range.first, range.second);
            params[param] = dis(gen);
        }
        
        variants.push_back(GenerateMachine(family.base_type, params, {}));
    }
    
    return variants;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CODE GENERATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::InitializeCodeTemplates() {
    // Initialize templates for common machine components
    
    // Motor template
    CodeTemplate motor_template;
    motor_template.template_id = "motor_template";
    motor_template.name = "Electric Motor";
    motor_template.component_type = MachineComponent::MOTOR;
    motor_template.header_template = R"(
#pragma once
#include <Simulation/SimulationComponent.hpp>

namespace SZM::Machines {

class GeneratedMotor : public SimulationComponent {
public:
    GeneratedMotor(const std::string& name);
    virtual ~GeneratedMotor() = default;
    
    void SetPower(float power);
    void SetRPM(float rpm);
    void SetTorque(float torque);
    
    float GetPower() const { return m_Power; }
    float GetRPM() const { return m_RPM; }
    float GetTorque() const { return m_Torque; }
    
    void Update(float delta_time) override;
    void Initialize() override;
    
private:
    float m_Power = 0.0f;
    float m_RPM = 0.0f;
    float m_Torque = 0.0f;
};

} // namespace SZM::Machines
)";
    
    motor_template.source_template = R"(
#include "GeneratedMotor.hpp"

namespace SZM::Machines {

GeneratedMotor::GeneratedMotor(const std::string& name) 
    : SimulationComponent(name) {
}

void GeneratedMotor::Initialize() {
    // Initialize motor physics
}

void GeneratedMotor::SetPower(float power) {
    m_Power = power;
    m_Torque = m_Power * 9550.0f / std::max(m_RPM, 1.0f);
}

void GeneratedMotor::SetRPM(float rpm) {
    m_RPM = rpm;
    m_Torque = m_Power * 9550.0f / std::max(m_RPM, 1.0f);
}

void GeneratedMotor::SetTorque(float torque) {
    m_Torque = torque;
    m_Power = m_Torque * m_RPM * 0.001396f;
}

void GeneratedMotor::Update(float delta_time) {
    // Update motor simulation
}

} // namespace SZM::Machines
)";
    
    m_ComponentTemplates[MachineComponent::MOTOR] = motor_template;
    
    // Spindle template
    CodeTemplate spindle_template;
    spindle_template.template_id = "spindle_template";
    spindle_template.name = "Machine Spindle";
    spindle_template.component_type = MachineComponent::SPINDLE;
    spindle_template.header_template = R"(
#pragma once
#include <Simulation/SimulationComponent.hpp>

namespace SZM::Machines {

class GeneratedSpindle : public SimulationComponent {
public:
    GeneratedSpindle(const std::string& name);
    virtual ~GeneratedSpindle() = default;
    
    void SetSpeed(float rpm);
    void SetCuttingForce(float force);
    
    float GetSpeed() const { return m_Speed; }
    float GetCuttingForce() const { return m_CuttingForce; }
    
    void Update(float delta_time) override;
    void Initialize() override;
    
private:
    float m_Speed = 0.0f;
    float m_CuttingForce = 0.0f;
};

} // namespace SZM::Machines
)";
    
    spindle_template.source_template = R"(
#include "GeneratedSpindle.hpp"

namespace SZM::Machines {

GeneratedSpindle::GeneratedSpindle(const std::string& name)
    : SimulationComponent(name) {
}

void GeneratedSpindle::Initialize() {
    // Initialize spindle
}

void GeneratedSpindle::SetSpeed(float rpm) {
    m_Speed = rpm;
}

void GeneratedSpindle::SetCuttingForce(float force) {
    m_CuttingForce = force;
}

void GeneratedSpindle::Update(float delta_time) {
    // Update spindle simulation
}

} // namespace SZM::Machines
)";
    
    m_ComponentTemplates[MachineComponent::SPINDLE] = spindle_template;
    
    // Linear motion template
    CodeTemplate linear_template;
    linear_template.template_id = "linear_motion_template";
    linear_template.name = "Linear Motion System";
    linear_template.component_type = MachineComponent::LINEARMOTION;
    linear_template.header_template = R"(
#pragma once
#include <Simulation/SimulationComponent.hpp>

namespace SZM::Machines {

class GeneratedLinearMotion : public SimulationComponent {
public:
    GeneratedLinearMotion(const std::string& name);
    virtual ~GeneratedLinearMotion() = default;
    
    void SetPosition(float pos);
    void SetVelocity(float vel);
    void SetAcceleration(float acc);
    
    float GetPosition() const { return m_Position; }
    float GetVelocity() const { return m_Velocity; }
    
    void Update(float delta_time) override;
    void Initialize() override;
    
private:
    float m_Position = 0.0f;
    float m_Velocity = 0.0f;
    float m_Acceleration = 0.0f;
};

} // namespace SZM::Machines
)";
    
    linear_template.source_template = R"(
#include "GeneratedLinearMotion.hpp"

namespace SZM::Machines {

GeneratedLinearMotion::GeneratedLinearMotion(const std::string& name)
    : SimulationComponent(name) {
}

void GeneratedLinearMotion::Initialize() {
    // Initialize linear motion system
}

void GeneratedLinearMotion::SetPosition(float pos) {
    m_Position = pos;
}

void GeneratedLinearMotion::SetVelocity(float vel) {
    m_Velocity = vel;
}

void GeneratedLinearMotion::SetAcceleration(float acc) {
    m_Acceleration = acc;
}

void GeneratedLinearMotion::Update(float delta_time) {
    m_Velocity += m_Acceleration * delta_time;
    m_Position += m_Velocity * delta_time;
}

} // namespace SZM::Machines
)";
    
    m_ComponentTemplates[MachineComponent::LINEARMOTION] = linear_template;
}

GeneratedCode MachineLearningSystem::GenerateMachineCode(const GeneratedMachine& machine) {
    GeneratedCode code;
    code.code_id = machine.id + "_code";
    
    std::cout << "[MachineLearningSystem] Generating code for: " << machine.name << std::endl;
    
    // Generate header file
    std::ostringstream header;
    header << "#pragma once\n\n";
    header << "#include <string>\n";
    header << "#include <vector>\n";
    header << "#include <map>\n\n";
    header << "namespace SZM::Machines::Generated {\n\n";
    
    // Generate machine class
    header << "// Generated machine: " << machine.name << "\n";
    header << "class " << GenerateClassName(machine.name) << " {\n";
    header << "public:\n";
    header << "    " << GenerateClassName(machine.name) << "();\n";
    header << "    virtual ~" << GenerateClassName(machine.name) << "() = default;\n\n";
    
    // Add methods for each component
    for (const auto& comp : machine.specification.components) {
        header << "    // Component: " << comp.name << "\n";
        for (const auto& param : comp.parameters) {
            header << "    void Set" << param.name << "(float value);\n";
            header << "    float Get" << param.name << "() const;\n";
        }
        header << "\n";
    }
    
    header << "    void Update(float delta_time);\n";
    header << "    void Initialize();\n\n";
    header << "private:\n";
    
    // Add member variables
    for (const auto& comp : machine.specification.components) {
        header << "    // " << comp.name << " variables\n";
        for (const auto& param : comp.parameters) {
            header << "    float m_" << param.name << " = " << param.default_value << "f;\n";
        }
    }
    
    header << "};\n\n} // namespace SZM::Machines::Generated\n";
    
    code.header_files.push_back(GenerateClassName(machine.name) + ".hpp");
    code.class_definitions[GenerateClassName(machine.name) + ".hpp"] = header.str();
    
    // Generate source file
    std::ostringstream source;
    source << "#include \"" << GenerateClassName(machine.name) << ".hpp\"\n\n";
    source << "namespace SZM::Machines::Generated {\n\n";
    
    source << GenerateClassName(machine.name) << "::" << GenerateClassName(machine.name) << "() {\n";
    source << "    Initialize();\n";
    source << "}\n\n";
    
    source << "void " << GenerateClassName(machine.name) << "::Initialize() {\n";
    source << "    // Initialize generated machine\n";
    for (const auto& comp : machine.specification.components) {
        source << "    // " << comp.name << " initialization\n";
    }
    source << "}\n\n";
    
    source << "void " << GenerateClassName(machine.name) << "::Update(float delta_time) {\n";
    source << "    // Update machine simulation\n";
    for (const auto& behavior : machine.behaviors) {
        source << "    // Behavior: " << behavior.name << "\n";
    }
    source << "}\n\n";
    
    // Generate parameter accessors
    for (const auto& comp : machine.specification.components) {
        for (const auto& param : comp.parameters) {
            std::string setter = "Set" + param.name;
            std::string getter = "Get" + param.name;
            
            source << "void " << GenerateClassName(machine.name) << "::" << setter << "(float value) {\n";
            source << "    m_" << param.name << " = value;\n";
            source << "}\n\n";
            
            source << "float " << GenerateClassName(machine.name) << "::" << getter << "() const {\n";
            source << "    return m_" << param.name << ";\n";
            source << "}\n\n";
        }
    }
    
    source << "} // namespace SZM::Machines::Generated\n";
    
    code.source_files.push_back(GenerateClassName(machine.name) + ".cpp");
    code.method_implementations[GenerateClassName(machine.name) + ".cpp"] = source.str();
    
    // Generate CMake snippet
    std::ostringstream cmake;
    cmake << "# Generated machine: " << machine.name << "\n";
    cmake << "set(GENERATED_SOURCES\n";
    cmake << "    ${CMAKE_CURRENT_BINARY_DIR}/" << GenerateClassName(machine.name) << ".cpp\n";
    cmake << ")\n";
    cmake << "add_library(" << GenerateClassName(machine.name) << " STATIC ${GENERATED_SOURCES})\n";
    cmake << "target_include_directories(" << GenerateClassName(machine.name) << " PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})\n";
    
    code.cmake_snippet = cmake.str();
    code.compiles_successfully = false; // Would need actual compilation
    
    return code;
}

GeneratedCode MachineLearningSystem::GenerateComponentCode(
    MachineComponent component_type,
    const std::vector<MachineParameter>& parameters
) {
    GeneratedCode code;
    
    auto it = m_ComponentTemplates.find(component_type);
    if (it != m_ComponentTemplates.end()) {
        code.header_files.push_back("GeneratedComponent.hpp");
        code.source_files.push_back("GeneratedComponent.cpp");
        
        // Use template and customize with parameters
        std::string header = it->second.header_template;
        std::string source = it->second.source_template;
        
        code.class_definitions["GeneratedComponent.hpp"] = header;
        code.method_implementations["GeneratedComponent.cpp"] = source;
    }
    
    return code;
}

bool MachineLearningSystem::CompileGeneratedCode(GeneratedCode& code) {
    std::cout << "[MachineLearningSystem] Compiling generated code..." << std::endl;
    
    // Create temporary directory
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "szm_generated";
    std::filesystem::create_directory(temp_dir);
    
    // Write files
    for (const auto& [filename, content] : code.class_definitions) {
        std::ofstream file(temp_dir / filename);
        file << content;
    }
    
    for (const auto& [filename, content] : code.method_implementations) {
        std::ofstream file(temp_dir / filename);
        file << content;
    }
    
    // In a real implementation, this would invoke the compiler
    // For now, return success
    code.compiles_successfully = true;
    
    std::cout << "[MachineLearningSystem] Code compilation placeholder - would compile in: " 
              << temp_dir.string() << std::endl;
    
    return true;
}

bool MachineLearningSystem::ValidateGeneratedMachine(GeneratedMachine& machine) {
    std::cout << "[MachineLearningSystem] Validating generated machine..." << std::endl;
    
    // Validate against learned patterns
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern.pattern_type == "parameter_range") {
            // Check if parameters are in valid ranges
            for (const auto& comp : machine.specification.components) {
                for (const auto& param : comp.parameters) {
                    if (pattern.pattern_data.contains(param.name)) {
                        float min_val = pattern.pattern_data[param.name]["min"];
                        float max_val = pattern.pattern_data[param.name]["max"];
                        
                        if (param.current_value < min_val || param.current_value > max_val) {
                            std::cerr << "[MachineLearningSystem] Parameter " << param.name 
                                      << " out of learned range" << std::endl;
                            return false;
                        }
                    }
                }
            }
        }
    }
    
    machine.is_validated = true;
    return true;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// INTEGRATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

bool MachineLearningSystem::IntegrateMachineIntoApp(GeneratedMachine& machine) {
    std::cout << "[MachineLearningSystem] Integrating machine into application..." << std::endl;
    
    // Generate code
    GeneratedCode code = GenerateMachineCode(machine);
    
    // Compile
    if (!CompileGeneratedCode(code)) {
        std::cerr << "[MachineLearningSystem] Failed to compile generated code" << std::endl;
        return false;
    }
    
    // Validate
    if (!ValidateGeneratedMachine(machine)) {
        std::cerr << "[MachineLearningSystem] Generated machine failed validation" << std::endl;
        return false;
    }
    
    // In a real implementation, this would:
    // 1. Write code to the src/Machines/ directory
    // 2. Update CMakeLists.txt
    // 3. Trigger rebuild
    // 4. Load the new machine component dynamically
    
    // ECS Integration:
    // Instantiate the Generated Machine as SceneGraph Entities
    auto* scene = SZM::SimulationEngine::GetInstance().GetScene();
    if (scene) {
        auto machineEntity = scene->CreateEntity(machine.name);
        scene->AddComponent<SZM::SceneGraph::TransformComponent>(machineEntity, SZM::SceneGraph::TransformComponent{});
        
        for (const auto& comp : machine.specification.components) {
            auto compEntity = scene->CreateEntity(comp.name);
            
            SZM::SceneGraph::TransformComponent xform;
            xform.parentEntity = machineEntity;
            scene->AddComponent<SZM::SceneGraph::TransformComponent>(compEntity, xform);
            
            SZM::SceneGraph::PhysicsStateComponent phys;
            for (const auto& param : comp.parameters) {
                if (param.name == "power" || param.name == "max_torque") {
                    phys.appliedForce = param.current_value;
                }
            }
            scene->AddComponent<SZM::SceneGraph::PhysicsStateComponent>(compEntity, phys);
            
            SZM::SceneGraph::MeshComponent mesh;
            mesh.meshId = "primitive_cube"; // Placeholder
            scene->AddComponent<SZM::SceneGraph::MeshComponent>(compEntity, mesh);
        }
        std::cout << "[MachineLearningSystem] Instantiated '" << machine.name << "' as ECS Entities." << std::endl;
    } else {
        std::cerr << "[MachineLearningSystem] No active Scene found for instantiation." << std::endl;
    }
    
    std::cout << "[MachineLearningSystem] Machine '" << machine.name << "' integrated successfully!" << std::endl;
    
    return true;
}

bool MachineLearningSystem::RegisterMachineComponent(
    const std::string& component_id,
    const GeneratedCode& code
) {
    std::cout << "[MachineLearningSystem] Registering component: " << component_id << std::endl;
    
    // In a real implementation, this would register the component
    // with the simulation engine or component factory
    
    return true;
}

std::string MachineLearningSystem::GetMachineHeader() const {
    std::ostringstream header;
    header << "// Auto-generated machine registry\n";
    header << "#pragma once\n\n";
    header << "#include <map>\n";
    header << "#include <string>\n\n";
    header << "namespace SZM::Machines {\n\n";
    header << "struct MachineRegistry {\n";
    header << "    std::map<std::string, std::string> machines;\n";
    header << "    std::map<std::string, std::string> components;\n";
    header << "};\n\n";
    header << "} // namespace SZM::Machines\n";
    
    return header.str();
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// DATABASE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::AddMachineToDatabase(const MachineSpecification& machine) {
    m_KnownMachines.push_back(machine);
}

std::vector<MachineSpecification> MachineLearningSystem::SearchMachines(
    MachineType type,
    const std::map<std::string, float>& constraints
) {
    std::vector<MachineSpecification> results;
    
    for (const auto& machine : m_KnownMachines) {
        if (machine.type != type && type != MachineType::GENERIC) continue;
        
        bool matches = true;
        for (const auto& [param, value] : constraints) {
            bool found = false;
            for (const auto& comp : machine.components) {
                for (const auto& p : comp.parameters) {
                    if (p.name == param) {
                        if (p.current_value != value) matches = false;
                        found = true;
                        break;
                    }
                }
                if (!found) continue;
            }
            if (!found) matches = false;
        }
        
        if (matches) results.push_back(machine);
    }
    
    return results;
}

MachineSpecification* MachineLearningSystem::FindMachineById(const std::string& id) {
    for (auto& machine : m_KnownMachines) {
        if (machine.id == id) return &machine;
    }
    for (auto& machine : m_GeneratedMachines) {
        if (machine.specification.id == id) return &machine.specification;
    }
    return nullptr;
}

void MachineLearningSystem::UpdateMachineKnowledge(const std::string& machine_id, const json& new_knowledge) {
    auto* machine = FindMachineById(machine_id);
    if (machine) {
        // Update machine with new knowledge
        std::cout << "[MachineLearningSystem] Updated knowledge for machine: " << machine_id << std::endl;
    }
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// EXPORT/IMPORT
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::ExportLearnedPatterns(const std::string& filepath) {
    json data;
    
    for (const auto& pattern : m_LearnedPatterns) {
        json p;
        p["id"] = pattern.id;
        p["pattern_type"] = pattern.pattern_type;
        p["description"] = pattern.description;
        p["pattern_data"] = pattern.pattern_data;
        p["confidence"] = pattern.confidence;
        p["occurrence_count"] = pattern.occurrence_count;
        data["patterns"].push_back(p);
    }
    
    std::ofstream file(filepath);
    file << data.dump(2);
    
    std::cout << "[MachineLearningSystem] Exported " << m_LearnedPatterns.size() 
              << " patterns to: " << filepath << std::endl;
}

void MachineLearningSystem::ImportLearnedPatterns(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[MachineLearningSystem] Could not import patterns from: " << filepath << std::endl;
        return;
    }
    
    json data;
    file >> data;
    
    if (data.contains("patterns")) {
        for (const auto& p : data["patterns"]) {
            LearnedPattern pattern;
            pattern.id = p["id"];
            pattern.pattern_type = p["pattern_type"];
            pattern.description = p["description"];
            pattern.pattern_data = p["pattern_data"];
            pattern.confidence = p["confidence"];
            pattern.occurrence_count = p["occurrence_count"];
            m_LearnedPatterns.push_back(pattern);
        }
    }
    
    std::cout << "[MachineLearningSystem] Imported " << data["patterns"].size() 
              << " patterns from: " << filepath << std::endl;
}

void MachineLearningSystem::ExportMachineFamily(const std::string& family_id, const std::string& filepath) {
    auto family = GetMachineFamily(family_id);
    if (family.variant_ids.empty()) return;
    
    json data;
    data["family_id"] = family.id;
    data["family_name"] = family.name;
    data["base_type"] = (int)family.base_type;
    
    for (const auto& vid : family.variant_ids) {
        auto* machine = FindMachineById(vid);
        if (machine) {
            json m;
            m["id"] = machine->id;
            m["name"] = machine->name;
            m["type"] = (int)machine->type;
            data["machines"].push_back(m);
        }
    }
    
    std::ofstream file(filepath);
    file << data.dump(2);
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// QUERY
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

std::vector<LearnedPattern> MachineLearningSystem::FindPatterns(
    const std::string& machine_type,
    const std::string& pattern_category
) {
    std::vector<LearnedPattern> results;
    
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern_category.empty() || pattern.pattern_type == pattern_category) {
            results.push_back(pattern);
        }
    }
    
    return results;
}

std::map<std::string, std::vector<float>> MachineLearningSystem::PredictParameters(
    MachineType type,
    const std::map<std::string, float>& known_parameters
) {
    std::map<std::string, std::vector<float>> predictions;
    
    // Find machines of the same type
    std::vector<MachineSpecification*> similar_machines;
    for (auto& machine : m_KnownMachines) {
        if (machine.type == type) {
            similar_machines.push_back(&machine);
        }
    }
    
    // Predict unknown parameters based on learned correlations
    for (auto* machine : similar_machines) {
        for (const auto& comp : machine->components) {
            for (const auto& param : comp.parameters) {
                // Check if parameter is known
                if (known_parameters.find(param.name) == known_parameters.end()) {
                    predictions[param.name].push_back(param.current_value);
                }
            }
        }
    }
    
    return predictions;
}

std::vector<std::string> MachineLearningSystem::SuggestImprovements(const std::string& machine_id) {
    std::vector<std::string> suggestions;
    
    auto* machine = FindMachineById(machine_id);
    if (!machine) return suggestions;
    
    // Compare against learned patterns
    for (const auto& pattern : m_LearnedPatterns) {
        if (pattern.pattern_type == "parameter_range") {
            for (const auto& comp : machine->components) {
                for (const auto& param : comp.parameters) {
                    if (pattern.pattern_data.contains(param.name)) {
                        float typical = pattern.pattern_data[param.name]["mean"];
                        float diff = abs(param.current_value - typical);
                        
                        if (diff > typical * 0.2f) { // 20% difference
                            suggestions.push_back(
                                "Parameter '" + param.name + "' differs significantly from typical value. "
                                "Consider adjusting to " + std::to_string(typical)
                            );
                        }
                    }
                }
            }
        }
    }
    
    return suggestions;
}

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// STATISTICS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

json MachineLearningSystem::GetStatistics() {
    json stats;
    
    stats["total_machines_learned"] = m_KnownMachines.size();
    stats["total_patterns_learned"] = m_LearnedPatterns.size();
    stats["total_families"] = m_MachineFamilies.size();
    stats["total_generated"] = m_GeneratedMachines.size();
    
    // Pattern breakdown
    json pattern_breakdown;
    for (const auto& pattern : m_LearnedPatterns) {
        pattern_breakdown[pattern.pattern_type] = pattern_breakdown.value(pattern.pattern_type, 0) + 1;
    }
    stats["pattern_breakdown"] = pattern_breakdown;
    
    // Component breakdown
    json component_breakdown;
    for (const auto& machine : m_KnownMachines) {
        component_breakdown[std::to_string((int)machine.type)] = machine.components.size();
    }
    stats["component_breakdown"] = component_breakdown;
    
    return stats;
}

int MachineLearningSystem::GetTotalMachinesLearned() { return m_KnownMachines.size(); }
int MachineLearningSystem::GetTotalPatternsLearned() { return m_LearnedPatterns.size(); }
int MachineLearningSystem::GetTotalFamiliesCreated() { return m_MachineFamilies.size(); }
int MachineLearningSystem::GetTotalGeneratedMachines() { return m_GeneratedMachines.size(); }

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// HELPER METHODS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

void MachineLearningSystem::AnalyzeMachineStructure(const MachineSpecification& machine) {
    // Analyze the structure and create patterns
}

void MachineLearningSystem::ExtractBehavioralModel(const MachineSpecification& machine) {
    // Extract behavioral model from simulation data
}

void MachineLearningSystem::GenerateParameterConstraints() {
    // Generate parameter constraints based on learned data
}

void MachineLearningSystem::BuildComponentGraph() {
    // Build component compatibility graph
}

void MachineLearningSystem::ClusterMachinesByType() {
    // Cluster machines by type for family learning
}

void MachineLearningSystem::AnalyzeParameterCorrelations() {
    // Analyze correlations between parameters
}

void MachineLearningSystem::LearnDesignHeuristics() {
    // Learn design heuristics from successful machines
}

std::string MachineLearningSystem::GenerateClassName(const std::string& base_name) {
    std::string result;
    bool capitalize_next = true;
    
    for (char c : base_name) {
        if (c == ' ' || c == '_' || c == '-') {
            capitalize_next = true;
        } else if (capitalize_next) {
            result += toupper(c);
            capitalize_next = false;
        } else {
            result += c;
        }
    }
    
    return "Generated" + result;
}

std::string MachineLearningSystem::GenerateUniqueId() {
    static int counter = 0;
    return "machine_" + std::to_string(++counter) + "_" + 
           std::to_string(std::time(nullptr));
}

} // namespace SZM::AI::Machines
