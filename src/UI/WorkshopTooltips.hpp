#pragma once

/**
 * Contextual help strings for the 3D machine-design workshop.
 * Wording references capabilities integrated from the codes/ bundles
 * (Bullet physics, Three.js viewer, FEA/code8, materials DB, FastAPI).
 */

namespace SZM::WorkshopTooltips {

// --- Component Browser tabs ---
inline constexpr const char* TabLibrary =
    "Browse indexed parts from the workshop library and code8 example bundles under codes/.";
inline constexpr const char* TabImport =
    "Import CAD (STEP, GEO) or create parametric primitives for your machine design.";
inline constexpr const char* TabPlacement =
    "Position, rotate, and scale parts before adding them to the active assembly.";
inline constexpr const char* TabVisualization =
    "Control viewport overlays: grid, labels, stress/thermal heatmaps (FEA bridge).";
inline constexpr const char* TabSettings =
    "Browser preferences: default materials, auto-placement, and display options.";

// --- Workshop Library ---
inline constexpr const char* LibrarySearch =
    "Filter parts by name, category, material, or description text.";
inline constexpr const char* LibraryCode8Path =
    "Folder scanned for legacy code8 FEA example meshes (codes/code 5, project root).";
inline constexpr const char* LibraryReloadCode8 =
    "Re-import all example parts from the code8 directory into the library.";
inline constexpr const char* LibraryImportPath =
    "Path to OBJ, STL, GEO, or STEP file. Uses geometry pipeline from codes integration.";
inline constexpr const char* LibraryImportButton =
    "Parse the file, compute bounds and mass estimate, and add to the part library.";
inline constexpr const char* LibraryAutoPlace =
    "Place the selected part in the active assembly using collision-aware spacing.";
inline constexpr const char* LibraryPartList =
    "Click a row to inspect mass, bounds, and artifacts for that catalog entry.";

// --- Part Editor ---
inline constexpr const char* EditorPartName =
    "Display name used in BOM, outliner, and simulation component labels.";
inline constexpr const char* EditorMaterial =
    "Material keyword (Steel, Aluminum, Titanium) sets density for mass calculation.";
inline constexpr const char* EditorDescription =
    "Engineering notes stored with the part; shown in properties and exports.";
inline constexpr const char* EditorSize =
    "Half-extents of a parametric box primitive in meters (SI units).";
inline constexpr const char* EditorAddToAssembly =
    "When enabled, new primitives are inserted into the active assembly immediately.";
inline constexpr const char* EditorCreateBlock =
    "Creates a box mesh, registers it in the library, and optionally places it.";

// --- Assembly ---
inline constexpr const char* AssemblyName =
    "Logical name for a machine sub-assembly (e.g. 'Conveyor Frame').";
inline constexpr const char* AssemblyCreate =
    "Start a new assembly graph node; instances are children of this assembly.";
inline constexpr const char* AssemblyPartCombo =
    "Choose which library part to instantiate in the active assembly.";
inline constexpr const char* AssemblyAutoPlacement =
    "Uses SuggestPlacement() to offset parts and reduce overlap in the viewport.";
inline constexpr const char* AssemblyPosition =
    "World position (meters) for the new instance origin.";
inline constexpr const char* AssemblyInstanceName =
    "Optional unique label; defaults to part name if left empty.";
inline constexpr const char* AssemblyAddPart =
    "Instantiate the selected catalog part at the placement transform.";
inline constexpr const char* AssemblyInstanceList =
    "Select an instance to edit transform, visibility, or simulation mapping.";
inline constexpr const char* AssemblyInstancePosition =
    "Drag to move the instance; updates live in the workshop viewport.";
inline constexpr const char* AssemblyInstanceRotation =
    "Euler rotation in degrees (X, Y, Z) applied to the instance.";
inline constexpr const char* AssemblyInstanceScale =
    "Non-uniform scale affects rendered size and mass estimate.";
inline constexpr const char* AssemblyHideInstance =
    "Toggle visibility without removing the instance from the assembly.";
inline constexpr const char* AssemblyRemoveInstance =
    "Delete this instance from the assembly (does not remove library part).";

// --- Viewport ---
inline constexpr const char* ViewportYaw =
    "Orbit camera heading around the assembly focus point.";
inline constexpr const char* ViewportPitch =
    "Camera elevation angle; clamped to avoid gimbal flip.";
inline constexpr const char* ViewportZoom =
    "Distance from camera target; use mouse wheel on canvas when focused.";
inline constexpr const char* ViewportGrid =
    "Show ground grid for spatial reference when placing machine parts.";
inline constexpr const char* ViewportLabels =
    "Draw instance names above parts in the 2D projected viewport.";
inline constexpr const char* ViewportAutoRotate =
    "Slow turntable rotation for presentation previews.";
inline constexpr const char* ViewportHelp =
    "Show keyboard/mouse hints overlay on the canvas.";
inline constexpr const char* ViewportFrameSelection =
    "Center camera on the selected instance (also press F on canvas).";
inline constexpr const char* ViewportResetCamera =
    "Restore default orbit, pitch, zoom, and target origin.";
inline constexpr const char* ViewportCanvas =
    "LMB drag: orbit | MMB: pan | RMB drag instance | Shift: fine move | Q/E: yaw | Z/X: scale";

// --- Simulation ---
inline constexpr const char* SimDuration =
    "Total simulated time in seconds for the workshop batch run.";
inline constexpr const char* SimTimestep =
    "Integration step; smaller values improve stability (codes FEA-style coupling).";
inline constexpr const char* SimHeatmap =
    "Color instances by stress ratio, temperature, or pass/fail danger flag.";
inline constexpr const char* SimSync =
    "Create SimulationEngine components for each assembly instance (mass, area).";
inline constexpr const char* SimRun =
    "Sync then advance physics for Duration using Timestep (stress + thermal).";
inline constexpr const char* SimForce =
    "Axial/load force in Newtons applied to the mapped simulation component.";
inline constexpr const char* SimHeat =
    "Thermal power in Watts; drives temperature rise vs material conductivity.";
inline constexpr const char* SimResultsTable =
    "Per-instance stress (MPa), temperature (C), load, and stability status.";

// --- Component Browser controls ---
inline constexpr const char* BrowserSearch = LibrarySearch;
inline constexpr const char* BrowserImportPath =
    "Full path to .step, .stp, or .geo file on disk.";
inline constexpr const char* BrowserImportStep =
    "Import ISO-10303 STEP solid for machining-style machine parts.";
inline constexpr const char* BrowserImportGeo =
    "Import GEO mesh from Gmsh/code8 workflows in codes/.";
inline constexpr const char* BrowserCreatePrimitive =
    "Add a simple solid to the library without external CAD.";
inline constexpr const char* BrowserPrimitiveType =
    "Box, cylinder, sphere, or cone — parametric placeholders for layout.";
inline constexpr const char* BrowserPlacementPos = AssemblyPosition;
inline constexpr const char* BrowserPlacementRot =
    "Rotation in degrees applied when adding the component to the scene.";
inline constexpr const char* BrowserPlacementScale =
    "Uniform scale factor for the placed instance.";
inline constexpr const char* BrowserAutoPlacement = AssemblyAutoPlacement;
inline constexpr const char* BrowserAddToScene = AssemblyAddPart;
inline constexpr const char* BrowserShowGrid = ViewportGrid;
inline constexpr const char* BrowserShowLabels = ViewportLabels;
inline constexpr const char* BrowserShowBounds =
    "Draw axis-aligned bounding boxes for each instance.";
inline constexpr const char* BrowserVizMode =
    "Solid, wireframe, or FEA-linked stress/thermal/displacement overlays.";
inline constexpr const char* BrowserHeatmapRange =
    "Clamp false-color scale for stress/thermal visualization.";

// --- Toolbar / workspaces ---
inline constexpr const char* ToolbarPlay =
    "Start or pause real-time simulation tick (Physics + SimulationEngine).";
inline constexpr const char* ToolbarReset =
    "Stop playback and reset speed to 1.0x.";
inline constexpr const char* ToolbarSpeed =
    "Multiplier on simulation delta-time per frame.";
inline constexpr const char* ToolbarStressOverlay =
    "Tint viewport by stress utilization (yield ratio from materials DB).";
inline constexpr const char* ToolbarTempOverlay =
    "Tint viewport by temperature ratio vs allowable limit.";
inline constexpr const char* ToolbarQuickBeam =
    "Add a steel beam component to the global simulation (demo template).";
inline constexpr const char* ToolbarQuickBracket =
    "Add an aluminum bracket component for thermal/stress demos.";

inline constexpr const char* WorkspaceOverview =
    "Dashboard, scene hierarchy, properties, visualizer, and AI console.";
inline constexpr const char* WorkspaceBuild =
    "3D workshop: library, assembly, part editor, viewport — machine design mode.";
inline constexpr const char* WorkspaceAnalyze =
    "Simulation controls, workshop FEA bridge, weight table, and results.";
inline constexpr const char* WorkspaceFull =
    "Show every docked panel for advanced multi-monitor setups.";

// --- Codes folder integrations (help markers) ---
inline constexpr const char* CodesBullet =
    "Optional rigid-body backend from codes/code 4 (Bullet3). Enable with -DSZM_ENABLE_BULLET_BACKEND=ON.";
inline constexpr const char* CodesThreeJs =
    "Web 3D preview from codes/three.js-dev; used when React webview is enabled.";
inline constexpr const char* CodesEntt =
    "Entity-component patterns from codes/entt-main for scalable scene data.";
inline constexpr const char* CodesFastApi =
    "REST simulation API (codes/fastapi-master) on port 8000 in lightweight mode.";
inline constexpr const char* CodesFEA =
    "Linear static FEA module (PCG solver) compatible with code8 mesh workflows.";

} // namespace SZM::WorkshopTooltips
