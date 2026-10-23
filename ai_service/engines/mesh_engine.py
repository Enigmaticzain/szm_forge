"""Mesh generation and conversion (Group 3)."""
from __future__ import annotations

import logging
import os
import tempfile
from typing import Any, Dict, Optional

try:
    from loguru import logger as _loguru
    logger = _loguru
except ImportError:
    logger = logging.getLogger("MeshEngine")
_HAS_GMSH = False
_HAS_MESHIO = False
_HAS_PYGMSH = False
_HAS_TRIMESH = False

try:
    import gmsh
    _HAS_GMSH = True
except ImportError:
    gmsh = None

try:
    import meshio
    _HAS_MESHIO = True
except ImportError:
    meshio = None

try:
    import pygmsh
    _HAS_PYGMSH = True
except ImportError:
    pygmsh = None

try:
    import trimesh
    import numpy as np
    _HAS_TRIMESH = True
except ImportError:
    trimesh = None
    np = None


def get_available_libraries() -> dict[str, bool]:
    return {
        "gmsh": _HAS_GMSH,
        "meshio": _HAS_MESHIO,
        "pygmsh": _HAS_PYGMSH,
        "trimesh": _HAS_TRIMESH,
    }


def get_active_backend() -> str:
    """Return the mesh backend that generate_mesh would use."""
    if _HAS_GMSH:
        return "gmsh"
    if _HAS_PYGMSH:
        return "pygmsh"
    if _HAS_TRIMESH:
        return "trimesh"
    return "none"


def generate_mesh(
    geometry_path: Optional[str] = None,
    settings: Optional[Dict[str, Any]] = None,
    box_dims: Optional[tuple[float, float, float]] = None,
) -> str:
    """Generate a volume mesh; returns output .msh path."""
    settings = settings or {}
    max_size = float(settings.get("max_element_size", 0.05))
    out_dir = settings.get("output_dir") or tempfile.mkdtemp(prefix="szm_mesh_")
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "mesh.msh")

    if _HAS_GMSH:
        gmsh.initialize()
        try:
            gmsh.model.add("szm")
            if geometry_path and os.path.exists(geometry_path):
                gmsh.merge(geometry_path)
            elif box_dims:
                gmsh.model.occ.addBox(0, 0, 0, *box_dims)
                gmsh.model.occ.synchronize()
            else:
                gmsh.model.occ.addBox(0, 0, 0, 1, 1, 1)
                gmsh.model.occ.synchronize()
            gmsh.model.mesh.setSize(gmsh.model.getEntities(0), max_size)
            gmsh.model.mesh.generate(3)
            gmsh.write(out_path)
            return out_path
        finally:
            gmsh.finalize()

    if _HAS_PYGMSH:
        with pygmsh.geo.Geometry() as geom:
            if box_dims:
                geom.add_box([0, 0, 0], list(box_dims), mesh_size=max_size)
            else:
                geom.add_box([0, 0, 0], [1, 1, 1], mesh_size=max_size)
            mesh = geom.generate_mesh()
            if _HAS_MESHIO:
                meshio.write(out_path, mesh)
                return out_path

    if _HAS_TRIMESH and box_dims:
        box = trimesh.creation.box(extents=box_dims)
        stl_path = os.path.join(out_dir, "fallback.stl")
        box.export(stl_path)
        return stl_path

    raise RuntimeError("No mesh backend available (install gmsh or trimesh)")


def convert_mesh(src: str, dst: str) -> str:
    if not _HAS_MESHIO:
        raise RuntimeError("meshio required for format conversion")
    m = meshio.read(src)
    meshio.write(dst, m)
    return dst


def mesh_to_fea_nodes_elements(msh_path: str) -> Dict[str, Any]:
    """Extract nodes and hex/tet elements for CalculiX input."""
    if _HAS_MESHIO:
        m = meshio.read(msh_path)
        return {
            "points": m.points.tolist(),
            "cells": {k: v.data.tolist() for k, v in m.cells} if m.cells else {},
        }
    if _HAS_TRIMESH:
        tm = trimesh.load(msh_path)
        return {"vertices": tm.vertices.tolist(), "faces": tm.faces.tolist()}
    raise RuntimeError("meshio or trimesh required")
