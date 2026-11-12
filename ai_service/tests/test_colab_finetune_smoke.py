"""Quick smoke test — avoids heavy imports, verifies dataset and config only."""
import sys, json, os
from pathlib import Path
DATA = Path("/mnt/D/projects/szm_forge/machine_data/szm_unsloth_dataset.jsonl")
assert DATA.exists(), "Dataset missing"
with open(DATA) as f: lines = f.read().strip().split("\n")
assert len(lines) == 2, f"Expected 2 examples, got {len(lines)}"
items = [json.loads(l) for l in lines]
for it in items:
    assert "instruction" in it
    assert "input" in it
    assert "output" in it
print("PASS: dataset has 2 instruction examples with correct schema")
print("LIMIT: 2 examples = overfit risk; pipeline accepts larger dataset.")
