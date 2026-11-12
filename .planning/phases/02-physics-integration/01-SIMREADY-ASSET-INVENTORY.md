---
reference: https://huggingface.co/datasets/nvidia/simready-assets
license_verified: CC-BY-4.0 (from dataset page)
primary_skill: szm-feature-reconstruction
specialist_skill: anatomy-mechanical-visualization
phase: A — Complete dataset inventory (read-only)
status: INVESTIGATION ONLY — no changes made to production code or git state
---

# PHASE A — NVIDIA SIMREADY ASSETS: INTEGRITY INVENTORY

## 1. VERIFICATION — REFERENCE

| Field | Value | Evidence Source |
|---|---|---|
| URL | https://huggingface.co/datasets/nvidia/simready-assets | Web fetch, page title |
| Dataset ID | nvidia/simready-assets | Page header |
| License declared | CC-BY-4.0 | Page license field + packaging.json |
| Total assets (declared) | 2,677 | Page title / dataset card |
| Total size (declared) | 490 GB | Page "Total file size" |
| Format | USD (OpenUSD) via `simready_usd/` subdir | Package tree inspection |
| Storage backend | xet (verified on page) | Page label |
| Status | Active; 4,201 commits; 3 contributors; updated ~14 hrs ago | Commit history |

## 2. ASSET PACKAGE STRUCTURE (VERIFIED — NO BULK DOWNLOAD)

Per package (`conveyor_belt_a47` inspected as representative):

```
<asset_package>/
  .metadata/
    LICENSE.txt                          (CC-BY-4.0 full text)
    com.nvidia.simready.conformance.Package@1.0.0.json
    com.nvidia.simready.conformance.Package-Candidate@1.0.0.json
    com.nvidia.simready.packaging.bom.json   (BOM manifest, 26.5 KB)
    com.nvidia.simready.root_usds.json       (USD root entry points)
  simready_usd/
    sm_<asset>_01.usd                     (primary USD file)
    .thumbs/                              (preview thumbnails)
  com.nvidia.simready.packaging.json      (package hash + license ref)
```

**Key finding:** Each asset carries a BOM (`bom.json`), conformance certificate, root USD manifest, and full license. The dataset is structured for programmatic ingestion, not just visual browsing.

## 3. CATEGORY BREAKDOWN (FROM FOLDER NAMES — VERIFIED VIA TREE VIEW)

Category groups extracted from `tree/main` listing (sampled, not exhaustive):

- **Medical/Healthcare:** CT_injection_system, IV_pole, Xray_machine, anesthesia_machine, bed_hospital, chair_patient_mobile, analyzer, surgical instruments
- **Industrial/Mechanical:** conveyor_belt, engineComponent_engine_oil_pan, airSystem_compressor, airSystem_air_filter, arm_articulated_support, barrier_fence/metal/gate, machinery
- **Facility/Equipment:** appliance_refrigerator/dishwasher/handDryer/toaster/extractor, furniture_workbench, rack_large_empty, workstation
- **Consumer/Household:** food_beverage_bottledWater, kitchenware_spoon, toy_marble, paint_container_spraycan, bedding
- **Electronics:** TV, interiorPart_sun_visor
- **Environmental/Barrier:** barrier_wall_plastic, blinds_window
- **General objects:** accessory_eyeglasses, bandage_medical, bin_waste, rack_large_empty

**Estimated engineering-relevant subset:** ~20-30% (mechanical, industrial, equipment, barrier, furniture, some medical equipment). The remainder is consumer/retail with lower engineering integration value.

## 4. TECHNICAL FORMAT ANALYSIS (VERIFIED)

- **USD entry:** `simready_usd/sm_<name>_01.usd`
- **Root manifest:** `.metadata/com.nvidia.simready.root_usds.json`
- **Physics/articulation:** Not verified per-asset without download; BOM references suggest structured metadata but exact physics properties (mass, inertia, friction, joints) require per-file parsing
- **Material:** USD materials embedded; not separately indexed
- **Collision:** Not independently verified; depends on USD physics prims
- **Scale/units:** USD default (meters / centimeters per prim) — must verify per-asset

**Verified absence:** No global metadata file listing all physics properties for all 2,677 assets. Per-asset inspection required.

## 5. REPRESENTATIVE ASSET INSPECTION (CONVEYOR_BELT_A47 — SELECTIVE)

| Property | Status | Evidence |
|---|---|---|
| Package ID | Verified | `com.nvidia.simready.packaging.json` |
| Size | 285 MB (verified) | Package tree |
| License | CC-BY-4.0 (verified) | `.metadata/LICENSE.txt` + packaging |
| USD file present | Yes | `simready_usd/` |
| BOM present | Yes | `bom.json` 26.5 KB |
| Root manifest | Yes | `root_usds.json` |
| Mass / density | **Unknown** — requires USD parsing | Not in packaging JSON |
| Inertia tensor | **Unknown** | Not in packaging JSON |
| Collision geometry | **Unknown** | Not in packaging JSON |
| Joints / articulation | **Inferred possible** — conveyor implies motion | Not verified in packaging |
| Material properties | **Unknown** — embedded in USD | Not separately indexed |
| Simulation parameters | **Unknown** | Not in metadata |

**Critical distinction:** The dataset is \"simulation-ready\" in the NVIDIA / SimReady Central sense — it provides USD assets intended for physics simulation environments. It does NOT guarantee that every asset has complete physics metadata, joint definitions, or validated simulation parameters.

## 6. SZM FORGE CURRENT STATE (VERIFIED — PRESERVED)

- Project: `/mnt/D/projects/szm_forge`
- Branch: `main`
- Modified files preserved: 64 unstaged, 33 untracked (none deleted)
- Existing mechanical module: `src/Assembly/KinematicSolver.cpp`, `Mates.hpp`, `KinematicTypes.hpp`
- Existing test suite: `tests/test_kinematic_solver_phase02.cpp`, `tests/main_test.cpp` (T15)
- Simulation engine: `src/Simulation/SimulationEngine.cpp`
- Telemetry contract: `.data/sim_telemetry_contract.md` (simulated only)
- Space: 7.1 GB available (97% used — tight)
- Skills loaded: `szm-feature-reconstruction`, `anatomy-mechanical-visualization`

## 7. INTEGRATION OPPORTUNITIES (VERIFIED VS INFERRED)

| SZM Module | Integration Potential | Evidence Level |
|---|---|---|
| `KinematicSolver` + `IMate3D` | Mechanical assemblies (conveyor, articulated arm, workbench) could become assembly nodes with mates | Verified architecture exists; assets unverified |
| `AssemblyGraph` + `BasePart` | Each asset could be registered as `BasePart` with BOM/quantity | Architecture exists (T10 verified) |
| `SimulationEngine` | Physical properties (mass, stress, temp) could be applied to components | Engine exists; asset properties unknown |
| `Mates.hpp` | Articulated assets (arm, conveyor) could use `CoincidentPlanarMate` / `ConcentricMate` | Architecture verified |
| `MeshGenerator` + `SceneGraph` | USD geometry → mesh for visualization | Existing pipeline |
| `MaterialDatabase` | USD materials → `MAT-*` IDs | Database exists (T08 verified) |
| `.data/sim_telemetry_contract.md` | All integrated telemetry must be labeled SIMULATED | Verified (existing) |

## 8. MISSING / UNVERIFIED (REQUIRED TO DISTINGUISH)

- Per-asset physics property completeness (mass, inertia, friction, restitution, joints)
- Per-asset USD prim hierarchy (articulation, joints, constraints)
- Whether GLB conversion preserves physics (likely not — USD physics prims lost)
- Which assets have validated simulation evidence (vs. geometry-only)
- Full list of all 2,677 package names (sampled only)
- Whether dataset includes electrical components with circuit interfaces (some likely — medical equipment, appliances)

## 9. LICENSING AND PROVENANCE STRATEGY (VERIFIED)

- Dataset: CC-BY-4.0
- Requirement: Preserve attribution per asset; do not insert NVIDIA branding into SZM Forge identity
- Strategy: Each ingested component carries `source: nvidia/simready-assets`, `package_id`, `license: CC-BY-4.0`, `modifications` record; component library is independent SZM Forge identity
- Storage: Original assets reference via `simready_usd/` path; no bulk copy needed if using reference + metadata indexing

## 10. RISK ASSESSMENT

| Risk | Severity | Mitigation |
|---|---|---|
| 490 GB bulk download impossible (7.1 GB free) | Critical | Selective inspection only; reference-based ingestion with metadata index |
| Per-asset physics unknown | High | Verify per-asset USD before claiming simulation capability; label unverified clearly |
| CC-BY-4.0 attribution required | Medium | Preserve `package_id`, `license`, `source` in every component metadata |
| USD → SZM conversion may lose physics | High | Investigate USD prim parsing; do not claim physics integration until verified |
| Existing 64 modified files preserved | Low | No destructive git operations; test on separate branch when approved |

## 11. PROPOSED ARCHITECTURE (NOT IMPLEMENTED — AWAITING APPROVAL)

```
Dataset Discovery (Hugging Face API / web)
  │
  ▼
Package Inspection (metadata, BOM, packaging.json, root_usds.json)
  │
  ▼
Selective USD Parsing (simready_usd/*.usd — per package, never bulk)
  │
  ▼
Functional Classification (mechanical / electrical / facility / consumer)
  │
  ▼
SZM Component Representation (BasePart / RigidBodyState / MaterialDB entry)
  │
  ▼
Application Integration (AssemblyGraph + KinematicSolver + SimulationEngine)
  │
  ▼
Validation (geometry check + physics property verification + telemetry contract label)
```

## 12. REPORTED FINDINGS SUMMARY

- **Verified:** Dataset exists (2,677 assets, CC-BY-4.0, 490 GB, USD format, structured packaging with BOM/conformance/license)
- **Verified:** Primary skill `szm-feature-reconstruction` loaded; specialist `anatomy-mechanical-visualization` loaded
- **Verified:** SZM Forge existing modules (KinematicSolver, SimulationEngine, AssemblyGraph, MaterialDB) are available for integration
- **Verified:** Disk space (7.1 GB) prevents bulk download — selective inspection only
- **Inferred:** ~20-30% of assets are engineering-relevant
- **Unknown:** Per-asset physics properties, joint definitions, validated simulation evidence, electrical interfaces
- **Not done:** No bulk download; no USD parsing; no production code modified; no test created; no static info-only feature created

## 13. NEXT PHASE REQUIRED

STOP — awaiting your approval.

Recommend Phase B (deep technical investigation of representative assets across categories) before Phase C/D.

Options:
- **B1:** Inspect 3-5 representative packages (mechanical, medical equipment, electrical, industrial) via selective USD file fetch
- **B2:** Proceed to Phase C gap analysis using verified inventory only (no more asset downloads)
- **B3:** Approve ingestion pipeline architecture design (Phase D) based on current verified data

No files modified. No assets downloaded. All existing project work preserved.
