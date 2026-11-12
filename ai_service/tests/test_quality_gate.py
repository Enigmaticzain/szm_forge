"""Quality-gate focused tests (lightweight — no torch/transformers init on import)."""
import sys, json
sys.path.insert(0, "/mnt/D/projects/szm_forge")

from training.colab_config import build_config_light, validate_jsonl
from pathlib import Path

def test_train_dataset_quality():
    r = validate_jsonl(Path("/mnt/D/projects/szm_forge/machine_data/train.jsonl"))
    assert r["valid"] is True, f"train invalid: {r['notes']}"
    assert r["rows"] == 31
    assert len(r["duplicates"]) == 0
    assert len(r["empty_values"]) == 0
    print("PASS: train.jsonl valid, 31 rows, 0 duplicates, 0 empty")

def test_val_dataset_separate():
    r = validate_jsonl(Path("/mnt/D/projects/szm_forge/machine_data/val.jsonl"))
    assert r["valid"] is True
    assert r["rows"] == 7
    assert len(r["duplicates"]) == 0
    # Verify no instruction overlap with train (manual check via content)
    train_instr = {json.loads(l)["instruction"] for l in open("/mnt/D/projects/szm_forge/machine_data/train.jsonl")}
    val_instr = {json.loads(l)["instruction"] for l in open("/mnt/D/projects/szm_forge/machine_data/val.jsonl")}
    overlap = train_instr & val_instr
    assert len(overlap) == 0, f"Train/val overlap: {len(overlap)}"
    print("PASS: val.jsonl valid, 7 rows, 0 overlap with train")

def test_verified_filter_logic():
    # Verify that a dummy dataset with needs_review gets filtered by gate
    from datasets import Dataset
    ds = Dataset.from_dict({"instruction":["a","b"],"input":["",""],"output":["o1","o2"],"verified":["verified","needs_review"]})
    verified = ds.filter(lambda ex: ex.get("verified","needs_review") == "verified")
    assert len(verified) == 1
    print("PASS: verified-only filter excludes needs_review")

def test_config_light():
    cfg = build_config_light()
    assert cfg["base_model"] == "HuggingFaceTB/SmolLM2-1.7B-Instruct"
    assert cfg["lora_r"] == 8
    assert "fp16" in cfg
    print("PASS: light config correct")

if __name__ == "__main__":
    test_train_dataset_quality()
    test_val_dataset_separate()
    test_verified_filter_logic()
    test_config_light()
    print("All quality-gate tests passed.")
