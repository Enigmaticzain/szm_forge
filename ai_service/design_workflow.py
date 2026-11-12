"""Prompt-driven design workflow built from SZM Forge's existing CAD and CAM tools."""
from __future__ import annotations

import math
import tempfile
from pathlib import Path
from typing import Any
from uuid import UUID, uuid4

from manufacturing_pipeline import CNCToolpathGenerator
from text_to_cad import text_to_cad

ARTIFACT_ROOT = Path(tempfile.gettempdir()) / "szm-forge-designs"
TOOL_DIAMETER_MM = 10.0
MAX_PREVIEW_XY_MM = 500.0
MAX_PREVIEW_DEPTH_MM = 100.0


def _positive_dimension(spec: dict[str, Any], key: str, *, allow_zero: bool = False) -> float:
    value = spec.get(key)
    if isinstance(value, bool):
        raise ValueError(f"{key} must be a finite dimension")
    try:
        dimension = float(value)
    except (TypeError, ValueError):
        raise ValueError(f"{key} is missing or is not numeric") from None

    minimum = 0.0 if allow_zero else math.nextafter(0.0, math.inf)
    if not math.isfinite(dimension) or dimension < minimum:
        qualifier = "non-negative" if allow_zero else "positive"
        raise ValueError(f"{key} must be finite and {qualifier}")
    return dimension


def _analyze_geometry(spec: dict[str, Any]) -> dict[str, Any]:
    """Measure an ideal primitive; this is not a mesh or structural analysis."""
    shape = spec.get("shape")
    try:
        if shape == "box":
            x = _positive_dimension(spec, "length")
            y = _positive_dimension(spec, "width")
            z = _positive_dimension(spec, "height")
            volume = x * y * z
        elif shape == "cylinder":
            radius = _positive_dimension(spec, "radius")
            z = _positive_dimension(spec, "height")
            x = y = radius * 2.0
            volume = math.pi * radius**2 * z
        elif shape == "sphere":
            radius = _positive_dimension(spec, "radius")
            x = y = z = radius * 2.0
            volume = (4.0 / 3.0) * math.pi * radius**3
        elif shape == "cone":
            radius1 = _positive_dimension(spec, "radius1")
            radius2 = _positive_dimension(spec, "radius2", allow_zero=True)
            z = _positive_dimension(spec, "height")
            x = y = max(radius1, radius2) * 2.0
            volume = (math.pi * z / 3.0) * (radius1**2 + radius1 * radius2 + radius2**2)
        elif shape == "torus":
            major_radius = _positive_dimension(spec, "major_radius")
            minor_radius = _positive_dimension(spec, "minor_radius")
            if minor_radius >= major_radius:
                raise ValueError("minor_radius must be smaller than major_radius for a non-self-intersecting torus")
            x = y = 2.0 * (major_radius + minor_radius)
            z = 2.0 * minor_radius
            volume = 2.0 * math.pi**2 * major_radius * minor_radius**2
        else:
            raise ValueError(f"unsupported primitive shape: {shape!r}")
    except ValueError as exc:
        return {
            "status": "blocked",
            "reason": str(exc),
            "bounding_box_mm": None,
            "nominal_volume_mm3": None,
            "structural_analysis": {
                "status": "not_run",
                "reason": "A valid parametric solid is required before structural analysis.",
            },
        }

    if not math.isfinite(volume) or volume <= 0.0:
        return {
            "status": "blocked",
            "reason": "The primitive dimensions do not produce a finite, positive volume.",
            "bounding_box_mm": None,
            "nominal_volume_mm3": None,
            "structural_analysis": {
                "status": "not_run",
                "reason": "A valid parametric solid is required before structural analysis.",
            },
        }

    return {
        "status": "preliminary",
        "bounding_box_mm": {
            "x": round(x, 4),
            "y": round(y, 4),
            "z": round(z, 4),
        },
        "nominal_volume_mm3": round(volume, 4),
        "basis": "Ideal primitive dimensions parsed from the text prompt; excludes unmodeled features.",
        "structural_analysis": {
            "status": "not_run",
            "reason": "Structural FEA needs explicit loads, restraints, material properties, and a validated mesh.",
        },
    }


def _make_manufacturing_preview(
    spec: dict[str, Any],
    geometry_analysis: dict[str, Any],
    include_toolpath_preview: bool,
) -> dict[str, Any]:
    result: dict[str, Any] = {
        "status": "review_required",
        "dfm_status": "not_assessed",
        "toolpath_preview": None,
        "limitations": [
            "This prompt-only workflow does not perform geometry-aware DFM or machine-specific collision checks.",
            "Any generated path is an illustrative 2.5D rectangular profile, not production-ready machine code.",
        ],
    }
    if not include_toolpath_preview:
        result["toolpath_status"] = "not_requested"
        return result

    if spec.get("shape") != "box":
        result["toolpath_status"] = "unsupported_shape"
        result["toolpath_message"] = "The current rectangular profile preview supports box primitives only."
        return result

    bounds = geometry_analysis["bounding_box_mm"]
    if bounds["x"] <= TOOL_DIAMETER_MM or bounds["y"] <= TOOL_DIAMETER_MM:
        result["toolpath_status"] = "blocked"
        result["toolpath_message"] = (
            f"Both XY dimensions must exceed the {TOOL_DIAMETER_MM:g} mm preview tool diameter."
        )
        return result
    if bounds["x"] > MAX_PREVIEW_XY_MM or bounds["y"] > MAX_PREVIEW_XY_MM:
        result["toolpath_status"] = "blocked"
        result["toolpath_message"] = (
            f"Toolpath preview is limited to {MAX_PREVIEW_XY_MM:g} mm in X and Y."
        )
        return result
    if bounds["z"] > MAX_PREVIEW_DEPTH_MM:
        result["toolpath_status"] = "blocked"
        result["toolpath_message"] = f"Toolpath preview depth is limited to {MAX_PREVIEW_DEPTH_MM:g} mm."
        return result

    preview = CNCToolpathGenerator.generate_rectangular_milling_job(
        component_name="SZM design preview",
        bounds={
            "length": bounds["x"],
            "width": bounds["y"],
            "depth": bounds["z"],
            "unit": "mm",
        },
        operation="profile",
        tool_diameter_mm=TOOL_DIAMETER_MM,
    )
    result["toolpath_preview"] = preview
    result["toolpath_status"] = "preview_only" if preview.get("status") == "success" else "failed"
    if preview.get("status") != "success":
        result["toolpath_message"] = "The local toolpath generator did not produce a preview."
    return result


def run_design_workflow(prompt: str, include_toolpath_preview: bool = False) -> dict[str, Any]:
    """Generate a parametric design and return preliminary analysis/manufacturing stages."""
    normalized_prompt = prompt.strip()
    if not normalized_prompt:
        raise ValueError("A non-empty part description is required.")

    design_id = str(uuid4())
    output_dir = ARTIFACT_ROOT / design_id
    output_dir.mkdir(parents=True, exist_ok=True, mode=0o700)

    cad_result = text_to_cad(normalized_prompt, str(output_dir))
    spec = cad_result.get("spec")
    geometry = cad_result.get("geometry")
    if not isinstance(spec, dict) or not isinstance(geometry, dict):
        raise RuntimeError("The text-to-CAD engine returned an invalid design payload.")

    step_download_url = None
    step_available = False
    step_file = cad_result.get("step_file")
    if cad_result.get("step_available") and isinstance(step_file, str):
        candidate = Path(step_file).resolve()
        if candidate.parent == output_dir.resolve() and candidate.is_file():
            step_available = True
            step_download_url = f"/api/workflow/design/{design_id}/step"

    design = {
        "status": cad_result.get("status", "error"),
        "prompt": normalized_prompt,
        "spec": spec,
        "geometry": geometry,
        "engine": cad_result.get("engine"),
        "step_available": step_available,
        "step_download_url": step_download_url,
        "validation_findings": cad_result.get("validation_findings", []),
    }
    if cad_result.get("status") != "success":
        return {
            "design_id": design_id,
            "status": "blocked",
            "design": design,
            "analysis": {
                "status": "not_run",
                "reason": "CAD generation did not pass its validation gate.",
            },
            "manufacturing": {
                "status": "not_run",
                "dfm_status": "not_assessed",
                "toolpath_status": "not_run",
            },
        }

    analysis = _analyze_geometry(spec)
    if analysis["status"] == "blocked":
        return {
            "design_id": design_id,
            "status": "blocked",
            "design": design,
            "analysis": analysis,
            "manufacturing": {
                "status": "not_run",
                "dfm_status": "not_assessed",
                "toolpath_status": "not_run",
            },
        }

    manufacturing = _make_manufacturing_preview(
        spec,
        analysis,
        include_toolpath_preview=include_toolpath_preview,
    )
    return {
        "design_id": design_id,
        "status": "review_required",
        "design": design,
        "analysis": analysis,
        "manufacturing": manufacturing,
    }


def resolve_step_artifact(design_id: str) -> Path | None:
    """Resolve a generated STEP file using a UUID, never a caller-provided path."""
    try:
        canonical_id = str(UUID(design_id))
    except (TypeError, ValueError, AttributeError):
        return None

    root = ARTIFACT_ROOT.resolve()
    output_dir = (ARTIFACT_ROOT / canonical_id).resolve()
    try:
        output_dir.relative_to(root)
    except ValueError:
        return None
    if not output_dir.is_dir():
        return None

    step_files = list(output_dir.glob("*.step"))
    if len(step_files) != 1:
        return None
    step_file = step_files[0].resolve()
    if step_file.parent != output_dir or not step_file.is_file():
        return None
    return step_file
