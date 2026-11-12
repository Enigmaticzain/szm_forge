"""Dataset generation / validation utility for SZM Forge mechanical-engineering tasks.
Does NOT fabricate engineering facts. Uses only verified format schema; content must come from user/verified sources.
Schema covers: CAD workflows, mechanical components, dimensions/units, material properties, engineering calculations, simulation, digital-twin reasoning.
"""
from __future__ import annotations
import json, sys, argparse
from pathlib import Path
from typing import List, Dict, Any

SCHEMA_REQUIRED = {"instruction", "input", "output"}
SCHEMA_QUALITY_CHECKLIST = [
    "Each instruction is a concrete mechanical-engineering task (not a paraphrase of existing).",
    "Input references actual CAD model, parameter, or material (not fabricated name).",
    "Output contains verifiable numerical result, specification, or step sequence.",
    "Dimensions include correct SI or imperial units.",
    "Material properties reference standard grades (e.g., Al 6061-T6, Steel A36).",
    "No duplicate instruction strings.",
    "No empty values in required fields.",
]

VALID_CATEGORIES = {
    "cad_workflow", "component_spec", "dimension_unit", "material_property",
    "calculation", "simulation_setup", "digital_twin", "failure_analysis"
}

def validate_dataset(path: Path) -> Dict[str, Any]:
    result = {"valid": False, "rows": 0, "duplicates": [], "empty_values": [],
              "missing_cols": [], "category_coverage": {}, "notes": []}
    if not path.exists():
        result["notes"].append("FILE MISSING")
        return result
    try:
        lines = path.read_text().strip().split("\n")
    except Exception as exc:
        result["notes"].append(f"READ ERROR: {exc}")
        return result
    seen = set()
    cats = set()
    for i, raw in enumerate(lines):
        try:
            obj = json.loads(raw)
        except Exception as exc:
            result["notes"].append(f"LINE {i+1} BAD JSON")
            continue
        result["rows"] += 1
        # Required fields
        for c in SCHEMA_REQUIRED:
            if c not in obj:
                result["missing_cols"].append(f"line{i+1}:{c}")
            else:
                v = obj[c]
                if isinstance(v, str) and v.strip() == "":
                    result["empty_values"].append(f"line{i+1}:{c}")
        # Category tagging (optional field)
        cat = obj.get("category", None)
        if cat:
            cats.add(cat)
        instr = obj.get("instruction", "")
        if instr in seen:
            result["duplicates"].append(f"line{i+1}")
        else:
            if instr: seen.add(instr)
    result["valid"] = (len(result["missing_cols"]) == 0 and len(result["duplicates"]) == 0 and len(result["empty_values"]) == 0)
    result["category_coverage"] = {c: True for c in cats}
    result["missing_categories"] = [c for c in VALID_CATEGORIES if c not in cats]
    result["notes"].append(f"ROWS: {result['rows']}; VALID: {result['valid']}")
    result["notes"].append(f"DUPLICATES: {len(result['duplicates'])}; EMPTY: {len(result['empty_values'])}; MISSING_COLS: {len(result['missing_cols'])}")
    return result

def print_quality_checklist():
    print("=== DATASET QUALITY CHECKLIST ===")
    for idx, item in enumerate(SCHEMA_QUALITY_CHECKLIST, 1):
        print(f"{idx}. {item}")
    print("=== REQUIRED FIELDS ===")
    print(SCHEMA_REQUIRED)
    print("=== VALID CATEGORIES ===")
    print(sorted(VALID_CATEGORIES))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--validate", default=str(Path("/mnt/D/projects/szm_forge/machine_data/szm_unsloth_dataset.jsonl")))
    args = parser.parse_args()
    res = validate_dataset(Path(args.validate))
    print(f"VALID: {res['valid']} | ROWS: {res['rows']}")
    print(f"NOTES: {res['notes']}")
    print(f"CATEGORIES FOUND: {res['category_coverage']}")
    print(f"MISSING CATEGORIES: {res['missing_categories']}")
    print_quality_checklist()

if __name__ == "__main__":
    main()
