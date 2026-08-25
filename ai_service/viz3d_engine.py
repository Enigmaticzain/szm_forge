"""3D visualization processing (Group 12)."""
from __future__ import annotations

import logging
import os
from typing import Any, Dict, List, Optional

logger = logging.getLogger("Viz3DEngine")

_HAS_OPEN3D = False
try:
    import open3d as o3d
    import numpy as np
    _HAS_OPEN3D = True
except ImportError:
    o3d = None
    np = None


def available_backends() -> Dict[str, bool]:
    return {"open3d": _HAS_OPEN3D}


def load_mesh(path: str) -> Dict[str, Any]:
    if not _HAS_OPEN3D:
        return {"status": "error", "message": "open3d not installed"}
    mesh = o3d.io.read_triangle_mesh(path)
    mesh.compute_vertex_normals()
    return {
        "status": "success",
        "vertices": len(mesh.vertices),
        "triangles": len(mesh.triangles),
        "bounds": {
            "min": mesh.get_min_bound().tolist(),
            "max": mesh.get_max_bound().tolist(),
        },
    }


def point_cloud_from_mesh(path: str, n_points: int = 1000) -> Dict[str, Any]:
    if not _HAS_OPEN3D:
        return {"status": "error", "message": "open3d not installed"}
    mesh = o3d.io.read_triangle_mesh(path)
    pcd = mesh.sample_points_uniformly(number_of_points=n_points)
    out = path.replace(os.path.splitext(path)[1], "_pcd.ply")
    o3d.io.write_point_cloud(out, pcd)
    return {"status": "success", "output": out, "points": len(pcd.points)}


def mesh_simplify(path: str, target_triangles: int = 1000) -> Dict[str, Any]:
    if not _HAS_OPEN3D:
        return {"status": "error", "message": "open3d not installed"}
    mesh = o3d.io.read_triangle_mesh(path)
    simplified = mesh.simplify_quadric_decimation(target_triangles)
    out = path.replace(os.path.splitext(path)[1], "_simplified.ply")
    o3d.io.write_triangle_mesh(out, simplified)
    return {"status": "success", "output": out, "triangles": len(simplified.triangles)}
