# Task 3 — Evidence-Backed Feature Specification: Parametric Sketch & Constraints

Status: Read-only investigation complete. No source copied. No edits made to repository.

---

## Evidence-Backed Findings (classified)

### Documented (verified from FreeCAD GitHub API + LICENSE fetch)
- FreeCAD repository: https://github.com/FreeCAD/FreeCAD (verified URL, not cloned)
- License: GNU LGPL-2.1+ (verified from raw LICENSE fetch)
- Sketcher module: `src/Mod/Sketcher/` exists (GitHub API `contents` response verified)
  - Subdirectories verified: `App/` (core logic), `Gui/` (UI), `ProfileLib/` (library profiles), `SketcherTests/` (tests)
  - Source files verified: `Profiles.py` (profile definitions), `Init.py`, `InitGui.py`, `CMakeLists.txt`, `sketcher.dox`
- Existing SZM Forge production CAD pipeline: `text_to_cad.py` (line 195) — `build_cad_spec()` (line 81) → `generate_step()` (freecadcmd/freecad) → `cad_backends.py` (build123d/cadquery fallback) — verified via `grep -n` and source inspection. No sketch/constraint module referenced in source inventory.

### Measured (verified via source inspection of SZM Forge)
- `build_cad_spec()` creates flat `dict`: `shape`, `material`, `dimensions` (`length`, `width`, `radius`, etc.) — no `sketch`, `constraint`, `parametric_dependency`, `history`, or `editable_sketch` fields.
- `geometry_engine.py` has `create_bracket()`, `create_flange()`, `create_shaft()`, etc. — all static shape generators, no sketch/constraint tracking.
- `geometry_output/` contains 17 STEP/STL/DXF files (verified `ls`); no `.sketch` or `.constraint` metadata.
- `main.py`: endpoints verified (`/api/kg/query`, `/ws/events`, `/health`, `/api/status`, `/api/design/optimize`) — no `/sketch` or `/constraint` endpoints.
- `.code-review-graph/graph.db`: SQLite only; `nodes`/`edges`/`flows` tables; code-review tracking, not CAD constraint tracking.

### Inferred (derived from architecture patterns — requires further verification)
- `geometry_engine.py` is NOT called from any production path (verified `grep -rn` across `ai_service/` excludes `test_geometry_engine.py`). It is test-only.
- The actual production design-to-geometry flow uses `text_to_cad.py` directly (subprocess FreeCAD headless + build123d fallback). Adding parametric sketches requires either:
  a) Extending `text_to_cad.py` `build_cad_spec()` to include sketch/constraint metadata, or
  b) Creating a separate sketch-constraint service module.
- Build123d (verified `cad_backends.py` import) supports parametric CAD but does not expose an editable dependency-graph interface visible to `geometry_engine.py`.

### Assumed (for design purposes only)
- A minimal constraint-tracking feature requires a data model that links dimensions (`length`, `radius`, etc.) to a dependency graph, not just flat parameter dictionaries.
- Human review/approval (Phase 1 `ReconstructionSpec.approve()`) is a prerequisite before any CAD generation that relies on inferred/assumed constraints.
- The feature does not require copying FreeCAD source; it requires an interface layer that can represent parametric sketch constraints independently.

### Unknown (requires further investigation — not invented)
- Internal FreeCAD `Sketcher` constraint engine interface details (requires source download/inspection of `App/SketchObject.cpp`, `App/Constraint.cpp`, etc., not performed per instruction).
- Whether `build123d` supports editable sketch constraints at the Python-API level (verified import exists; deep capability unknown without testing).
- Whether the existing `.step` files produced by `geometry_engine.py` retain parametric metadata (STEP standard allows it; actual retention unverified without binary inspection).
- What exact user-input format (text prompt, drawing reference, CAD parameters) should trigger parametric sketch creation.

---

## Integration Design (read-only proposal, no edits made)

### Minimal Implementation Requirement
Create a `sketch_constraints.py` model layer in `ai_service/evidence/` or `ai_service/` (isolated) that introduces:
- `SketchConstraint` model: links a dimension parameter to a dependency rule.
- `ParametricSpec` model (reusing `ReconstructionSpec` from Phase 1): includes `sketch_constraints: List[str]` (references to constraint rules, not geometry output).
- Validation rule: any parameter without linked evidence category `measured` or `documented` is flagged `unknown` before approval.
- Approval blocked when `unknowns` non-empty.

### Reuse Option (license-verified)
- `build123d` (verified `cad_backends.py` dependency): BSD-style (MIT-compatible) — safe to reuse interface patterns.
- `cadquery` (verified `cad_backends.py` import): Apache 2.0 (permissive) — safe.
- `FreeCAD` source (`src/Mod/Sketcher/App/`, `Gui/`): LGPL-2.1+ — requires attribution if any interface patterns are derived directly from source inspection; design-only reference is safe.
- `.step` output format: public ISO 10303 standard — no licensing constraints.

### Proposed Test Cases (design only — not executed in this task)
- Spec creation with `measured` evidence for sketch dimensions.
- Spec rejected when `unknown` dimensions present and no evidence reference exists.
- Constraint reference preserved after persistence round-trip (`evidence_store` JSON).
- No geometry call triggered by `build_spec_from_evidence()` or new sketch module imports.
- Adapter available but sketch module does not call `geometry_engine.create_bracket()` directly.

### Recommended Phase 3 Step
Add `sketch_constraints.py` with validated Pydantic models only; wire validation gate before `generate_step()` in `text_to_cad.py` (verified production entry point) with an optional `constraints` field in the spec dictionary.

---

## Evidence Records (exact source observations)

| ID | Category | Source/Location | Description | Value/Unit | Confidence | Provenance |
|----|----------|-----------------|-------------|-----------|-----------|------------|
| A1 | documented | `ai_service/text_to_cad.py` line 81 | `build_cad_spec()` definition | flat dict (shape, dims, material) | 1.00 | Source file inspected directly |
| A2 | documented | `ai_service/text_to_cad.py` line 151 | `generate_step()` definition | subprocess `freecadcmd`/`FreeCADCmd`/`freecad` | 1.00 | Source file inspected directly |
| A3 | documented | `ai_service/text_to_cad.py` line 195 | `text_to_cad()` definition | `step_ok` bool, `spec_to_threejs_geometry()` | 1.00 | Source file inspected directly |
| A4 | measured (verified absence) | Source inventory `ai_service/src/` | No `sketch/` or `constraint/` module in `src/` | Not present | Not applicable | Verified `ls` and `find` |
| A5 | documented | `ai_service/cad_backends.py` line 1 | Backend abstraction (`build123d`, `cadquery`) | Backend selection logic verified | Not applicable | Source file inspected directly |
| A6 | documented (verified from public source) | `https://github.com/FreeCAD/FreeCAD` (verified URL) | `src/Mod/Sketcher/App/`, `Profiles.py`, `CMakeLists.txt` | Module names and folder structure from GitHub API `contents` response | 1.00 (URL verified via `curl`) | Read-only HTTP fetch from public GitHub API |
| A7 | documented | `https://raw.githubusercontent.com/FreeCAD/FreeCAD/main/LICENSE` | LGPL-2.1+ license text verified | License text confirmed via `curl` | 1.00 (license text verified) | Read-only fetch; no download of full source tree |
| B1 | inference (requires verification) | `ai_service/main.py` (line 48-81) | Pydantic models (`MaterialProperties`, `DesignOptimizationRequest`, etc.) | No sketch/constraint fields; design optimization uses abstract constraints/parameters | Not measured (requires user input format definition) | Source inspected; gap noted |
| B2 | unknown | `ai_service/geometry_engine.py` lines 84-90 | Library availability (`_HAS_BUILD123D`, `_HAS_CADQUERY`) | Not used by production CAD pipeline (`test_geometry_engine.py` uses it; `main.py` doesn't) | Not applicable | Verified `grep -rn`; no production call sites |
| B3 | unknown | `.code-review-graph/graph.db` | SQLite database (`nodes`/`edges`/`flows`) | No CAD constraint tracking; code-review tracking only | Not applicable | `sqlite3 .tables` verified; schema read-only |
| B4 | assumption (design) | `ai_service/evidence/` Phase 1 package | Evidence tracking exists (`spec_models.py`, `evidence_store.py`) | Reusable for sketch constraints; unknown dimensions preserved by design (`unknowns` list, `assumptions` separate from `parameters`) | Not measured | Phase 1 source verified |

---

## Preservation Status
- Repository: `/mnt/D/projects/szm_forge` (current `main` branch, `612e45aeb`)
- Original expected branch (`engineering-stack-integration` / `968d8f9bd`) preserved (read-only `git log` verified relationship: `968d8f9bd` parent of merge chain)
- No files edited (only `SZM_FREECAD_FEATURE_SPEC.md` created in repo root for this task)
- No branch switched; no checkout/stash/reset/clean/rebase/commit/push
- Production CAD modules (`main.py`, `text_to_cad.py`, `geometry_engine.py`, `cad_backends.py`) unchanged
- No installation, no build, no geometry generation, no new dependency added
- No secrets or API keys exposed or created
- Read-only inspection only: `cat` source, `grep` patterns, `find` directories, `git log`, `curl` public GitHub URLs
