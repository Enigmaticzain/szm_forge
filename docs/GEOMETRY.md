# SZM Forge — Geometry Engine Documentation

## Overview

The Geometry Engine (`ai_service/geometry_engine.py`) provides Python-based parametric solid modeling, mesh processing, 2D geometry, format conversion, and assembly support for the SZM Forge platform.

## Library Matrix

| Library | Version | License | Purpose |
|---------|---------|---------|---------|
| **build123d** | 0.11.1 | Apache 2.0 | Parametric solid modeling (wraps OpenCASCADE) |
| **trimesh** | 5.0.0 | MIT | Mesh processing, boolean ops, analysis |
| **shapely** | 2.1.2 | BSD | 2D geometric operations |
| **numpy-stl** | 4.0.0 | BSD | Fast STL read/write |
| **ezdxf** | 1.4.4 | MIT | DXF read/write (AutoCAD format) |
| **scipy** | (existing) | BSD | Spatial math, KD-trees |
| **pyvista** | 0.48.4 | MIT | 3D visualization |

## Capabilities

### 1. Smart Part Creation (build123d)

Create parametric engineering parts. All dimensions in **mm**.

```python
from geometry_engine import (
    create_bracket, create_shaft, create_flange,
    create_housing, create_gear, create_plate,
    export_step
)

# L-bracket with 2 holes and 1mm fillets
bracket = create_bracket(length=80, width=40, thickness=5, holes=2, fillet_r=1.0)
export_step(bracket, "bracket.step")

# Shaft with keyway and chamfered ends
shaft = create_shaft(diameter=20, length=100, keyway=True, chamfer=True)

# Flange with 6-bolt pattern
flange = create_flange(outer_d=80, inner_d=30, thickness=10, bolt_holes=6, bolt_d=8)

# Box housing with internal bore
housing = create_housing(length=120, width=80, height=60, wall_thickness=5, bore_d=30)

# Spur gear (simplified involute)
gear = create_gear(module=2, num_teeth=20, thickness=10, bore_d=10)

# Plate with grid hole pattern
plate = create_plate(length=200, width=100, thickness=5, hole_pattern="grid")
```

### 2. 2D Profile Builder (shapely)

```python
from geometry_engine import create_2d_profile, offset_profile, profile_to_3d

# Pentagon profile
profile = create_2d_profile([(0,0), (100,0), (100,50), (50,75), (0,50)])

# Offset inward by 3mm (for wall thickness)
inner = offset_profile(profile, -3.0)

# Extrude to 3D
solid = profile_to_3d(profile, height=10)
```

### 3. Mesh Operations (trimesh)

```python
from geometry_engine import import_mesh, repair_mesh, mesh_properties, simplify_mesh

mesh = import_mesh("part.stl")
mesh = repair_mesh(mesh)             # Fix normals, fill holes
props = mesh_properties(mesh)        # volume, area, bbox, center of mass
simple = simplify_mesh(mesh, 5000)   # Reduce to 5000 faces
```

### 4. Format Conversion

```python
from geometry_engine import export_step, export_stl, export_dxf, convert_format

export_step(part, "part.step")       # STEP (for manufacturing)
export_stl(part, "part.stl")         # STL (for 3D printing)
export_dxf(profile_2d, "cut.dxf")   # DXF (for laser cutting)
convert_format("part.step", "part.stl")  # Auto-convert
```

### 5. Geometry Analysis

```python
from geometry_engine import check_interference, calculate_clearance, fit_bounding_box, check_manufacturability

result = check_interference(part1, part2)   # {"interferes": bool}
dist = calculate_clearance(part1, part2)    # min distance in mm
bbox = fit_bounding_box(part)               # extents, volume, center
warnings = check_manufacturability(part, process="cnc")
```

### 6. Assembly Support

```python
from geometry_engine import create_assembly, add_to_assembly, export_assembly

asm = create_assembly("MyAssembly")
add_to_assembly(asm, shaft, "Shaft", position=(0, 0, 0))
add_to_assembly(asm, flange, "Flange", position=(0, 0, 50))
export_assembly(asm, "assembly.step")
```

## Installation

```bash
pip install build123d trimesh[easy] shapely numpy-stl ezdxf scipy pyvista
```

## File Locations

| File | Description |
|------|-------------|
| `ai_service/geometry_engine.py` | Main geometry module |
| `ai_service/test_geometry_engine.py` | Test suite (16 tests) |
| `ai_service/geometry_output/` | Generated STEP/STL/DXF files |
| `ai_service/text_to_cad.py` | Existing text-to-CAD pipeline (unchanged) |
| `src/Geometry/` | Existing C++ geometry module (unchanged) |
