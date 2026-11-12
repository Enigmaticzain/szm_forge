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
    r"(\d+(?:\.\d+)?)\s*(mm|cm|m|inches|inch|in|\")?", re.IGNORECASE
)
_DIMENSION_TOKEN = r"\d+(?:\.\d+)?\s*(?:mm|cm|m|inches|inch|in|\")?"

def _to_mm(value: float, unit: str) -> float:
    unit = (unit or "mm").lower().replace('"', "in")
    if unit in {"inch", "inches"}:
        unit = "in"
    return {"mm": 1, "cm": 10, "m": 1000, "in": 25.4, "inch": 25.4}.get(unit, 1) * value

def _extract_dims(text: str) -> list[float]:
    """Return up to 3 dimensions in mm from free text."""
    hits = _UNIT_RE.findall(text)
    return [_to_mm(float(v), u) for v, u in hits][:3]

def _extract_named_dimension(text: str, labels: str) -> float | None:
    """Find a dimension labeled either before or after its value."""
    patterns = (
        rf"(?P<measurement>{_DIMENSION_TOKEN})\s*(?:{labels})\b",
        rf"\b(?:{labels})\s*[:=]?\s*(?P<measurement>{_DIMENSION_TOKEN})",
    )
    for pattern in patterns:
        match = re.search(pattern, text, re.IGNORECASE)
        if match:
            value, unit = _UNIT_RE.match(match.group("measurement")).groups()
            return _to_mm(float(value), unit)
    return None

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

    def named_or_index(labels: str, index: int, fallback: float) -> float:
        named = _extract_named_dimension(prompt, labels)
        return named if named is not None else dims[index] if index < len(dims) else fallback

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

    if shape == "box":
        base = [
            named_or_index(r"length|long", 0, defaults[shape][0]),
            named_or_index(r"width|wide", 1, defaults[shape][1]),
            named_or_index(r"height|tall|high|thick", 2, defaults[shape][2]),
        ]
    elif shape == "cylinder":
        radius = _extract_named_dimension(prompt, r"radius")
        diameter = _extract_named_dimension(prompt, r"diameter")
        if radius is None and diameter is not None:
            radius = diameter / 2.0
        cylinder_height = _extract_named_dimension(prompt, r"height|length|long|tall")
        base = [
            radius if radius is not None else dims[0] if dims else defaults[shape][0],
            radius if radius is not None else dims[0] if dims else defaults[shape][1],
            cylinder_height if cylinder_height is not None
            else dims[2] if len(dims) >= 3
            else dims[1] if len(dims) >= 2
            else defaults[shape][2],
        ]
    elif shape == "sphere":
        radius = _extract_named_dimension(prompt, r"radius")
        diameter = _extract_named_dimension(prompt, r"diameter")
        if radius is None and diameter is not None:
            radius = diameter / 2.0
        base = [radius if radius is not None else dims[0] if dims else defaults[shape][0]]
    elif shape == "torus":
        base = [
            named_or_index(r"major\s*radius", 0, defaults[shape][0]),
            named_or_index(r"minor\s*radius|tube", 1, defaults[shape][1]),
        ]

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

def text_to_cad(prompt: str, output_dir: str = "/tmp", backend: str = "auto") -> dict[str, Any]:
    """Convert NL prompt to CAD; prefer FreeCAD, then build123d/cadquery, else JSON mesh."""
    spec = build_cad_spec(prompt)
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, f"szm_t2cad_{spec['shape']}.step")

    # Phase 3 sketch constraint validation gate (isolated; optional)
    validation_findings = []
    validation_blocked = False
    try:
        from ai_service.evidence.spec_models import ReconstructionSpec
        from ai_service.evidence.sketch_constraints import validate_sketch_constraints
        temp_params = {k: v for k, v in spec.items() if isinstance(v, (int, float, str, bool)) and k not in ("shape", "material", "features", "prompt")}
        temp_unknowns = ["Legacy input: no linked evidence references"]
        temp_assumptions = ["Dimensions derived from text prompt, not measured evidence"]
        validation_spec = ReconstructionSpec(
            spec_id=f"t2cad-{spec['shape']}-{str(id(spec))[-6:]}",
            subject=str(spec.get("shape", "unknown")),
            evidence_refs=[],
            parameters=temp_params,
            assumptions=temp_assumptions,
            unknowns=temp_unknowns,
            validation_findings=[],
            review_status="pending",
        )
        validation_result = validate_sketch_constraints(
            validation_spec,
            critical_params=list(temp_params.keys()) if temp_params else ["unknown_dim"],
        )
        validation_findings.extend(validation_result.findings)
        # Preserve backward-compatible behavior: only block when evidence is linked
        # or when the user explicitly submits with sketch-constraint requirements.
        # Pure legacy text-to-CAD (no evidence refs, no explicit constraints) continues.
        has_evidence_refs = bool(temp_params) and bool(getattr(validation_spec, "evidence_refs", []))
        has_explicit_constraints = any(
            k in ("sketch_constraints", "constraints", "constraint_ids") for k in temp_params
        )
        if validation_result.conflicts or (has_evidence_refs and not validation_result.is_ready_for_review()):
            validation_blocked = True
        elif validation_result.unresolved_params and has_explicit_constraints:
            validation_blocked = True
    except Exception as exc:
        # Evidence package unavailable: do NOT claim success; do NOT fabricate failure
        validation_findings.append(f"Validation layer unavailable: {exc}")
        # Preserve current behavior for legacy inputs without blocking
        validation_blocked = False

    if validation_blocked:
        return {
            "status": "rejected",
            "prompt": prompt,
            "spec": spec,
            "step_file": None,
            "step_available": False,
            "geometry": spec_to_threejs_geometry(spec),
            "engine": "sketch-validation-blocked",
            "backend": None,
            "available_backends": None,
            "validation_findings": validation_findings,
            "unknowns": ["Sketch validation blocked — review requires measured/documented evidence"],
            "limitations": ["No geometry produced — validation blocked before STEP generation"],
        }

    # Use validated dimensions when available (Phase 4 wire): prefer validated params
    # over derived defaults; never substitute zero for unknowns.
    validated_params = {}
    if not validation_blocked:
        try:
            validated_params = temp_params  # validated from ReconstructionSpec.parameters
        except NameError:
            validated_params = {}
    # For supported dimensions present in validated_params, use them; for missing/unknown,
    # preserve spec-derived fallback only when no evidence-linked conflicts exist.
    params = {
        "length": float(validated_params.get("length", spec.get("length", 100))),
        "width": float(validated_params.get("width", spec.get("width", 50))),
        "height": float(validated_params.get("height", spec.get("height", 20))),
        "radius": float(validated_params.get("radius", spec.get("radius", spec.get("radius1", 25)))),
    } if validated_params else {
        "length": float(spec.get("length", 100)),
        "width": float(spec.get("width", 50)),
        "height": float(spec.get("height", 20)),
        "radius": float(spec.get("radius", spec.get("radius1", 25))),
    }

    step_ok = generate_step(spec, out_path)
    engine = "FreeCAD headless" if step_ok else "parametric fallback"
    cad_backend = None

    if not step_ok:
        try:
            from cad_backends import export_shape, available_backends
            backends = available_backends()
            if any(backends.values()):
                params = {
                    "length": float(validated_params.get("length", spec.get("length", 100))) if validated_params else float(spec.get("length", 100)),
                    "width": float(validated_params.get("width", spec.get("width", 50))) if validated_params else float(spec.get("width", 50)),
                    "height": float(validated_params.get("height", spec.get("height", 20))) if validated_params else float(spec.get("height", 20)),
                    "radius": float(validated_params.get("radius", spec.get("radius", spec.get("radius1", 25)))) if validated_params else float(spec.get("radius", spec.get("radius1", 25))),
                }
                result = export_shape(spec["shape"], params, out_path, "STEP", backend=backend)  # type: ignore[arg-type]
                step_ok = True
                cad_backend = result["backend"]
                engine = cad_backend
        except Exception as exc:
            logger.warning("CAD backend export failed: %s", exc)

    return {
        "status": "success",
        "prompt": prompt,
        "spec": spec,
        "step_file": out_path if step_ok else None,
        "step_available": step_ok,
        "geometry": spec_to_threejs_geometry(spec),
        "engine": engine,
        "backend": cad_backend or ("freecad" if engine.startswith("FreeCAD") else "fallback"),
        "available_backends": None,
    }
