"""Text-to-CAD AI engine for SZM Forge.

Pipeline:
  1. NL prompt → structured CAD spec (rule-based + keyword extraction)
  2. CAD spec → STEP file via FreeCAD headless (if available)
  3. Fallback: return parametric JSON mesh spec for Three.js rendering
"""
from __future__ import annotations
import re
import subprocess
import tempfile
import os
import json
import logging
from typing import Any

logger = logging.getLogger("TextToCAD")

# ── Unit normalisation ─────────────────────────────────────────────────────────

_UNIT_RE = re.compile(
    r"(\d+(?:\.\d+)?)\s*(mm|cm|m|in|inch|inches|\")?", re.IGNORECASE
)

def _to_mm(value: float, unit: str) -> float:
    unit = (unit or "mm").lower().rstrip("s").replace('"', "in")
    return {"mm": 1, "cm": 10, "m": 1000, "in": 25.4, "inch": 25.4}.get(unit, 1) * value

def _extract_dims(text: str) -> list[float]:
    """Return up to 3 dimensions in mm from free text."""
    hits = _UNIT_RE.findall(text)
    return [_to_mm(float(v), u) for v, u in hits][:3]

# ── Shape classifier ───────────────────────────────────────────────────────────

_SHAPE_KEYWORDS: dict[str, list[str]] = {
    "box":      ["box", "cube", "block", "rectangular", "plate", "bracket", "housing", "enclosure"],
    "cylinder": ["cylinder", "rod", "shaft", "tube", "pipe", "barrel", "column", "pin", "bolt"],
    "sphere":   ["sphere", "ball", "globe", "dome"],
    "cone":     ["cone", "funnel", "nozzle", "taper"],
    "torus":    ["torus", "ring", "donut", "washer", "gasket"],
}

def _classify_shape(text: str) -> str:
    lower = text.lower()
    for shape, kws in _SHAPE_KEYWORDS.items():
        if any(kw in lower for kw in kws):
            return shape
    return "box"

# ── Material hint ──────────────────────────────────────────────────────────────

_MAT_KEYWORDS: dict[str, list[str]] = {
    "steel":    ["steel", "iron", "stainless"],
    "aluminum": ["aluminum", "aluminium", "al6061", "al7075"],
    "titanium": ["titanium", "ti-6al-4v"],
    "plastic":  ["plastic", "pla", "abs", "nylon", "polymer"],
    "copper":   ["copper", "brass", "bronze"],
}

def _classify_material(text: str) -> str:
    lower = text.lower()
    for mat, kws in _MAT_KEYWORDS.items():
        if any(kw in lower for kw in kws):
            return mat
    return "steel"

# ── Feature extraction ─────────────────────────────────────────────────────────

def _extract_features(text: str) -> dict[str, Any]:
    lower = text.lower()
    return {
        "fillet":   bool(re.search(r"fillet|round|chamfer|smooth", lower)),
        "hole":     bool(re.search(r"hole|bore|drill|through.hole|pocket", lower)),
        "thread":   bool(re.search(r"thread|screw|bolt|nut|m\d+", lower)),
        "hollow":   bool(re.search(r"hollow|tube|pipe|shell|wall", lower)),
    }

# ── Spec builder ───────────────────────────────────────────────────────────────

def build_cad_spec(prompt: str) -> dict[str, Any]:
    dims = _extract_dims(prompt)
    shape = _classify_shape(prompt)

    # Sensible defaults per shape
    defaults: dict[str, list[float]] = {
        "box":      [100, 50, 20],
        "cylinder": [20, 20, 80],   # radius, radius, height
        "sphere":   [30],
        "cone":     [20, 5, 60],    # r1, r2, height
        "torus":    [30, 8],        # major_r, minor_r
    }
    base = defaults[shape]
    for i, v in enumerate(dims):
        if i < len(base):
            base[i] = v

    spec: dict[str, Any] = {
        "shape": shape,
        "material": _classify_material(prompt),
        "features": _extract_features(prompt),
        "prompt": prompt,
    }

    if shape == "box":
        spec.update({"length": base[0], "width": base[1] if len(base) > 1 else base[0], "height": base[2] if len(base) > 2 else base[0]})
    elif shape == "cylinder":
        spec.update({"radius": base[0], "height": base[2] if len(base) > 2 else base[1]})
    elif shape == "sphere":
        spec.update({"radius": base[0]})
    elif shape == "cone":
        spec.update({"radius1": base[0], "radius2": base[1] if len(base) > 1 else base[0] * 0.2, "height": base[2] if len(base) > 2 else base[0] * 3})
    elif shape == "torus":
        spec.update({"major_radius": base[0], "minor_radius": base[1] if len(base) > 1 else base[0] * 0.25})

    return spec

# ── FreeCAD headless STEP generation ──────────────────────────────────────────

_FREECAD_SCRIPT = """
import FreeCAD, Part, sys, json

spec = json.loads(sys.argv[1])
shape_type = spec["shape"]
out_path = spec["out_path"]

if shape_type == "box":
    s = Part.makeBox(spec["length"], spec["width"], spec["height"])
elif shape_type == "cylinder":
    s = Part.makeCylinder(spec["radius"], spec["height"])
elif shape_type == "sphere":
    s = Part.makeSphere(spec["radius"])
elif shape_type == "cone":
    s = Part.makeCone(spec["radius1"], spec["radius2"], spec["height"])
elif shape_type == "torus":
    s = Part.makeTorus(spec["major_radius"], spec["minor_radius"])
else:
    s = Part.makeBox(100, 50, 20)

if spec.get("features", {}).get("fillet"):
    try:
        edges = s.Edges
        s = s.makeFillet(min(3.0, spec.get("length", 100) * 0.03), edges)
    except Exception:
        pass

Part.export([s], out_path)
print("OK:" + out_path)
"""

def generate_step(spec: dict[str, Any], out_path: str) -> bool:
    """Try FreeCAD headless. Returns True on success."""
    with tempfile.NamedTemporaryFile("w", suffix=".py", delete=False) as f:
        f.write(_FREECAD_SCRIPT)
        script_path = f.name

    spec_json = json.dumps({**spec, "out_path": out_path})
    for binary in ["freecadcmd", "FreeCADCmd", "freecad"]:
        try:
            result = subprocess.run(
                [binary, script_path, spec_json],
                capture_output=True, text=True, timeout=30
            )
            if result.returncode == 0 and "OK:" in result.stdout:
                os.unlink(script_path)
                return True
        except (FileNotFoundError, subprocess.TimeoutExpired):
            continue

    os.unlink(script_path)
    return False

# ── Fallback: parametric JSON mesh for Three.js ───────────────────────────────

def spec_to_threejs_geometry(spec: dict[str, Any]) -> dict[str, Any]:
    """Return a Three.js-compatible geometry descriptor."""
    shape = spec["shape"]
    geo: dict[str, Any] = {"type": shape}

    if shape == "box":
        geo.update({"width": spec["length"], "height": spec["height"], "depth": spec["width"]})
    elif shape == "cylinder":
        geo.update({"radiusTop": spec["radius"], "radiusBottom": spec["radius"], "height": spec["height"], "radialSegments": 32})
    elif shape == "sphere":
        geo.update({"radius": spec["radius"], "widthSegments": 32, "heightSegments": 16})
    elif shape == "cone":
        geo.update({"radiusBottom": spec["radius1"], "radiusTop": spec["radius2"], "height": spec["height"], "radialSegments": 32})
    elif shape == "torus":
        geo.update({"radius": spec["major_radius"], "tube": spec["minor_radius"], "radialSegments": 16, "tubularSegments": 48})

    return geo

# ── Public API ─────────────────────────────────────────────────────────────────

def text_to_cad(prompt: str, output_dir: str = "/tmp") -> dict[str, Any]:
    spec = build_cad_spec(prompt)
    out_path = os.path.join(output_dir, f"szm_t2cad_{spec['shape']}.step")

    step_ok = generate_step(spec, out_path)

    return {
        "status": "success",
        "prompt": prompt,
        "spec": spec,
        "step_file": out_path if step_ok else None,
        "step_available": step_ok,
        "geometry": spec_to_threejs_geometry(spec),
        "engine": "FreeCAD headless" if step_ok else "parametric fallback",
    }
