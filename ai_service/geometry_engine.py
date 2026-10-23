"""SZM Forge Geometry Engine — Unified Python geometry module.

Uses best-in-class open-source libraries:
  - build123d (solid modeling, STEP export)
  - trimesh (mesh processing)
  - shapely (2D geometry)
  - numpy-stl (fast STL I/O)
  - ezdxf (DXF I/O)
  - scipy.spatial (spatial math)
"""
from __future__ import annotations
import os
import math
import logging
from typing import Any, Optional

logger = logging.getLogger("GeometryEngine")

# ── Library availability detection ────────────────────────────────────────────
_HAS_BUILD123D = False
_HAS_TRIMESH = False
_HAS_SHAPELY = False
_HAS_NUMPY_STL = False
_HAS_EZDXF = False
_HAS_SCIPY = False
_HAS_CADQUERY = False
_HAS_SKSPATIAL = False

try:
    import build123d as bd
    _HAS_BUILD123D = True
except ImportError:
    logger.warning("build123d not available")

try:
    import cadquery as cq
    _HAS_CADQUERY = True
except ImportError:
    logger.warning("cadquery not available")

try:
    from skspatial.objects import Plane, Sphere, Line
    from skspatial.objects import Points as SkPoints
    _HAS_SKSPATIAL = True
except ImportError:
    logger.warning("scikit-spatial not available")

try:
    import trimesh
    _HAS_TRIMESH = True
except ImportError:
    logger.warning("trimesh not available")

try:
    import shapely
    from shapely.geometry import Polygon, MultiPolygon
    from shapely import affinity
    _HAS_SHAPELY = True
except ImportError:
    logger.warning("shapely not available")

try:
    import stl
    from stl import mesh as stl_mesh
    _HAS_NUMPY_STL = True
except ImportError:
    logger.warning("numpy-stl not available")

try:
    import ezdxf
    _HAS_EZDXF = True
except ImportError:
    logger.warning("ezdxf not available")

try:
    from scipy.spatial import ConvexHull, KDTree
    _HAS_SCIPY = True
except ImportError:
    logger.warning("scipy not available")

import numpy as np


def _validate_positive(**kwargs: float) -> None:
    """Raise ValueError if any dimension is not positive."""
    for name, val in kwargs.items():
        if val <= 0:
            raise ValueError(f"{name} must be positive, got {val}")


def get_available_libraries() -> dict[str, bool]:
    """Return dict of library availability."""
    return {
        "build123d": _HAS_BUILD123D,
        "cadquery": _HAS_CADQUERY,
        "trimesh": _HAS_TRIMESH,
        "shapely": _HAS_SHAPELY,
        "numpy_stl": _HAS_NUMPY_STL,
        "ezdxf": _HAS_EZDXF,
        "scipy": _HAS_SCIPY,
        "scikit_spatial": _HAS_SKSPATIAL,
    }


def fit_plane_to_points(points: list[list[float]]) -> dict[str, Any]:
    """Fit a plane to 3D points using scikit-spatial."""
    if not _HAS_SKSPATIAL:
        raise RuntimeError("scikit-spatial required for plane fitting")
    pts = SkPoints(points)
    plane = Plane.best_fit(pts)
    return {
        "normal": list(plane.normal),
        "point": list(plane.point),
        "rmse": float(plane.distance_points(pts).mean()),
    }


def fit_sphere_to_points(points: list[list[float]]) -> dict[str, Any]:
    """Fit a sphere to 3D points."""
    if not _HAS_SKSPATIAL:
        raise RuntimeError("scikit-spatial required for sphere fitting")
    pts = SkPoints(points)
    sphere = Sphere.best_fit(pts)
    return {
        "center": list(sphere.center),
        "radius": float(sphere.radius),
        "rmse": float(sphere.distance_points(pts).mean()),
    }


def fit_line_to_points(points: list[list[float]]) -> dict[str, Any]:
    """Fit a 3D line to points using scikit-spatial."""
    if not _HAS_SKSPATIAL:
        raise RuntimeError("scikit-spatial required for line fitting")
    pts = SkPoints(points)
    line = Line.best_fit(pts)
    return {
        "point": list(line.point),
        "direction": list(line.direction),
        "rmse": float(line.distance_points(pts).mean()),
    }


def create_box_cad(
    length: float,
    width: float,
    height: float,
    backend: str = "auto",
) -> tuple[Any, str]:
    """Create box using cad_backends abstraction."""
    from cad_backends import create_box
    return create_box(length, width, height, backend=backend)


# ═════════════════════════════════════════════════════════════════════════════
# CAPABILITY 1: SMART PART CREATION (build123d)
# ═════════════════════════════════════════════════════════════════════════════

def create_bracket(
    length: float = 100.0,
    width: float = 50.0,
    thickness: float = 5.0,
    holes: int = 2,
    fillet_r: float = 2.0,
) -> Any:
    """Create an L-bracket with configurable holes and fillets. Dims in mm."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for create_bracket")
    _validate_positive(length=length, width=width, thickness=thickness)

    with bd.BuildPart() as part:
        # Horizontal leg
        with bd.BuildSketch():
            bd.Rectangle(length, width)
        bd.extrude(amount=thickness)
        # Vertical leg
        with bd.BuildSketch(bd.Plane.XZ.offset(width / 2 - thickness)):
            with bd.Locations((0, length / 4)):
                bd.Rectangle(length, length / 2)
        bd.extrude(amount=thickness)
        # Holes on horizontal leg
        if holes > 0:
            with bd.BuildSketch(part.faces().sort_by(bd.Axis.Z)[-1]):
                spacing = length / (holes + 1)
                for i in range(holes):
                    x = -length / 2 + spacing * (i + 1)
                    with bd.Locations((x, 0)):
                        bd.Circle(thickness * 0.8)
            bd.extrude(amount=-thickness, mode=bd.Mode.SUBTRACT)
        # Fillets
        if fillet_r > 0:
            try:
                bd.fillet(part.edges(), radius=min(fillet_r, thickness * 0.4))
            except Exception:
                pass  # Some edges can't be filleted
    return part.part


def create_shaft(
    diameter: float = 20.0,
    length: float = 100.0,
    keyway: bool = False,
    chamfer: bool = True,
) -> Any:
    """Create a cylindrical shaft. Dims in mm."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for create_shaft")
    _validate_positive(diameter=diameter, length=length)
    r = diameter / 2

    with bd.BuildPart() as part:
        with bd.BuildSketch():
            bd.Circle(r)
        bd.extrude(amount=length)
        if keyway:
            kw = diameter * 0.25
            kd = diameter * 0.08
            with bd.BuildSketch(part.faces().sort_by(bd.Axis.Z)[-1]):
                with bd.Locations((0, r - kd / 2)):
                    bd.Rectangle(kw, kd)
            bd.extrude(amount=-length, mode=bd.Mode.SUBTRACT)
        if chamfer:
            try:
                top_bottom = (
                    part.edges().sort_by(bd.Axis.Z)[-1:] +
                    part.edges().sort_by(bd.Axis.Z)[:1]
                )
                bd.chamfer(top_bottom, length=diameter * 0.05)
            except Exception:
                pass
    return part.part


def create_flange(
    outer_d: float = 80.0,
    inner_d: float = 30.0,
    thickness: float = 10.0,
    bolt_holes: int = 6,
    bolt_d: float = 8.0,
) -> Any:
    """Create a circular flange with bolt pattern. Dims in mm."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for create_flange")
    _validate_positive(outer_d=outer_d, inner_d=inner_d, thickness=thickness, bolt_d=bolt_d)
    if inner_d >= outer_d:
        raise ValueError("inner_d must be less than outer_d")

    with bd.BuildPart() as part:
        with bd.BuildSketch():
            bd.Circle(outer_d / 2)
            bd.Circle(inner_d / 2, mode=bd.Mode.SUBTRACT)
        bd.extrude(amount=thickness)
        if bolt_holes > 0:
            bolt_circle_r = (outer_d / 2 + inner_d / 2) / 2
            with bd.BuildSketch(part.faces().sort_by(bd.Axis.Z)[-1]):
                with bd.PolarLocations(bolt_circle_r, bolt_holes):
                    bd.Circle(bolt_d / 2)
            bd.extrude(amount=-thickness, mode=bd.Mode.SUBTRACT)
    return part.part


def create_housing(
    length: float = 120.0,
    width: float = 80.0,
    height: float = 60.0,
    wall_thickness: float = 5.0,
    bore_d: float = 30.0,
) -> Any:
    """Create a box housing with internal bore. Dims in mm."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for create_housing")
    _validate_positive(length=length, width=width, height=height, wall_thickness=wall_thickness)

    with bd.BuildPart() as part:
        with bd.BuildSketch():
            bd.Rectangle(length, width)
        bd.extrude(amount=height)
        with bd.BuildSketch(part.faces().sort_by(bd.Axis.Z)[-1]):
            bd.Rectangle(length - 2 * wall_thickness, width - 2 * wall_thickness)
        bd.extrude(amount=-(height - wall_thickness), mode=bd.Mode.SUBTRACT)
        if bore_d > 0:
            with bd.BuildSketch(bd.Plane.XZ):
                bd.Circle(bore_d / 2)
            bd.extrude(amount=width, both=True, mode=bd.Mode.SUBTRACT)
    return part.part


def create_gear(
    module: float = 2.0,
    num_teeth: int = 20,
    thickness: float = 10.0,
    bore_d: float = 10.0,
) -> Any:
    """Create a simplified spur gear. Dims in mm."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for create_gear")
    _validate_positive(module=module, thickness=thickness, bore_d=bore_d)
    if num_teeth < 6:
        raise ValueError("num_teeth must be >= 6")

    pitch_r = module * num_teeth / 2
    outer_r = pitch_r + module
    root_r = pitch_r - 1.25 * module
    tooth_angle = 360.0 / num_teeth

    with bd.BuildPart() as part:
        with bd.BuildSketch():
            bd.Circle(root_r)
            for i in range(num_teeth):
                angle = i * tooth_angle
                a_rad = math.radians(angle)
                hw = module * 0.8
                cx, cy = pitch_r * math.cos(a_rad), pitch_r * math.sin(a_rad)
                with bd.Locations((cx, cy)):
                    bd.Circle(hw)
            bd.Circle(bore_d / 2, mode=bd.Mode.SUBTRACT)
        bd.extrude(amount=thickness)
    return part.part


def create_plate(
    length: float = 200.0,
    width: float = 100.0,
    thickness: float = 5.0,
    hole_pattern: str = "grid",
    hole_d: float = 6.0,
    hole_rows: int = 2,
    hole_cols: int = 4,
) -> Any:
    """Create a flat plate with hole pattern ('grid', 'linear', 'circular'). Dims in mm."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for create_plate")
    _validate_positive(length=length, width=width, thickness=thickness, hole_d=hole_d)

    with bd.BuildPart() as part:
        with bd.BuildSketch():
            bd.Rectangle(length, width)
        bd.extrude(amount=thickness)

        top_face = part.faces().sort_by(bd.Axis.Z)[-1]
        if hole_pattern in ("grid", "linear", "circular") and (hole_cols > 0 or hole_rows > 0):
            with bd.BuildSketch(top_face):
                if hole_pattern == "grid":
                    sx = length / (hole_cols + 1)
                    sy = width / (hole_rows + 1)
                    for r in range(hole_rows):
                        for c in range(hole_cols):
                            x = -length / 2 + sx * (c + 1)
                            y = -width / 2 + sy * (r + 1)
                            with bd.Locations((x, y)):
                                bd.Circle(hole_d / 2)
                elif hole_pattern == "linear":
                    sx = length / (hole_cols + 1)
                    for c in range(hole_cols):
                        x = -length / 2 + sx * (c + 1)
                        with bd.Locations((x, 0)):
                            bd.Circle(hole_d / 2)
                elif hole_pattern == "circular":
                    n = hole_cols * hole_rows
                    cr = min(length, width) * 0.35
                    with bd.PolarLocations(cr, n):
                        bd.Circle(hole_d / 2)
            bd.extrude(amount=-thickness, mode=bd.Mode.SUBTRACT)
    return part.part


# ═════════════════════════════════════════════════════════════════════════════
# CAPABILITY 2: 2D PROFILE BUILDER (shapely)
# ═════════════════════════════════════════════════════════════════════════════

def create_2d_profile(
    points: list[tuple[float, float]],
    holes: Optional[list[list[tuple[float, float]]]] = None,
) -> Any:
    """Create a 2D polygon from points with optional holes. Returns shapely Polygon."""
    if not _HAS_SHAPELY:
        raise RuntimeError("shapely required for create_2d_profile")
    if len(points) < 3:
        raise ValueError("Need at least 3 points")
    return Polygon(points, holes=holes)


def offset_profile(profile: Any, distance: float) -> Any:
    """Offset a 2D profile inward (negative) or outward (positive). Returns shapely Polygon."""
    if not _HAS_SHAPELY:
        raise RuntimeError("shapely required for offset_profile")
    result = profile.buffer(distance, join_style="mitre")
    if result.is_empty:
        raise ValueError(f"Offset by {distance} produced empty geometry")
    return result


def profile_to_3d(profile: Any, height: float) -> Any:
    """Extrude a shapely 2D profile into a 3D solid using build123d."""
    if not _HAS_BUILD123D or not _HAS_SHAPELY:
        raise RuntimeError("build123d and shapely required for profile_to_3d")
    _validate_positive(height=height)

    coords = list(profile.exterior.coords)
    with bd.BuildPart() as part:
        with bd.BuildSketch():
            bd.Polygon([(x, y) for x, y in coords[:-1]])
        bd.extrude(amount=height)
    return part.part


# ═════════════════════════════════════════════════════════════════════════════
# CAPABILITY 3: MESH OPERATIONS (trimesh)
# ═════════════════════════════════════════════════════════════════════════════

def import_mesh(filepath: str) -> Any:
    """Import STL/OBJ/PLY mesh file. Returns trimesh object."""
    if not _HAS_TRIMESH:
        raise RuntimeError("trimesh required for import_mesh")
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"File not found: {filepath}")
    return trimesh.load(filepath)


def boolean_union(mesh1: Any, mesh2: Any) -> Any:
    """Combine two meshes via boolean union."""
    if not _HAS_TRIMESH:
        raise RuntimeError("trimesh required for boolean_union")
    return trimesh.boolean.union([mesh1, mesh2])


def boolean_cut(mesh1: Any, mesh2: Any) -> Any:
    """Subtract mesh2 from mesh1 via boolean difference."""
    if not _HAS_TRIMESH:
        raise RuntimeError("trimesh required for boolean_cut")
    return trimesh.boolean.difference([mesh1, mesh2])


def repair_mesh(mesh: Any) -> Any:
    """Fix non-watertight meshes: fill holes, fix normals, merge vertices."""
    if not _HAS_TRIMESH:
        raise RuntimeError("trimesh required for repair_mesh")
    mesh.merge_vertices()
    # trimesh 5.x API: filter via nondegenerate_faces property
    if hasattr(mesh, 'nondegenerate_faces'):
        mask = mesh.nondegenerate_faces()
        mesh.update_faces(mask)
    mesh.remove_unreferenced_vertices()
    mesh.fix_normals()
    trimesh.repair.fill_holes(mesh)
    return mesh


def mesh_properties(mesh: Any) -> dict[str, Any]:
    """Return volume, surface area, bounding box, center of mass."""
    if not _HAS_TRIMESH:
        raise RuntimeError("trimesh required for mesh_properties")
    props: dict[str, Any] = {
        "is_watertight": mesh.is_watertight,
        "face_count": len(mesh.faces),
        "vertex_count": len(mesh.vertices),
        "bounding_box_extents": mesh.bounding_box.extents.tolist(),
        "center_mass": mesh.center_mass.tolist(),
    }
    if mesh.is_watertight:
        props["volume"] = float(mesh.volume)
        props["surface_area"] = float(mesh.area)
    return props


def simplify_mesh(mesh: Any, target_faces: int) -> Any:
    """Reduce mesh complexity while preserving shape."""
    if not _HAS_TRIMESH:
        raise RuntimeError("trimesh required for simplify_mesh")
    if target_faces >= len(mesh.faces):
        return mesh
    return mesh.simplify_quadric_decimation(target_faces)


# ═════════════════════════════════════════════════════════════════════════════
# CAPABILITY 4: FORMAT CONVERSION
# ═════════════════════════════════════════════════════════════════════════════

def export_stl(part: Any, filepath: str, binary: bool = True) -> str:
    """Export build123d part to STL. Returns filepath."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for export_stl")
    os.makedirs(os.path.dirname(os.path.abspath(filepath)), exist_ok=True)
    bd.export_stl(part, filepath)
    return filepath


def export_step(part: Any, filepath: str) -> str:
    """Export build123d part to STEP. Returns filepath."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for export_step")
    os.makedirs(os.path.dirname(os.path.abspath(filepath)), exist_ok=True)
    bd.export_step(part, filepath)
    return filepath


def export_dxf(profile_2d: Any, filepath: str) -> str:
    """Export shapely 2D profile to DXF. Returns filepath."""
    if not _HAS_EZDXF or not _HAS_SHAPELY:
        raise RuntimeError("ezdxf and shapely required for export_dxf")
    os.makedirs(os.path.dirname(os.path.abspath(filepath)), exist_ok=True)
    doc = ezdxf.new(dxfversion="R2010")
    msp = doc.modelspace()
    coords = list(profile_2d.exterior.coords)
    msp.add_lwpolyline(coords, close=True)
    if hasattr(profile_2d, "interiors"):
        for interior in profile_2d.interiors:
            msp.add_lwpolyline(list(interior.coords), close=True)
    doc.saveas(filepath)
    return filepath


def import_step(filepath: str) -> Any:
    """Import STEP file. Returns build123d Shape."""
    if not _HAS_BUILD123D:
        raise RuntimeError("build123d required for import_step")
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"File not found: {filepath}")
    parts = bd.import_step(filepath)
    return parts


def import_dxf(filepath: str) -> Any:
    """Import DXF file. Returns shapely polygon from first polyline."""
    if not _HAS_EZDXF or not _HAS_SHAPELY:
        raise RuntimeError("ezdxf and shapely required for import_dxf")
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"File not found: {filepath}")
    doc = ezdxf.readfile(filepath)
    msp = doc.modelspace()
    for entity in msp:
        if entity.dxftype() == "LWPOLYLINE":
            pts = [(p[0], p[1]) for p in entity.get_points()]
            return Polygon(pts)
    raise ValueError("No polyline found in DXF file")


def convert_format(input_file: str, output_file: str) -> str:
    """Auto-detect and convert between geometry formats. Returns output path."""
    if not os.path.exists(input_file):
        raise FileNotFoundError(f"File not found: {input_file}")

    in_ext = os.path.splitext(input_file)[1].lower()
    out_ext = os.path.splitext(output_file)[1].lower()
    os.makedirs(os.path.dirname(os.path.abspath(output_file)), exist_ok=True)

    if in_ext in (".step", ".stp") and out_ext == ".stl":
        part = import_step(input_file)
        return export_stl(part, output_file)
    elif in_ext == ".stl" and out_ext in (".obj", ".ply", ".glb"):
        m = import_mesh(input_file)
        m.export(output_file)
        return output_file
    elif in_ext in (".obj", ".ply") and out_ext == ".stl":
        m = import_mesh(input_file)
        m.export(output_file)
        return output_file
    elif in_ext == ".dxf" and out_ext == ".dxf":
        raise ValueError("Input and output are the same format")
    else:
        raise ValueError(f"Conversion from {in_ext} to {out_ext} not supported")


# ═════════════════════════════════════════════════════════════════════════════
# CAPABILITY 5: GEOMETRY ANALYSIS
# ═════════════════════════════════════════════════════════════════════════════

def _part_to_trimesh(part: Any) -> Any:
    """Convert build123d part to trimesh via temporary STL."""
    if not _HAS_BUILD123D or not _HAS_TRIMESH:
        raise RuntimeError("build123d and trimesh required")
    import tempfile
    with tempfile.NamedTemporaryFile(suffix=".stl", delete=False) as f:
        tmp = f.name
    try:
        bd.export_stl(part, tmp)
        return trimesh.load(tmp)
    finally:
        os.unlink(tmp)


def check_interference(part1: Any, part2: Any) -> dict[str, Any]:
    """Check if two build123d parts collide using AABB + sample-point test."""
    m1 = _part_to_trimesh(part1)
    m2 = _part_to_trimesh(part2)
    # Quick AABB overlap test
    bb1_min, bb1_max = m1.bounds
    bb2_min, bb2_max = m2.bounds
    aabb_overlap = np.all(bb1_min <= bb2_max) and np.all(bb2_min <= bb1_max)
    if not aabb_overlap:
        return {"interferes": False, "contacts": []}
    # Sample points from m2 and test if any are inside m1
    points = m2.sample(500)
    inside = m1.contains(points)
    colliding = bool(np.any(inside))
    return {"interferes": colliding, "contacts": ["point_in_solid"] if colliding else []}


def calculate_clearance(part1: Any, part2: Any) -> float:
    """Calculate minimum distance between two build123d parts. Returns mm."""
    m1 = _part_to_trimesh(part1)
    m2 = _part_to_trimesh(part2)
    # Use scipy KDTree for nearest-point distance
    if _HAS_SCIPY:
        tree = KDTree(m1.vertices)
        dists, _ = tree.query(m2.vertices)
        return float(np.min(dists))
    # Fallback: brute force sample
    from scipy.spatial.distance import cdist
    d = cdist(m1.sample(200), m2.sample(200))
    return float(np.min(d))


def fit_bounding_box(part: Any) -> dict[str, Any]:
    """Return minimum bounding box dimensions for a build123d part."""
    m = _part_to_trimesh(part)
    obb = m.bounding_box_oriented
    return {
        "extents": m.bounding_box.extents.tolist(),
        "obb_extents": obb.extents.tolist(),
        "volume": float(m.bounding_box.volume),
        "center": m.bounding_box.centroid.tolist(),
    }


def check_manufacturability(part: Any, process: str = "cnc") -> list[str]:
    """Basic DFM check. Returns list of warnings."""
    m = _part_to_trimesh(part)
    warnings: list[str] = []

    if not m.is_watertight:
        warnings.append("Part is not watertight — may cause manufacturing issues")

    extents = m.bounding_box.extents
    min_dim = float(min(extents))
    if min_dim < 0.5:
        warnings.append(f"Very thin feature detected: {min_dim:.2f}mm")

    if process == "cnc":
        if float(max(extents)) > 1000:
            warnings.append(f"Part exceeds typical CNC envelope: {max(extents):.0f}mm")
    elif process == "3d_print":
        if float(max(extents)) > 300:
            warnings.append(f"Part exceeds typical 3D print bed: {max(extents):.0f}mm")

    if len(m.faces) > 50000:
        warnings.append(f"High face count ({len(m.faces)}) — consider simplifying")

    return warnings


# ═════════════════════════════════════════════════════════════════════════════
# CAPABILITY 6: ASSEMBLY SUPPORT
# ═════════════════════════════════════════════════════════════════════════════

class Assembly:
    """Lightweight assembly container for build123d parts."""

    def __init__(self, name: str = "Assembly"):
        self.name = name
        self._parts: list[dict[str, Any]] = []

    def add_part(
        self,
        part: Any,
        name: str = "Part",
        position: tuple[float, float, float] = (0, 0, 0),
        rotation: tuple[float, float, float] = (0, 0, 0),
    ) -> None:
        """Add a part to the assembly at given position and rotation (degrees)."""
        self._parts.append({
            "part": part,
            "name": name,
            "position": position,
            "rotation": rotation,
        })

    @property
    def part_count(self) -> int:
        return len(self._parts)

    def check_interference(self) -> list[dict[str, Any]]:
        """Check all part pairs for collisions."""
        results = []
        for i in range(len(self._parts)):
            for j in range(i + 1, len(self._parts)):
                info = check_interference(
                    self._parts[i]["part"],
                    self._parts[j]["part"],
                )
                if info["interferes"]:
                    results.append({
                        "part_a": self._parts[i]["name"],
                        "part_b": self._parts[j]["name"],
                        **info,
                    })
        return results

    def export_step(self, filepath: str) -> str:
        """Export assembly as compound STEP."""
        if not _HAS_BUILD123D:
            raise RuntimeError("build123d required for assembly export")
        os.makedirs(os.path.dirname(os.path.abspath(filepath)), exist_ok=True)
        compound = bd.Compound(children=[p["part"] for p in self._parts])
        bd.export_step(compound, filepath)
        return filepath


def create_assembly(name: str = "Assembly") -> Assembly:
    """Create a new empty assembly."""
    return Assembly(name)


def add_to_assembly(
    assembly: Assembly,
    part: Any,
    name: str = "Part",
    position: tuple[float, float, float] = (0, 0, 0),
    rotation: tuple[float, float, float] = (0, 0, 0),
) -> None:
    """Add a part to an assembly."""
    assembly.add_part(part, name, position, rotation)


def check_assembly_interference(assembly: Assembly) -> list[dict[str, Any]]:
    """Check all parts in assembly for collisions."""
    return assembly.check_interference()


def export_assembly(assembly: Assembly, filepath: str) -> str:
    """Export full assembly to STEP."""
    return assembly.export_step(filepath)
