"""Lightweight config/dataset validation — no torch/transformers imports.
Separated so tests run quickly without initializing heavy ML libraries.
"""
from __future__ import annotations
import json
from pathlib import Path
from typing import Dict, Any, Optional, List

DATASET_PATH = Path("/mnt/D/projects/szm_forge/machine_data/szm_unsloth_dataset.jsonl")

REQUIRED_COLS = {"instruction", "input", "output"}
BASE_MODEL = "HuggingFaceTB/SmolLM2-1.7B-Instruct"  # MIT, open, instruction-tuned

def validate_jsonl(path: Path) -> Dict[str, Any]:
    """Syntax + schema + duplicates + empty-value check."""
    result = {"path": str(path), "rows": 0, "valid": False,
              "duplicates": [], "empty_values": [], "missing_cols": [],
              "notes": []}
    if not path.exists():
        result["notes"].append("FILE MISSING")
        return result
    try:
        lines = path.read_text().strip().split("\n")
    except Exception as exc:
        result["notes"].append(f"READ ERROR: {exc}")
        return result
    seen = set()
    duplicates = []
    empty_vals = []
    for i, raw in enumerate(lines):
        try:
            obj = json.loads(raw)
        except Exception as exc:
            result["notes"].append(f"LINE {i+1} BAD JSON: {exc}")
            continue
        result["rows"] += 1
        # Check required cols
        for c in REQUIRED_COLS:
            if c not in obj:
                result["missing_cols"].append(f"line{i+1}:{c}")
        # Empty value check (instruction should not be empty)
        for c in REQUIRED_COLS:
            v = obj.get(c)
            if v is not None and (isinstance(v, str) and v.strip() == ""):
                empty_vals.append(f"line{i+1}:{c}")
        # Duplicates by instruction content
        instr = obj.get("instruction", "")
        if instr in seen:
            duplicates.append(f"line{i+1}")
        else:
            seen.add(instr)
    result["duplicates"] = duplicates
    result["empty_values"] = empty_vals
    result["valid"] = (len(result["missing_cols"]) == 0 and len(duplicates) == 0 and len(result["notes"]) == 0)
    if result["rows"] < 10:
        result["notes"].append(f"DATASET SMALL: {result['rows']} rows; not production-ready.")
    if duplicates:
        result["notes"].append(f"DUPLICATES FOUND: {duplicates}")
    if empty_vals:
        result["notes"].append(f"EMPTY VALUES: {empty_vals}")
    return result

def build_config_light() -> Dict[str, Any]:
    return {
        "base_model": BASE_MODEL,
        "lora_r": 8, "lora_alpha": 16,
        "lora_target_modules": ["q_proj", "v_proj", "k_proj", "o_proj"],
        "max_seq_length": 256, "seed": 42,
        "max_steps": 3,  # dataset small; override when larger
        "learning_rate": 2e-4, "per_device_train_batch_size": 1,
        "gradient_accumulation_steps": 2, "fp16": False,  # set when cuda
        "report_to": "none",
    }
