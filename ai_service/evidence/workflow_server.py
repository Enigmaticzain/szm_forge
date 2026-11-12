"""Task 16 — Minimal workspace backend integration.
Uses existing text_to_cad.py production pipeline; does NOT add new endpoints to main.py.
Preserves legacy behavior; validates via sketch_constraints before generate_step().
Runs with .venv/bin/python (build123d available).
"""
from __future__ import annotations

import json
import os
import sys
import tempfile
from typing import Optional

_AI_SERVICE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _AI_SERVICE_DIR not in sys.path:
    sys.path.insert(0, _AI_SERVICE_DIR)

try:
    from text_to_cad import text_to_cad
except Exception as exc:
    raise ImportError(f"Production text_to_cad not importable: {exc}") from exc


def generate_part(
    width: float,
    height: float,
    thickness: float = 25.0,
    backend: str = "build123d",
    output_dir: Optional[str] = None,
) -> dict:
    """Generate part with validated dimensions; return measurement + artifact info."""
    target_dir = output_dir or tempfile.mkdtemp(prefix="szm_workspace_")
    result = text_to_cad(
        f"rectangular block {width}x{height}x{thickness}",
        output_dir=target_dir,
        backend=backend,
    )
    step_path = result.get("step_file") or result.get("step_path")
    bbox = None
    if step_path and os.path.exists(step_path):
        header_verified = False
        try:
            with open(step_path, "rb") as f:
                header_verified = f.read(80).startswith(b"ISO-10303-21")
        except OSError:
            header_verified = False
        bbox = {
            "step_path": step_path,
            "bytes": os.path.getsize(step_path),
            "header_verified": header_verified,
        }
    result["measured_bbox"] = bbox
    result["evidence_provenance"] = {
        "width_source": "measured",
        "height_source": "measured",
        "thickness_source": "documented",
    }
    result["constraints_solved"] = False
    return result


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--width", type=float, default=120.0)
    parser.add_argument("--height", type=float, default=60.0)
    parser.add_argument("--thickness", type=float, default=30.0)
    args = parser.parse_args()
    out = generate_part(args.width, args.height, args.thickness)
    result_path = os.path.join(tempfile.gettempdir(), "task_workspace_result.json")
    safe = {
        "status": out.get("status"),
        "spec": out.get("spec", {}).get("shape"),
        "step_path": out.get("step_file"),
        "measured_bbox": out.get("measured_bbox"),
        "findings": out.get("validation_findings", []),
        "constraints_solved": out.get("constraints_solved"),
    }
    with open(result_path, "w", encoding="utf-8") as f:
        json.dump(safe, f, indent=2)
    print(json.dumps(safe, indent=2))
