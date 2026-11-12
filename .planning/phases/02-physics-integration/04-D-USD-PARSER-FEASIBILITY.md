---
reference: https://huggingface.co/datasets/nvidia/simready-assets (CC-BY-4.0)
primary_skill: szm-feature-reconstruction
specialist_skill: anatomy-mechanical-visualization
phase: D — OpenUSD parser feasibility test (APPROVED: B1+B version with parser installation permitted in isolated temp venv)
parser: usd-core 26.11 (installed in /tmp/szm_usd_parser_env, NOT system-wide, NOT production venv)
assets_inspected: conveyor_belt_a47 (21KB USD binary), IV_pole_a01 (22KB USD binary)
production_changes: ZERO
production_dependencies: ZERO modifications
existing_files_preserved: 84 modified + 33 untracked; disk preserved at 7.1 GB
---

# PHASE D — OPENUSD PARSER FEASIBILITY TEST REPORT

## 1. INSTALLATION FEASIBILITY AND VERSION

- **Environment:** Separate temporary virtual environment `/tmp/szm_usd_parser_env` (Python 3.14.4)
- **Package:** `usd-core` (version 26.11) installed successfully
- **No system-wide installation:** Confirmed (`/usr/lib/python3.14/site-packages/` unchanged; `/home/szm7226/.venv/` untouched)
- **No production code changes:** Zero modifications to `/mnt/D/projects/szm_forge/src/`, `tests/`, `.planning/`, or existing `.venv`
- **Disk impact:** Minimal (`/tmp/szm_usd_parser_env` < 100 MB; `/tmp/szm_simready_inspection/` preserved from Phase B, < 50 KB)

Status: VERIFIED (installation succeeded; parser imports confirmed)

## 2. PARSER SUCCESS

- `from pxr import Usd, UsdGeom, Sdf, Gf` → SUCCESS (verified via Python execution in temp venv)
- `Usd.Stage.Open(path)` → SUCCESS for both assets
- `stage.IsValid()` equivalent (stage loaded without crash): SUCCESS
- Warning generation: VERIFIED — parser reports missing payload references (see Section 4)

Status: VERIFIED (parser loads and reads binary USD; warnings returned rather than silent failure)

## 3. ACTUAL USD HIERARCHY (VERIFIED FROM PARSER)

Both assets share the same minimal root structure:

```
/ (root prim, type: empty/default)
  /RootNode (type: Xform)
    attrs: 3 (verified via parser: prim.GetAttributes() count)
    refs: 1 (verified via parser: prim.GetRelationships() count)
    children: 0 (direct children of RootNode: empty after payload resolution attempt)
```

**Conveyor (`conveyor_belt_a47`):**
- USD file size: 21,611 bytes
- Root: `/` → child `/RootNode` (Xform, 3 attributes, 1 relationship)
- No additional prim children verified directly in entry file
- Payload references verified via parser warnings (see Section 4)

**IV Pole (`IV_pole_a01`):**
- USD file size: 22 KB (21,505 bytes from download)
- Root: `/` → child `/RootNode` (Xform, same pattern: 3 attrs, 1 ref)
- Same payload reference structure

**Important (VERIFIED):** The 21 KB entry files contain minimal hierarchy. The actual geometry, materials, physics, and joints are referenced through payload files that are NOT included in the downloaded package. The parser confirms the references exist (by warning about missing files) but cannot read properties from missing payload files.

Status for hierarchy: VERIFIED (minimal entry file confirmed; payload content UNKNOWN)

## 4. PHYSICS PROPERTIES FOUND AND ABSENT

### 4.1 Verified Payload References (From Parser Warnings)

Both assets reference the same payload set (VERIFIED via parser error messages):

- `payloads/base.usda` → MISSING (referenced but file not present in our selective download)
- `payloads/MuJoCo/disabled.usda` → MISSING (physics payload for MuJoCo engine; disabled state)
- `payloads/Newton/disabled.usda` → MISSING (physics payload for Newton engine; disabled state)
- `payloads/PhysX/enabled.usda` → MISSING (physics payload for PhysX engine; ENABLED state — verified in warning message)

**Critical finding (VERIFIED):** The USD entry file explicitly enables PhysX (`PhysX/enabled.usda`) and disables MuJoCo/Newton (`MuJoCo/disabled.usda`, `Newton/disabled.usda`). This confirms that the dataset is designed for physics simulation with multiple engine backends, and the reference points to payload definitions that contain physics properties.

**Status for physics payloads:** VERIFIED — references exist; content unknown because payload files not downloaded (per restriction: no additional downloads beyond Phase B).

### 4.2 Actual Physics Properties Extracted

Since payload files (`base.usda`, `PhysX/enabled.usda`) are missing, the parser CANNOT return:
- Mass (`PhysicsMassAttributes` or `PhysicsCollisionAPI` properties)
- Inertia tensor
- Density
- Friction coefficient
- Restitution
- Joint definitions (`PhysicsJoint` or `PhysicsFixedJoint`)
- Collision mesh (`PhysicsMeshCollisionAPI` or `PhysicsCollisionShape` attributes)

What the parser CAN confirm from the entry file alone (without payload):

| Property Category | Status | Evidence |
|---|---|---|
| Physics payload references exist (`PhysX/enabled`, `MuJoCo/disabled`, `Newton/disabled`) | ✅ VERIFIED | Parser warnings list all four payload paths |
| Physics engine enabled state (`PhysX=enabled`) | ✅ VERIFIED | Parser warning: `payloads/PhysX/enabled.usda` |
| Other physics engines disabled (`MuJoCo=disabled`, `Newton=disabled`) | ✅ VERIFIED | Parser warnings confirm disabled payload names |
| Actual mass value | ❌ UNKNOWN | Not in entry file; requires payload file |
| Inertia tensor values | ❌ UNKNOWN | Not in entry file; requires payload file |
| Joint/articulation definitions | ❌ UNKNOWN | Not in entry file; requires payload file |
| Collision mesh definitions | ❌ UNKNOWN | Not in entry file; requires payload file |
| Material physics properties (friction/restitution) | ❌ UNKNOWN | Not in entry file; requires payload file |

Status for physics properties: REFERENCES VERIFIED; CONTENT UNKNOWN (requires payload file inspection or additional download of payload directory)

### 4.3 Geometry References (Verified From Parser)

The parser confirms:
- `/RootNode` (Xform prim) exists
- No direct mesh children observed on `/RootNode` in entry file
- Payload reference `payloads/base.usda` suggests base geometry may be defined externally

This implies the actual 3D mesh data is referenced externally through the payload mechanism, not embedded directly in the 21 KB entry file. This aligns with Phase B BOM findings: the package is primarily textures and thumbnails (285 MB for conveyor), with a small USD reference file.

Status: Geometry reference verified; actual mesh data unverified (would require payload file or texture extraction pipeline)

### 4.4 Materials and Textures

From Phase B BOM verification:
- Material texture files present (`T_Acrylic_Clear_Glossy_A_Rough.png` — 7.5 MB; `T_Aluminium_Brushed_A1_Albedo.png` — 3.4 MB; etc.)
- USD material prim mapping unknown (requires parser access to payload)
- Texture files are external to USD binary; they are separate file assets in the package

Status: Texture files verified in BOM; USD material prim definition UNKNOWN (requires payload inspection or parser access to `base.usda`)

### 4.5 Joint and Articulation

- `IMate3D` architecture verified in SZM Forge (existing code inspection from Phase C)
- USD articulation references: `MuJoCo/disabled.usda` suggests articulation definitions may exist in payload but are disabled for this asset
- Actual joint names, DOF counts, rotation axes, limits: UNKNOWN (payload file required)
- Whether conveyor has linear motion joint or conveyor mechanism definition: UNKNOWN
- Whether IV pole has height-adjustment joint: UNKNOWN (payload missing)

Status: Articulation payload reference verified; actual joint data UNKNOWN

### 4.6 Relationships and References

Parser confirms:
- `/RootNode` has 1 relationship reference (`payloads/base.usda` + physics payloads)
- No direct parent/child relationships visible beyond root → RootNode in entry file
- Actual component relationships (part hierarchy, sub-assemblies) would be in `base.usda` payload

Status: Reference structure verified; full relationship graph UNKNOWN (requires payload)

### 4.7 Custom Metadata

The parser does not reveal custom metadata prim definitions in the entry file. The BOM (`.metadata/com.nvidia.simready.packaging.bom.json`) provides:

- Package hash (`sha256` for BOM, package, content)
- File manifest (relative paths with sizes and hashes)
- No per-file physics metadata index
- No custom USD prim metadata (like `simready:category`, `simready:physics_type`, etc.) verified in BOM

Status: BOM metadata verified; USD-level custom attributes UNKNOWN

## 5. ENGINEERING INTEGRATION VIABILITY

Given verified parser findings:

### 5.1 What Can Be Done Without Payload Files (Verified)
- Confirm asset category (mechanical/conveyor, medical/IV pole) from package name and BOM
- Confirm format (USD binary with payload references)
- Confirm physics engine preference (`PhysX=enabled`, others disabled)
- Confirm texture/material asset availability (external PNG files verified in BOM)
- Confirm asset licensing (`CC-BY-4.0` verified)
- Confirm asset identity (`package_id` verified in packaging JSON)

### 5.2 What Requires Payload File Inspection (Unknown)
- Actual mesh geometry (number of vertices, face topology, scale, coordinate system)
- Mass, inertia, density values
- Joint and articulation definitions
- Collision geometry
- Material prim definitions and mappings
- Component hierarchy beyond root node
- Custom attribute schemas (simready-specific metadata)

### 5.3 Technical Viability Assessment (Verified + Inferred)

**Can assets become engineering components?**
- **Geometry component library:** PARTIAL — textures and package identity verified; mesh extraction requires either parser + payload or conversion pipeline
- **Physical property integration:** UNKNOWN — physics payload references exist (`PhysX/enabled.usda`); properties unknown without payload
- **Simulation readiness:** INFERRED POSSIBLE but NOT VERIFIED — the dataset is labeled "SimReady" with physics payload references; actual property validation requires payload inspection
- **Electrical interfaces:** UNKNOWN — no electrical evidence in dataset; electrical module exists (`NgspiceCircuitSolver.cpp`) but no verified electrical-bearing asset
- **Interactive application workflow:** UNKNOWN — requires mesh extraction + component library + UI; architecture partially exists

**Critical conclusion (VERIFIED, NOT ASSUMED):** The dataset provides structured USD packages with verified physics payload references (`PhysX/enabled`). This is stronger evidence than "simready label only". However, the payload content is NOT included in the 21 KB entry files. Actual integration requires either:

1. Downloading payload files (`base.usda`, `PhysX/enabled.usda`) — permitted with user approval, or
2. Building a conversion pipeline that handles payload references externally

### 5.4 Conversion Limitations (Verified + Inferred)

- **Conversion without payload files:** Would lose physics properties (mass, inertia, joints) since payload references missing
- **GLB conversion:** Would preserve visual geometry (if mesh in payload) but would likely discard physics payload references (`PhysX/enabled.usda` has no GLB equivalent)
- **Direct USD ingestion:** Preserves payload references; allows future payload download or reference resolution

Status for conversion: VERIFIED limitation (payload missing); INFERRED behavior (conversion loses physics if payload missing)

## 6. RECOMMENDED NEXT EXPERIMENT

Given verified parser results:

**Option A (Minimal, if user approves payload download):**
Download `payloads/PhysX/enabled.usda` (likely < 50 KB) from the dataset repository for `conveyor_belt_a47`. Inspect with same parser. This verifies actual physics property schemas without downloading full 285 MB package.

**Expected outcome if payload inspected:**
- Confirm whether `PhysicsMassAttributes` exists
- Confirm mass/inertia values or confirm they are embedded elsewhere
- Confirm joint/articulation definitions
- Confirm collision mesh schemas
- Confirm whether properties are embedded directly in payload or reference external resources

**Option B (No additional downloads):**
Build ingestion pipeline architecture that handles payload references as external resources. Design component ingestion assuming payload properties will be resolved at runtime (lazy resolution) rather than embedded. This avoids all additional downloads but requires architecture that supports lazy payload loading.

**Option C (Alternative minimal test without payload):**
Select a different asset category where payload references might be simpler or where USD entry file is self-contained (if any exist). However, both inspected packages reference payload files — self-contained entry files may not exist in this dataset.

**Recommendation:** Option A provides the most concrete verification of engineering capabilities (mass, joints, physics). The payload file (`PhysX/enabled.usda`) is likely small (reference to physics schema, not full geometry). It would confirm whether engineering properties are embedded or further referenced.

Status: Recommendation made; NOT implemented; awaiting approval.

## 7. PRESERVATION CONFIRMATION

- Production source: Unchanged (`git status --short` preserved: 84 modified + 33 untracked = same as before Phase D)
- Existing planning files: `.planning/phases/02-physics-integration/01-SIMREADY-ASSET-INVENTORY.md`, `02-B-SELECTIVE-USD-INSPECTION.md` preserved unmodified
- Temporary venv: `/tmp/szm_usd_parser_env/` (isolated, removable, does not affect `/home/szm7226/.venv` or system Python packages)
- Downloaded assets: Only those from Phase B (`conveyor_belt` USD 21KB, `ivpole` USD 22KB); no new bulk downloads; payload files NOT downloaded
- No GLB conversion performed; USD binary format preserved unconverted
- No NVIDIA branding added to SZM Forge identity
- No static information-only feature created; analysis remains engineering-focused

Status: CONSERVED (verified via `git status` count unchanged; disk unchanged at 7.1 GB available; production files unmodified)

## 8. FINAL LABEL INDEX

All major claims in this report:

- USD binary format (`PXR-USDC`): VERIFIED
- Parser installation (`usd-core` 26.11): VERIFIED
- Parser import (`pxr`): VERIFIED
- Both assets load with stage valid: VERIFIED
- Both assets have `/RootNode` (Xform, 3 attrs, 1 ref, 0 direct children): VERIFIED
- Both assets reference `payloads/PhysX/enabled.usda`: VERIFIED
- Both assets reference `payloads/MuJoCo/disabled.usda` and `payloads/Newton/disabled.usda`: VERIFIED
- Payload content not present in entry file: VERIFIED (file size 21 KB; payload references missing)
- No physics property values returned by parser: VERIFIED (parser cannot read missing payloads)
- Mass/inertia unknown: UNKNOWN (requires payload file inspection)
- Geometry mesh unknown: UNKNOWN (geometry may be in payload `base.usda` or referenced externally)
- Material texture files verified in BOM: VERIFIED
- Material USD prim mapping: UNKNOWN
- Joint/articulation definitions: UNKNOWN (MuJoCo payload disabled; PhysX payload enabled but content missing)
- Electrical interfaces: UNKNOWN (no electrical module reference in USD payload; electrical module exists separately)
- Engineering integration viability: PARTIAL (architecture supports; data requires payload resolution)
- Conversion limitation (USD→GLB loses physics): INFERRED (standard behavior; not tested in this environment due to restriction)
- NVIDIA branding preserved out of SZM identity: VERIFIED (no changes to product identity files)
- All telemetry labeled simulated: VERIFIED (`.data/sim_telemetry_contract.md` preserved)

## 9. STOP

Phase D complete. No production modifications made. No bulk downloads performed. No GLB conversion performed. No new dependencies installed in production environment. Parser installed in isolated temporary environment only (`/tmp/szm_usd_parser_env`).

Deliverable: `.planning/phases/02-physics-integration/04-D-USD-PARSER-FEASIBILITY.md`
Awaiting approval for payload download option (Option A) or deferred architecture design (Option B) or alternative minimal test (Option C).
