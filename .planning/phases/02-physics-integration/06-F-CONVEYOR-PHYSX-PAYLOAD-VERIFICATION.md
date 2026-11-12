---
reference: https://huggingface.co/datasets/nvidia/simready-assets (CC-BY-4.0)
primary_skill: szm-feature-reconstruction
specialist_skill: anatomy-mechanical-visualization
phase: F — Conveyor PhysX payload verification (PAUSED — file unavailable)
parser: usd-core 26.11 in /tmp/szm_usd_parser_env (isolated temp venv)
asset: conveyor_belt_a47 / sm_conveyor_belt_a47_01
payload_inspected: instances_physx.usda (NOT DOWNLOADABLE — 404)
production_changes: ZERO
production_dependencies: ZERO modifications
existing_files_preserved: 84 modified + 33 untracked; disk preserved
---

## 1. PRE-DOWNLOAD SAFETY CHECKS

### 1.1 Remote file path
`https://huggingface.co/datasets/nvidia/simready-assets/resolve/main/conveyor_belt_a47/simready_usd/payloads/PhysX/instances_physx.usda`

### 1.2 Remote file size
**404 Not Found** — The file does not exist at the specified location. The server returns:
- `content-length: 15` (minimal HTTP response)
- `HTTP/2 404` status code
- `content-type: text/plain`

### 1.3 Disk space
- `/tmp` filesystem: 5.4 GB available (5.6 GB total, 248 MB used)
- Sufficient space for a 50 KB file

### 1.4 Isolated OpenUSD environment
- `/tmp/szm_usd_parser_env` exists and contains `python3` (python3.14.4)
- `usd-core` 26.11 is installed and functional (verified in Phase D)
- No system-wide installations modified

### 1.5 Git status (pre-download)
- No modifications to production files
- 84 modified + 33 untracked files preserved
- No changes to `.planning/` directory

## 2. DOWNLOAD RESULT

### 2.1 Download attempt
```
curl -I "https://huggingface.co/datasets/nvidia/simready-assets/resolve/main/conveyor_belt_a47/simready_usd/payloads/PhysX/instances_physx.usda"
```
**Result:** HTTP 404 Not Found — file does not exist at the URL.

### 2.2 Size verification
- Expected max: 50 KB (50,000 bytes)
- Actual: **File not present** — cannot verify size
- **Status:** ❌ **CANNOT DOWNLOAD** — file missing from source

## 3. TECHNICAL INSPECTION (PENDING)

Cannot proceed with inspection because the required file is unavailable. The `enabled.usda` payload (19.8 KB) references 82 instances of `instances_physx.usda` (paths like `../instances_physx.usda@</Instances/sm_XX_mesh_00>`), but this file cannot be downloaded.

## 4. PHYSICS FINDINGS (UNKNOWN — FILE MISSING)

| Property | Status | Evidence |
|----------|--------|----------|
| **Physics properties** | ❌ UNKNOWN | Cannot inspect `instances_physx.usda` — file not available |
| **Mass / inertia** | ❌ UNKNOWN | No physics properties in `enabled.usda`; cannot verify in `instances_physx.usda` |
| **Collision geometry** | ❌ UNKNOWN | No collision data in `enabled.usda`; cannot verify in `instances_physx.usda` |
| **Joints / articulation** | ❌ UNKNOWN | No joint definitions in `enabled.usda`; cannot verify in `instances_physx.usda` |
| **Materials / friction** | ❌ UNKNOWN | No material bindings in `enabled.usda`; cannot verify in `instances_physx.usda` |
| **Physics API usage** | ❌ UNKNOWN | Cannot confirm `UsdPhysics.RigidBodyAPI`, `UsdPhysics.MassAPI`, etc. |

## 5. SZM FORGE ARCHITECTURE COMPATIBILITY

### Existing SZM Forge Capabilities (from Phase C)
- `KinematicSolver` / `IMate3D` (CoincidentPlanarMate, ConcentricMate) — available
- `AssemblyGraph` / `BasePart` registration — available
- `SimulationEngine` / `Tick` loop — available
- `MaterialDatabase` — available
- `SceneGraph` hierarchy — available
- `Physics` modules (PhysicsJoint, PhysicsRevoluteJoint, etc.) — available

### Compatibility Assessment
- **Architecture:** The asset structure (70 instances under `/RootNode/Geometry/`) maps directly to SZM Forge's `BasePart` and `AssemblyGraph` concepts.
- **Reusability:** All existing SZM Forge components can be used to ingest the geometry.
- **Missing:** Physics properties (mass, inertia, collision, joints) are required to make the asset physically viable. Without `instances_physx.usda`, these cannot be verified.

## 6. REMAINING TECHNICAL GAPS

1. **Missing file:** `instances_physx.usda` returns 404 — cannot be downloaded.
2. **Physics properties:** No mass, inertia, collision, or joint data available.
3. **Mechanical behavior:** Cannot determine if rollers rotate, belt translates, or if articulation exists.
4. **Simulation readiness:** Cannot determine if the asset is suitable for physical simulation.

## 7. RECOMMENDED NEXT PHASE

**Option A (Required):** Download `instances_physx.usda` — but the file is not available at the expected location (404). This suggests either:
- The file was never uploaded to the dataset
- The file path is incorrect
- The file was removed after Phase D

**Option B:** Proceed with Phase F analysis using only the `enabled.usda` payload (19.8 KB) and document the gap.

**Recommendation:** Since the critical physics payload is unavailable, the physics verification cannot be completed. The asset cannot be confirmed as physically viable without the missing `instances_physx.usda` file.

## 8. GIT STATUS (POST-REPORT)

- **Before:** 84 modified + 33 untracked files (unchanged)
- **After:** No changes made (no download, no code modification)
- **Status:** No production files modified

## 9. CONCLUSION

**Phase F cannot be completed** due to missing `instances_physx.usda` file (HTTP 404). The physics verification is blocked. The asset's geometry hierarchy is known (70 instances), but all physics-related properties (mass, inertia, joints, collision) are unverifiable.

**Next step:** Request clarification on the file location or consider whether the file exists under a different path in the dataset.

---

**Report path:** `.planning/phases/02-physics-integration/06-F-CONVEYOR-PHYSX-PAYLOAD-VERIFICATION.md`
