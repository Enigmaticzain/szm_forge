# Machine Learning System - Complete Guide

## Overview

Your AI **CAN** learn from machine data and generate new capabilities for the app. The Machine Learning System enables:

1. **Learn from machine data** - CAD files, specifications, simulation results
2. **Discover patterns** - Common structures, parameter relationships, design rules
3. **Generate new machines** - Create new machine configurations automatically
4. **Generate code** - Produce C++ code that integrates into the app
5. **Validate & integrate** - Compile and add new capabilities to the app

## System Architecture

```
┌────────────────────────────────────────────────────────────────────┐
│                    MACHINE LEARNING SYSTEM                          │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐       │
│  │ DATA INGEST  │───▶│   PATTERN    │───▶│   MACHINE   │       │
│  │              │    │   LEARNING   │    │  GENERATION │       │
│  │ • JSON specs │    │              │    │              │       │
│  │ • CAD files  │    │ • Parameters │    │ • Variants  │       │
│  │ • CSV params │    │ • Structures │    │ • Families  │       │
│  │ • Sim data   │    │ • Relations  │    │ • Custom    │       │
│  └──────────────┘    └──────────────┘    └──────────────┘       │
│                                               │                    │
│                                               ▼                    │
│                    ┌──────────────────────────────────────┐       │
│                    │        CODE GENERATION               │       │
│                    │                                      │       │
│                    │  • C++ Header files (.hpp)          │       │
│                    │  • C++ Source files (.cpp)          │       │
│                    │  • CMake integration                 │       │
│                    │  • Component registration            │       │
│                    └──────────────────────────────────────┘       │
│                                               │                    │
│                                               ▼                    │
│                    ┌──────────────────────────────────────┐       │
│                    │        APP INTEGRATION               │       │
│                    │                                      │       │
│                    │  • Compile new components            │       │
│                    │  • Register with simulation engine   │       │
│                    │  • Add to machine database           │       │
│                    │  • Make available in UI              │       │
│                    └──────────────────────────────────────┘       │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

## Quick Start

### 1. Prepare Your Machine Data

Create JSON files describing your machines:

```json
// machine_data/my_cnc_mill.json
{
    "name": "My CNC Mill",
    "manufacturer": "Custom",
    "type": "cnc_mill",
    "components": [
        {
            "name": "Main Spindle",
            "type": "spindle",
            "parameters": [
                {"name": "power", "min": 1, "max": 15, "default": 5, "unit": "kW"},
                {"name": "max_speed", "min": 1000, "max": 10000, "default": 5000, "unit": "RPM"}
            ]
        },
        {
            "name": "Linear Table",
            "type": "linear_motion",
            "parameters": [
                {"name": "travel_x", "min": 500, "max": 2000, "default": 1000, "unit": "mm"},
                {"name": "travel_y", "min": 300, "max": 1000, "default": 500, "unit": "mm"}
            ]
        }
    ],
    "capabilities": ["kinematics", "dynamics"],
    "tags": ["cnc", "milling", "3-axis"]
}
```

### 2. Train the AI

```bash
# Ingest machine data and learn patterns
python ai_service/machine_learning_training.py --data_dir ./machine_data --learn
```

### 3. Generate New Machines

```bash
# Generate a CNC mill variant
python ai_service/machine_learning_training.py --generate --type cnc_mill --count 5

# With custom parameters
python ai_service/machine_learning_training.py --generate --type robot_arm \
    --params '{"payload": 15.0, "reach": 1800}'
```

### 4. Integrate into App (C++)

```cpp
#include <AI/SZM_AI.hpp>
#include <AI/MachineLearningSystem/MachineLearningSystem.hpp>

void SetupNewMachineCapabilities() {
    auto& ml = SZM::AI::Machines::MachineLearningSystem::GetInstance();
    ml.Initialize();
    
    // Load your machine data
    ml.IngestMachineDataset({
        "./machine_data/cnc_mill_1.json",
        "./machine_data/cnc_mill_2.json",
        "./machine_data/cnc_mill_3.json"
    });
    
    // Learn patterns from the data
    ml.LearnFromMachines();
    
    // Generate a new machine
    auto generated = ml.GenerateMachine(
        SZM::AI::Machines::MachineType::CNC_MILL,
        {{"power", 7.5f}, {"max_speed", 6000.0f}, {"travel_x", 1200.0f}},
        {"kinematics", "dynamics"}
    );
    
    // Generate C++ code for it
    auto code = ml.GenerateMachineCode(generated);
    
    // Compile and integrate
    if (ml.CompileGeneratedCode(code)) {
        ml.IntegrateMachineIntoApp(generated);
    }
}
```

## What the AI Learns

### 1. Parameter Ranges
```
Input: Multiple machines with "power" parameter
Output: "power" typically ranges from 1kW to 15kW, mean 5kW
```

### 2. Structural Patterns
```
Input: Machines with common component combinations
Output: "CNC Mill always has: Spindle + Linear Motion X + Linear Motion Y"
```

### 3. Component Relationships
```
Input: Which components appear together
Output: "Spindle is 95% likely paired with Encoder"
```

### 4. Design Constraints
```
Input: Parameter relationships
Output: "Higher power requires stronger cooling"
```

## Machine Types Supported

| Type | Description | Typical Components |
|------|-------------|-------------------|
| `cnc_mill` | CNC Milling Machine | Spindle, Linear Motion, Coolant |
| `cnc_lathe` | CNC Lathe | Spindle, Carriage, Tailstock |
| `printer_3d` | 3D Printer | Extruder, Bed, Linear Motion |
| `laser_cutter` | Laser Cutter | Laser Source, Linear Motion, Focus |
| `plasma_cutter` | Plasma Cutter | Plasma Torch, Linear Motion |
| `robot_arm` | Robot Arm | Joints, End Effector, Controller |
| `conveyor` | Conveyor Belt | Motor, Belt, Rollers |
| `press` | Hydraulic Press | Cylinder, Frame, Controller |
| `injector` | Injection Molder | Screw, Barrel, Mold |
| `custom` | Custom Machine | User-defined |

## Generated Code Example

Input: Generate a CNC mill with custom parameters

Output (C++ Header):
```cpp
#pragma once

namespace SZM::Machines::Generated {

class GeneratedCncMill {
public:
    GeneratedCncMill();
    virtual ~GeneratedCncMill() = default;
    
    // Component: Main Spindle
    void SetPower(float value);
    float GetPower() const;
    void SetMaxSpeed(float value);
    float GetMaxSpeed() const;
    
    // Component: Linear Table
    void SetTravelX(float value);
    float GetTravelX() const;
    void SetTravelY(float value);
    float GetTravelY() const;
    
    void Initialize();
    void Update(float delta_time);
    
private:
    float m_Power = 7.5f;
    float m_MaxSpeed = 6000.0f;
    float m_TravelX = 1200.0f;
    float m_TravelY = 500.0f;
};

} // namespace SZM::Machines::Generated
```

## C++ API Reference

### Initialization
```cpp
auto& ml = MachineLearningSystem::GetInstance();
ml.Initialize();
```

### Data Ingestion
```cpp
// Single file
ml.IngestMachineData("./machine_data/my_machine.json");

// Multiple files
ml.IngestMachineDataset({"./data/machine1.json", "./data/machine2.json"});

// CAD file (extracts geometry info)
ml.IngestCADFile("./cad/robot_arm.step");
```

### Pattern Learning
```cpp
// Learn from all ingested machines
ml.LearnFromMachines();

// Or learn specific patterns
ml.LearnParameterRanges();
ml.LearnStructuralPatterns();
ml.LearnComponentRelationships();
ml.LearnDesignConstraints();
```

### Machine Generation
```cpp
// Generate a new machine
auto machine = ml.GenerateMachine(
    MachineType::CNC_MILL,
    {{"power", 7.5f}, {"max_speed", 6000.0f}},
    {"kinematics", "dynamics"}
);

// Generate variants of existing machine
auto variants = ml.GenerateFamilyVariants(family_id, 5);

// Generate a modification of existing machine
auto variant = ml.GenerateVariant(base_machine_id, {{"power", 10.0f}});
```

### Code Generation
```cpp
// Generate C++ code
auto code = ml.GenerateMachineCode(machine);

// Compile
if (ml.CompileGeneratedCode(code)) {
    // Integration successful
    ml.IntegrateMachineIntoApp(machine);
}
```

### Database Queries
```cpp
// Search machines by type
auto cnc_mills = ml.SearchMachines(MachineType::CNC_MILL, {});

// Find specific machine
auto* machine = ml.FindMachineById("machine_123");

// Get suggestions for improvement
auto suggestions = ml.SuggestImprovements(machine_id);
```

### Statistics
```cpp
json stats = ml.GetStatistics();
std::cout << "Machines learned: " << stats["total_machines_learned"] << std::endl;
std::cout << "Patterns discovered: " << stats["total_patterns_learned"] << std::endl;
```

## Python Training Service

The Python service provides additional ML capabilities:

```bash
# Ingest and learn
python ai_service/machine_learning_training.py --data_dir ./machine_data --learn

# Generate machines
python ai_service/machine_learning_training.py --generate --type cnc_mill --count 10

# Output to directory
python ai_service/machine_learning_training.py --generate --output_dir ./generated
```

### Python API

```python
from machine_learning_training import MachineLearningPipeline

# Create pipeline
pipeline = MachineLearningPipeline('./machine_data')

# Ingest data
pipeline.ingest_data()

# Learn patterns
pipeline.learn_patterns()

# Generate machines
machines = pipeline.generate_machines('cnc_mill', count=5)

# Save output
for machine in machines:
    pipeline.save_output(machine, './generated')
```

## Data Format Reference

### Machine Specification (JSON)
```json
{
    "id": "unique_id",
    "name": "Human readable name",
    "manufacturer": "Company name",
    "model": "Model number",
    "type": "cnc_mill|robot_arm|etc",
    "components": [...],
    "specifications": {"key": "value"},
    "capabilities": ["kinematics", "dynamics"],
    "tags": ["tag1", "tag2"]
}
```

### Component
```json
{
    "id": "comp_001",
    "name": "Component Name",
    "type": "spindle|motor|linear_motion|etc",
    "parameters": [...],
    "compatible_with": ["comp_002", "comp_003"]
}
```

### Parameter
```json
{
    "name": "power",
    "category": "performance",
    "unit": "kW",
    "min": 1.0,
    "max": 15.0,
    "default": 5.0,
    "description": "Motor power rating"
}
```

## Tips for Better Learning

1. **Provide diverse examples** - Include 5+ machines of each type
2. **Use consistent naming** - Same parameter names across machines
3. **Include ranges** - Always specify min/max/default values
4. **Add metadata** - Tags and specifications help pattern matching
5. **Include successful designs** - AI learns what works well

## Troubleshooting

### No patterns detected
- Need at least 3 machines of the same type
- Ensure consistent parameter names
- Check that JSON files are valid

### Low confidence generation
- More training data improves confidence
- Include machines with similar specifications
- Ensure parameter ranges are realistic

### Code doesn't compile
- Generated code is placeholder - needs manual review
- Check for naming conflicts with existing code
- Verify all includes are correct

## Next Steps

1. **Prepare your machine data** - Create JSON specs for your machines
2. **Train the AI** - Run the training pipeline
3. **Generate variations** - Create new machine configurations
4. **Review generated code** - Check and refine the output
5. **Integrate** - Add new capabilities to the app
6. **Iterate** - Add more data and retrain for better results

## Integration with Existing Systems

The Machine Learning System integrates with:
- **Simulation Engine** - New machines can be simulated
- **Physics Engine** - Physics models for components
- **Geometry Module** - CAD integration for machine geometry
- **UI System** - Machine selection and configuration panels
