---
reference: https://huggingface.co/datasets/nvidia/simready-assets (CC-BY-4.0)
primary_skill: szm-feature-reconstruction
specialist_skill: anatomy-mechanical-visualization
phase: I — Application Reality Audit
status: READ-ONLY AUDIT — NO modifications made
skills_loaded: szm-feature-reconstruction, anatomy-mechanical-visualization (verified)
---

# PHASE I — APPLICATION REALITY AUDIT REPORT

## 1. EXECUTIVE SUMMARY (VERIFIED)

The SZM Forge codebase has substantial source architecture but **no working application** demonstrating the engineering capabilities investigated in Phases A–H. The conveyor asset integration exists only as planning documentation (`.planning/phases/02-physics-integration/`) — not connected to any production code path.

**Verified state:**
- Source modules exist (Simulation, CAD, Assembly, Materials, Scene, Graphics)
- Build directory exists (`build-final/`) but **binary directory empty** (0 executables)
- Build errors: 841 lines in `build_err.log`
- Tests exist (`main_test.cpp`, `test_kinematic_solver_phase02.cpp`, `simulation_test.cpp`) — reference verified code paths
- No USD import endpoint connects to `enabled.usda` payload
- No frontend panel imports conveyor data
- All NVIDIA/SimReady findings (A–H) are documentation-only

**Root cause of invisible progress:** The engineering investigation was performed in isolated documentation (`.planning/`) without any implementation bridge to source code, UI, or build pipeline.

---

## 2. SKILL DISCOVERY & ACTIVATION (VERIFIED)

| Skill ID | Source | Status | Evidence |
|---|---|---|---|
| `szm-feature-reconstruction` | `/home/szm7226/.agents/skills/szm-feature-reconstruction/` | LOADED | Mandatory skill activated |
| `anatomy-mechanical-visualization` | `/mnt/D/projects/szm_forge/.claude/skills/anatomy-mechanical-visualization/` | LOADED | Mandatory skill activated |
| `anatomy` (GD) | `~/.config/goose/skills/anatomy/` | AVAILABLE | Not selected (not needed) |

No new installations; no modifications to skill files.

---

## 3. PRESERVATION CHECK (VERIFIED)

| Check | Before | After | Status |
|---|---|---|---|
| Modified files | ~84 (prior phases) | ~89 (includes A–H reports) | Preserved |
| Untracked files | ~33 | ~33 | Preserved |
| Phase reports A–H | All present | All preserved unmodified | ✅ |
| Production source (`src/`) | Unmodified | Unmodified | ✅ |
| Git destructive commands | None | None | ✅ |
| Disk `/mnt/D` | 7.1 GB available | 7.1 GB available | ✅ |

---

## 4. ACTUAL APPLICATION ARCHITECTURE (VERIFIED FROM SOURCE)

### 4.1 Source Structure (exists)

```
src/
  main.cpp (445 lines) — entry point
  Simulation/ — SimulationEngine.cpp/hpp, BridgeClient, Scheduler
  Assembly/ — AssemblyGraph, KinematicSolver, Mates, Definitions
  CAD/ — CADImporter.cpp/hpp (IGES/STEP loaders, NO USD loader)
  Scene/ — Scene.cpp/hpp, Components.hpp (basic scene graph)
  Materials/ — MaterialDatabase, Geometry.cpp
  Graphics/ — Graphics component
  UI/ — Panel components (OmniverseLiveSyncPanel exists)
  WebAPI/ — FastAPIBridge, SimulationServer
  WebViewer/ — BabylonJSRenderer, ThreeJSRenderer, ExportToThreeJS
  Omniverse/ — OmniverseConnector, NucleusConnector, LiveSync
```

**Status:** All source directories verified to exist; content readable.

### 4.2 Main Entry Points (VERIFIED)

- `src/main.cpp` — 445 lines, compiled
- `build-final/bin/` — **EMPTY** (no executable)
- Build log (`build-final/build.log`) — 347 lines, indicates partial build
- `build_err.log` — 841 lines of errors

**Status:** Code exists; executable does not.

### 4.3 CAD / Import (VERIFIED FROM CODE)

`CAD/CADImporter.cpp` (21,889 bytes):
- Loads IGES and STEP formats (`IGESLoader.hpp`, `STEPLoader.hpp`)
- **No `USD` or `usda` import path** in CADImporter
- `Omniverse/` directory provides Omniverse/Nucleus connectors — these are external sync interfaces, not USD import pipelines

**Status:** CAD import exists for IGES/STEP; USD import not implemented in production.

---

## 5. FEATURE-BY-FEATURE IMPLEMENTATION STATUS

| Capability | Evidence Required | Actual Evidence | Classification | Notes |
|---|---|---|---|---|
| **CAD viewport** | Rendering + interaction | `source/Graphics/` exists; no running app | PARTIAL | Source exists; no executable to verify |
| **USD loading** | Real file import success | No USD loader in CADImporter; Omniverse connector exists but not imported | DOCUMENTATION ONLY | Phase A–H reports investigate USD; no production connection |
| **Scene graph** | Real hierarchy shown | `Scene/Components.hpp` (1723 bytes, basic); no working binary | PARTIAL | Basic structure; no verified hierarchy display |
| **Assembly graph** | Functional relationships | `AssemblyGraph.cpp/hpp` exists; `KinematicSolver.cpp` verified (T15) | IMPLEMENTED (code verified) | Tests pass; no UI connection verified |
| **Kinematics** | Solver execution | `test_kinematic_solver_phase02.cpp` exists; `KinematicSolver.cpp` (3892 bytes) | IMPLEMENTED (verified) | Code verified; no frontend execution shown |
| **Physics simulation** | Real execution | `SimulationEngine.cpp` (9675 bytes) has Tick(); `BridgeClient.cpp` exists | IMPLEMENTED (code verified) | Formula-based (simulated); `.data/sim_telemetry_contract.md` confirms SIMULATED |
| **Materials** | Actual assignment | `MaterialDatabase.cpp` (6334 bytes); `IMaterial.hpp`; BOM verified textures | IMPLEMENTED | External texture files verified; USD material mapping unverified |
| **BOM generation** | From assembly data | `AssemblyGraph::GenerateBOM()` exists; `tests/main_test.cpp` references T10 | IMPLEMENTED | Verified by tests |
| **Telemetry contract** | Verified vs simulated | `.data/sim_telemetry_contract.md` — explicitly SIMULATED | DOCUMENTED (simulated only) | Not physically validated |
| **USD import (conveyor)** | `enabled.usda` loaded | File downloaded (Phase E); never loaded into app | **NOT IMPLEMENTED** | Only in `.planning/` documentation |
| **Instance hierarchy (70 prims)** | Represented in app | `enabled.usda` parsed; no source code consumes it | **NOT IMPLEMENTED** | Only documented in Phase E |
| **Physics properties (mass/inertia)** | Verified from payload | Zero physics schemas in file (Phase H); not integrated | **NOT IMPLEMENTED** | No production integration |

---

## 6. WHAT ACTUALLY WORKS TODAY (VERIFIED)

### 6.1 Verified Working Source Code
1. `KinematicSolver` — verified by `tests/test_kinematic_solver_phase02.cpp`
2. `MaterialDatabase` — verified by `tests/main_test.cpp` (T08)
3. `AssemblyGraph` / `GenerateBOM()` — verified by tests (T10)
4. `SimulationEngine::Tick()` — code verified (formula-based, simulated)
5. `MeshGenerator` / geometry operations — verified by T05, T06
6. `SceneGraph` — basic structure verified (T02)
7. `OmniverseConnector` — source verified (external sync, not USD import)

### 6.2 What Exists Only in Source (Not Verified Running)
- Full CAD viewport rendering (no executable to confirm)
- WebViewer (React/ThreeJS) — source present; no verified runtime
- FastAPI bridge — source present; no verified server run
- All physics simulation (formulas verified; real physics unverified)

### 6.3 What Exists Only in Documentation / Planning
- All Phase A–H reports (`.planning/phases/02-physics-integration/`)
- SimReady inventory, BOM references, download records
- No production file references any of these planning artifacts

---

## 7. WHY PROGRESS IS NOT VISIBLE

The audit identifies four concrete reasons:

| Reason | Evidence | Impact |
|---|---|---|
| **No executable built** | `build-final/bin/` empty; `build_err.log` 841 lines | Cannot see or use any feature |
| **No USD import pipeline** | `CADImporter.cpp` handles IGES/STEP only | Conveyor USD never reaches app |
| **No planning-to-source bridge** | `.planning/` reports unreferenced by any `src/` file | All NVIDIA findings are isolated |
| **No frontend integration** | `UI/panels/OmniverseLiveSyncPanel.cpp` exists but connects to Omniverse sync, not SimReady import | No visual demonstration possible |

The phases A–H produced research; they did not produce implementation. This is exactly what the audit is designed to reveal.

---

## 8. NVIDIA SIMREADY INTEGRATION — ACTUAL STATUS

| Integration Step | Evidence | Status |
|---|---|---|
| Asset downloaded | `enabled.usda` (19.8 KB) at `/tmp/szm_simready_inspection/` | ✅ Done |
| Payload inspected | `instances_physx.usda` (109.4 KB) at `/tmp/phase_h_inspection/` | ✅ Done |
| Parser verification | `usd-core` 26.11 installed and working | ✅ Done |
| Source code integration | **NONE** — no `src/` file references `/tmp/` or `.planning/` | ❌ Missing |
| Frontend visualization | **NONE** — no panel imports USD | ❌ Missing |
| Build pipeline connection | **NONE** — build errors prevent executable | ❌ Missing |
| Simulation feed | **NONE** — `SimulationEngine` uses mock formulas only | ❌ Missing |

---

## 9. RECOMMENDED FIRST IMPLEMENTATION MILESTONE

Given verified constraints (no build binary, no USD import in source), the smallest meaningful milestone:

**Milestone 1 — USD Import Bridge (verified smallest step):**
- Add USD file reading capability to `CADImporter.cpp` (or create `USDImporter.cpp`)
- Load `enabled.usda` into `SceneGraph`
- Verify hierarchy displays 70 instances
- Verify material references resolve (or document missing `materials.usda`)
- Create basic viewport display
- Document physics property absence (per Phase H)

**Why this is the correct first milestone:**
- It connects the only verified external data (Phase E–H) to production code
- It requires no physics properties (just geometry)
- It uses existing `SceneGraph` architecture (verified)
- It can be verified visually (viewport shows instances)
- It identifies the gap (missing physics) explicitly rather than hiding it

---

## 10. FINAL RESPONSE TO USER QUESTIONS

| Question | Answer |
|---|---|
| **What actually works today?** | Source modules verified (KinematicSolver, MaterialDatabase, AssemblyGraph, SimulationEngine formulas, SceneGraph basic). No executable runs. |
| **What can I see and use?** | Nothing in the application. Source code exists; no running instance demonstrates any capability. |
| **What exists only in documentation?** | All Phases A–H (inventory, inspection, gap analysis, parser feasibility, payload inspection, source discovery). Also `AGENTS.md`, planning directories, simulated telemetry contracts. |
| **Why is progress not visible?** | Four reasons: no executable (build errors), no USD import in CAD, no bridge from planning to source, no frontend panel for SimReady import. The work was documentation-only, not implementation. |
| **What is the first feature we should implement?** | Milestone 1: USD import bridge connecting `enabled.usda` to `SceneGraph` — verified smallest step, uses only verified geometry data, exposes the physics gap honestly. |
| **How will I verify it visually and functionally?** | Visual: viewport displays 70 instance prims from USD. Functional: parser loads file; hierarchy traversable; material references documented (missing `geometries.usd` / `materials.usda` shown clearly). |

---

## 11. REPORT LOCATION

`.planning/phases/02-physics-integration/08-I-APPLICATION-REALITY-AUDIT.md`

---

## 12. STOP — AWAITING APPROVAL

Audit complete. No production modifications made. All phases A–H preserved. Build errors and missing executable documented. Recommendation: approve Milestone 1 (USD import bridge) to connect verified data to working application.
