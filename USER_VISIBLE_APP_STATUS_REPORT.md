# SZM Forge User-Visible App Status Report

**Prepared on:** May 19, 2026  
**Focus:** What a user can actually use today  
**Scope:** Desktop app first, web app second  
**Important distinction:** This report measures **usable product completion**, not code volume

## 1. Why This Report Exists

The previous full project report measured how much engineering work exists in the repository.

That is **not the same** as asking:

- what works when the app opens
- what a normal user can really do
- what is connected end-to-end
- what feels complete

This report uses the stricter lens:

**If a user opens the app today, what is actually usable?**

## 2. Main Conclusion

Your impression is valid.

From a **user-visible app** perspective, `SZM Forge` is still at a **very early stage**.

There is real code in the repo, but much of it is:

- backend/module code not exposed in the app
- partially wired UI
- informational panels without real editing
- preview/demo behavior
- placeholder integration
- mock or static content

### Honest product-level assessment

If measured by **repo engineering work**, the project is substantial.

If measured by **usable app completion**, the project feels closer to:

- **Desktop app:** around **5% to 15% complete**
- **Web dashboard:** around **20% to 35% complete visually**, but much less in true engine integration

These are judgment estimates, not exact metrics, but they match the current code wiring much better than the earlier repo-wide audit.

## 3. Audit Method

This report was prepared by checking:

- the actual desktop startup path
- which panels the desktop app really registers
- whether those panels change real application state or only local UI state
- where mock/demo/placeholder behavior is explicitly present in code
- which web frontend routes depend on real backend endpoints versus fallback demo data

### Limitation

This was not a live GUI click-through with full desktop interaction in a graphical session.

It is a **runtime wiring audit** based on current startup code and panel implementations, which is still very strong evidence for what the app can and cannot do.

## 4. What Actually Launches in the Desktop App

The desktop app path starts in:

- `src/main_desktop.cpp`
- `src/Core/Application.cpp`

### Important startup reality

`Application::Init()` already registers a full set of desktop panels:

- Console
- Scene Outliner
- Properties
- Simulation Control
- Simulation Visualizer
- Component Builder
- Toolbar
- Dashboard
- Workshop Library
- Part Editor
- Workshop Assembly
- Workshop Viewport
- Workshop Simulation

Then `main_desktop.cpp` registers **another** set of advanced panels on the same `UIManager` singleton:

- AdvancedViewportPanel
- SimulationPlaybackPanel
- AnalysisResultsPanel
- SelectionPropertiesPanel
- VisualizationDebugPanel
- HeatmapLegendPanel
- PerformanceMonitorPanel
- ViewportSettingsPanel

### What this means for the user

The app starts with **many panels**, but panel count should not be mistaken for product completion.

A large number of visible panels are:

- read-only
- mock-backed
- partly wired
- disconnected from one another
- dependent on data paths that are not really populated

## 5. Desktop App: What Is Genuinely Usable Today

These are the parts I would count as **actually usable**, not just present.

## 5.1 Basic simulation component workflow

### Usable

The user can:

- add simulation components
- view component list
- select a component
- adjust force
- adjust heat input
- remove components
- see stress / temperature / danger state update

### Evidence

- `src/UI/panels/SimulationPanel.cpp`
- `src/Simulation/SimulationEngine.cpp`

### User-visible value

This is one of the clearest working flows in the desktop app.

### Reality level

**Real but simple.**

It is a small engineering demo workflow, not a full CAD/simulation environment.

## 5.2 Component builder with material presets

### Usable

The user can:

- choose a material preset
- define area and thickness
- create a component
- use quick templates like beam, plate, copper wire, titanium alloy

### Evidence

- `src/UI/panels/ComponentBuilderPanel.cpp`

### User-visible value

This works as a convenience layer around the simulation demo.

### Reality level

**Usable**, but still tied to the simple component-based simulation model.

## 5.3 Workshop primitive / assembly flow

This is the strongest visible desktop area after the basic simulation panel.

### Usable

The user can:

- browse indexed workshop parts
- inspect selected part metadata
- create a primitive block part
- assign simple material and description
- create or switch assemblies
- add parts into an assembly
- move / rotate / hide / remove instances
- view a workshop viewport
- sync assembly items into the simulation engine
- apply force / heat to synced instances
- run a workshop simulation pass

### Evidence

- `src/Workshop3D/WorkshopPanels.cpp`
- `src/Workshop3D/WorkshopManager.cpp`

### User-visible value

This is the closest thing in the desktop app to a real end-to-end product flow:

1. create/import part
2. place it in assembly
3. sync to simulation
4. run simplified simulation
5. inspect results

### Reality level

**Partially usable**, and likely the most promising desktop path today.

It still feels prototype-grade, but it is closer to a product workflow than most other panels.

## 5.4 Workshop viewport

### Usable

The user can:

- see assembly instances drawn
- rotate camera via yaw/pitch sliders
- change zoom
- toggle grid/labels
- view color changes based on selected heatmap mode

### Evidence

- `src/Workshop3D/WorkshopPanels.cpp`

### Reality level

**Usable**, but it is still a simplified custom viewport representation, not a full integrated 3D engineering viewport.

## 5.5 Dashboard as status surface

### Usable

The dashboard can show:

- component count
- dangerous count
- average stress
- average temperature ratio
- recent plots
- warning summaries

### Evidence

- `src/UI/panels/DashboardPanel.cpp`

### Reality level

**Usable as a monitoring panel**, but it is not a feature workflow by itself.

It helps once simulation data exists, but it does not make the app feel complete on its own.

## 6. Desktop App: What Exists but Feels Only Partially Real

These features are visible enough to notice, but are not complete enough to feel solid.

## 6.1 Simulation Visualizer

### What works

- search box
- stress/temperature toggles
- wireframe toggle
- component list
- basic visual representation
- selected component details

### What limits it

- it is not a real integrated 3D engineering viewport
- it uses projected boxes and UI drawing
- the code itself notes: **graphics module integration pending**

### Evidence

- `src/UI/panels/SimulationVisualizerPanel.cpp`

### Reality level

**Partially real.**

It is useful as a visual demo, but not yet convincing as a finished viewport.

## 6.2 Part import in workshop/library

### What works

- file path entry
- import action hooks
- workshop part registration flow

### What limits it

- CAD import depth is limited
- STEP import can fall back to placeholder/proxy behavior
- true tessellation/import depth is not complete

### Evidence

- `src/Workshop3D/CADImporter.cpp`
- `src/CAD/CADImporter.cpp`

### Reality level

**Partially usable**, but still fragile and incomplete from a user standpoint.

## 6.3 Advanced panel suite from `main_desktop.cpp`

These panels exist:

- AdvancedViewportPanel
- SimulationPlaybackPanel
- AnalysisResultsPanel
- SelectionPropertiesPanel
- VisualizationDebugPanel
- HeatmapLegendPanel
- PerformanceMonitorPanel
- ViewportSettingsPanel

### What they appear to be

Mostly:

- controls
- readouts
- render/debug helpers
- analysis summaries

### Main issue

They depend on deeper graphics/selection/visualization systems that are not clearly shown to be filled by a real end-to-end live workflow in the current app.

### Example warning signs

- debug panel uses fixed performance numbers like `FPS: 60`
- some panels summarize data from specialized visualizer singletons whose runtime population is unclear from the current visible workflow

### Reality level

**Present, but not counted as core usable product flow.**

## 7. Desktop App: What Is Mostly Placeholder, Mock, Cosmetic, or Not Really Wired

This section explains why the app can feel “1% done.”

## 7.1 Scene Outliner

### Problem

The outliner starts with **default mock nodes**:

- `Engine_Assembly_01`
- `Chassis_Frame`
- `Sensor_Array`

It is not clearly driven by a real scene graph in normal app usage.

### Evidence

- `src/UI/panels/SceneOutliner.cpp`

### User impact

This immediately makes the app feel fake or disconnected.

## 7.2 Properties panel

### Problem

The panel literally shows:

- transform editing: pending
- material editing: pending
- constraint editing: pending

### Evidence

- `src/UI/panels/PropertiesPanel.cpp`

### User impact

This is visible unfinished UI, so users correctly feel the app is incomplete.

## 7.3 Console command flow

### Problem

The console accepts input, but replies:

- `Command processing is not wired yet.`

### Evidence

- `src/UI/panels/ConsolePanel.cpp`

### User impact

It looks interactive, but is not really functional.

## 7.4 Toolbar playback controls

### Problem

The toolbar exposes:

- Play
- Pause
- Reset
- Speed slider

But the actual application loop still calls:

- `SimulationEngine::Tick(m_DeltaTime)` every frame

regardless of toolbar play/pause state.

### Evidence

- `src/UI/panels/ToolbarPanel.cpp`
- `src/Core/Application.cpp`

### User impact

The controls look important, but much of their state is only local UI state.

So the toolbar feels more complete than it really is.

## 7.5 Thermal, electrical, and AI system integration

### Problem

These systems exist in code, but the desktop app still has TODOs for:

- thermal engine tick
- electrical engine tick
- AI assistant update

### Evidence

- `src/Core/Application.cpp`

### User impact

The app may claim broad capabilities, but those domains are not truly alive in the main runtime.

## 7.6 Physics preview / settings claims

Some UI/settings text refers to physics preview or advanced behavior, but not all of it is really active end-to-end.

### Evidence

- workshop settings and component browser references
- placeholder notes in workshop-related UI

### User impact

The app promises more than it actually performs visibly.

## 8. Desktop App Summary by Panel

## 8.1 Panels I would classify as genuinely useful now

- `SimulationPanel`
- `ComponentBuilderPanel`
- `DashboardPanel`
- `Workshop Library`
- `Part Editor`
- `Workshop Assembly`
- `Workshop Viewport`
- `Workshop Simulation`

## 8.2 Panels I would classify as partial

- `SimulationVisualizerPanel`
- `AdvancedViewportPanel`
- `SimulationPlaybackPanel`
- `AnalysisResultsPanel`
- `SelectionPropertiesPanel`
- `HeatmapLegendPanel`
- `PerformanceMonitorPanel`
- `ViewportSettingsPanel`

## 8.3 Panels I would classify as visibly unfinished or weakly wired

- `SceneOutliner`
- `PropertiesPanel`
- `ConsolePanel`
- parts of `ToolbarPanel`
- parts of workshop/CAD import flow

## 9. Web App Status

The web dashboard is a different story.

It is **more polished visually** than the desktop app, but it is also **less connected to the real engine**.

## 9.1 What the web app does well

The frontend is professionally structured:

- modern route shell
- dashboard
- jobs page
- material library page
- analysis page
- scene viewer page

### Evidence

- `webui/src/App.jsx`
- `webui/src/pages/`

### User-visible impression

The web app probably feels more “finished” at first glance than the desktop app.

## 9.2 What the backend actually provides

The Node backend currently provides only a small real endpoint set:

- `POST /api/jobs`
- `GET /api/jobs/:jobId`
- `GET /api/jobs`
- `DELETE /api/jobs/:jobId`
- `GET /api/materials`
- `GET /api/materials/:name`
- `GET /api/stats`
- `GET /health`

### Evidence

- `webui/backend/server.js`

## 9.3 Where the web app depends on preview/fallback data

The frontend requests:

- `/api/stats`
- `/api/jobs`
- `/api/materials`
- `/api/scene`

But the backend does **not** provide `/api/scene`.

So the scene viewer falls back to `demoScene`.

The frontend also intentionally ships:

- `demoStats`
- `demoMaterials`
- preview jobs
- preview scene

### Evidence

- `webui/src/lib/forgeApi.js`
- `webui/src/pages/SceneViewer.jsx`
- `webui/src/pages/Dashboard.jsx`
- `webui/src/pages/Analysis.jsx`

## 9.4 What this means

The web app is:

- good as a product shell
- good as a UX prototype
- good as a preview/demo layer

But it is **not yet deeply integrated** with the actual engineering backend.

### Honest web status

The web app is **visually ahead** of the desktop app, but **functionally still shallow**.

## 10. The Real Reason the App Feels Incomplete

The app feels incomplete because the visible experience is dominated by one or more of these patterns:

- mock scene data
- pending inspector features
- not-wired console actions
- toolbar controls that do not control the core loop
- advanced panels without a strong data pipeline behind them
- workshop/CAD import paths that still rely on placeholders or simplified proxies
- simulation domains existing in code but not alive in the main desktop runtime

From a user’s perspective, this is exactly what “not even 1% complete” feels like.

## 11. Honest Product-Level Completion Estimate

These are judgment-based, but they fit the current code much better than any repo-wide count.

## 11.1 Desktop app

### By user-visible completion

Estimated: **5% to 15%**

Why:

- a few small flows work
- workshop flow is promising
- basic simulation editing works
- many visible panels are not truly complete
- many important controls are cosmetic or partial

## 11.2 Web app

### By user-visible completion

Estimated: **20% to 35% visually**

Why:

- polished structure and navigation
- several screens render well
- jobs/materials/stats are at least demo-functional
- scene/analysis are not fully backed by real APIs
- integration to the actual engineering engine remains shallow

## 11.3 Whole product

### By real end-user product readiness

Estimated: **very early prototype**

The repo has meaningful engineering work, but the user experience is still far from a cohesive product.

## 12. Strongest Real User Flow Today

If someone asked “what is the most real part of the app right now?”, my answer would be:

### Best current desktop flow

1. create or choose a simple part
2. add it into workshop assembly
3. sync assembly to simulation
4. apply force / heat
5. inspect simple results in workshop simulation/dashboard panels

That is the most believable app flow currently present.

It is still prototype-grade, but it is more real than the rest.

## 13. Weakest Visible Areas

The features most likely to break user trust immediately are:

- mock scene outliner
- pending properties panel
- console command box with no real command handling
- playback controls not actually controlling the main simulation loop
- broad claims of thermal/electrical/AI capability without visible runtime integration

## 14. Final Conclusion

The previous repo-wide audit and your user experience are both true, but they are measuring different things.

### Repo truth

There is a lot of code and subsystem work in `szm_forge`.

### User truth

The actual app still feels barely complete.

### Most honest single-sentence summary

**SZM Forge has substantial engineering work behind it, but the current user-facing app is still only an early prototype with a few real flows and many incomplete or weakly integrated surfaces.**

## 15. Recommended Next Step

The best next audit after this would be:

**“Desktop App Reality Fix Plan”**

That report would identify:

1. which visible panels should stay
2. which ones should be hidden until wired
3. which flows need to be completed first
4. how to move the app from “prototype feeling” to “usable product feeling”

