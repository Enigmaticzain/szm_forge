from pathlib import Path

import design_workflow
from text_to_cad import build_cad_spec


def test_named_cylinder_dimensions_keep_radius_and_length_distinct():
    spec = build_cad_spec("Aluminum cylinder shaft 25mm radius 200mm long")

    assert spec["shape"] == "cylinder"
    assert spec["radius"] == 25
    assert spec["height"] == 200


def test_named_box_dimensions_and_inches_are_normalized():
    spec = build_cad_spec("2 inches long 1 inch wide 0.5 inch thick aluminum box")

    assert spec["length"] == 50.8
    assert spec["width"] == 25.4
    assert spec["height"] == 12.7


def _cad_result(spec, geometry, step_file=None):
    return {
        "status": "success",
        "spec": spec,
        "geometry": geometry,
        "engine": "parametric fallback",
        "step_file": str(step_file) if step_file else None,
        "step_available": step_file is not None,
    }


def test_design_workflow_builds_preliminary_box_analysis(monkeypatch, tmp_path):
    monkeypatch.setattr(design_workflow, "ARTIFACT_ROOT", tmp_path / "designs")
    monkeypatch.setattr(
        design_workflow,
        "text_to_cad",
        lambda prompt, output_dir: _cad_result(
            {
                "shape": "box",
                "prompt": prompt,
                "length": 100,
                "width": 50,
                "height": 20,
                "features": {},
            },
            {"type": "box", "width": 100, "height": 20, "depth": 50},
        ),
    )

    result = design_workflow.run_design_workflow("100 x 50 x 20 mm box")

    assert result["status"] == "review_required"
    assert result["design"]["geometry"]["type"] == "box"
    assert result["analysis"]["status"] == "preliminary"
    assert result["analysis"]["bounding_box_mm"] == {"x": 100, "y": 50, "z": 20}
    assert result["analysis"]["nominal_volume_mm3"] == 100000
    assert result["analysis"]["structural_analysis"]["status"] == "not_run"
    assert result["manufacturing"]["toolpath_status"] == "not_requested"


def test_toolpath_preview_is_explicit_and_non_production(monkeypatch, tmp_path):
    monkeypatch.setattr(design_workflow, "ARTIFACT_ROOT", tmp_path / "designs")
    monkeypatch.setattr(
        design_workflow,
        "text_to_cad",
        lambda prompt, output_dir: _cad_result(
            {
                "shape": "box",
                "prompt": prompt,
                "length": 100,
                "width": 50,
                "height": 20,
                "features": {},
            },
            {"type": "box", "width": 100, "height": 20, "depth": 50},
        ),
    )
    calls = []

    def generate_toolpath(**kwargs):
        calls.append(kwargs)
        return {"status": "success", "gcode": "G21\nM30", "preview_points": []}

    monkeypatch.setattr(
        design_workflow.CNCToolpathGenerator,
        "generate_rectangular_milling_job",
        generate_toolpath,
    )

    result = design_workflow.run_design_workflow(
        "100 x 50 x 20 mm box",
        include_toolpath_preview=True,
    )

    assert result["manufacturing"]["toolpath_status"] == "preview_only"
    assert result["manufacturing"]["toolpath_preview"]["gcode"] == "G21\nM30"
    assert "not production-ready" in result["manufacturing"]["limitations"][1]
    assert calls[0]["operation"] == "profile"
    assert calls[0]["bounds"]["unit"] == "mm"


def test_toolpath_preview_rejects_non_box_primitive(monkeypatch, tmp_path):
    monkeypatch.setattr(design_workflow, "ARTIFACT_ROOT", tmp_path / "designs")
    monkeypatch.setattr(
        design_workflow,
        "text_to_cad",
        lambda prompt, output_dir: _cad_result(
            {"shape": "cylinder", "prompt": prompt, "radius": 10, "height": 80, "features": {}},
            {"type": "cylinder", "radiusTop": 10, "radiusBottom": 10, "height": 80},
        ),
    )

    result = design_workflow.run_design_workflow(
        "10 mm radius 80 mm cylinder",
        include_toolpath_preview=True,
    )

    assert result["manufacturing"]["toolpath_status"] == "unsupported_shape"
    assert result["manufacturing"]["toolpath_preview"] is None


def test_invalid_geometry_blocks_downstream_stages(monkeypatch, tmp_path):
    monkeypatch.setattr(design_workflow, "ARTIFACT_ROOT", tmp_path / "designs")
    monkeypatch.setattr(
        design_workflow,
        "text_to_cad",
        lambda prompt, output_dir: _cad_result(
            {"shape": "box", "prompt": prompt, "length": 0, "width": 50, "height": 20},
            {"type": "box", "width": 0, "height": 20, "depth": 50},
        ),
    )

    result = design_workflow.run_design_workflow("degenerate box")

    assert result["status"] == "blocked"
    assert result["analysis"]["status"] == "blocked"
    assert result["manufacturing"]["status"] == "not_run"


def test_step_artifact_resolution_is_limited_to_workflow_uuid(monkeypatch, tmp_path):
    artifact_root = tmp_path / "designs"
    monkeypatch.setattr(design_workflow, "ARTIFACT_ROOT", artifact_root)

    def create_step(prompt, output_dir):
        step_file = Path(output_dir) / "szm_t2cad_box.step"
        step_file.write_text("ISO-10303-21;", encoding="ascii")
        return _cad_result(
            {
                "shape": "box",
                "prompt": prompt,
                "length": 100,
                "width": 50,
                "height": 20,
            },
            {"type": "box", "width": 100, "height": 20, "depth": 50},
            step_file,
        )

    monkeypatch.setattr(design_workflow, "text_to_cad", create_step)

    result = design_workflow.run_design_workflow("box")
    artifact = artifact_root / result["design_id"] / "szm_t2cad_box.step"

    assert result["design"]["step_available"] is True
    assert result["design"]["step_download_url"].endswith(f"{result['design_id']}/step")
    assert design_workflow.resolve_step_artifact(result["design_id"]) == artifact.resolve()
    assert design_workflow.resolve_step_artifact("../../etc/passwd") is None
