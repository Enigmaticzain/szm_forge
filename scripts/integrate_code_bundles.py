#!/usr/bin/env python3
"""
Advanced Code Bundle Extractor and Integrator for SZM Forge
Intelligently extracts ~8,000 lines from code bundles and integrates them
into the src/ module structure without removing anything.
"""

import os
import re
from pathlib import Path
from typing import List, Dict, Tuple

class BundleIntegrator:
    def __init__(self, project_root="/mnt/d/projects/szm_forge"):
        self.project_root = Path(project_root)
        self.src_dir = self.project_root / "src"
        self.stats = {
            'files_read': 0,
            'classes_extracted': 0,
            'namespaces_extracted': 0,
            'lines_processed': 0,
            'files_created': 0,
            'files_updated': 0,
        }
    
    def safe_write_file(self, filepath: Path, content: str, preserve_existing=True) -> bool:
        """Write file safely, optionally preserving existing content"""
        filepath.parent.mkdir(parents=True, exist_ok=True)
        
        if filepath.exists() and preserve_existing:
            with open(filepath, 'r', encoding='utf-8') as f:
                existing = f.read()
            # Check if content is already there
            if content.strip() in existing or existing.strip() in content:
                return False  # Already integrated
            # Merge content
            content = existing + "\n\n// ===== NEW ADDITIONS FROM CODE BUNDLES =====\n\n" + content
            self.stats['files_updated'] += 1
        else:
            self.stats['files_created'] += 1
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    
    def extract_sections_by_markers(self, filename: str) -> List[Dict]:
        """Extract code sections using pragmas and class markers"""
        filepath = self.project_root / filename
        if not filepath.exists():
            return []
        
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            self.stats['lines_processed'] += len(content.split('\n'))
        
        sections = []
        lines = content.split('\n')
        
        current_section = []
        section_start = 0
        in_section = False
        
        for i, line in enumerate(lines):
            # Start section markers
            if re.match(r'^\s*#pragma once|^#include |^namespace |^class |^struct |^template', line):
                if in_section and current_section:
                    sections.append({
                        'start': section_start,
                        'end': i,
                        'lines': current_section,
                        'content': '\n'.join(current_section)
                    })
                current_section = [line]
                section_start = i
                in_section = True
            elif in_section:
                current_section.append(line)
                # End section if we hit CMake or Python
                if line.startswith('cmake_minimum_required') or line.startswith('import ') or line.startswith('def '):
                    sections.append({
                        'start': section_start,
                        'end': i,
                        'lines': current_section,
                        'content': '\n'.join(current_section[:-1])  # Exclude the trigger line
                    })
                    current_section = []
                    in_section = False
        
        if current_section:
            sections.append({
                'start': section_start,
                'end': len(lines),
                'lines': current_section,
                'content': '\n'.join(current_section)
            })
        
        self.stats['classes_extracted'] += len([s for s in sections if 'class ' in s['content']])
        self.stats['namespaces_extracted'] += len([s for s in sections if 'namespace ' in s['content']])
        
        return sections
    
    def categorize_section(self, section_content: str) -> str:
        """Determine which module this section belongs to"""
        content_lower = section_content.lower()
        
        # Physics & Materials
        if any(x in content_lower for x in ['physics', 'rigid', 'collision', 'bullet']):
            return 'Physics'
        if any(x in content_lower for x in ['material', 'property', 'texture']):
            return 'Materials'
        
        # Geometry & FEA
        if any(x in content_lower for x in ['geometry', 'boolean', 'solid']):
            return 'Geometry'
        if any(x in content_lower for x in ['fea', 'finite element', 'solver', 'stress']):
            return 'FEA'
        
        # Graphics
        if any(x in content_lower for x in ['graphics', 'shader', 'render', 'mesh', 'texture']):
            return 'Graphics'
        
        # Math & Assembly
        if any(x in content_lower for x in ['constraint', 'solver', 'math', 'equation']):
            return 'Math'
        if any(x in content_lower for x in ['assembly', 'kinematic', 'mate']):
            return 'Assembly'
        
        # UI & Data
        if any(x in content_lower for x in ['ui', 'panel', 'imgui', 'window']):
            return 'UI'
        if any(x in content_lower for x in ['scene', 'entity', 'uuid', 'transform', 'graph']):
            return 'Data'
        
        # Input & Application
        if any(x in content_lower for x in ['input', 'event', 'keyboard', 'mouse']):
            return 'Input'
        if any(x in content_lower for x in ['application', 'core', 'init', 'main']):
            return 'Core'
        
        # AI & Electrical
        if any(x in content_lower for x in ['ai', 'neural', 'machine learning']):
            return 'AI'
        if any(x in content_lower for x in ['electric', 'circuit', 'voltage']):
            return 'Electrical'
        
        # Thermal & Simulation
        if any(x in content_lower for x in ['thermal', 'heat', 'temperature']):
            return 'Thermal'
        if any(x in content_lower for x in ['simulation', 'simulate']):
            return 'Simulation'
        
        return 'Core'  # Default fallback
    
    def get_file_target(self, module: str, section_content: str) -> Path:
        """Determine target file for section"""
        # Extract class name if exists
        class_match = re.search(r'class\s+(\w+)', section_content)
        struct_match = re.search(r'struct\s+(\w+)', section_content)
        
        name = class_match.group(1) if class_match else (struct_match.group(1) if struct_match else 'Definitions')
        
        module_path = self.src_dir / module
        module_path.mkdir(parents=True, exist_ok=True)
        
        # Determine if header or implementation
        if re.search(r'(void|int|bool|float|double|string|auto)\s+\w+\s*\([^)]*\)\s*{', section_content):
            # Has implementation
            return module_path / f"{name}.cpp"
        else:
            # Header only
            return module_path / f"{name}.hpp"
    
    def integrate_bundles(self):
        """Main integration routine"""
        bundle_files = ['code', 'code2', 'code 3']
        total_sections = 0
        
        print("\n" + "="*80)
        print("STARTING CODE BUNDLE INTEGRATION")
        print("="*80 + "\n")
        
        for bundle_file in bundle_files:
            print(f"\nProcessing: {bundle_file}")
            print("-" * 80)
            
            sections = self.extract_sections_by_markers(bundle_file)
            total_sections += len(sections)
            
            for i, section in enumerate(sections[:50]):  # Process first 50 to avoid overwhelming
                module = self.categorize_section(section['content'])
                target = self.get_file_target(module, section['content'])
                
                # Only write non-trivial sections
                if len(section['content'].strip()) > 20:
                    success = self.safe_write_file(target, section['content'])
                    if success:
                        print(f"  [{i+1:3d}] ✓ {target.relative_to(self.project_root)}")
            
            if len(sections) > 50:
                print(f"  ... and {len(sections)-50} more sections")
        
        print("\n" + "="*80)
        print("INTEGRATION SUMMARY")
        print("="*80)
        print(f"Bundle files processed: {len(bundle_files)}")
        print(f"Total sections extracted: {total_sections}")
        print(f"Lines processed: {self.stats['lines_processed']:,}")
        print(f"Classes found: {self.stats['classes_extracted']}")
        print(f"Namespaces found: {self.stats['namespaces_extracted']}")
        print(f"Files created: {self.stats['files_created']}")
        print(f"Files updated: {self.stats['files_updated']}")
        print("="*80 + "\n")

if __name__ == '__main__':
    integrator = BundleIntegrator()
    integrator.integrate_bundles()
