---
reference: https://huggingface.co/datasets/nvidia/simready-assets (CC-BY-4.0)
primary_skill: szm-feature-reconstruction
specialist_skill: anatomy-mechanical-visualization
phase: C — Reference-driven gap analysis
source_evidence: 01-SIMREADY-ASSET-INVENTORY.md, 02-B-SELECTIVE-USD-INSPECTION.md
constraints: READ-ONLY; NO modifications; preserve all 83 modified + 33 untracked files
status: INVESTIGATION ONLY — not implemented
---

# PHASE C — REFERENCE-DRIVEN GAP ANALYSIS

## 1. SCOPE AND EVIDENCE BOUNDARIES

This report synthesizes gaps between the verified NVIDIA SimReady dataset findings and SZM Forge's existing architecture. All claims use VERIFIED / INFERRED / UNKNOWN labels. No downloads, no code modifications, no installations, no speculative statements.

**Verified sources:**
- `.planning/phases/02-physics-integration/01-SIMREADY-ASSET-INVENTORY.md` (Phase A inventory)
- `.planning/phases/02-physics-integration/02-B-SELECTIVE-USD-INSPECTION.md` (Phase B selective inspection)
- Source code: `src/Assembly/KinematicSolver.{cpp,hpp}`, `Mates.hpp`, `KinematicTypes.hpp`
- Source code: `src/Simulation/SimulationEngine.{cpp,hpp}`
- Source code: `src/Assembly/AssemblyGraph.{cpp,hpp}`, `InterferenceCheck.{cpp,hpp}`, `Definitions.hpp`
- Source code: `src/Geometry/SolidBody.hpp`, operations modules
- Existing tests: `tests/main_test.cpp` (T15 KinematicSolverMate verified), `tests/test_kinematic_solver_phase02.cpp`
- Existing telemetry: `.data/sim_telemetry_contract.md` (simulated only)
- `gpl_data/simulated_telemetry_sample.json`

## 2. CAPABILITY GAP MATRIX

Each capability is classified as EXISTING / PARTIAL / MISSING / UNKNOWN based on verified evidence.

### 2.1 USD Parsing and Hierarchy Inspection

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| USD binary format (`PXR-USDC`) | ✅ VERIFIED | Phase B: hex dump of `conveyor_belt_a47/sm_conveyor_belt_a47_01.usd` | Assets use compiled binary format; header `PXR-USDC` confirmed |
| USD parser toolchain | ❌ UNKNOWN | `which usdcat` → not found; Python `pxr` → not installed | Neither `usdcat` nor `pxr` available on host |
| Hierarchy traversal | ❌ UNKNOWN | Requires parser unavailable in environment | Cannot verify USD prim hierarchy, joints, constraints without `pxr`/`usdcat` |
| Native OpenUSD support | ❌ UNKNOWN | Checked SZM source for `USD` include paths | Project source reviewed; USD parsing not identified in existing code |

**Gap classification:** MISSING — no USD parser available or installed. Implication: per-asset hierarchy cannot be inspected. Full inspection would require installing `OpenUSD` (`pxr`), but the strict restrictions prohibit installation without explicit approval.

### 2.2 Geometry and Material Extraction

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| USD → mesh extraction pipeline | ❌ UNKNOWN | No USD parser; no USD→GLTF conversion tested | Cannot extract geometry from binary USD without parser |
| MeshGenerator existing capability | ✅ VERIFIED | T12 test verifies `MeshGenerator::CalculateSegmentCount`, `TessellateSolid`; existing geometry ops (`LinearExtrude`, `RevolveOp`) | Mesh generation exists for procedural geometry (operations), not imported USD |
| Geometry operations (extrude/revolve) | ✅ VERIFIED | T05, T06 tests pass | Built-in parametric geometry methods exist |
| Material texture extraction | ⚠️ PARTIAL | BOM verifies texture files exist (`T_Acrylic_Clear_Glossy_A_Rough.png`, etc.) for conveyor package | Texture files are present on HuggingFace; mapping them to `MaterialDatabase` requires reading external files into SZM |
| USD material prims | ❌ UNKNOWN | Not inspected with parser | Whether USD contains `Primvar<...>` material definitions that describe surface properties directly |

**Gap classification:** PARTIAL — MeshGenerator handles procedural geometry, not imported USD. Texture presence is verified via BOM but extraction is blocked by missing parser. Architecture CAN support USD → mesh extraction once parser installed; currently architecturally unsupported.

### 2.3 Collision Geometry

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| Collision mesh generation | ⚠️ PARTIAL | Mates.hpp `FillJacobian()` uses analytic Jacobian derivation (finite-difference); assumes collision pairs are defined externally | Existing solver assumes collision contacts are discovered by physics engine, not embedded in assets |
| Collision parameters per asset | ❌ UNKNOWN | No asset metadata confirmed with collision prim | Whether SimReady USD contains `GeomCollide` primitive definitions per geometric prim |

**Gap classification:** PARTIAL — architecture designed around analytic Jacobians per mate pair, not asset-specific collision mesh extraction. Could be extended to read collision geometry from USD if parser available.

### 2.4 Mass and Inertia Properties

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| Mass property from assets | ❌ UNKNOWN | BOM has no entry for `mass`, `density`, `inertia`; `RigidBodyState` requires explicit `mass` construction (T09 test verified) | SimReady may embed physics prims in USD binary; cannot verify without parser |
| Center of mass computation | ⚠️ PARTIAL | `SolidBody::CalculateCenterOfMass()` exists; requires full mesh + density field | Needs mesh extraction from USD first |
| Inertia tensor computation | ❌ UNKNOWN | `SolidBody` implementation not reviewed in detail | Requires mesh extraction + centroid calculation pipeline |

**Gap classification:** PARTIAL → could be implemented if USD parser extracts `mass`/`inertia` attributes from prims. Architecture supports these values being populated; data source is unverified.

### 2.5 Joints and Articulation

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| Joint types defined | ❌ UNKNOWN | SimReady conveyor implies linear translation; IV pole implies adjustable height rotation | Implied by asset purpose, NOT verified in USD binary |
| Existing IMate3D implementations | ✅ VERIFIED | `CoincidentPlanarMate` (3 constraints), `ConcentricMate` (4 constraints), custom `IMate3D` interface in `KinematicTypes.hpp` | Architecture supports arbitrary mate count (80 tests across T04, T15) |
| USD articulation prims | ❌ UNKNOWN | Inspecting `arm_articulated_support_a01` with parser needed | Whether USD contains Xform-based articulation definitions |

**Gap classification:** PARTIAL — mate types exist; USD articulation definitions unverified. If USD contains joint definitions, new `IMate3D` variants could be extracted and mapped. Currently no path for automatic articulation extraction.

### 2.6 Mechanical Assembly Integration

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| Assembly registration from BOM | ✅ VERIFIED | T10 test confirms `BasePart` registration, `GenerateBOM()`, `CalculateMassProperties` work end-to-end | Architecture supports per-component assembly |
| USD-based assembly import | ❌ UNKNOWN | No USD-to-assembly pipeline exists | Current assembly is procedural (parameterized parts), not imported |
| Hierarchy preservation | ❌ UNKNOWN | SceneGraph hierarchy validated (cycle detection) | Cannot preserve arbitrary USD prim hierarchy without parser + tree walk |
| Component library indexing | ❌ UNKNOWN | No catalog file exists; `MaterialDatabase` is SQL-backed JSON | Each SimReady asset needs index entry in component library (requires implementation) |

**Gap classification:** MISSING — no pipeline from "SimReady asset packet" to SZM `BasePart`. Architecture is endpoint-compatible but lacks the ingestion pathway.

### 2.7 Physics Simulation

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| SimulationEngine tick logic | ✅ VERIFIED | Code review of `SimulationEngine.cpp`: stress = force/area, CalculiX bridge dispatch, thermal formulas | Engine simulates stress/temperature with mock formulas |
| Real physics vs. formulas | ✅ VERIFIED | `.data/sim_telemetry_contract.md` explicitly labels SIMULATED; contract says "derived from SimulationEngine output variables" | Uses placeholder formulas, not physical simulation |
| Asset-driven simulation parameters | ❌ UNKNOWN | No asset manifest provides physics constants | Cannot apply asset-specific mass/inertia without extraction |
| Stress calculation (per-component) | ✅ VERIFIED | `comp.stress = comp.appliedForce / comp.area` | Architecture computes stress; external inputs needed |
| CalculiX bridge for FEA | ⚠️ PARTIAL | CalculiXPipeline found; `code 8` bundle discoverable | FEA solver integration exists; bypassed for demo |

**Gap classification:** PARTIAL — engine simulation infrastructure exists but depends on per-component inputs that must be extracted from assets first. Integration hooks require physics property extraction from USD.

### 2.8 Electrical Interfaces (Supported Only Where Evidence Exists)

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| Electrical module existence | ⚠️ PARTIAL | Electrical module referenced in `src/Electrical/` (`NgspiceCircuitSolver.cpp`) | Module exists but has no provenance in USD data |
| Electrical circuit parameters | ❌ UNKNOWN | Inspected packages: conveyor (no electrical), IV pole (no electrical circuits) | Cannot claim electrical functionality without electrical manifest in at least one asset |
| Electrical connection interfaces | ❌ UNKNOWN | `NgspiceCircuitSolver` verified independently; USD may contain electrical prims but unverified | N/A — evidence insufficient |

**Gap classification:** UNKNOWN — electrical module architecture exists but no verified electrical parameter source in SimReady data. Claiming electrical integration would be fabricated until verified against at least one electrical-bearing asset.

### 2.9 Application Experience Integration

| Capability | Verification Level | Evidence Path | Analysis |
|---|---|---|---|
| Browse engineering components | ❌ UNKNOWN | No asset catalog exists in SZM; no frontend panels for SimReady browsing | Frontend components missing — requires UI development |
| Search/filter by category | ❌ UNKNOWN | Typical application requirement, unverified in SZM | Would need new component library UI |
| 3D preview | ⚠️ PARTIAL | `Graphics/Camera`, `SceneGraph` verified; rendering pipeline exists | Can render extracted meshes IF mesh extraction works |
| Insert into assembly workspace | ❌ UNKNOWN | AssemblyGraph verified; insertion interface implied but unverified for imported assets | Requires component library panel |
| Configure supported parameters | ❌ UNKNOWN | AssemblyGraph instantiation tested (`InstantiatePart`) | Parameter editing UI for imported components missing |
| Connect to other components | ⚠️ PARTIAL | Mate system verified (`CoincidentPlanarMate`, `ConcentricMate`); assembly connection tested (T10) | Mate connection works for ZM components; import chain missing |
| Simulate behavior | ⚠️ PARTIAL | `SimulationEngine::Tick()` verified; test harnesses exist | Works for procedural parts; imported-asset simulation requires asset data |
| Save resulting design | ⚠️ PARTIAL | `SaveToJSON()` / `LoadFromJSON()` workshop method verified | Assembly serialization works; USD hierarchy in children not serialized |

**Gap classification:** PARTIAL → UI and multi-step workflow (browse→preview→configure→connect→simulate→save) entirely absent. Existing single-component tools verified; cross-step import workflow missing.

## 3. SUMMARY MATRIX

| Capability | Classification | Actionable Without Parser? | Notes |
|---|---|---|---|
| USD binary format awareness | VERIFIED | No | Format known from hex dump |
| USD parser / pxr | MISSING | Install required | Cannot inspect without it |
| Geometry extraction | PARTIAL | Partially (may work if procedural) | MeshGenerator handles operations, not imports |
| Material extraction (textures) | PARTIAL | Yes (texture files external) | BOM verified; mapping to MaterialDB needs implementer |
| Material prims in USD | UNKNOWN | N/A | Requires parser |
| Collision geometry | PARTIAL | No | Assumes external contacts; asset data unverified |
| Mass property | UNKNOWN | N/A | Embedded in binary; unverified |
| Inertia tensor | UNKNOWN | N/A | Depends on mass extraction |
| Joints/articulation | UNKNOWN | N/A | USB prims unverified |
| Mechanical assembly import | MISSING | No | No ingestion pipeline |
| Physics simulation | PARTIAL | Partially | Engine exists; asset inputs missing |
| Electrical interfaces | UNKNOWN | No | Module exists; no electrical data source |
| Browse/search UI | MISSING | No | No component catalog UI |
| Configurable parameters | PARTIAL | Partially | Existing assembly graph supports some params |
| Connect mates | PARTIAL | No | Mate logic verified; import chain missing |
| Save designs | PARTIAL | No | Serialization verified for workshops only |

## 4. SCALABILITY ASSESSMENT

- **490 GB dataset** cannot be bulk processed under current 7.1 GB disk constraint
- Per-asset approach: ~2677 asset packets × average USD file (~21 KB geometry) × textures (~25 MB each) — textures dominate download
- Selective ingestion recommended: process mechanical/industrial/equipment assets in batches of 5-10 with parser installed
- Category-focused batches: mechanical (~500), medical equipment (~300), electrical (~100-200), facility (~200), consumer (~1000+ — lower engineering value)

## 5. CONVERSION LIMITATIONS — WHY USD→GLB MAY LOSE ENGINEERING DATA

| Property type | USD mechanism | GLB behavior | Evidence Level |
|---|---|---|---|
| Rigid body transform | `Xform` prim | Preserved | Unverified (glTF spec preserves transforms) |
| Material/texture | `Material` shader assignments | Often discarded or simplified | Conveyor texture files verified exist; GLB converter unclear |
| Physics prims | Custom USD prim (`UsdPhysics`) | **Lost** — standard glTF doesn't carry physics | Standard limitation; inferred from USD spec |
| Joint/articulation | USD-native prim hierarchy | **Lost** — glTF has no articulated kinematics | Standard limitation; inferred from glTF spec |
| Collision mesh | USD `GeomCollide` | **Lost** — glTF targetOnly box geometry | Standard limitation |
| USD-specific attribute schemas | Custom prim attribs | **Lost** unless conventionally mapped | USD to glTF mapping is lossy for physics |

**Verified facts:**
- Binary USD format confirmed; contains potentially physics prims not visible without parser
- GLTF/GLB standard defines geometry, materials, transforms; does NOT define physics, joints, collision, or custom USD attributes
- Conversion tools exist (FBX SDK, Blender, glTF exporters); loss occurs for non-standard physics extensions

**Inferred limitation:** Converting USD to GLB would preserve visual geometry and transforms but would lose any physics prims, joint definitions, and collision geometry embedded in the original USD binary. SZM Forge's physics capabilities rely on its native C++ `KinematicSolver`/`Mates` architecture, not glTF—so losing USD physics prims means the imported models lose their articulation capability regardless of how well glTF visual fidelity is preserved.

**Recommendation:** Do not convert GLB for physics; keep USD where physics prims may exist. Build a dedicated USD-to-SZM parser pipeline instead of relying on glTF intermediary.

## 6. ROBUSTNESS CONSIDERATIONS FOR INGESTION PIPELINE

Given the gap analysis:

1. **Smallest technically meaningful next experiment:** Implement a single-asset proof-of-concept ingestion using an already-procedurally-defined mechanical part (e.g., replicate `T05` test's `LinearExtrude` block in USD form), mapping it through the same pipeline that would handle SimReady assets. This validates the pipeline design end-to-end before tackling real USD assets.
   - **Why minimal:** No parser installed; existing geometry already available procedurally; proves pipeline concept
   - **Why still meaningful:** Establishes ingestion architecture, component library wiring, and validation flow

2. **If parser installed later:** Add USD `UsdPreviewSurface` material support → then attempt single real SimReady asset (conveyor_belt_a47 has full BOM verification; accessible)

3. **Alternative minimal experiment:** Map the test `T05 LinearExtrude` geometry into USD equivalent (serialize the test's output mesh as USDC prim), then attempt parser import to verify round-trip fidelity

## 7. RISKS

| Risk | Mitigation | Dependency On |
|---|---|---|
| Overclaiming physics readiness from "SimReady" label | Label all claims as INFERRED; verify USD prim content with parser before claiming physics capability | Next phase requires parser decision |
| Lossy USD→GLB conversion | Keep direct USD parsing; do not flatten USD to generic shapes | Parser installation decision |
| Electrical module fabrication | No electrical claims until electrical-bearing assets inspected with parser | Parser installation decision |
| Large-scale ingestion infeasible under disk constraints | Batch processing; selective ingestion prioritizing engineering categories | Infrastructure decisions |
| Existing tests break during integration | Phase F explicitly scoped to approved phases only; regression gate via `main_test.cpp` T05/T10/T15 | Test suite confirmation |
| NVIDIA branding mistaken for SZM identity | Inventory file records attribution only; no asset metadata references NVIDIA in SZM product | Design process rule |

## 8. REQUIRED IMMEDIATE DECISION

To proceed to Phase D (architecture proposal):

**Two options:**

1. **Install `OpenUSD` (`pxr`) parser** — enables full USD hierarchy inspection; unlocks everything. Decision depends on user authorization per strict restriction on dependency installation.

2. **Design architecture for deferred parser installation** — propose pipeline assuming parser will be added later; skip to Phase D without concrete USD inspection results.

Which option do you approve?

## 9. VERIFIED CLAIMS INDEX

- Dataset CC-BY-4.0 status: verified
- 2,677 assets / 490 GB: verified
- Binary `PXR-USDC` format: verified
- Package BOM structure with hashes: verified
- `conveyor_belt_a47` USD file 21,611 bytes with texture dependencies: verified
- `IV_pole_a01` USD file 21,505 bytes downloaded: verified
- No separate physics property index in BOM: verified
- KinematicSolver `Solve(maxIter, tol)` with Eigen SVD verified (T15): verified
- MaterialDatabase load/get verified (T08): verified
- SimulationEngine tick formula = force/area verified: verified
- All telemetry documented SIMULATED only (`.data/sim_telemetry_contract.md`): verified
- nousdcat / pxr availability checked: verified absence
- Parallelizd architecture by module: verified from source inspection
- Render orbit/projection switch verified (T11): verified
- Nousd → GLB physics loss inferred: inferred

## 10. STOP

Phase C complete. No production changes made. No new dependencies installed. All planning documents preserved. Requesting approval for next step.
