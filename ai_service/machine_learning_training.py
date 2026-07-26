"""
SZM Forge - Machine Learning Training Service
=============================================

This service trains the AI to learn from machine data and generate
new machine capabilities for the application.

Usage:
    python machine_learning_training.py --ingest ./machines_data/
    python machine_learning_training.py --learn
    python machine_learning_training.py --generate --type cnc_mill --count 5
    python machine_learning_training.py --train-model
"""

import os
import sys
import json
import argparse
from pathlib import Path
from dataclasses import dataclass, asdict, field
from typing import List, Dict, Optional, Tuple, Any
from datetime import datetime
import random

# Optional ML imports
try:
    import numpy as np
    NUMPY_AVAILABLE = True
except ImportError:
    NUMPY_AVAILABLE = False
    print("Warning: NumPy not available. Using fallback implementations.")

try:
    import torch
    import torch.nn as nn
    import torch.optim as optim
    TORCH_AVAILABLE = True
except ImportError:
    TORCH_AVAILABLE = False
    print("Warning: PyTorch not available. Using fallback implementations.")


# =============================================================================
# DATA STRUCTURES
# =============================================================================

@dataclass
class MachineParameter:
    name: str
    category: str
    unit: str = ""
    min_value: float = 0.0
    max_value: float = 100.0
    default_value: float = 50.0
    current_value: float = 50.0
    description: str = ""


@dataclass
class MachineComponent:
    id: str
    name: str
    component_type: str
    parameters: List[MachineParameter] = field(default_factory=list)
    compatible_with: List[str] = field(default_factory=list)
    metadata: Dict = field(default_factory=dict)


@dataclass
class MachineSpecification:
    id: str
    name: str
    manufacturer: str = ""
    model: str = ""
    machine_type: str = "generic"
    components: List[MachineComponent] = field(default_factory=list)
    capabilities: List[str] = field(default_factory=list)
    specifications: Dict[str, str] = field(default_factory=dict)
    source_file: str = ""
    tags: List[str] = field(default_factory=list)


@dataclass
class LearnedPattern:
    id: str
    pattern_type: str
    description: str
    machine_types: List[str] = field(default_factory=list)
    component_types: List[str] = field(default_factory=list)
    pattern_data: Dict[str, Any] = field(default_factory=dict)
    confidence: float = 0.0
    occurrence_count: int = 0
    examples: List[str] = field(default_factory=list)


@dataclass
class GeneratedMachine:
    id: str
    name: str
    machine_type: str
    specification: Dict
    confidence_score: float
    generated_code_path: str = ""
    is_compiled: bool = False


# =============================================================================
# MACHINE DATA PARSER
# =============================================================================

class MachineDataParser:
    """
    Parses machine data from various formats:
    - JSON specification files
    - CSV parameter tables
    - STEP/IGES CAD files (placeholder)
    - Simulation result files
    """
    
    @staticmethod
    def parse_json_spec(filepath: str) -> Optional[MachineSpecification]:
        """Parse JSON machine specification."""
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)
            
            spec = MachineSpecification(
                id=data.get('id', f'machine_{len(data)}'),
                name=data.get('name', 'Unknown Machine'),
                manufacturer=data.get('manufacturer', ''),
                model=data.get('model', ''),
                machine_type=data.get('type', 'generic'),
                source_file=filepath
            )
            
            # Parse components
            for comp_data in data.get('components', []):
                params = []
                for p_data in comp_data.get('parameters', []):
                    params.append(MachineParameter(
                        name=p_data.get('name', 'param'),
                        category=p_data.get('category', 'general'),
                        unit=p_data.get('unit', ''),
                        min_value=p_data.get('min', 0.0),
                        max_value=p_data.get('max', 100.0),
                        default_value=p_data.get('default', 50.0),
                        current_value=p_data.get('default', 50.0),
                        description=p_data.get('description', '')
                    ))
                
                spec.components.append(MachineComponent(
                    id=comp_data.get('id', f'comp_{len(spec.components)}'),
                    name=comp_data.get('name', 'Component'),
                    component_type=comp_data.get('type', 'generic'),
                    parameters=params,
                    compatible_with=comp_data.get('compatible_with', []),
                    metadata=comp_data.get('metadata', {})
                ))
            
            # Parse specifications
            spec.specifications = data.get('specifications', {})
            
            # Parse tags
            spec.tags = data.get('tags', [])
            
            # Parse capabilities
            spec.capabilities = data.get('capabilities', [])
            
            return spec
            
        except Exception as e:
            print(f"Error parsing {filepath}: {e}")
            return None
    
    @staticmethod
    def parse_csv_params(filepath: str) -> List[MachineParameter]:
        """Parse CSV parameter table."""
        params = []
        
        # Simple CSV parsing
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        if len(lines) < 2:
            return params
        
        headers = lines[0].strip().split(',')
        
        for line in lines[1:]:
            values = line.strip().split(',')
            if len(values) >= len(headers):
                param = MachineParameter(name=values[0])
                for i, header in enumerate(headers[1:], 1):
                    try:
                        value = float(values[i]) if i < len(values) else 0.0
                        if header == 'min':
                            param.min_value = value
                        elif header == 'max':
                            param.max_value = value
                        elif header == 'default':
                            param.default_value = value
                            param.current_value = value
                    except ValueError:
                        pass
                params.append(param)
        
        return params
    
    @staticmethod
    def parse_cad_file(filepath: str) -> Optional[Dict]:
        """Parse CAD file (STEP/IGES) - placeholder implementation."""
        # In production, this would use CAD kernels like OpenCASCADE
        extension = filepath.lower().split('.')[-1]
        
        if extension in ['stp', 'step', 'iges', 'igs']:
            return {
                'type': 'cad_file',
                'format': extension,
                'filepath': filepath,
                'bounding_box': [0, 0, 0, 100, 100, 50],  # Placeholder
                'features': [],
                'note': 'CAD parsing placeholder - implement with OpenCASCADE or Mayo'
            }
        
        return None


# =============================================================================
# PATTERN LEARNING ENGINE
# =============================================================================

class PatternLearningEngine:
    """
    Analyzes machine data to discover patterns and relationships.
    """
    
    def __init__(self):
        self.patterns = []
        self.machines = []
    
    def learn_from_machines(self, machines: List[MachineSpecification]):
        """Learn patterns from a collection of machines."""
        self.machines = machines
        print(f"Learning from {len(machines)} machines...")
        
        self.learn_parameter_ranges()
        self.learn_structural_patterns()
        self.learn_component_relationships()
        self.learn_design_constraints()
        
        print(f"Learned {len(self.patterns)} patterns")
        return self.patterns
    
    def learn_parameter_ranges(self):
        """Learn typical parameter ranges across machines."""
        param_values = {}
        
        for machine in self.machines:
            for comp in machine.components:
                for param in comp.parameters:
                    if param.name not in param_values:
                        param_values[param.name] = []
                    param_values[param.name].append(param.current_value)
        
        for name, values in param_values.items():
            if len(values) >= 3:
                pattern = LearnedPattern(
                    id=f"param_range_{name}",
                    pattern_type="parameter_range",
                    description=f"Parameter '{name}' typical range",
                    pattern_data={
                        'min': min(values),
                        'max': max(values),
                        'mean': sum(values) / len(values),
                        'std': self.std_dev(values) if len(values) > 1 else 0
                    },
                    confidence=min(1.0, len(values) / 10.0),
                    occurrence_count=len(values)
                )
                self.patterns.append(pattern)
    
    def learn_structural_patterns(self):
        """Learn common structural patterns (component combinations)."""
        structures = {}
        
        for machine in self.machines:
            comp_names = sorted([c.name for c in machine.components])
            key = '+'.join(comp_names)
            structures[key] = structures.get(key, 0) + 1
        
        for structure, count in structures.items():
            if count >= 2:
                pattern = LearnedPattern(
                    id=f"structure_{len(self.patterns)}",
                    pattern_type="structure",
                    description=f"Common component combination: {structure}",
                    machine_types=[machine.machine_type for machine in self.machines],
                    pattern_data={'components': structure, 'frequency': count},
                    confidence=min(1.0, count / 10.0),
                    occurrence_count=count
                )
                self.patterns.append(pattern)
    
    def learn_component_relationships(self):
        """Learn which components commonly appear together."""
        relationships = {}
        
        for machine in self.machines:
            comp_names = [c.name for c in machine.components]
            for i, c1 in enumerate(comp_names):
                for c2 in comp_names[i+1:]:
                    key = f"{c1}<->{c2}"
                    relationships[key] = relationships.get(key, 0) + 1
        
        for relationship, count in relationships.items():
            if count >= 2:
                parts = relationship.split('<->')
                pattern = LearnedPattern(
                    id=f"relationship_{len(self.patterns)}",
                    pattern_type="component_relationship",
                    description=f"{parts[0]} commonly paired with {parts[1]}",
                    component_types=parts,
                    pattern_data={'frequency': count, 'components': parts},
                    confidence=min(1.0, count / 10.0),
                    occurrence_count=count
                )
                self.patterns.append(pattern)
    
    def learn_design_constraints(self):
        """Learn constraints between parameters."""
        for machine in self.machines:
            for comp in machine.components:
                if len(comp.parameters) >= 2:
                    # Learn pairwise relationships
                    for i, p1 in enumerate(comp.parameters):
                        for p2 in comp.parameters[i+1:]:
                            if p1.category == p2.category:
                                pattern = LearnedPattern(
                                    id=f"constraint_{len(self.patterns)}",
                                    pattern_type="constraint",
                                    description=f"{p1.name} related to {p2.name}",
                                    component_types=[comp.name],
                                    pattern_data={
                                        'param1': p1.name,
                                        'param2': p2.name,
                                        'category': p1.category
                                    },
                                    confidence=0.6,
                                    occurrence_count=1
                                )
                                self.patterns.append(pattern)
    
    def std_dev(self, values: List[float]) -> float:
        """Calculate standard deviation."""
        if len(values) < 2:
            return 0.0
        mean = sum(values) / len(values)
        variance = sum((v - mean) ** 2 for v in values) / len(values)
        return variance ** 0.5


# =============================================================================
# MACHINE GENERATOR
# =============================================================================

class MachineGenerator:
    """
    Generates new machine configurations based on learned patterns.
    """
    
    def __init__(self, patterns: List[LearnedPattern]):
        self.patterns = patterns
    
    def generate_machine(
        self,
        machine_type: str,
        parameters: Dict[str, float],
        required_capabilities: List[str] = None
    ) -> GeneratedMachine:
        """Generate a new machine based on type and parameters."""
        
        print(f"Generating {machine_type} machine...")
        
        # Get structural patterns for this type
        structure_patterns = [p for p in self.patterns 
                             if p.pattern_type == 'structure' 
                             and machine_type in p.machine_types]
        
        # Get parameter ranges
        param_patterns = {p.pattern_data.get('param', p.id): p 
                         for p in self.patterns 
                         if p.pattern_type == 'parameter_range'}
        
        # Build specification
        spec = {
            'id': f'generated_{datetime.now().strftime("%Y%m%d_%H%M%S")}',
            'name': f'Generated {machine_type.title()}',
            'machine_type': machine_type,
            'components': [],
            'capabilities': required_capabilities or [],
            'specifications': parameters
        }
        
        # Add components based on structure patterns
        for pattern in structure_patterns[:3]:  # Top 3 patterns
            components = pattern.pattern_data.get('components', '').split('+')
            for comp_name in components[:5]:  # Max 5 components
                comp = {
                    'id': f'gen_comp_{len(spec["components"])}',
                    'name': comp_name.strip(),
                    'type': 'generated',
                    'parameters': []
                }
                
                # Add parameters
                for param_name, value in parameters.items():
                    param_pattern = param_patterns.get(param_name)
                    if param_pattern:
                        min_val = param_pattern.pattern_data.get('min', value * 0.5)
                        max_val = param_pattern.pattern_data.get('max', value * 1.5)
                    else:
                        min_val = value * 0.5
                        max_val = value * 1.5
                    
                    comp['parameters'].append({
                        'name': param_name,
                        'category': 'performance',
                        'min': min_val,
                        'max': max_val,
                        'default': value,
                        'current': value
                    })
                
                spec['components'].append(comp)
        
        # Calculate confidence
        confidence = 0.5
        for pattern in structure_patterns:
            confidence += pattern.confidence * 0.1
        confidence = min(1.0, confidence)
        
        return GeneratedMachine(
            id=spec['id'],
            name=spec['name'],
            machine_type=machine_type,
            specification=spec,
            confidence_score=confidence
        )
    
    def generate_variants(
        self,
        base_machine: MachineSpecification,
        count: int = 5,
        variation_factor: float = 0.2
    ) -> List[GeneratedMachine]:
        """Generate variants of an existing machine."""
        variants = []
        
        # Extract parameters from base machine
        base_params = {}
        for comp in base_machine.components:
            for param in comp.parameters:
                base_params[param.name] = param.current_value
        
        # Generate variations
        for i in range(count):
            modified_params = {}
            for name, value in base_params.items():
                # Add random variation
                variation = random.uniform(-variation_factor, variation_factor) * value
                modified_params[name] = value + variation
            
            variant = self.generate_machine(
                base_machine.machine_type,
                modified_params
            )
            variant.name = f"{variant.name} Variant {i+1}"
            variants.append(variant)
        
        return variants


# =============================================================================
# CODE GENERATOR
# =============================================================================

class CodeGenerator:
    """
    Generates C++ code for machine implementations.
    """
    
    def generate_machine_code(self, machine: GeneratedMachine) -> Dict[str, str]:
        """Generate C++ header and source files for a machine."""
        
        class_name = self.to_camel_case(machine.name.replace(' ', ''))
        
        # Generate header
        header = self.generate_header(machine, class_name)
        
        # Generate source
        source = self.generate_source(machine, class_name)
        
        # Generate CMake snippet
        cmake = self.generate_cmake(machine, class_name)
        
        return {
            'header': f'{class_name}.hpp',
            'source': f'{class_name}.cpp',
            'header_content': header,
            'source_content': source,
            'cmake_snippet': cmake
        }
    
    def generate_header(self, machine: GeneratedMachine, class_name: str) -> str:
        """Generate C++ header file."""
        
        spec = machine.specification
        
        header = f"""#pragma once

#include <string>
#include <vector>
#include <map>

namespace SZM::Machines::Generated {{

/**
 * Generated Machine: {spec['name']}
 * Type: {spec['machine_type']}
 * Confidence: {machine.confidence_score:.1%}
 * Generated: {datetime.now().isoformat()}
 */
class {class_name} {{
public:
    {class_name}();
    virtual ~{class_name}() = default;
    
    // Component access
"""
        
        for comp in spec.get('components', []):
            header += f"\n    // Component: {comp['name']}\n"
            for param in comp.get('parameters', []):
                param_name = param['name']
                setter = f"Set{self.to_pascal_case(param_name)}"
                getter = f"Get{self.to_pascal_case(param_name)}"
                header += f"    void {setter}(float value);\n"
                header += f"    float {getter}() const;\n"
        
        header += """
    
    // Simulation
    void Initialize();
    void Update(float delta_time);
    
private:
"""
        
        for comp in spec.get('components', []):
            header += f"    // {comp['name']} state\n"
            for param in comp.get('parameters', []):
                header += f"    float m_{param['name']} = {param['default']}f;\n"
        
        header += "};\n\n} // namespace SZM::Machines::Generated\n"
        
        return header
    
    def generate_source(self, machine: GeneratedMachine, class_name: str) -> str:
        """Generate C++ source file."""
        
        spec = machine.specification
        
        source = f"""#include "{class_name}.hpp"

namespace SZM::Machines::Generated {{

{class_name}::{class_name}() {{
    Initialize();
}}

void {class_name}::Initialize() {{
    // Initialize machine components
"""
        
        for comp in spec.get('components', []):
            source += f"    // {comp['name']} initialization\n"
        
        source += """}

void {class_name}::Update(float delta_time) {{
    // Update machine simulation
""".format(class_name=class_name)
        
        for behavior in spec.get('behaviors', []):
            source += f"    // Behavior: {behavior.get('name', 'unnamed')}\n"
        
        source += """}

"""
        
        for comp in spec.get('components', []):
            for param in comp.get('parameters', []):
                param_name = param['name']
                setter = f"Set{self.to_pascal_case(param_name)}"
                getter = f"Get{self.to_pascal_case(param_name)}"
                
                source += f"void {class_name}::{setter}(float value) {{\n"
                source += f"    m_{param_name} = value;\n"
                source += "}\n\n"
                
                source += f"float {class_name}::{getter}() const {{\n"
                source += f"    return m_{param_name};\n"
                source += "}\n\n"
        
        source += "} // namespace SZM::Machines::Generated\n"
        
        return source
    
    def generate_cmake(self, machine: GeneratedMachine, class_name: str) -> str:
        """Generate CMake snippet."""
        
        return f"""# Generated machine: {machine.name}
add_library({class_name} STATIC)
target_sources({class_name} PRIVATE ${{CMAKE_CURRENT_BINARY_DIR}}/{class_name}.cpp)
target_include_directories({class_name} PUBLIC ${{CMAKE_CURRENT_SOURCE_DIR}})
"""
    
    @staticmethod
    def to_camel_case(name: str) -> str:
        """Convert name to CamelCase."""
        return ''.join(word.capitalize() for word in name.replace('-', ' ').split())
    
    @staticmethod
    def to_pascal_case(name: str) -> str:
        """Convert name to PascalCase."""
        return ''.join(word.capitalize() for word in name.replace('_', ' ').split())


# =============================================================================
# MAIN PIPELINE
# =============================================================================

class MachineLearningPipeline:
    """
    Main pipeline for machine learning workflow.
    """
    
    def __init__(self, data_dir: str = './machine_data'):
        self.data_dir = Path(data_dir)
        self.machines = []
        self.patterns = []
        self.pattern_engine = PatternLearningEngine()
        self.generator = None
        self.code_generator = CodeGenerator()
    
    def ingest_data(self) -> int:
        """Ingest machine data from files."""
        print(f"Ingesting machine data from: {self.data_dir}")
        
        count = 0
        for filepath in self.data_dir.rglob('*.json'):
            spec = MachineDataParser.parse_json_spec(str(filepath))
            if spec:
                self.machines.append(spec)
                count += 1
                print(f"  Loaded: {spec.name}")
        
        print(f"Ingested {count} machines")
        return count
    
    def learn_patterns(self) -> int:
        """Learn patterns from ingested machines."""
        print("Learning patterns...")
        
        self.patterns = self.pattern_engine.learn_from_machines(self.machines)
        self.generator = MachineGenerator(self.patterns)
        
        print(f"Learned {len(self.patterns)} patterns")
        return len(self.patterns)
    
    def generate_machines(
        self,
        machine_type: str,
        count: int = 1,
        parameters: Dict[str, float] = None
    ) -> List[GeneratedMachine]:
        """Generate new machines."""
        if not self.generator:
            print("Error: No patterns learned yet")
            return []
        
        params = parameters or self.get_typical_parameters(machine_type)
        
        machines = []
        for i in range(count):
            machine = self.generator.generate_machine(machine_type, params)
            machines.append(machine)
        
        return machines
    
    def get_typical_parameters(self, machine_type: str) -> Dict[str, float]:
        """Get typical parameters for a machine type."""
        # Extract from patterns
        param_patterns = [p for p in self.patterns if p.pattern_type == 'parameter_range']
        
        params = {}
        for pattern in param_patterns:
            if 'mean' in pattern.pattern_data:
                params[pattern.id.replace('param_range_', '')] = pattern.pattern_data['mean']
        
        # Add defaults if not enough
        if machine_type == 'cnc_mill':
            params.setdefault('power', 5.0)
            params.setdefault('spindle_speed', 3000.0)
            params.setdefault('travel_x', 1000.0)
            params.setdefault('travel_y', 500.0)
            params.setdefault('travel_z', 500.0)
        elif machine_type == 'robot_arm':
            params.setdefault('degrees_of_freedom', 6.0)
            params.setdefault('payload', 10.0)
            params.setdefault('reach', 1500.0)
        elif machine_type == 'printer_3d':
            params.setdefault('build_volume_x', 200.0)
            params.setdefault('build_volume_y', 200.0)
            params.setdefault('build_volume_z', 200.0)
            params.setdefault('layer_height', 0.2)
        
        return params
    
    def generate_code(self, machine: GeneratedMachine) -> Dict[str, str]:
        """Generate code for a machine."""
        return self.code_generator.generate_machine_code(machine)
    
    def save_output(self, machine: GeneratedMachine, output_dir: str = './generated'):
        """Save generated machine to files."""
        output_path = Path(output_dir)
        output_path.mkdir(exist_ok=True)
        
        # Generate and save code
        code = self.generate_code(machine)
        
        for filename, content in [('header_content', 'header'), ('source_content', 'source')]:
            filepath = output_path / f"{machine.id}_{code[filename.replace('_content', '')]}"
            with open(filepath, 'w') as f:
                f.write(code[filename])
            print(f"Saved: {filepath}")
        
        # Save specification
        spec_path = output_path / f"{machine.id}_spec.json"
        with open(spec_path, 'w') as f:
            json.dump(machine.specification, f, indent=2)
        
        # Save metadata
        meta = {
            'id': machine.id,
            'name': machine.name,
            'machine_type': machine.machine_type,
            'confidence': machine.confidence_score,
            'generated_at': datetime.now().isoformat(),
            'files': {
                'header': code['header'],
                'source': code['source'],
                'specification': f"{machine.id}_spec.json"
            }
        }
        meta_path = output_path / f"{machine.id}_meta.json"
        with open(meta_path, 'w') as f:
            json.dump(meta, f, indent=2)
        
        return str(output_path)


# =============================================================================
# MAIN ENTRY POINT
# =============================================================================

def main():
    parser = argparse.ArgumentParser(description="SZM Forge Machine Learning Training")
    parser.add_argument('--data_dir', default='./machine_data', help='Machine data directory')
    parser.add_argument('--output_dir', default='./generated', help='Output directory')
    parser.add_argument('--ingest', action='store_true', help='Ingest machine data')
    parser.add_argument('--learn', action='store_true', help='Learn patterns')
    parser.add_argument('--generate', action='store_true', help='Generate machines')
    parser.add_argument('--type', default='cnc_mill', help='Machine type to generate')
    parser.add_argument('--count', type=int, default=1, help='Number of machines to generate')
    parser.add_argument('--params', help='Parameters as JSON string')
    
    args = parser.parse_args()
    
    # Create pipeline
    pipeline = MachineLearningPipeline(args.data_dir)
    
    # Ingest data
    if args.ingest or args.learn:
        count = pipeline.ingest_data()
        if count == 0:
            print("No machine data found. Creating sample data...")
            create_sample_data(args.data_dir)
            count = pipeline.ingest_data()
    
    # Learn patterns
    if args.learn:
        pattern_count = pipeline.learn_patterns()
        print(f"Learned {pattern_count} patterns")
    
    # Generate machines
    if args.generate:
        # Learn first if not done
        if not pipeline.patterns:
            pipeline.ingest_data()
            pipeline.learn_patterns()
        
        # Parse parameters
        params = json.loads(args.params) if args.params else None
        
        # Generate machines
        machines = pipeline.generate_machines(args.type, args.count, params)
        
        for machine in machines:
            print(f"\nGenerated: {machine.name}")
            print(f"  Confidence: {machine.confidence_score:.1%}")
            print(f"  Components: {len(machine.specification['components'])}")
            
            # Save output
            output = pipeline.save_output(machine, args.output_dir)
            print(f"  Saved to: {output}")
    
    return 0


def create_sample_data(output_dir: str):
    """Create sample machine data for testing."""
    sample_dir = Path(output_dir)
    sample_dir.mkdir(exist_ok=True)
    
    # Sample CNC Mill
    cnc_mill = {
        'id': 'cnc_mill_001',
        'name': 'Sample CNC Mill',
        'manufacturer': 'Generic',
        'model': 'CNC-1000',
        'type': 'cnc_mill',
        'components': [
            {
                'id': 'spindle_001',
                'name': 'Main Spindle',
                'type': 'spindle',
                'parameters': [
                    {'name': 'power', 'category': 'performance', 'min': 1, 'max': 15, 'default': 5},
                    {'name': 'max_speed', 'category': 'performance', 'min': 1000, 'max': 10000, 'default': 5000},
                    {'name': 'torque', 'category': 'performance', 'min': 10, 'max': 100, 'default': 50}
                ]
            },
            {
                'id': 'table_001',
                'name': 'Work Table',
                'type': 'linear_motion',
                'parameters': [
                    {'name': 'travel_x', 'category': 'geometry', 'min': 500, 'max': 2000, 'default': 1000},
                    {'name': 'travel_y', 'category': 'geometry', 'min': 300, 'max': 1000, 'default': 500},
                    {'name': 'travel_z', 'category': 'geometry', 'min': 300, 'max': 800, 'default': 500}
                ]
            }
        ],
        'capabilities': ['kinematics', 'dynamics'],
        'tags': ['cnc', 'milling', '3-axis']
    }
    
    with open(sample_dir / 'sample_cnc_mill.json', 'w') as f:
        json.dump(cnc_mill, f, indent=2)
    
    # Sample Robot Arm
    robot_arm = {
        'id': 'robot_arm_001',
        'name': 'Sample Robot Arm',
        'manufacturer': 'Generic',
        'model': 'RA-6X',
        'type': 'robot_arm',
        'components': [
            {
                'id': 'base_001',
                'name': 'Base Joint',
                'type': 'rotary_joint',
                'parameters': [
                    {'name': 'max_torque', 'category': 'performance', 'min': 100, 'max': 1000, 'default': 500},
                    {'name': 'max_speed', 'category': 'performance', 'min': 50, 'max': 200, 'default': 100}
                ]
            },
            {
                'id': 'arm_001',
                'name': 'Arm Segment',
                'type': 'linkage',
                'parameters': [
                    {'name': 'length', 'category': 'geometry', 'min': 500, 'max': 2000, 'default': 1000},
                    {'name': 'weight', 'category': 'physical', 'min': 5, 'max': 50, 'default': 20}
                ]
            }
        ],
        'capabilities': ['kinematics', 'inverse_kinematics'],
        'tags': ['robot', 'arm', '6-axis']
    }
    
    with open(sample_dir / 'sample_robot_arm.json', 'w') as f:
        json.dump(robot_arm, f, indent=2)
    
    print(f"Created sample data in: {output_dir}")


if __name__ == '__main__':
    sys.exit(main())
