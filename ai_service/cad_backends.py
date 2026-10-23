"""CAD backend abstraction — build123d (default) and cadquery."""
from __future__ import annotations

import logging
from typing import Any, Literal, Optional

logger = logging.getLogger("CadBackends")

BackendName = Literal["build123d", "cadquery", "auto"]

_HAS_BUILD123D = False
_HAS_CADQUERY = False

try:
    import build123d as bd
    _HAS_BUILD123D = True
except ImportError:
    bd = None

try:
    import cadquery as cq
    _HAS_CADQUERY = True
except ImportError:
    cq = None


def available_backends() -> dict[str, bool]:
    return {"build123d": _HAS_BUILD123D, "cadquery": _HAS_CADQUERY}


def resolve_backend(preferred: BackendName = "auto") -> str:
    if preferred == "build123d" and _HAS_BUILD123D:
        return "build123d"
    if preferred == "cadquery" and _HAS_CADQUERY:
        return "cadquery"
    if preferred == "auto":
        if _HAS_BUILD123D:
            return "build123d"
        if _HAS_CADQUERY:
            return "cadquery"
    raise RuntimeError("No CAD backend available (install build123d or cadquery)")


def create_box(
    length: float,
    width: float,
    height: float,
    backend: BackendName = "auto",
) -> tuple[Any, str]:
    name = resolve_backend(backend)
    if name == "build123d":
        with bd.BuildPart() as part:
            bd.Box(length, width, height)
        return part.part, name
    wp = cq.Workplane("XY").box(length, width, height)
    return wp, name


def create_cylinder(
    radius: float,
    height: float,
    backend: BackendName = "auto",
) -> tuple[Any, str]:
    name = resolve_backend(backend)
    if name == "build123d":
        with bd.BuildPart() as part:
            bd.Cylinder(radius=radius, height=height)
        return part.part, name
    wp = cq.Workplane("XY").circle(radius).extrude(height)
    return wp, name


def export_step(part: Any, path: str, backend: str) -> str:
    if backend == "build123d":
        from build123d import export_step as b3d_export
        b3d_export(part, path)
        return path
    if backend == "cadquery":
        cq.exporters.export(part, path)
        return path
    raise ValueError(f"Unknown backend: {backend}")


def create_from_spec(
    shape: str,
    params: dict[str, float],
    backend: BackendName = "auto",
) -> tuple[Any, str]:
    """Create a solid from a shape name + params; returns (part, backend_name)."""
    name = resolve_backend(backend)
    shape = (shape or "box").lower()
    if shape == "cylinder":
        return create_cylinder(
            float(params.get("radius", 10.0)),
            float(params.get("height", 50.0)),
            backend=name,  # type: ignore[arg-type]
        )
    if shape == "sphere":
        r = float(params.get("radius", 25.0))
        if name == "build123d":
            with bd.BuildPart() as part:
                bd.Sphere(radius=r)
            return part.part, name
        return cq.Workplane("XY").sphere(r), name
    # default box
    return create_box(
        float(params.get("length", 100.0)),
        float(params.get("width", 50.0)),
        float(params.get("height", 20.0)),
        backend=name,  # type: ignore[arg-type]
    )


def export_shape(
    shape: str,
    params: dict[str, float],
    path: str,
    fmt: str = "STEP",
    backend: BackendName = "auto",
) -> dict[str, Any]:
    """Create + export a shape; report which CAD backend was used."""
    part, used = create_from_spec(shape, params, backend=backend)
    fmt_u = (fmt or "STEP").upper()
    if fmt_u in {"STEP", "STP"}:
        export_step(part, path, used)
    elif used == "cadquery" and cq is not None:
        cq.exporters.export(part, path)
    elif used == "build123d":
        # Prefer STEP for build123d; fall back to same export for IGES-like paths
        export_step(part, path if path.lower().endswith((".step", ".stp")) else path + ".step", used)
    else:
        raise RuntimeError(f"Export format {fmt_u} unsupported without CAD backend")
    return {"status": "success", "file_path": path, "backend": used, "format": fmt_u}
