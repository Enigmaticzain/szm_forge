---
reference_dataset: nvidia/simready-assets (CC-BY-4.0)
primary_skill: szm-feature-reconstruction
specialist_skill: anatomy-mechanical-visualization
inspection_type: SELECTIVE USD INSPECTION (B1 — approved)
packages_inspected: 2 (max 5, combined USD files < 50 KB; package metadata + BOM < 30 KB; total < 100 KB — well under 1 GB limit)
downloads_performed: 2 selective USD files + 2 BOM/metadata files
production_changes: NONE
bulk_download: NONE
new_dependencies: NONE installed
---

# PHASE B — SELECTIVE USD INSPECTION REPORT

## 1. SELECTED ASSETS (VERIFIED IDs + SIZES)

| Asset ID | Category | USD Size | Package Size (declared) | Evidence Source |
|---|---|---|---|---|
| `conveyor_belt_a47` | Mechanical / Industrial | 21,611 bytes (21 KB) — verified via BOM hash `sha256: 2d7b3bd...` | 285 MB (verified from tree page) | BOM downloaded; USD file downloaded |
| `IV_pole_a01` | Medical Equipment | 21,505 bytes (22 KB) — verified via download | Not directly verified; estimated < 200 MB based on similar package structure | USD file downloaded (22 KB); BOM not downloaded for this asset |

**Note:** Both USD entry files are small binary references (`PXR-USDC` header verified). The 285 MB package size for conveyor is dominated by texture files (`.thumbs/` PNGs + material textures like `T_ConveyorBelt_A01_Decal_ORM.png` — 2.5 MB) rather than geometry data embedded in the USD file.

## 2. USD FORMAT ANALYSIS (VERIFIED)

- **Header verified:** `PXR-USDC` (compiled USD binary format, not ASCII `.usd` or `.usda`)
- **Binary signature confirmed:** `5058 522d 5553 4443` = `PXR-USDC` in hex dump
- **File size discrepancy:** USD entry point (~21 KB) does NOT contain full geometry; references external textures/dependencies
- **Implication:** Full geometry/material inspection requires either:
  a) USD parser (`usdcat`, `pxr`, `OpenUSD`) to traverse hierarchy
  b) Access to referenced texture/material files within package
  c) Conversion pipeline (USD → GLTF/GLB) which MAY lose physics prims

**Status:** Binary format verified. Full hierarchy parsing requires USD library (`pxr` unavailable on this host; installation blocked by dependency rule).

## 3. BOM FINDINGS (VERIFIED FROM `.metadata/com.nvidia.simready.packaging.bom.json`)

Conveyor package BOM verified contents:

- `simready_usd/sm_conveyor_belt_a47_01.usd` (21,611 bytes)
- `simready_usd/.thumbs/*.png` (thumbnails: 634 KB, 1.1 MB, 793 KB, 2.5 MB)
- Material texture files (`Textures/`): 7.5 MB (`T_Acrylic_Clear_Glossy_A_Rough.png`), 3.4 MB (`T_Aluminium_Brushed_A1_Albedo.png`), 2.3 MB (`Normal`), 7.5 MB (`ORM`), 2.0 MB (`A1_Normal`), 2.1 MB (`Trim_Normal`), 2.5 MB (`Trim_ORM`), 1.1 MB (`Decal_Albedo`), 1.3 MB (`Decal_ORM`)
- **No explicit physics property files** (mass, inertia, collision) found in BOM
- **No joint/articulation manifest** separate from USD file
- **No electrical interface metadata** (not expected for conveyor)

**Key finding (VERIFIED):** BOM confirms this is a visualization + texture package with a small USD reference file. Simulation properties (mass, friction, joints) must be embedded in the USD binary or inferred from geometry — not separately documented.

## 4. PHYSICAL PROPERTIES (DISTINGUISHED)

| Property | Conveyor (`a47`) | IV Pole (`a01`) | Evidence Level |
|---|---|---|---|
| Geometry (mesh) | **Unknown** — embedded in USD binary; not parsed | **Unknown** — embedded in USD binary | Binary verified; content unparsed |
| Scale / units | **Unknown** — requires USD prim parsing | **Unknown** | Unverified |
| Coordinate system | **Unknown** — requires USD inspection | **Unknown** | Unverified |
| Mass / density | **Unknown** — no separate BOM entry for physics | **Unknown** — no separate BOM entry | Unverified |
| Center of mass | **Unknown** — not in BOM | **Unknown** | Unverified |
| Inertia tensor | **Unknown** — not in BOM | **Unknown** | Unverified |
| Collision geometry | **Unknown** — not separately indexed | **Unknown** | Unverified |
| Rigid body definition | **Unknown** — may be USD physics prim; requires parser | **Unknown** | Unverified |
| Joints / articulation | **Inferred possible** (conveyor implies motion mechanism) — NOT verified in USD | **Inferred possible** (IV pole may have adjustable joints) — NOT verified | Inferred only |
| Materials (visual) | Verified — texture files present (`Acrylic_Clear`, `Aluminium_Brushed`, `ConveyorBelt_Decal`) | Unknown — textures not inspected | Verified for conveyor |
| Physics materials (friction, restitution) | **Unknown** — no separate physics manifest | **Unknown** | Unverified |
| Connection / mounting points | **Unknown** — requires USD hierarchy inspection | **Unknown** | Unverified |
| Simulation parameters | **Unknown** — no parameter manifest | **Unknown** | Unverified |

**Critical distinction:** The dataset is labeled "SimReady" (NVIDIA's term for simulation-ready assets). This means assets are intended for physics simulation environments, but it does NOT guarantee that every property listed above exists, is validated, or is directly usable by SZM Forge's `KinematicSolver`. Actual engineering usability requires per-file USD parsing.

## 5. ENGINEERING USABILITY ASSESSMENT

### Conveyor Belt (`a47`)
- **Can be inserted into assembly:** Yes (geometry exists; assembly requires USD parsing + mesh generation)
- **Can geometry be edited/inspected:** Partially — requires USD parser or conversion to `MeshGenerator`
- **Can participate in mechanical simulation:** **Inferred only** — USD binary may contain physics prims; requires `pxr`/parser verification
- **Physical properties usable directly:** **Unknown** — not documented in BOM; must be extracted from USD binary
- **Usable connection/articulation info:** **Unknown** — conveyor implies linear motion; no joint definitions verified
- **Additional engineering models needed:** Yes — mass/inertia parameters, joint definitions, motion constraints, connection points

### IV Pole (`a01`)
- **Can be inserted into assembly:** Yes (geometry exists; small USD reference file)
- **Can participate in mechanical simulation:** **Inferred only** — medical equipment; articulation possible but unverified
- **Electrical functionality:** Not applicable; no electrical interfaces expected or verified
- **Physical properties:** Same as conveyor — unknown without USD parsing
- **Connection points:** Unknown (mounting base, adjustable height mechanism unverified)

## 6. SZM FORGE INTEGRATION MAPPING (VERIFIED ARCHITECTURE VS REQUIRED EXTENSIONS)

| SZM Module | Verified Capability | Integration Requirement | Status |
|---|---|---|---|
| `KinematicSolver` | Exists (`Solve()` with Eigen SVD, 6-DOF bodies) | Asset needs `IMate3D` mapping; USD joint definitions must be mapped to `CoincidentPlanarMate` / `ConcentricMate` / new mate types | Requires USD parsing; design possible |
| `Mates.hpp` | `CoincidentPlanarMate` (3 constraints), `ConcentricMate` (4 constraints) verified | Conveyor motion = linear translation (potential `IMate3D` extension); IV pole = adjustable height (potential rotation constraint) | Design possible; unverified in USD |
| `AssemblyGraph` | BOM generation verified (`GenerateBOM()` from `BasePart`) | Asset can be registered as `BasePart` with `PartName` = asset ID, `Mass` = unknown (needs extraction) | Architecture supports; data missing |
| `SimulationEngine` | Tick logic exists (`Tick()`, stress/temp formulas documented) | Physical properties must be extracted from USD and applied; `.data/sim_telemetry_contract.md` requires SIMULATED label | Architecture exists; properties unverified |
| `MaterialDatabase` | `LoadStandardLibraries()` verified; `GetMaterial()` verified | USD materials (`Acrylic_Clear`, `Aluminium_Brushed`) could be mapped to `MAT-*` IDs; requires property extraction | Design possible; mapping unverified |
| `SceneGraph` | Hierarchy + cycle detection verified (`AddChild()`, serialization) | USD hierarchy → `SceneGraph` nodes; requires hierarchy parser | Design possible |
| `MeshGenerator` | Tessellation config verified; segment count formula verified | USD meshes → `Profile2D` / `MeshGenerator` output; requires mesh extraction | Design possible |
| `MeshGenerator` + `Camera` | Orbit, projection switch, view matrix verified | USD assets can be rendered with existing camera; requires mesh import pipeline | Architecture exists |

**Integration conclusion (VERIFIED + INFERRED):** SZM Forge's architecture CAN support these assets, but the pipeline requires:
1. USD parser (`usdcat` or `pxr` / Python `usd-core`)
2. Mesh extraction from USD binary
3. Material property extraction and mapping to `MaterialDatabase`
4. Physics property extraction (mass, inertia) for `SimulationEngine`
5. Joint/article mapping for `IMate3D` if USD contains articulation prims

## 7. MISSING CAPABILITIES (VERIFIED GAPS — NOT ASSUMED)

Based on verified dataset inspection (not inference from "SimReady" label alone):

1. **No separate physics manifest per asset** — physics properties must be embedded in USD binary or absent
2. **No joint/articulation metadata** — `IMate3D` mapping requires USD inspection; unverified
3. **No electrical interface documentation** — no circuit parameters, voltage/current ratings, connection types
4. **No validated simulation evidence** — BOM confirms package structure, not physics validation results
5. **No per-asset mass/inertia/collision index** — must be extracted from binary
6. **No GLB conversion pipeline verified** — conversion to GLB would likely lose USD physics prims; requires testing
7. **No component library indexing** — assets are packets, not catalog entries in `SZM Forge`

## 8. CONVERSION LIMITATIONS (VERIFIED / INFERRED / UNKNOWN)

| Conversion Path | Verified | Limitation | Evidence |
|---|---|---|---|
| USD (binary USDC) → mesh | **Unknown** — requires parser | Binary format requires `usdcat`/`pxr`; not installed | Binary header verified; parser unavailable |
| USD → GLTF/GLB | **Not attempted** — dependency uninstalled | Likely loses physics prims, articulation, material parameters | Inferred from USD design docs |
| USD textures → `MeshGenerator` materials | **Unknown** — requires extraction pipeline | Texture files present (verified); mapping to `MaterialDatabase` requires extraction | BOM verified textures |
| USD hierarchy → `SceneGraph` | **Unknown** — requires parser | USD prim hierarchy must be traversed; `SceneGraph` supports hierarchy | Architecture verified |

## 9. LICENSING AND ATTRIBUTION (VERIFIED)

- **Dataset license:** CC-BY-4.0 (verified on dataset page + `packaging.json` + `.metadata/LICENSE.txt`)
- **Per-package attribution:** `package_id`: `simready.hf.nvidia.simready-assets.<asset_id>.<date>`
- **Required preservation:** Original `package_id`, license reference, source (`https://huggingface.co/datasets/nvidia/simready-assets`), modifications (none yet — no production changes)
- **NVIDIA branding:** Must NOT be inserted into SZM Forge product identity; assets must be presented as independent engineering components
- **Storage strategy:** Reference original package IDs; no bulk copy needed for ingestion pipeline; selective download only for inspection

## 10. VALIDATION STATUS (VERIFIED / INFERRED / UNKNOWN — PER CLAIM)

- **VERIFIED:** Dataset exists (2,677 assets, 490 GB, CC-BY-4.0, USD binary format, structured BOM/conformance/license per package)
- **VERIFIED:** Two representative packages inspected (conveyor belt, IV pole) — both have small USD reference files (~21 KB) with large texture dependencies
- **VERIFIED:** Binary format is `PXR-USDC`; requires parser for full inspection
- **VERIFIED:** BOM contains file manifest with hashes; no separate physics property index
- **VERIFIED:** Material texture files present; physics/articulation properties unverified
- **INFERRED:** ~20-30% engineering-relevant assets; conveyor implies linear motion mechanism; IV pole implies adjustable height mechanism
- **UNKNOWN:** Per-asset physics properties (mass, inertia, friction, joints); full USD hierarchy; electrical interfaces; validated simulation evidence; GLB conversion behavior; mesh extraction results
- **NOT CLAIMED:** No integration implemented; no physics properties applied; no assembly created; no component library updated; no static info-only feature created; no NVIDIA branding inserted; no bulk dataset downloaded

## 11. NEXT PHASE RECOMMENDATION (NOT IMPLEMENTED — AWAITING APPROVAL)

Given verified limitations:

**Phase C (Gap Analysis) is ready:** We have verified dataset structure + architecture mapping + identified missing capabilities (USD parser, physics extraction, joint mapping, material mapping). No more asset downloads are required to proceed with gap analysis.

**Phase D (Architecture Proposal) requires:** Decision on whether to install USD parser (`usdcore` / `pxr`) or design conversion pipeline; decision on selective ingestion vs. reference-based ingestion.

**Before Phase F (Implementation):** Must verify at least one representative USD file's full hierarchy (requires parser or conversion test) to confirm physics/articulation claims.

**STOP — Phase B complete. Awaiting approval for Phase C/D.**

No production modifications made. No new dependencies installed. All existing files (64 unstaged, 33 untracked) preserved. Disk space unchanged (7.1 GB available). Inventory file preserved at `.planning/phases/02-physics-integration/01-SIMREADY-ASSET-INVENTORY.md`. USD inspection results preserved at `/tmp/szm_simready_inspection/` (temporary, not in production repo).
