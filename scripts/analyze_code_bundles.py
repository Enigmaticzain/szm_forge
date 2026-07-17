#!/usr/bin/env python3
"""
Comprehensive Code Bundle Extractor for SZM Forge
Extracts classes, functions, and structures from bundled code files
and places them in appropriate module directories.
"""

import os
import re
import sys
from pathlib import Path
from typing import List, Tuple, Dict

class CodeExtractor:
    def __init__(self, project_root="/mnt/d/projects/szm_forge"):
        self.project_root = Path(project_root)
        self.src_dir = self.project_root / "src"
        self.mappings = {
            # Map class names to target directories
            'Application': 'Core',
            'EventBus': 'Core',
            'IPanel': 'UI',
            'UIManager': 'UI',
            'SceneOutliner': 'UI/panels',
            'ConsolePanel': 'UI/panels',
            'SceneGraph': 'Data',
            'Entity': 'Data',
            'UUID': 'Data',
            'UUIDGenerator': 'Data',
            'Transform': 'Data',
            'Physics': 'Physics',
            'RigidBody': 'Physics',
            'Collision': 'Physics',
            'Material': 'Materials',
            'Geometry': 'Geometry',
            'Boolean': 'Geometry/operations',
            'Mesh': 'Graphics',
            'Shader': 'Graphics',
            'Camera': 'Graphics',
            'FEA': 'FEA',
            'Constraint': 'Math',
            'Solver': 'Math',
        }
        
    def extract_file_bundles(self) -> Dict[str, Tuple[int, int]]:
        """Map source code files to line ranges for extraction"""
        bundles = {}
        
        # Read and parse code files
        for bundle_file in ['code', 'code2', 'code 3']:
            bundle_path = self.project_root / bundle_file
            if bundle_path.exists():
                with open(bundle_path, 'r', encoding='utf-8') as f:
                    lines = f.readlines()
                bundles[bundle_file] = len(lines)
        
        return bundles
    
    def find_class_definitions(self, filename: str) -> List[Dict]:
        """Find all class definitions and their boundaries"""
        filepath = self.project_root / filename
        if not filepath.exists():
            return []
        
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            lines = content.split('\n')
        
        classes = []
        in_class = False
        brace_depth = 0
        class_start = 0
        class_name = ""
        
        for i, line in enumerate(lines):
            # Detect class start
            match = re.match(r'^\s*(class|struct)\s+(\w+)', line)
            if match and not in_class:
                in_class = True
                class_name = match.group(2)
                class_start = i
                brace_depth = 0
                class_type = match.group(1)
            
            if in_class:
                brace_depth += line.count('{') - line.count('}')
                
                # Detect class end
                if brace_depth == 0 and '{' in lines[class_start]:
                    if i > class_start:  # Ensure we have content
                        classes.append({
                            'name': class_name,
                            'type': class_type,
                            'start': class_start,
                            'end': i + 1,
                            'content': '\n'.join(lines[class_start:i+1])
                        })
                        in_class = False
        
        return classes
    
    def get_target_module(self, name: str) -> str:
        """Determine target module based on class name"""
        for key, module in self.mappings.items():
            if key.lower() in name.lower():
                return module
        
        # Default mappings
        if 'Physics' in name or 'Rigid' in name or 'Collision' in name:
            return 'Physics'
        elif 'Material' in name:
            return 'Materials'
        elif 'Geometry' in name or 'Boolean' in name:
            return 'Geometry'
        elif 'FEA' in name or 'Mesh' in name:
            return 'FEA'
        elif 'Shader' in name or 'Mesh' in name or 'Graphics' in name:
            return 'Graphics'
        elif 'Constraint' in name or 'Solver' in name:
            return 'Math'
        elif 'UI' in name or 'Panel' in name:
            return 'UI'
        elif 'Scene' in name or 'Entity' in name or 'UUID' in name:
            return 'Data'
        else:
            return 'Core'
    
    def count_total_lines(self) -> int:
        """Count total lines in all bundles"""
        total = 0
        for bundle_file in ['code', 'code2', 'code 3']:
            bundle_path = self.project_root / bundle_file
            if bundle_path.exists():
                with open(bundle_path, 'r', encoding='utf-8') as f:
                    total += len(f.readlines())
        return total

if __name__ == '__main__':
    extractor = CodeExtractor()
    
    print("=" * 70)
    print("SZM FORGE - CODE BUNDLE ANALYSIS")
    print("=" * 70)
    print()
    
    # Analyze bundles
    bundles = extractor.extract_file_bundles()
    print("BUNDLE FILES FOUND:")
    total_lines = 0
    for name, size in bundles.items():
        print(f"  {name:20s} {size:6d} lines")
        total_lines += size
    print(f"  {'TOTAL':20s} {total_lines:6d} lines")
    print()
    
    # Analyze classes
    print("CLASS DEFINITIONS FOUND:")
    for bundle_file in ['code', 'code2']:
        classes = extractor.find_class_definitions(bundle_file)
        print(f"\n  {bundle_file}:")
        for cls in classes[:10]:
            target = extractor.get_target_module(cls['name'])
            print(f"    ✓ {cls['name']:30s} → {target}/")
        if len(classes) > 10:
            print(f"    ... and {len(classes)-10} more")
    
    print()
    print("=" * 70)
    print(f"TOTAL CODE LINES AVAILABLE: {total_lines}")
    print("EXTRACTION READY")
    print("=" * 70)
