/**
 * Central tooltip catalog — used in Theme 1 (Classic) and Theme 2 (Modern).
 * References codes/ integrations: Bullet, Three.js, EnTT, FastAPI, FEA/code8.
 */

export const tooltips = {
  themes: {
    modern: 'Theme 2 — Modern unified workspace with 3D viewport, hierarchy, and analysis panels.',
    classic: 'Theme 1 — Classic dashboard with telemetry, stress, thermal, and manufacturing views.',
    switchModern: 'Switch to the modern engineering UI (recommended for machine design).',
    switchClassic: 'Switch to the classic command-center dashboard layout.',
  },
  workspace: {
    commandCenter: 'Command Center — executive overview for design, simulation, manufacturing, and SZM AI coordination.',
    engineering: 'Engineering — 3D modeling, assemblies, part library, and placement (Workshop3D).',
    simulation: 'Simulation — FEA stress/thermal runs linked to codes/ linear solver.',
    digitalTwin: 'Digital Twin — Live sensor telemetry and predictive health monitoring.',
    manufacturing: 'Manufacturing — DFM, scheduling, and production quality views.',
    aiAssistant: 'AI Engine — Design suggestions using local knowledge base + codes/ ML bundles.',
    fleetCommand: 'Fleet Command — Multi-asset monitoring across deployed machines.',
    designing: 'Designing — Dedicated sub-system layout and conceptual design (Body, Circuit, etc).',
    textToCAD: 'Text → CAD — Describe a part in natural language and generate a 3D geometry + STEP file.',
    knowledgeBase: 'Knowledge Base — Materials library and engineering reference (physics, manufacturing, standards).',
  },
  viewport: {
    shaded: 'Shaded — PBR-style solid rendering with lighting.',
    wireframe: 'Wireframe — Mesh edges only for inspection.',
    xray: 'X-Ray — Semi-transparent solids for internal geometry.',
    thermal: 'Thermal — Temperature field false-color (thermal engine).',
    stress: 'Stress — Von Mises utilization vs yield (FEA bridge).',
    electrical: 'Electrical — Current density / potential field overlay.',
    orbit: 'Orbit camera — drag with left mouse on viewport.',
    pan: 'Pan — middle mouse or shift+drag.',
    zoom: 'Zoom — scroll wheel or zoom buttons.',
    grid: 'Toggle engineering grid for machine layout.',
    axes: 'Show XYZ axes indicator.',
    measure: 'Measure distance between picked points.',
    section: 'Section cut — inspect internal assemblies.',
  },
  simulation: {
    run: 'Run simulation — advances stress + thermal solvers (Ctrl+Enter).',
    pause: 'Pause the active solver job.',
    reset: 'Reset loads, time, and visualization buffers.',
    syncApi: 'Sync with REST API on localhost:8000 (codes/fastapi-master).',
  },
  settings: {
    showTooltips: 'Show rich tooltips and (?) help markers across the application.',
    fontSize: 'Base UI font size for readability (both themes).',
    highContrast: 'Increase contrast for accessibility.',
    reducedMotion: 'Reduce animations (loading, pulses, transitions).',
    gridVisible: 'Display viewport grid in 3D views.',
    gpuAccel: 'Use GPU path when Bullet/graphics backends are enabled.',
    cloudSync: 'Sync project state to configured cloud endpoint.',
    codesBullet: 'Rigid-body physics from codes/code 4 (Bullet3). Enable via -DSZM_ENABLE_BULLET_BACKEND=ON.',
    codesThreeJs: 'WebGL preview from codes/three.js-dev (React viewport).',
    codesEntt: 'ECS scene data from codes/entt-main.',
    codesFastApi: 'Remote simulation control via FastAPI on port 8000.',
    codesFea: 'FEA meshes and code8 examples from codes/code 5–8.',
  },
  hierarchy: {
    panel: 'Project tree — assemblies, parts, constraints, materials, and simulations.',
    search: 'Filter hierarchy nodes by name or type.',
    filter: 'Filter visible node types in the tree.',
    part: 'Solid part in the machine assembly.',
    assembly: 'Sub-assembly grouping multiple parts.',
    simulation: 'Linked FEA or multi-physics study node.',
    sensor: 'Digital twin sensor binding.',
  },
  analysis: {
    panel: 'Live results from C++ SimulationEngine when backend is connected.',
    results: 'Stress, temperature, and safety status per component.',
    telemetry: 'Time-series sensor and solver telemetry.',
    ai: 'AI design insights (codes/ pytorch optional).',
    solver: 'Linear solver metrics — iteration count, residual, DOFs.',
  },
  properties: {
    panel: 'Inspect geometry, material, and simulation data for the selected node.',
    geometry: 'Volume, mass, bounds from mesh or parametric primitive.',
    material: 'Properties from SZM materials database.',
    simulation: 'Loads and boundary conditions on the selected part.',
  },
  backend: {
    connected: 'Linked to C++ WebAPI — live simulation data on localhost:8000.',
    offline: 'Demo mode — run ./build/SZM_Forge to enable live physics API.',
    refresh: 'Poll simulation status from the embedded HTTP server.',
  },
  legacy: {
    overview: 'Command Center — system overview and KPI dashboard.',
    stress: 'FEA stress and strain visualization.',
    thermal: 'Thermal distribution and heat flux.',
    electrical: 'Electrical network analysis view.',
    digitalTwin: 'Digital Twin — live sensor data and predictive monitoring.',
    manufacturing: 'Manufacturing — production and quality control.',
  },
} as const;

export type TooltipKey = typeof tooltips;
