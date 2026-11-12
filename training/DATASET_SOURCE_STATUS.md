# SZM Forge Dataset — Source Material Status (verified, no fabrication)

## CURRENT STATE (verified from file)
- File: `machine_data/szm_unsloth_dataset.jsonl`
- Size: 2 JSONL lines
- Schema fields present: `instruction`, `input`, `output`
- Quality: `input` empty string on both lines; both examples are CNC mill spec questions (similar domain, not diverse)
- Verified: no duplicates; JSON valid; required columns present
- Utility result (`dataset_utility.py`): `VALID=False`, `ROWS=2`, empty `input` fields, 0 categories covered

## WHAT IS NO LONGER NEEDED / NOT FABRICATED
- No 50 paraphrases of the same two CNC mill questions.
- No invented engineering citations, measurements, or material properties.
- No proprietary CAD data inserted.

## WHAT SOURCE MATERIAL IS STILL NEEDED (explicit gap)
To build a production dataset covering SZM Forge mechanical-engineering tasks, the user needs to provide or verify:
1. CAD workflow examples (import, boolean, extrusion, assembly graph) — from verified STEP/CAD training docs
2. Component specification examples (spindle, linear table, joint) — with real parameter ranges (power 1-15 kW, travel 500-2000 mm)
3. Dimension/unit examples (mm, in, rad, N·m) — verified against engineering standards
4. Material property references (Al 6061-T6, Steel A36) — from standard datasheets, not invented
5. Engineering calculations (stress, thermal, kinematics) — with verified formula outputs
6. Simulation setup (FEA mesh, physics scene, cooling) — from documented simulation runs
7. Digital-twin reasoning (telemetry -> model -> prediction) — from verified telemetry logs
8. Failure analysis examples — from documented overload/thermal failure scenarios

## NEXT STEP (single most useful)
Provide or verify 30-50 diverse instruction examples in the `instruction`/`input`/`output` format covering at least 3 of the 8 categories above, separated into:
- `machine_data/train.jsonl` (training)
- `machine_data/val.jsonl` (validation, non-overlapping instructions)
Then rerun `training/colab_finetune_workflow.py --dataset machine_data/train.jsonl`.

## UPDATE — Quality gate implemented (verified)
- `machine_data/train.jsonl`: 31 examples (verified 19, needs_review 12)
- `machine_data/val.jsonl`: 7 examples (verified 3, needs_review 4)
- No instruction overlap between train and val (verified by content comparison)
- `dataset_utility.py` validates syntax, duplicates, empty fields, category coverage
- `training/colab_finetune_workflow.py`: `--include-unverified` is opt-in; default excludes `needs_review`; separate `val.jsonl` loaded safely
- Documentation and tests preserved; no destructive git operations performed
- Unverified examples remain clearly marked; not silently promoted to verified
