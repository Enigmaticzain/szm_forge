#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>

namespace SZM::AI::Machines {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MACHINE DATA TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class MachineType {
    GENERIC,
    CNC_MILL,
    CNC_LATHE,
    PRINTER_3D,
    LASER_CUTTER,
    PLASMA_CUTTER,
    ROBOT_ARM,
    CONVEYOR,
    PRESS,
    INJECTOR,
    CUSTOM
};

enum class MachineComponent {
    FRAME,
    MOTOR,
    SPINDLE,
    LINEARMOTION,
    GEARBOX,
    PUMP,
    VALVE,
    SENSOR,
    CONTROLLER,
    COOLING,
    ELECTRICAL,
    CUSTOM
};

enum class SimulationCapability {
    KINEMATICS,
    DYNAMICS,
    THERMAL,
    STRUCTURAL,
    FLUIDS,
    ELECTRICAL,
    CONTROL_SYSTEM
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MACHINE SPECIFICATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct MachineParameter {
    std::string name;
    std::string category;
    std::string unit;
    float min_value;
    float max_value;
    float default_value;
    float current_value;
    std::string description;
};

struct MachineComponentSpec {
    std::string id;
    std::string name;
    MachineComponent type;
    std::vector<MachineParameter> parameters;
    std::vector<std::string> compatible_with;
    json metadata;
};

struct MachineSpecification {
    std::string id;
    std::string name;
    std::string manufacturer;
    std::string model;
    MachineType type;
    std::vector<MachineComponentSpec> components;
    std::vector<SimulationCapability> capabilities;
    std::map<std::string, std::string> specifications;
    std::string source_file;
    std::vector<std::string> tags;
};

struct MachineGeometry {
    std::string geometry_id;
    std::vector<float> bounding_box;  // [min_x, min_y, min_z, max_x, max_y, max_z]
    std::string mesh_file;
    std::vector<std::string> cad_layers;
    std::map<std::string, std::vector<float>> feature_positions;
};

struct MachineBehavior {
    std::string behavior_id;
    std::string name;
    std::string description;
    std::vector<std::string> required_components;
    std::map<std::string, float> parameters;
    std::string physics_model;
    std::string control_logic;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// LEARNED MACHINE KNOWLEDGE
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct LearnedPattern {
    std::string id;
    std::string pattern_type;  // "structure", "behavior", "parameter_range", "constraint"
    std::string description;
    std::vector<std::string> machine_types;
    std::vector<std::string> component_types;
    json pattern_data;
    float confidence;
    int occurrence_count;
    std::vector<std::string> examples;
};

struct MachineFamily {
    std::string id;
    std::string name;
    MachineType base_type;
    std::vector<std::string> variant_ids;
    std::map<std::string, std::vector<float>> common_parameters;  // param_name -> [min, max, typical]
    std::vector<LearnedPattern> common_patterns;
    std::vector<std::string> design_rules;
    std::string generation_prompt;
};

struct GeneratedMachine {
    std::string id;
    std::string name;
    MachineType type;
    MachineSpecification specification;
    MachineGeometry geometry;
    std::vector<MachineBehavior> behaviors;
    std::string generated_code_path;
    std::vector<std::string> imported_patterns;
    float confidence_score;
    bool is_compiled;
    bool is_validated;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// CODE GENERATION
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

struct CodeTemplate {
    std::string template_id;
    std::string name;
    MachineComponent component_type;
    std::string header_template;
    std::string source_template;
    std::vector<std::string> required_includes;
    std::map<std::string, std::string> parameter_mappings;
};

struct GeneratedCode {
    std::string code_id;
    std::vector<std::string> header_files;
    std::vector<std::string> source_files;
    std::vector<std::string> dependencies;
    std::map<std::string, std::string> class_definitions;
    std::map<std::string, std::string> method_implementations;
    std::string cmake_snippet;
    bool compiles_successfully;
    std::string error_log;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MACHINE LEARNING SYSTEM
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class MachineLearningSystem {
public:
    static MachineLearningSystem& GetInstance();

    // Lifecycle
    void Initialize();
    void Shutdown();
    bool IsInitialized() const { return m_IsInitialized; }

    // ===== DATA INGESTION =====
    
    void IngestMachineData(const std::string& data_path);
    void IngestCADFile(const std::string& cad_file_path);
    void IngestSpecification(const std::string& spec_json_path);
    void IngestSimulationResult(const std::string& result_path);
    void IngestFromDatabase(const std::string& db_connection);
    
    // Batch ingestion
    void IngestMachineDataset(const std::vector<std::string>& data_paths);
    
    // ===== PATTERN LEARNING =====
    
    void LearnFromMachines();
    void LearnParameterRanges();
    void LearnStructuralPatterns();
    void LearnBehavioralPatterns();
    void LearnDesignConstraints();
    void LearnComponentRelationships();
    
    // ===== MACHINE FAMILIES =====
    
    void CreateMachineFamily(MachineType type, const std::string& family_name);
    void AddToFamily(const std::string& family_id, const std::string& machine_id);
    MachineFamily GetMachineFamily(const std::string& family_id);
    std::vector<MachineFamily> GetAllFamilies();
    
    // ===== MACHINE GENERATION =====
    
    GeneratedMachine GenerateMachine(
        MachineType type,
        const std::map<std::string, float>& parameters,
        const std::vector<std::string>& required_capabilities
    );
    
    GeneratedMachine GenerateVariant(
        const std::string& base_machine_id,
        const std::map<std::string, float>& modifications
    );
    
    std::vector<GeneratedMachine> GenerateFamilyVariants(
        const std::string& family_id,
        int count
    );
    
    // ===== CODE GENERATION =====
    
    GeneratedCode GenerateMachineCode(const GeneratedMachine& machine);
    GeneratedCode GenerateComponentCode(
        MachineComponent component_type,
        const std::vector<MachineParameter>& parameters
    );
    
    bool CompileGeneratedCode(GeneratedCode& code);
    bool ValidateGeneratedMachine(GeneratedMachine& machine);
    
    // ===== INTEGRATION =====
    
    bool IntegrateMachineIntoApp(GeneratedMachine& machine);
    bool RegisterMachineComponent(const std::string& component_id, 
                                  const GeneratedCode& code);
    std::string GetMachineHeader() const;
    
    // ===== DATABASE =====
    
    void AddMachineToDatabase(const MachineSpecification& machine);
    std::vector<MachineSpecification> SearchMachines(
        MachineType type,
        const std::map<std::string, float>& constraints
    );
    MachineSpecification* FindMachineById(const std::string& id);
    void UpdateMachineKnowledge(const std::string& machine_id, 
                               const json& new_knowledge);
    
    // ===== EXPORT/IMPORT =====
    
    void ExportLearnedPatterns(const std::string& filepath);
    void ImportLearnedPatterns(const std::string& filepath);
    void ExportMachineFamily(const std::string& family_id, const std::string& filepath);
    
    // ===== QUERY =====
    
    std::vector<LearnedPattern> FindPatterns(
        const std::string& machine_type,
        const std::string& pattern_category
    );
    
    std::map<std::string, std::vector<float>> PredictParameters(
        MachineType type,
        const std::map<std::string, float>& known_parameters
    );
    
    std::vector<std::string> SuggestImprovements(const std::string& machine_id);
    
    // ===== STATISTICS =====
    
    json GetStatistics();
    int GetTotalMachinesLearned();
    int GetTotalPatternsLearned();
    int GetTotalFamiliesCreated();
    int GetTotalGeneratedMachines();

private:
    MachineLearningSystem() = default;

    MachineLearningSystem(const MachineLearningSystem&) = delete;
    MachineLearningSystem& operator=(const MachineLearningSystem&) = delete;

    bool m_IsInitialized = false;

    // Data storage
    std::vector<MachineSpecification> m_KnownMachines;
    std::vector<LearnedPattern> m_LearnedPatterns;
    std::map<std::string, MachineFamily> m_MachineFamilies;
    std::vector<GeneratedMachine> m_GeneratedMachines;
    
    // Code templates
    std::map<MachineComponent, CodeTemplate> m_ComponentTemplates;
    
    // Helper methods
    void AnalyzeMachineStructure(const MachineSpecification& machine);
    void ExtractBehavioralModel(const MachineSpecification& machine);
    void GenerateParameterConstraints();
    void BuildComponentGraph();
    std::string GenerateClassName(const std::string& base_name);
    std::string GenerateUniqueId();
    void InitializeCodeTemplates();
    
    // Code generation helpers
    std::string GenerateHeaderContent(const GeneratedMachine& machine);
    std::string GenerateSourceContent(const GeneratedMachine& machine);
    std::string GenerateSimulationComponent(const MachineBehavior& behavior);
    std::string GeneratePhysicsModel(const MachineBehavior& behavior);
    std::string GenerateControlLogic(const MachineBehavior& behavior);
    
    // Pattern learning helpers
    void ClusterMachinesByType();
    void AnalyzeParameterCorrelations();
    void LearnDesignHeuristics();
    
    // Constants
    static constexpr int MIN_PATTERN_OCCURRENCES = 3;
    static constexpr float MIN_PATTERN_CONFIDENCE = 0.6f;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MACHINE LEARNING CALLBACKS
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

using MachineLearnedCallback = std::function<void(const MachineSpecification&)>;
using PatternDiscoveredCallback = std::function<void(const LearnedPattern&)>;
using MachineGeneratedCallback = std::function<void(const GeneratedMachine&)>;
using CodeGeneratedCallback = std::function<void(const GeneratedCode&)>;
using IntegrationCompleteCallback = std::function<void(bool, const std::string&)>;

struct MachineLearningCallbacks {
    MachineLearnedCallback on_machine_learned;
    PatternDiscoveredCallback on_pattern_discovered;
    MachineGeneratedCallback on_machine_generated;
    CodeGeneratedCallback on_code_generated;
    IntegrationCompleteCallback on_integration_complete;
};

inline void SetMachineLearningCallbacks(const MachineLearningCallbacks& callbacks);

} // namespace SZM::AI::Machines
