# SZM Forge User Workflow Findings

## Walkthrough context

Tested the React UI in `new ui/` using its production preview on 2026-11-11. The UI build completed successfully. The browser session was signed in as `LOCAL_ADMIN`; the project was empty (`PROJECT: —`, `NO MODEL`). In this preview environment, API requests such as `/api/status` and `/api/export/usd` returned HTTP 500. The browser also reported WebGL disabled, which is relevant to the viewport finding below.

## Findings

### P1 — Testing Lab produced numerical estimates with no component loaded

- **Where:** [`TestingLabWorkspace.tsx`](<new ui/src/components/TestingLabWorkspace.tsx>), especially the stress and thermal fallback paths.
- **Observed:** With “No parts — load via import” selected, both **RUN STRESS** and **RUN THERMAL** remained enabled and produced numerical outputs. Stress reported `σ≈10.0 MPa` for the default 1000 N; thermal reported `ΔL≈0.072 mm @ 80°C`.
- **Why it needs work:** These fallbacks use fixed stand-in formulas and can run without a selected part, its geometry, or its material. Despite the “offline estimate” label, the result can be mistaken for an engineering analysis.
- **Next:** Disable both actions until a valid component is selected; clearly distinguish any heuristic estimate from solver results and require validated inputs before reporting engineering values.
- **Status:** First fix applied. Both actions are disabled until a component is selected; stress requires explicit positive cross-section area and thickness; solver failures now appear in the work log and toast instead of being replaced by synthetic numerical results. Thermal backend fidelity and full solver-input mapping still need separate verification.

### P1 — Global RUN reports success without running an analysis

- **Where:** [`ModernApp.tsx`](<new ui/src/ModernApp.tsx>), `runSolver`.
- **Observed:** Clicking **RUN** from the empty Command Center displayed “Solver finished” while the footer still showed `PROJECT: —` and `NO MODEL`.
- **Why it needs work:** The handler advances a short simulated progress loop and marks the run complete. It only calls an analysis for assembly members or furniture; imported `bodyParts` do not enter either branch.
- **Next:** Gate the command on supported model/solver inputs, include imported parts in a real analysis path, and only show success after a solver returns a valid result. Show a clear empty-state message otherwise.
- **Status:** Empty projects and imported-scene-only projects now stop immediately with a specific “Simulation unavailable” message instead of reporting completion. Connecting imported scene parts to a real solver remains separate work.

### P1 — STEP/DXF import inserts a placeholder, not the source geometry

- **Where:** [`ModernApp.tsx`](<new ui/src/ModernApp.tsx>) import handler; the import dialog is also rendered there.
- **Observed:** The dialog accepts `.step`, `.stp`, and `.dxf`, but its own description says STEP/DXF add a placeholder solid. The handler inserts a fixed 80×40×20 box for these formats; only STL is parsed as mesh geometry.
- **Why it needs work:** Users can mistake the placeholder for a successful import and proceed with the wrong shape.
- **Next:** Complete the STEP/DXF geometry bridge, or reject/defer those formats explicitly and label the placeholder as a preview that cannot be analyzed or exported as the source model.

### P1 — USD export uses sample geometry and fails without user feedback

- **Where:** [`TopCommandBar.tsx`](<new ui/src/components/TopCommandBar.tsx>), `handleExportUSD`.
- **Observed:** Export was available with no model. The request payload is hard-coded to a `SZM_Forge_Export` project containing a sample `Base_Plate`; in this preview the export request returned HTTP 500, and the UI showed no error or success message (only a console error).
- **Why it needs work:** This does not export the active scene and can leave users believing an export succeeded when no file was produced.
- **Next:** Serialize the actual current project/scene, disable export when there is nothing to export, and show actionable success/failure feedback.

### P1 — Viewport-dependent workspaces can blank the app when WebGL is unavailable

- **Where:** [`ViewportThree.tsx`](<new ui/src/components/ViewportThree.tsx>), WebGL renderer initialization.
- **Observed:** Selecting Geometry or Simulation in the test browser caused an uncaught `THREE.WebGLRenderer: Error creating WebGL context` and the page became blank. The browser reported WebGL disabled, so this needs confirmation on the intended desktop/GPU setup.
- **Why it needs work:** There is no visible fallback when renderer initialization fails; the user loses the workspace rather than receiving an explanation or usable alternative.
- **Next:** Handle renderer creation failures and provide a clear unsupported-GPU/WebGL state; add a test for the fallback.
- **Status:** Geometry mode errors are now contained and show a WebGL guidance message while leaving the mode tabs usable. Browser-verified that Sketch can still be opened when the default Mesh viewport fails. The other viewport entry points still need similar recovery handling.

### P1 — Sketch extrusion reported a result without creating a project part

- **Where:** [`SketchEditor.tsx`](<new ui/src/components/geometry/SketchEditor.tsx>) and [`geometry.ts`](<new ui/src/offline/geometry.ts>).
- **Observed:** Drawing a 100×100 mm rectangle and extruding it 10 mm showed `Volume: 100000.0 mm³`, but the footer remained `NO MODEL` and the part was not available to other workspaces.
- **Why it needs work:** A successful-looking extrusion was only a numerical report; it did not create an editable scene object. Invalid/open profiles also silently fell back to a sample box in the offline engine.
- **Next:** Extend extrusion to arcs and multi-loop profiles if those sketch types are intended to be supported.
- **Status:** Extrusions of closed line profiles and circles now create a custom scene part and use profile area for the volume result. Open/unsupported profiles are rejected with a visible message. Browser-verified the model state changes to `MODEL LIVE` and `Sketch Extrusion 1` is available in Testing Lab. The part is in current-session state; project save/reload persistence remains unimplemented.

### P1 — Created scene geometry is not persisted as a project

- **Where:** [`ProjectContext.tsx`](<new ui/src/store/ProjectContext.tsx>), `bodyParts` state.
- **Observed:** `bodyParts` is held only in React state and is not included in project storage. Sketch extrusion is now available to other workspaces during the session, but a reload loses the new part.
- **Why it needs work:** A successful design operation can be lost when the user closes or reloads the app.
- **Next:** Define and implement project save/load for scene objects, including custom geometry serialization and restoration.

### P2 — AI workspace shows sample analysis as current, and several actions do nothing

- **Where:** [`AIWorkspace.tsx`](<new ui/src/components/AIWorkspace.tsx>) and [`mockData.ts`](<new ui/src/data/mockData.ts>).
- **Observed:** With `NO MODEL`, the AI workspace still showed turbine/blade-specific values (for example 847.3 MPa, fatigue safety factor 1.82, and five active insights). Clicking **REVIEW CURRENT MODEL**, **SUGGEST OPTIMIZATION**, or **CHECK SAFETY RISKS** produced no visible action; these buttons have no handlers.
- **Why it needs work:** The display implies live model-specific findings while no model is loaded, and the primary calls to action are inert.
- **Next:** Clearly label sample/demo content, suppress model-specific metrics without a loaded model, and wire these actions to real workflows or present them as disabled with an explanation.

### P2 — Backend health indicators do not reflect failed API requests

- **Where:** [`BackendContext.tsx`](<new ui/src/store/BackendContext.tsx>), [`forgeApi.ts`](<new ui/src/services/forgeApi.ts>), and [`CommandCenterWorkspace.tsx`](<new ui/src/components/CommandCenterWorkspace.tsx>).
- **Observed:** The Command Center showed “Backend bridge CONNECTED” while `/api/status` repeatedly returned HTTP 500. The context sets `connected` to `true` on refresh, and the offline engine helper also reports available independently of a remote backend.
- **Why it needs work:** Users cannot tell whether a feature is using the in-browser offline engine, a live service, or a failed optional backend. AI/backend-dependent panels can appear live while their requests fail.
- **Next:** Separate offline-engine readiness from remote-service health, report failures in the relevant panels, and avoid “connected/live” labels for unavailable services.

### P1 — Manufacturing workspace presents hard-coded sample data as live project state

- **Where:** [`ManufacturingDashboard.tsx`](<new ui/src/components/ManufacturingDashboard.tsx>).
- **Observed:** With no active project or model, the workspace showed an “AI Hub Assembly” BOM, 42 parts, $47.2K estimated cost, completed/active pipeline percentages, tolerance margins, and analysis results. These values are literals in the component. Its DFM request also uses a fixed project name and thickness, and the screen continued to say “Running analysis against Python backend...” after the request failed.
- **Why it needs work:** A user cannot tell demo content from actual manufacturing readiness, and could make decisions based on unrelated example values.
- **Next:** Bind the dashboard to the active project and show an explicit empty/demo state when no project data exists; update analysis status from actual requests.

### P2 — Manufacturing actions fail silently and use a fixed sample part

- **Where:** [`ManufacturingDashboard.tsx`](<new ui/src/components/ManufacturingDashboard.tsx>), CNC G-code and slicer handlers.
- **Observed:** **GENERATE CNC G-CODE** raised a backend-required error only in the browser console. The request uses fixed assembly dimensions; the slicer request also hard-codes component ID, name, and dimensions. Neither failure surfaced as a visible message.
- **Why it needs work:** Users get no actionable feedback and neither action is tied to the active design.
- **Next:** Use the current project geometry, disable unsupported actions when inputs/services are missing, and show visible progress, success, and failure states.

### P2 — Pipeline action exposes a backend-only feature while claiming the bridge is connected

- **Where:** Command Center pipeline action and backend health indicators.
- **Observed:** **EXECUTE PIPELINE** displayed “Pipeline failed: Automation pipeline requires backend connectivity (set VITE_ENABLE_BACKEND=true)” while the dashboard and footer still showed the backend as connected/live.
- **Why it needs work:** The app presents an executable action and contradictory service health; the failure is only explained after the click.
- **Next:** Gate the action on real backend capability and make status labels distinguish the offline engine from optional remote services.

## Overall recommendation

Prioritize truthful engineering outcomes first: block analyses without valid inputs, distinguish demos/estimates from solver results, and prevent placeholder imports or sample exports from being mistaken for real project data. Then address renderer fallback and service-health reporting so users can recover from unsupported or unavailable dependencies.

## Work started

The Testing Lab empty-input and synthetic-fallback issue, global RUN empty-model success issue, Sketch extrusion path, and Geometry-mode WebGL fallback have initial fixes. See the `Status` notes above. Other findings remain open.
