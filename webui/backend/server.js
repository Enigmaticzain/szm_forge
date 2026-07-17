// SZM Forge integration backend
// Unifies jobs, workshop, assemblies, module status, scene data, and AI proxies.

const express = require('express');
const cors = require('cors');
const http = require('http');
const socketIo = require('socket.io');
const axios = require('axios');
const morgan = require('morgan');

const app = express();
const server = http.createServer(app);
const io = socketIo(server, {
  cors: {
    origin: process.env.CORS_ORIGIN || '*',
    methods: ['GET', 'POST', 'DELETE'],
  },
});

const PORT = Number(process.env.PORT || 8000);
const AI_SERVICE_URL = process.env.AI_SERVICE_URL || 'http://localhost:8001';
const STARTED_AT = Date.now();

app.use(cors());
app.use(express.json({ limit: '50mb' }));
app.use(morgan('dev'));

const jobQueue = new Map();
const resultCache = new Map();

const aiProbeCache = {
  checkedAt: 0,
  live: false,
  detail: null,
};

function cloneData(value) {
  return JSON.parse(JSON.stringify(value));
}

function replaceArrayContents(target, values) {
  target.splice(0, target.length, ...cloneData(values));
}

const initialMaterials = [
  {
    id: 'MAT-STEEL-STRUCT',
    name: 'Structural Steel',
    family: 'Metal',
    density: 7850,
    youngs_modulus: 200,
    poissons_ratio: 0.3,
    yield_strength: 250,
    ultimate_strength: 400,
    thermal_conductivity: 50,
    thermal_expansion: 1.2e-5,
    tags: ['Load bearing', 'Workshop default', 'Structural'],
  },
  {
    id: 'MAT-AL-6061-T6',
    name: '6061-T6 Aluminum',
    family: 'Metal',
    density: 2700,
    youngs_modulus: 68.9,
    poissons_ratio: 0.33,
    yield_strength: 276,
    ultimate_strength: 310,
    thermal_conductivity: 167,
    thermal_expansion: 2.32e-5,
    tags: ['Lightweight', 'Thermal path', 'Machined'],
  },
  {
    id: 'MAT-TI-6AL4V',
    name: 'Ti-6Al-4V Titanium',
    family: 'Metal',
    density: 4430,
    youngs_modulus: 113.8,
    poissons_ratio: 0.342,
    yield_strength: 880,
    ultimate_strength: 950,
    thermal_conductivity: 6.7,
    thermal_expansion: 8.6e-6,
    tags: ['High strength', 'Aerospace', 'Fatigue resistant'],
  },
  {
    id: 'MAT-CU-C110',
    name: 'C110 Copper',
    family: 'Metal',
    density: 8960,
    youngs_modulus: 117,
    poissons_ratio: 0.34,
    yield_strength: 69,
    ultimate_strength: 220,
    thermal_conductivity: 385,
    thermal_expansion: 1.68e-5,
    tags: ['Electrical', 'Thermal spreader', 'Conductive'],
  },
];

const materials = cloneData(initialMaterials);

const initialComponentCatalog = [
  {
    id: 'part_frame_base',
    name: 'Frame Base',
    category: 'Structure',
    materialId: 'MAT-STEEL-STRUCT',
    mass: 24.8,
    source: 'Workshop primitive',
    analysisModes: ['Structural', 'Mass Properties', 'Interference'],
    geometry: { type: 'box', args: [8, 0.7, 6], color: '#27476a' },
    description: 'Primary foundation plate for the actuation fixture.',
  },
  {
    id: 'part_left_tower',
    name: 'Left Tower',
    category: 'Support',
    materialId: 'MAT-AL-6061-T6',
    mass: 7.2,
    source: 'Workshop primitive',
    analysisModes: ['Structural', 'Motion'],
    geometry: { type: 'box', args: [1.1, 5.4, 1.2], color: '#d88432' },
    description: 'Left support column for shaft alignment.',
  },
  {
    id: 'part_right_tower',
    name: 'Right Tower',
    category: 'Support',
    materialId: 'MAT-AL-6061-T6',
    mass: 7.2,
    source: 'Workshop primitive',
    analysisModes: ['Structural', 'Motion'],
    geometry: { type: 'box', args: [1.1, 5.4, 1.2], color: '#d88432' },
    description: 'Right support column for shaft alignment.',
  },
  {
    id: 'part_bridge',
    name: 'Bridge Beam',
    category: 'Structure',
    materialId: 'MAT-STEEL-STRUCT',
    mass: 13.6,
    source: 'Code 8 import',
    analysisModes: ['Structural', 'Modal'],
    geometry: { type: 'box', args: [5.8, 0.8, 1.4], color: '#17324b' },
    description: 'Cross-member carrying payload and shaft loads.',
  },
  {
    id: 'part_drive_shaft',
    name: 'Drive Shaft',
    category: 'Motion',
    materialId: 'MAT-TI-6AL4V',
    mass: 4.9,
    source: 'STEP reference',
    analysisModes: ['Motion', 'Modal', 'Failure Prediction'],
    geometry: { type: 'cylinder', args: [0.45, 0.45, 4.2, 42], color: '#1f7d78' },
    description: 'High-strength shaft for actuation transfer.',
  },
  {
    id: 'part_sensor_pod',
    name: 'Sensor Pod',
    category: 'Controls',
    materialId: 'MAT-CU-C110',
    mass: 3.1,
    source: 'Library asset',
    analysisModes: ['Electrical', 'Thermal'],
    geometry: { type: 'sphere', args: [1.1, 40, 32], color: '#5f88b1' },
    description: 'Instrumentation package mounted above the bridge.',
  },
  {
    id: 'part_heat_sink',
    name: 'Heat Sink Cartridge',
    category: 'Thermal',
    materialId: 'MAT-AL-6061-T6',
    mass: 3.4,
    source: 'Library asset',
    analysisModes: ['Thermal', 'Optimization'],
    geometry: { type: 'box', args: [2.4, 0.8, 2], color: '#86a3b8' },
    description: 'Thermal management insert for power electronics.',
  },
  {
    id: 'part_busbar',
    name: 'Copper Busbar',
    category: 'Electrical',
    materialId: 'MAT-CU-C110',
    mass: 2.1,
    source: 'Library asset',
    analysisModes: ['Electrical', 'Thermal', 'Mass Properties'],
    geometry: { type: 'box', args: [3.1, 0.35, 0.5], color: '#99613f' },
    description: 'High-conductivity electrical link for routed power.',
  },
];

const componentCatalog = cloneData(initialComponentCatalog);

const initialAssemblies = [
  {
    id: 'assy_actuated_bracket',
    name: 'Actuated Bracket Study',
    description: 'Mixed-discipline fixture combining structure, motion, thermal, and electrical routing.',
    coordinateSystem: 'World Z Up',
    bounds: '540 x 260 x 180 mm',
    tags: ['Workshop ready', 'Simulation ready', 'Viewer synced'],
    active: true,
  },
];

const assemblies = cloneData(initialAssemblies);

const initialAssemblyInstances = [
  {
    id: 'inst_frame_base',
    assemblyId: 'assy_actuated_bracket',
    partId: 'part_frame_base',
    name: 'Frame Base',
    componentId: 4821,
    position: [0, -1.8, 0],
    rotation: [0, 0, 0],
  },
  {
    id: 'inst_left_tower',
    assemblyId: 'assy_actuated_bracket',
    partId: 'part_left_tower',
    name: 'Left Tower',
    componentId: 4822,
    position: [-2.2, 1.2, 0],
    rotation: [0, 0, 0],
  },
  {
    id: 'inst_right_tower',
    assemblyId: 'assy_actuated_bracket',
    partId: 'part_right_tower',
    name: 'Right Tower',
    componentId: 4823,
    position: [2.2, 1.2, 0],
    rotation: [0, 0, 0],
  },
  {
    id: 'inst_bridge',
    assemblyId: 'assy_actuated_bracket',
    partId: 'part_bridge',
    name: 'Bridge Beam',
    componentId: 4824,
    position: [0, 4.1, 0],
    rotation: [0, 0, 0],
  },
  {
    id: 'inst_drive_shaft',
    assemblyId: 'assy_actuated_bracket',
    partId: 'part_drive_shaft',
    name: 'Drive Shaft',
    componentId: 4825,
    position: [0, 4.2, 0],
    rotation: [0, 0, Math.PI / 2],
  },
  {
    id: 'inst_sensor_pod',
    assemblyId: 'assy_actuated_bracket',
    partId: 'part_sensor_pod',
    name: 'Sensor Pod',
    componentId: 4826,
    position: [0, 6.7, 0],
    rotation: [0, 0, 0],
  },
];

const assemblyInstances = cloneData(initialAssemblyInstances);

const simulationTemplates = [
  {
    id: 'structural_static',
    name: 'Structural Static',
    simulationType: 'Structural',
    module: 'FEA',
    target: 'assembly',
    priority: 'high',
    description: 'Routes assembly geometry through the structural readiness lane.',
    outputs: ['max stress', 'utilization', 'displacement'],
    defaultComponentId: 4821,
  },
  {
    id: 'thermal_map',
    name: 'Thermal Map',
    simulationType: 'Thermal',
    module: 'Thermal',
    target: 'assembly',
    priority: 'high',
    description: 'Evaluates hot spots, spread paths, and peak temperatures.',
    outputs: ['peak temperature', 'gradient', 'hot zones'],
    defaultComponentId: 4826,
  },
  {
    id: 'motion_study',
    name: 'Motion Study',
    simulationType: 'Motion',
    module: 'Assembly',
    target: 'assembly',
    priority: 'normal',
    description: 'Uses the assembly lane to review movement, range, and alignments.',
    outputs: ['travel envelope', 'collision risk', 'reach'],
    defaultComponentId: 4825,
  },
  {
    id: 'modal_sweep',
    name: 'Modal Sweep',
    simulationType: 'Modal',
    module: 'FEA',
    target: 'assembly',
    priority: 'normal',
    description: 'Targets natural frequencies and resonance risk.',
    outputs: ['first mode', 'frequency band', 'resonance risk'],
    defaultComponentId: 4824,
  },
  {
    id: 'interference_scan',
    name: 'Interference Check',
    simulationType: 'Interference',
    module: 'Assembly',
    target: 'assembly',
    priority: 'normal',
    description: 'Checks part clearances using assembly interference tooling.',
    outputs: ['contact pairs', 'minimum clearance'],
    defaultComponentId: 4822,
  },
  {
    id: 'mass_properties',
    name: 'Mass Properties',
    simulationType: 'Mass Properties',
    module: 'Workshop3D',
    target: 'assembly',
    priority: 'normal',
    description: 'Computes total mass, center of mass, and category split.',
    outputs: ['total mass', 'center of mass', 'material mix'],
    defaultComponentId: 4821,
  },
  {
    id: 'electrical_load',
    name: 'Electrical Load',
    simulationType: 'Electrical',
    module: 'Electrical',
    target: 'component',
    priority: 'medium',
    description: 'Reviews power routing, voltage drop, and thermal coupling.',
    outputs: ['voltage drop', 'power loss', 'current density'],
    defaultComponentId: 4826,
  },
  {
    id: 'failure_prediction',
    name: 'Failure Prediction',
    simulationType: 'Failure Prediction',
    module: 'AI Predictions',
    target: 'component',
    priority: 'medium',
    description: 'Uses the AI service contract for risk scoring and failure modes.',
    outputs: ['failure probability', 'risk drivers', 'recommended mitigations'],
    defaultComponentId: 4825,
  },
  {
    id: 'design_optimization',
    name: 'Design Optimization',
    simulationType: 'Optimization',
    module: 'AI',
    target: 'assembly',
    priority: 'medium',
    description: 'Creates a design-improvement loop across structure, mass, and heat.',
    outputs: ['weight delta', 'cost delta', 'suggested edits'],
    defaultComponentId: 4824,
  },
  {
    id: 'material_estimation',
    name: 'Material Estimation',
    simulationType: 'Material Estimation',
    module: 'AI',
    target: 'component',
    priority: 'low',
    description: 'Fills missing properties from known constraints and category hints.',
    outputs: ['estimated properties', 'confidence', 'recommended tests'],
    defaultComponentId: 4822,
  },
];

const moduleCatalog = [
  {
    id: '010_core',
    name: 'Core',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/Core',
    summary: 'Application lifecycle, event loop, startup splash, and tick orchestration.',
    capabilities: ['Window lifecycle', 'Event bus', 'Startup splash', 'System tick'],
  },
  {
    id: '020_ui',
    name: 'UI',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/UI',
    summary: 'Dockable panels, dashboards, and workshop surfaces.',
    capabilities: ['Dashboard panels', 'Properties panel', 'Simulation panel', 'Panel registry'],
  },
  {
    id: '030_input',
    name: 'Input',
    status: 'complete',
    exposure: 'desktop',
    sourcePath: 'src/Input',
    summary: 'Input manager, event definitions, and SpaceMouse hooks.',
    capabilities: ['Keyboard input', 'Mouse input', 'SpaceMouse driver'],
  },
  {
    id: '040_graphics',
    name: 'Graphics',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/Graphics',
    summary: 'Viewport, renderer, heatmaps, and advanced visualization adapters.',
    capabilities: ['Interactive viewport', 'Stress visualization', 'Thermal visualization', 'Selection'],
  },
  {
    id: '050_data',
    name: 'Data',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/Data',
    summary: 'Scene graph, identifiers, and state containers.',
    capabilities: ['Scene graph', 'UUID system', 'State mapping'],
  },
  {
    id: '060_math',
    name: 'Math / Constraints',
    status: 'complete',
    exposure: 'metadata',
    sourcePath: 'src/Math',
    summary: 'Constraint solver infrastructure from the integrated code drops.',
    capabilities: ['Distance constraints', 'Constraint solving', 'Readiness metrics'],
  },
  {
    id: '070_geometry',
    name: 'Geometry',
    status: 'partial',
    exposure: 'metadata',
    sourcePath: 'src/Geometry',
    summary: 'Solid bodies, booleans, extrude/revolve operations, and mesh generation.',
    capabilities: ['Boolean ops', 'Extrude', 'Revolve', 'Mesh generation'],
  },
  {
    id: '110_materials',
    name: 'Materials',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/Materials',
    summary: 'Thread-safe material registry backing simulations and workshop parts.',
    capabilities: ['Material library', 'Custom materials', 'Property lookup'],
  },
  {
    id: '120_assembly',
    name: 'Assembly',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/Assembly',
    summary: 'Mate constraints, kinematic solving, and interference hooks.',
    capabilities: ['Kinematic solver', 'Mate constraints', 'Interference check'],
  },
  {
    id: '130_physics',
    name: 'Physics',
    status: 'complete',
    exposure: 'desktop',
    sourcePath: 'src/Physics',
    summary: 'Physics engine plus Bullet integration and collision helpers.',
    capabilities: ['Rigid body backend', 'Collision detection', 'Physics adapter'],
  },
  {
    id: '300_thermal',
    name: 'Thermal',
    status: 'pending',
    exposure: 'job-template',
    sourcePath: 'src/Thermal',
    summary: 'Thermal solver contracts exist, and the app now surfaces them through launch templates and analysis pages.',
    capabilities: ['Thermal solver', 'Thermal maps', 'Heat loading'],
  },
  {
    id: '320_fea',
    name: 'FEA',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/FEA',
    summary: 'Sparse linear solve pipeline with meshing and stress post-processing.',
    capabilities: ['Linear static solve', 'Mesher', 'Stress calculation', 'Modal prep'],
  },
  {
    id: '400_electrical',
    name: 'Electrical',
    status: 'pending',
    exposure: 'job-template',
    sourcePath: 'src/Electrical',
    summary: 'Circuit solver APIs are present; the app now exposes them as queueable studies and system inventory.',
    capabilities: ['Circuit solve', 'PlatformIO bridge', 'Load review'],
  },
  {
    id: '500_cea_ai',
    name: 'AI',
    status: 'pending',
    exposure: 'proxy',
    sourcePath: 'src/AI',
    summary: 'Desktop AI hooks are still pending, but the web surface now exposes the AI service capability layer and legacy model endpoints.',
    capabilities: ['Knowledge graph', 'Reasoning', 'Training report', 'Optimization'],
  },
  {
    id: 'workshop3d',
    name: 'Workshop3D',
    status: 'complete',
    exposure: 'live',
    sourcePath: 'src/Workshop3D',
    summary: 'Part library, assembly management, visualization setup, and simulation staging.',
    capabilities: ['Part library', 'Assembly manager', 'Simulation setup', 'Component browser'],
  },
  {
    id: 'webviewer',
    name: 'WebViewer',
    status: 'headers-complete',
    exposure: 'metadata',
    sourcePath: 'src/WebViewer',
    summary: 'Three.js export and sync surfaces are defined but not fully implemented in C++ yet.',
    capabilities: ['Three.js renderer', 'Scene export', 'Web sync'],
  },
  {
    id: 'webapi',
    name: 'WebAPI',
    status: 'headers-complete',
    exposure: 'live',
    sourcePath: 'src/WebAPI',
    summary: 'The original C++ bridge remains partial, but the app now has a full web integration contract through this Node service.',
    capabilities: ['Jobs API', 'Analysis API', 'Material API', 'Scene API'],
  },
  {
    id: 'cad',
    name: 'CAD',
    status: 'headers-complete',
    exposure: 'metadata',
    sourcePath: 'src/CAD',
    summary: 'CAD import/export contracts exist and are now represented in the app surface and workshop workflow.',
    capabilities: ['STEP import', 'IGES import', 'STL import'],
  },
  {
    id: 'ai_predictions',
    name: 'AI Predictions',
    status: 'headers-complete',
    exposure: 'proxy',
    sourcePath: 'src/AI/Predictions',
    summary: 'Prediction headers exist, while the app uses the Python AI service endpoints as the live interaction layer.',
    capabilities: ['Failure predictor', 'Thermal predictor', 'Design optimizer'],
  },
];

function clamp(value, min, max) {
  return Math.min(Math.max(value, min), max);
}

function round(value, digits = 2) {
  return Number(value.toFixed(digits));
}

function makeId(prefix) {
  return `${prefix}_${Date.now()}_${Math.random().toString(36).slice(2, 8)}`;
}

function slugify(value) {
  return value
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, '_')
    .replace(/^_+|_+$/g, '');
}

function toFiniteNumber(value, fallback = 0) {
  const parsed = Number(value);
  return Number.isFinite(parsed) ? parsed : fallback;
}

function makeMaterialId(name) {
  return `MAT-${slugify(name).replace(/_/g, '-').toUpperCase()}`;
}

function normalizeTags(value) {
  const rawValues = Array.isArray(value) ? value : String(value || '').split(',');
  return Array.from(
    new Set(
      rawValues
        .map((tag) => String(tag).trim())
        .filter(Boolean),
    ),
  ).slice(0, 8);
}

function getMaterialById(materialId) {
  return materials.find((material) => material.id === materialId) || null;
}

function getComponentById(componentId) {
  return componentCatalog.find((component) => component.id === componentId) || null;
}

function getActiveAssembly() {
  return assemblies.find((assembly) => assembly.active) || assemblies[0] || null;
}

function getAssemblyInstances(assemblyId) {
  return assemblyInstances.filter((instance) => instance.assemblyId === assemblyId);
}

function addVectors(left, right) {
  return [
    (left?.[0] || 0) + (right?.[0] || 0),
    (left?.[1] || 0) + (right?.[1] || 0),
    (left?.[2] || 0) + (right?.[2] || 0),
  ];
}

function subtractVectors(left, right) {
  return [
    (left?.[0] || 0) - (right?.[0] || 0),
    (left?.[1] || 0) - (right?.[1] || 0),
    (left?.[2] || 0) - (right?.[2] || 0),
  ];
}

function vectorLength(vector) {
  return Math.sqrt(
    ((vector?.[0] || 0) ** 2)
      + ((vector?.[1] || 0) ** 2)
      + ((vector?.[2] || 0) ** 2),
  );
}

function normalizeVector(vector, fallback = [0, 1, 0]) {
  const length = vectorLength(vector);
  if (length < 1e-6) {
    return normalizeVector(fallback, [0, 1, 0]);
  }

  return vector.map((value) => value / length);
}

function roundVector(vector, digits = 3) {
  return vector.map((value) => round(value, digits));
}

function rotatePoint(point, rotation = [0, 0, 0]) {
  const [rx = 0, ry = 0, rz = 0] = rotation;
  const [x = 0, y = 0, z = 0] = point;

  const cx = Math.cos(rx);
  const sx = Math.sin(rx);
  const cy = Math.cos(ry);
  const sy = Math.sin(ry);
  const cz = Math.cos(rz);
  const sz = Math.sin(rz);

  const x1 = x;
  const y1 = y * cx - z * sx;
  const z1 = y * sx + z * cx;

  const x2 = x1 * cy + z1 * sy;
  const y2 = y1;
  const z2 = -x1 * sy + z1 * cy;

  return [
    x2 * cz - y2 * sz,
    x2 * sz + y2 * cz,
    z2,
  ];
}

function geometryHalfExtents(geometry = {}) {
  const args = Array.isArray(geometry.args) ? geometry.args.map((value) => Math.abs(Number(value) || 0)) : [];

  switch (geometry.type) {
    case 'sphere': {
      const radius = args[0] || 1;
      return [radius, radius, radius];
    }
    case 'cylinder': {
      const radius = Math.max(args[0] || 0.5, args[1] || 0.5);
      const halfHeight = (args[2] || 1.5) / 2;
      return [radius, halfHeight, radius];
    }
    case 'box':
    default:
      return [
        (args[0] || 1) / 2,
        (args[1] || 1) / 2,
        (args[2] || 1) / 2,
      ];
  }
}

function computePartBounds(part) {
  const halfExtents = geometryHalfExtents(part);
  const [hx, hy, hz] = halfExtents;
  const localCorners = [
    [-hx, -hy, -hz],
    [hx, -hy, -hz],
    [hx, hy, -hz],
    [-hx, hy, -hz],
    [-hx, -hy, hz],
    [hx, -hy, hz],
    [hx, hy, hz],
    [-hx, hy, hz],
  ];

  const position = Array.isArray(part.position) ? part.position : [0, 0, 0];
  const rotation = Array.isArray(part.rotation) ? part.rotation : [0, 0, 0];
  const worldCorners = localCorners.map((corner) => addVectors(rotatePoint(corner, rotation), position));

  const min = [Infinity, Infinity, Infinity];
  const max = [-Infinity, -Infinity, -Infinity];

  worldCorners.forEach((corner) => {
    for (let axis = 0; axis < 3; axis += 1) {
      min[axis] = Math.min(min[axis], corner[axis]);
      max[axis] = Math.max(max[axis], corner[axis]);
    }
  });

  const size = max.map((value, axis) => value - min[axis]);
  const center = min.map((value, axis) => value + size[axis] / 2);

  return {
    min: roundVector(min),
    max: roundVector(max),
    size: roundVector(size),
    center: roundVector(center),
  };
}

function buildSceneSpatial(parts) {
  if (!parts.length) {
    return {
      min: [0, 0, 0],
      max: [0, 0, 0],
      size: [0, 0, 0],
      center: [0, 0, 0],
      radius: 0,
    };
  }

  const min = [Infinity, Infinity, Infinity];
  const max = [-Infinity, -Infinity, -Infinity];

  parts.forEach((part) => {
    const bounds = part.bounds || computePartBounds(part);
    for (let axis = 0; axis < 3; axis += 1) {
      min[axis] = Math.min(min[axis], bounds.min[axis]);
      max[axis] = Math.max(max[axis], bounds.max[axis]);
    }
  });

  const size = max.map((value, axis) => value - min[axis]);
  const center = min.map((value, axis) => value + size[axis] / 2);
  const radius = Math.sqrt(size.reduce((sum, value) => sum + value ** 2, 0)) / 2;

  return {
    min: roundVector(min),
    max: roundVector(max),
    size: roundVector(size),
    center: roundVector(center),
    radius: round(radius, 3),
  };
}

function buildAssemblySummary(assembly) {
  if (!assembly) {
    return null;
  }

  const instances = getAssemblyInstances(assembly.id);
  const mass = instances.reduce((total, instance) => {
    const component = getComponentById(instance.partId);
    return total + (component?.mass || 0);
  }, 0);

  const categories = new Set();
  const materialsUsed = new Set();
  instances.forEach((instance) => {
    const component = getComponentById(instance.partId);
    if (component?.category) {
      categories.add(component.category);
    }
    if (component?.materialId) {
      materialsUsed.add(component.materialId);
    }
  });

  return {
    ...assembly,
    instanceCount: instances.length,
    totalMass: round(mass, 1),
    categories: Array.from(categories),
    materials: Array.from(materialsUsed),
    simulationModes: Array.from(
      new Set(
        instances.flatMap((instance) => getComponentById(instance.partId)?.analysisModes || []),
      ),
    ),
    instances: instances.map((instance) => {
      const component = getComponentById(instance.partId);
      return {
        ...instance,
        partName: component?.name || 'Unknown part',
        category: component?.category || 'Unknown',
        materialId: component?.materialId || null,
        mass: component?.mass || 0,
      };
    }),
  };
}

function buildScene() {
  const assembly = getActiveAssembly();
  const summary = buildAssemblySummary(assembly);

  if (!summary) {
    return {
      title: 'Empty assembly',
      bounds: '0 x 0 x 0 mm',
      coordinateSystem: 'World Z Up',
      tags: ['No active assembly'],
      parts: [],
    };
  }

  const parts = summary.instances.map((instance, index) => {
    const component = getComponentById(instance.partId);
    const material = getMaterialById(component?.materialId);
    const geometry = component?.geometry || { type: 'box', args: [1, 1, 1], color: '#17324b' };

    const part = {
      id: instance.id,
      componentId: instance.componentId,
      name: instance.partName,
      category: instance.category,
      type: geometry.type,
      args: geometry.args,
      position: instance.position,
      rotation: instance.rotation,
      color: geometry.color,
      materialId: component?.materialId || null,
      materialName: material?.name || 'Unknown material',
      mass: round(component?.mass || 0, 2),
      source: component?.source || 'Unknown source',
      analysisModes: component?.analysisModes || [],
      description: component?.description || '',
      sortOrder: index,
    };

    part.bounds = computePartBounds(part);
    return part;
  });

  const spatial = buildSceneSpatial(parts);
  const categories = new Set();
  const analysisModes = new Set();
  const materialIds = new Set();

  parts.forEach((part, index) => {
    if (part.category) {
      categories.add(part.category);
    }
    if (part.materialId) {
      materialIds.add(part.materialId);
    }
    part.analysisModes.forEach((mode) => analysisModes.add(mode));

    const fallbackDirection = [
      Math.cos(index * 1.618),
      0.25 + ((index % 3) * 0.15),
      Math.sin(index * 1.618),
    ];
    const explodeDirection = normalizeVector(
      subtractVectors(part.bounds.center, spatial.center),
      fallbackDirection,
    );
    part.explodeDirection = roundVector(explodeDirection);
  });

  return {
    title: summary.name,
    bounds: summary.bounds,
    coordinateSystem: summary.coordinateSystem,
    tags: summary.tags,
    metrics: {
      partCount: parts.length,
      totalMass: round(summary.totalMass || 0, 1),
      materialCount: materialIds.size,
      categoryCount: categories.size,
      analysisModeCount: analysisModes.size,
    },
    spatial,
    parts,
  };
}

function buildFeatureCatalog() {
  return moduleCatalog.flatMap((module) =>
    module.capabilities.map((capability) => ({
      id: `${module.id}:${slugify(capability)}`,
      moduleId: module.id,
      module: module.name,
      status: module.status,
      exposure: module.exposure,
      capability,
      sourcePath: module.sourcePath,
    })),
  );
}

function buildSystemStats() {
  const jobs = Array.from(jobQueue.values());
  const runningJobs = jobs.filter((job) => job.status === 'running').length;
  const pendingJobs = jobs.filter((job) => job.status === 'pending').length;
  const completedJobs = jobs.filter((job) => job.status === 'completed').length;

  return {
    queueDepth: pendingJobs,
    activeJobs: runningJobs,
    completedToday: completedJobs,
    totalJobs: jobs.length,
    cacheSize: resultCache.size,
    uptime: Math.floor((Date.now() - STARTED_AT) / 1000),
    moduleCount: moduleCatalog.length,
    partCount: componentCatalog.length,
    assemblyCount: assemblies.length,
  };
}

function buildAnalysisOverview() {
  const stats = buildSystemStats();
  const jobs = Array.from(jobQueue.values());
  const completionRate = stats.totalJobs
    ? Math.round((stats.completedToday / stats.totalJobs) * 100)
    : 0;
  const integratedModules = moduleCatalog.filter((module) => module.exposure !== 'metadata').length;
  const structuralJobs = jobs.filter((job) => job.simulationType === 'Structural').length;
  const thermalJobs = jobs.filter((job) => job.simulationType === 'Thermal').length;
  const motionJobs = jobs.filter((job) => job.simulationType === 'Motion').length;

  const readiness = [
    {
      id: 'structural',
      label: 'Structural studies',
      value: clamp(42 + structuralJobs * 16 + componentCatalog.length * 2, 0, 96),
    },
    {
      id: 'thermal',
      label: 'Thermal studies',
      value: clamp(36 + thermalJobs * 18 + materials.length * 3, 0, 92),
    },
    {
      id: 'motion',
      label: 'Motion studies',
      value: clamp(38 + motionJobs * 20 + assemblies.length * 8, 0, 90),
    },
    {
      id: 'integration',
      label: 'Integrated module coverage',
      value: Math.round((integratedModules / moduleCatalog.length) * 100),
    },
  ];

  const topMaterials = [...materials]
    .sort((left, right) => right.yield_strength / right.density - left.yield_strength / left.density)
    .slice(0, 4);

  return {
    metrics: {
      completionRate,
      materialCoverage: materials.length,
      activeStudyLanes: new Set(jobs.map((job) => job.simulationType)).size,
      queuePressure: stats.queueDepth,
    },
    readiness,
    topMaterials,
    recommendations: [
      {
        id: 'rec_thermal_followup',
        title: 'Run a thermal follow-up on conductive components',
        detail: 'Copper and aluminum parts are present in the active assembly, but only part of the thermal lane is currently exercised.',
        status: 'pending',
      },
      {
        id: 'rec_review_completed',
        title: 'Promote completed studies into review',
        detail: 'Completed results are now visible in the queue and viewer, so the next leverage point is faster review routing.',
        status: 'running',
      },
      {
        id: 'rec_expand_pending_modules',
        title: 'Close the gap on pending module hooks',
        detail: 'Thermal, electrical, and desktop AI tick hooks are represented in the app, but still rely on staged or proxy execution.',
        status: 'warning',
      },
    ],
  };
}

function buildDashboardOverview(services) {
  const stats = buildSystemStats();
  const activeAssembly = buildAssemblySummary(getActiveAssembly());
  const jobs = Array.from(jobQueue.values())
    .sort((left, right) => new Date(right.createdAt).getTime() - new Date(left.createdAt).getTime())
    .slice(0, 6);

  const exposureSummary = moduleCatalog.reduce(
    (summary, module) => {
      summary[module.exposure] = (summary[module.exposure] || 0) + 1;
      return summary;
    },
    {},
  );

  return {
    stats,
    activeAssembly,
    recentJobs: jobs,
    featureHighlights: [
      {
        id: 'highlight_workshop',
        title: 'Workshop + assembly flow',
        route: '/workshop',
        detail: 'Create or browse parts, add them into the active assembly, and feed the viewer directly.',
      },
      {
        id: 'highlight_jobs',
        title: 'Simulation launch templates',
        route: '/jobs',
        detail: 'Structural, thermal, motion, AI, interference, and electrical studies are launchable from one lane.',
      },
      {
        id: 'highlight_ai',
        title: 'AI reasoning surface',
        route: '/ai',
        detail: 'Knowledge graph, reasoning, optimization, and failure prediction are visible through one app route.',
      },
      {
        id: 'highlight_systems',
        title: 'Module integration map',
        route: '/systems',
        detail: 'Track what is live, proxied, desktop-only, or still metadata-backed.',
      },
    ],
    moduleSummary: {
      complete: moduleCatalog.filter((module) => module.status === 'complete').length,
      partial: moduleCatalog.filter((module) => module.status === 'partial').length,
      pending: moduleCatalog.filter((module) => module.status === 'pending').length,
      headersComplete: moduleCatalog.filter((module) => module.status === 'headers-complete').length,
      exposure: exposureSummary,
    },
    serviceSummary: {
      healthy: services.filter((service) => service.status === 'healthy').length,
      degraded: services.filter((service) => service.status === 'degraded').length,
      metadata: services.filter((service) => service.status === 'metadata').length,
    },
  };
}

function buildWorkshopOverview() {
  return {
    activeAssembly: buildAssemblySummary(getActiveAssembly()),
    assemblies: assemblies.map((assembly) => buildAssemblySummary(assembly)),
    materials,
    components: componentCatalog.map((component) => ({
      ...component,
      material: getMaterialById(component.materialId),
    })),
    importFormats: ['STEP', 'GEO', 'Primitive'],
    templates: simulationTemplates.filter((template) =>
      ['Structural', 'Thermal', 'Motion', 'Interference', 'Mass Properties'].includes(
        template.simulationType,
      ),
    ),
    settings: {
      autoSave: true,
      snapToGrid: false,
      gridSize: 0.1,
      defaultMaterial: 'MAT-STEEL-STRUCT',
      physicsPreview: false,
    },
  };
}

function buildJobResult(job) {
  const template = simulationTemplates.find((item) => item.id === job.templateId);
  const assembly = buildAssemblySummary(getActiveAssembly());
  const materialCount = materials.length;
  const targetInstance = assembly?.instances?.find((instance) => instance.componentId === job.componentId);
  const targetName = targetInstance?.partName || job.componentName || assembly?.name || 'Active assembly';

  switch (template?.simulationType) {
    case 'Structural':
      return {
        success: true,
        target: targetName,
        maxStressMPa: 186,
        utilization: 0.74,
        maxDisplacementMm: 1.8,
        assembly: assembly?.name,
      };
    case 'Thermal':
      return {
        success: true,
        target: targetName,
        peakTemperatureC: 82,
        averageTemperatureC: 56,
        hotspots: ['Sensor Pod', 'Heat Sink Cartridge'],
      };
    case 'Motion':
      return {
        success: true,
        target: targetName,
        travelDegrees: 38,
        clearanceMm: 4.2,
        collisionRisk: 'low',
      };
    case 'Modal':
      return {
        success: true,
        target: targetName,
        firstModeHz: 113,
        resonanceRisk: 'moderate',
        dominantRegion: 'Bridge Beam',
      };
    case 'Interference':
      return {
        success: true,
        target: targetName,
        minimumClearanceMm: 2.7,
        contactsFound: 0,
      };
    case 'Mass Properties':
      return {
        success: true,
        target: targetName,
        totalMassKg: assembly?.totalMass || 0,
        centerOfMass: [0.2, 1.9, 0.1],
        materialFamilies: materialCount,
      };
    case 'Electrical':
      return {
        success: true,
        target: targetName,
        voltageDropV: 0.18,
        powerLossW: 3.6,
        currentDensityAmm2: 4.9,
      };
    case 'Failure Prediction':
      return {
        success: true,
        target: targetName,
        probability: 0.18,
        riskDrivers: ['shaft fatigue', 'support misalignment'],
        recommendation: 'Increase inspection cadence near the shaft support pair.',
      };
    case 'Optimization':
      return {
        success: true,
        target: targetName,
        weightReductionPercent: 9.4,
        stiffnessDeltaPercent: -1.8,
        recommendedChanges: ['lighten bridge beam pockets', 'revise tower wall thickness'],
      };
    case 'Material Estimation':
      return {
        success: true,
        target: targetName,
        predictedYoungsModulusGPa: 72,
        predictedYieldStrengthMPa: 295,
        confidence: 0.81,
      };
    default:
      return {
        success: true,
        target: targetName,
        completed: true,
      };
  }
}

function suggestPlacement(index) {
  const column = index % 4;
  const row = Math.floor(index / 4);
  return [column * 2.4 - 3.6, 0.6 + row * 0.25, row * 1.8];
}

function volumeForShape(shape, payload) {
  if (shape === 'sphere') {
    const radius = Number(payload.radius || payload.size?.[0] || 0.8);
    return (4 / 3) * Math.PI * radius ** 3;
  }
  if (shape === 'cylinder') {
    const radius = Number(payload.radius || payload.size?.[0] || 0.45);
    const height = Number(payload.height || payload.size?.[2] || 2.2);
    return Math.PI * radius * radius * height;
  }

  const size = Array.isArray(payload.size) ? payload.size : [1.4, 0.8, 1];
  return Number(size[0]) * Number(size[1]) * Number(size[2]);
}

function geometryForPayload(shape, payload) {
  if (shape === 'sphere') {
    const radius = Number(payload.radius || payload.size?.[0] || 0.8);
    return {
      type: 'sphere',
      args: [radius, 32, 24],
      color: payload.color || '#5f88b1',
    };
  }

  if (shape === 'cylinder') {
    const radius = Number(payload.radius || payload.size?.[0] || 0.45);
    const height = Number(payload.height || payload.size?.[2] || 2.4);
    return {
      type: 'cylinder',
      args: [radius, radius, height, 32],
      color: payload.color || '#1f7d78',
    };
  }

  const size = Array.isArray(payload.size) ? payload.size : [1.6, 0.8, 1.2];
  return {
    type: 'box',
    args: [Number(size[0]), Number(size[1]), Number(size[2])],
    color: payload.color || '#17324b',
  };
}

async function probeAiService(force = false) {
  const maxAgeMs = 15000;
  if (!force && Date.now() - aiProbeCache.checkedAt < maxAgeMs) {
    return aiProbeCache;
  }

  try {
    const response = await axios.get(`${AI_SERVICE_URL}/health`, { timeout: 1500 });
    aiProbeCache.checkedAt = Date.now();
    aiProbeCache.live = true;
    aiProbeCache.detail = response.data;
  } catch (error) {
    aiProbeCache.checkedAt = Date.now();
    aiProbeCache.live = false;
    aiProbeCache.detail = {
      status: 'offline',
      service: 'SZM_Forge_AI_v2',
      reason: error.message,
    };
  }

  return aiProbeCache;
}

async function tryAiGet(path) {
  try {
    const response = await axios.get(`${AI_SERVICE_URL}${path}`, { timeout: 2000 });
    return response.data;
  } catch (error) {
    return null;
  }
}

async function tryAiPost(path, payload) {
  try {
    const response = await axios.post(`${AI_SERVICE_URL}${path}`, payload, { timeout: 2500 });
    return response.data;
  } catch (error) {
    return null;
  }
}

async function buildServicesOverview() {
  const aiProbe = await probeAiService();

  return [
    {
      id: 'service_node_backend',
      name: 'Node integration backend',
      status: 'healthy',
      exposure: 'web-live',
      summary: 'Serves the unified app contract for jobs, materials, scene, workshop, and system inventory.',
      routeCount: 16,
    },
    {
      id: 'service_job_queue',
      name: 'Simulation queue',
      status: 'healthy',
      exposure: 'web-live',
      summary: 'In-memory queue simulating structural, thermal, motion, electrical, and AI job lanes.',
      routeCount: 3,
    },
    {
      id: 'service_workshop',
      name: 'Workshop integration layer',
      status: 'healthy',
      exposure: 'web-live',
      summary: 'Part library, assembly state, component creation, and scene sync are exposed through the app.',
      routeCount: 4,
    },
    {
      id: 'service_ai_proxy',
      name: 'AI service proxy',
      status: aiProbe.live ? 'healthy' : 'degraded',
      exposure: 'proxy',
      summary: aiProbe.live
        ? 'Proxying live responses from the FastAPI AI service.'
        : 'AI service unavailable right now, so the app is serving local fallback reasoning and capability metadata.',
      routeCount: 4,
    },
    {
      id: 'service_webviewer_bridge',
      name: 'WebViewer bridge',
      status: 'metadata',
      exposure: 'metadata',
      summary: 'C++ WebViewer hooks are still headers-first, but the app exposes live scene data and viewer state.',
      routeCount: 1,
    },
    {
      id: 'service_cad_bridge',
      name: 'CAD import bridge',
      status: 'metadata',
      exposure: 'metadata',
      summary: 'CAD contracts are represented in workshop metadata and import affordances, pending deeper backend execution.',
      routeCount: 1,
    },
  ];
}

async function buildAiOverview() {
  const aiProbe = await probeAiService();
  const remoteModels = aiProbe.live ? await tryAiGet('/api/models') : null;
  const status = aiProbe.live ? 'healthy' : 'degraded';

  return {
    status,
    service: aiProbe.live ? 'SZM Forge AI v2.0' : 'Fallback reasoning mode',
    transport: aiProbe.live ? 'proxy-live' : 'local-fallback',
    models:
      remoteModels?.models || {
        material_estimator: 'PyTorch neural network',
        failure_predictor: 'Random forest',
        design_optimizer: 'Genetic algorithm + gradient descent',
        knowledge_graph: 'SZM Forge ontology v2.0',
        causal_reasoner: 'Logic-based inference engine',
        agentic_system: 'Planning and tool orchestration',
      },
    capabilities: [
      {
        title: 'Knowledge graph queries',
        endpoint: '/api/kg/query',
        status: aiProbe.live ? 'healthy' : 'degraded',
      },
      {
        title: 'Causal reasoning',
        endpoint: '/api/reason',
        status: aiProbe.live ? 'healthy' : 'degraded',
      },
      {
        title: 'Design problem solving',
        endpoint: '/api/solve-design-problem',
        status: aiProbe.live ? 'healthy' : 'degraded',
      },
      {
        title: 'Failure prediction',
        endpoint: '/api/predict-failure',
        status: aiProbe.live ? 'healthy' : 'degraded',
      },
      {
        title: 'Design optimization',
        endpoint: '/api/optimize-design',
        status: aiProbe.live ? 'healthy' : 'degraded',
      },
    ],
    recentAiJobs: Array.from(jobQueue.values()).filter((job) =>
      ['Failure Prediction', 'Optimization', 'Material Estimation'].includes(job.simulationType),
    ),
  };
}

function buildFallbackReasoning(problem, mode) {
  return {
    mode,
    verdict: 'FEASIBLE WITH FOLLOW-UPS',
    problem,
    derived_facts: [
      'assembly contains multi-material load path',
      'structural and thermal lanes should both be exercised',
      'AI recommendations should be validated against completed studies',
    ],
    next_actions: [
      'Run structural static and thermal map on the active assembly',
      'Review shaft fatigue risk if motion or modal studies surface oscillation',
      'Promote successful runs into the viewer and analysis workspace',
    ],
  };
}

function buildFallbackDesignSolution(problemDescription, objectives = []) {
  return {
    status: 'fallback',
    problem: problemDescription,
    objectives,
    proposal: {
      summary: 'Reduce bridge mass while preserving shaft support stiffness.',
      recommended_changes: [
        'pocket the bridge beam to lower mass',
        'keep tower stiffness high near shaft supports',
        'shift heat-sensitive electronics away from the bridge top surface',
      ],
      follow_up_studies: ['Structural Static', 'Thermal Map', 'Failure Prediction'],
    },
  };
}

function normalizeJob(job) {
  const targetInstance = assemblyInstances.find((instance) => instance.componentId === job.componentId);

  return {
    ...job,
    componentName: job.componentName || targetInstance?.name || null,
    progress: round(job.progress || 0, 1),
  };
}

function seedJobs() {
  const seededJobs = [
    {
      jobId: 'job_seed_structural',
      templateId: 'structural_static',
      simulationType: 'Structural',
      componentId: 4821,
      assemblyId: 'assy_actuated_bracket',
      status: 'running',
      progress: 74,
      createdAt: new Date(Date.now() - 8 * 60 * 1000).toISOString(),
    },
    {
      jobId: 'job_seed_thermal',
      templateId: 'thermal_map',
      simulationType: 'Thermal',
      componentId: 4826,
      assemblyId: 'assy_actuated_bracket',
      status: 'pending',
      progress: 18,
      createdAt: new Date(Date.now() - 19 * 60 * 1000).toISOString(),
    },
    {
      jobId: 'job_seed_motion',
      templateId: 'motion_study',
      simulationType: 'Motion',
      componentId: 4825,
      assemblyId: 'assy_actuated_bracket',
      status: 'completed',
      progress: 100,
      createdAt: new Date(Date.now() - 42 * 60 * 1000).toISOString(),
    },
    {
      jobId: 'job_seed_modal',
      templateId: 'modal_sweep',
      simulationType: 'Modal',
      componentId: 4824,
      assemblyId: 'assy_actuated_bracket',
      status: 'failed',
      progress: 61,
      error: 'Resonance threshold crossed during sweep.',
      createdAt: new Date(Date.now() - 86 * 60 * 1000).toISOString(),
    },
    {
      jobId: 'job_seed_failure',
      templateId: 'failure_prediction',
      simulationType: 'Failure Prediction',
      componentId: 4825,
      assemblyId: 'assy_actuated_bracket',
      status: 'completed',
      progress: 100,
      createdAt: new Date(Date.now() - 115 * 60 * 1000).toISOString(),
    },
  ];

  seededJobs.forEach((job) => {
    const normalized = normalizeJob({
      ...job,
      result: job.status === 'completed' ? buildJobResult(job) : null,
    });

    jobQueue.set(normalized.jobId, normalized);
    if (normalized.result) {
      resultCache.set(normalized.jobId, normalized.result);
    }
  });
}

seedJobs();

function resetState() {
  stopJobWorker();
  replaceArrayContents(materials, initialMaterials);
  replaceArrayContents(componentCatalog, initialComponentCatalog);
  replaceArrayContents(assemblies, initialAssemblies);
  replaceArrayContents(assemblyInstances, initialAssemblyInstances);
  jobQueue.clear();
  resultCache.clear();
  aiProbeCache.checkedAt = 0;
  aiProbeCache.live = false;
  aiProbeCache.detail = null;
  seedJobs();
}

app.get('/health', (req, res) => {
  res.json({ status: 'healthy', service: 'SZM_Forge_Backend' });
});

app.get('/api/stats', (req, res) => {
  res.json(buildSystemStats());
});

app.get('/api/dashboard/overview', async (req, res) => {
  const services = await buildServicesOverview();
  res.json(buildDashboardOverview(services));
});

app.get('/api/materials', (req, res) => {
  const filter = String(req.query.filter || '').toLowerCase();
  const family = String(req.query.family || '').toLowerCase();

  const filtered = materials.filter((material) => {
    const matchesFilter = !filter
      || material.name.toLowerCase().includes(filter)
      || material.tags.some((tag) => tag.toLowerCase().includes(filter));
    const matchesFamily = !family || material.family.toLowerCase() === family;
    return matchesFilter && matchesFamily;
  });

  res.json(filtered);
});

app.post('/api/materials', (req, res) => {
  const name = String(req.body.name || '').trim();
  const family = String(req.body.family || 'Custom').trim() || 'Custom';
  const density = toFiniteNumber(req.body.density);
  const youngsModulus = toFiniteNumber(req.body.youngs_modulus);
  const poissonsRatio = toFiniteNumber(req.body.poissons_ratio, 0.3);
  const yieldStrength = toFiniteNumber(req.body.yield_strength);
  const ultimateStrength = toFiniteNumber(req.body.ultimate_strength, yieldStrength);
  const thermalConductivity = toFiniteNumber(req.body.thermal_conductivity);
  const thermalExpansion = toFiniteNumber(req.body.thermal_expansion);
  const tags = normalizeTags(req.body.tags);

  if (!name) {
    return res.status(400).json({ error: 'Material name is required' });
  }

  if (materials.some((material) => material.name.toLowerCase() === name.toLowerCase())) {
    return res.status(409).json({ error: 'A material with that name already exists' });
  }

  if (density <= 0 || youngsModulus <= 0 || yieldStrength <= 0) {
    return res.status(400).json({
      error: 'Density, Young\'s modulus, and yield strength must be positive numbers',
    });
  }

  const baseId = makeMaterialId(name);
  const materialId = materials.some((material) => material.id === baseId)
    ? `${baseId}-${materials.length + 1}`
    : baseId;

  const material = {
    id: materialId,
    name,
    family,
    density,
    youngs_modulus: youngsModulus,
    poissons_ratio: clamp(poissonsRatio, 0, 0.499),
    yield_strength: yieldStrength,
    ultimate_strength: Math.max(ultimateStrength, yieldStrength),
    thermal_conductivity: Math.max(0, thermalConductivity),
    thermal_expansion: Math.max(0, thermalExpansion),
    tags: tags.length > 0 ? tags : ['Custom', family],
  };

  materials.push(material);
  io.emit('material:created', material);

  return res.status(201).json({
    status: 'created',
    material,
  });
});

app.get('/api/materials/:materialId', (req, res) => {
  const material = getMaterialById(req.params.materialId);
  if (!material) {
    return res.status(404).json({ error: 'Material not found' });
  }
  return res.json(material);
});

app.get('/api/components', (req, res) => {
  const query = String(req.query.q || '').toLowerCase();
  const category = String(req.query.category || '').toLowerCase();

  const filtered = componentCatalog
    .filter((component) => {
      const matchesQuery = !query
        || component.name.toLowerCase().includes(query)
        || component.description.toLowerCase().includes(query);
      const matchesCategory = !category || component.category.toLowerCase() === category;
      return matchesQuery && matchesCategory;
    })
    .map((component) => ({
      ...component,
      material: getMaterialById(component.materialId),
    }));

  res.json(filtered);
});

app.post('/api/components', (req, res) => {
  const name = String(req.body.name || '').trim();
  const shape = String(req.body.shape || 'box').toLowerCase();
  const materialId = String(req.body.materialId || 'MAT-STEEL-STRUCT');
  const material = getMaterialById(materialId);

  if (!name) {
    return res.status(400).json({ error: 'Component name is required' });
  }

  if (!material) {
    return res.status(400).json({ error: 'Unknown material' });
  }

  const geometry = geometryForPayload(shape, req.body);
  const volume = volumeForShape(shape, req.body);
  const component = {
    id: `${slugify(name)}_${Date.now()}`,
    name,
    category: String(req.body.category || 'Custom'),
    materialId,
    mass: round((volume * material.density) / 1000, 2),
    source: 'Web workshop',
    analysisModes: Array.isArray(req.body.analysisModes) && req.body.analysisModes.length > 0
      ? req.body.analysisModes
      : ['Structural', 'Mass Properties'],
    geometry,
    description: String(req.body.description || 'Component created from the web workshop.'),
  };

  componentCatalog.push(component);
  io.emit('component:created', component);

  return res.status(201).json({
    status: 'created',
    component: {
      ...component,
      material,
    },
  });
});

app.get('/api/assemblies', (req, res) => {
  res.json(assemblies.map((assembly) => buildAssemblySummary(assembly)));
});

app.get('/api/assemblies/:assemblyId', (req, res) => {
  const assembly = assemblies.find((item) => item.id === req.params.assemblyId);
  if (!assembly) {
    return res.status(404).json({ error: 'Assembly not found' });
  }
  return res.json(buildAssemblySummary(assembly));
});

app.post('/api/assemblies/:assemblyId/instances', (req, res) => {
  const assembly = assemblies.find((item) => item.id === req.params.assemblyId);
  const component = getComponentById(req.body.partId);

  if (!assembly) {
    return res.status(404).json({ error: 'Assembly not found' });
  }

  if (!component) {
    return res.status(404).json({ error: 'Component not found' });
  }

  const currentCount = getAssemblyInstances(assembly.id).length;
  const position = Array.isArray(req.body.position)
    ? req.body.position.map(Number)
    : suggestPlacement(currentCount);

  const instance = {
    id: makeId('inst'),
    assemblyId: assembly.id,
    partId: component.id,
    name: String(req.body.name || component.name),
    componentId: 4900 + assemblyInstances.length,
    position,
    rotation: Array.isArray(req.body.rotation)
      ? req.body.rotation.map(Number)
      : [0, 0, 0],
  };

  assemblyInstances.push(instance);
  io.emit('assembly:updated', buildAssemblySummary(assembly));
  io.emit('scene:update', buildScene());

  return res.status(201).json({
    status: 'added',
    instance,
    assembly: buildAssemblySummary(assembly),
  });
});

app.delete('/api/assemblies/:assemblyId/instances/:instanceId', (req, res) => {
  const assembly = assemblies.find((item) => item.id === req.params.assemblyId);
  if (!assembly) {
    return res.status(404).json({ error: 'Assembly not found' });
  }

  const index = assemblyInstances.findIndex(
    (instance) =>
      instance.assemblyId === assembly.id && instance.id === req.params.instanceId,
  );

  if (index === -1) {
    return res.status(404).json({ error: 'Assembly instance not found' });
  }

  const [removed] = assemblyInstances.splice(index, 1);
  const summary = buildAssemblySummary(assembly);

  io.emit('assembly:updated', summary);
  io.emit('scene:update', buildScene());

  return res.json({
    status: 'removed',
    instance: removed,
    assembly: summary,
  });
});

app.get('/api/workshop/overview', (req, res) => {
  res.json(buildWorkshopOverview());
});

app.get('/api/scene', (req, res) => {
  res.json(buildScene());
});

app.get('/api/simulation/templates', (req, res) => {
  res.json(simulationTemplates);
});

app.post('/api/jobs', (req, res) => {
  const template = simulationTemplates.find((item) => item.id === req.body.templateId)
    || simulationTemplates.find((item) => item.simulationType === req.body.simulationType);

  if (!template) {
    return res.status(400).json({ error: 'Unknown simulation template' });
  }

  const assembly = getActiveAssembly();
  const targetAssemblyId = String(req.body.assemblyId || assembly?.id || '');
  const targetAssembly = assemblies.find((item) => item.id === targetAssemblyId);

  if (targetAssemblyId && !targetAssembly) {
    return res.status(400).json({ error: 'Unknown assembly target' });
  }

  const requestedComponentId = req.body.componentId === undefined || req.body.componentId === null || req.body.componentId === ''
    ? Number(template.defaultComponentId || 0)
    : Number(req.body.componentId);

  const targetInstance = Number.isFinite(requestedComponentId)
    ? getAssemblyInstances(targetAssemblyId || assembly?.id || '').find(
        (instance) => instance.componentId === requestedComponentId,
      ) || assemblyInstances.find((instance) => instance.componentId === requestedComponentId)
    : null;

  if (req.body.componentId !== undefined && req.body.componentId !== null && req.body.componentId !== '' && !targetInstance) {
    return res.status(400).json({ error: 'Unknown assembly component target' });
  }

  const jobId = makeId('job');
  const job = normalizeJob({
    jobId,
    templateId: template.id,
    simulationType: template.simulationType,
    module: template.module,
    componentId: targetInstance?.componentId || Number(template.defaultComponentId || 0),
    componentName: targetInstance?.name || null,
    assemblyId: targetAssemblyId,
    status: 'pending',
    progress: 0,
    priority: req.body.priority || template.priority || 'normal',
    parameters: req.body.parameters || {},
    createdAt: new Date().toISOString(),
    result: null,
  });

  jobQueue.set(jobId, job);
  io.emit('job:created', job);
  io.emit('stats:update', buildSystemStats());

  return res.status(201).json({ jobId, status: 'submitted', job });
});

app.get('/api/jobs', (req, res) => {
  const limit = Number(req.query.limit || 50);
  const simulationType = String(req.query.simulationType || '');
  const status = String(req.query.status || '');
  const componentId = req.query.componentId ? Number(req.query.componentId) : null;

  let jobs = Array.from(jobQueue.values()).sort(
    (left, right) => new Date(right.createdAt).getTime() - new Date(left.createdAt).getTime(),
  );

  if (simulationType) {
    jobs = jobs.filter((job) => job.simulationType === simulationType);
  }
  if (status) {
    jobs = jobs.filter((job) => job.status === status);
  }
  if (componentId) {
    jobs = jobs.filter((job) => job.componentId === componentId);
  }

  res.json(jobs.slice(0, limit));
});

app.get('/api/jobs/:jobId', (req, res) => {
  const job = jobQueue.get(req.params.jobId);
  if (!job) {
    return res.status(404).json({ error: 'Job not found' });
  }
  return res.json(job);
});

app.delete('/api/jobs/:jobId', (req, res) => {
  const job = jobQueue.get(req.params.jobId);
  if (!job) {
    return res.status(404).json({ error: 'Job not found' });
  }

  if (job.status === 'completed' || job.status === 'failed') {
    return res.status(409).json({ error: 'Job can no longer be cancelled' });
  }

  job.status = 'cancelled';
  job.progress = Math.min(job.progress, 99);
  jobQueue.set(job.jobId, job);
  io.emit('job:cancelled', job.jobId);
  io.emit('stats:update', buildSystemStats());

  return res.json({ status: 'cancelled', job });
});

app.get('/api/analysis/overview', (req, res) => {
  res.json(buildAnalysisOverview());
});

app.get('/api/modules', (req, res) => {
  res.json(moduleCatalog);
});

app.get('/api/features', (req, res) => {
  res.json(buildFeatureCatalog());
});

app.get('/api/services', async (req, res) => {
  res.json(await buildServicesOverview());
});

app.get('/api/systems/overview', async (req, res) => {
  const services = await buildServicesOverview();
  res.json({
    modules: moduleCatalog,
    services,
    coverage: {
      totalModules: moduleCatalog.length,
      liveExposures: moduleCatalog.filter((module) => module.exposure === 'live').length,
      desktopOnly: moduleCatalog.filter((module) => module.exposure === 'desktop').length,
      metadataOnly: moduleCatalog.filter((module) => module.exposure === 'metadata').length,
      proxyBacked: moduleCatalog.filter((module) => module.exposure === 'proxy').length,
    },
    gaps: moduleCatalog
      .filter((module) => ['pending', 'headers-complete', 'partial'].includes(module.status))
      .map((module) => ({
        id: module.id,
        name: module.name,
        status: module.status,
        summary: module.summary,
      })),
  });
});

app.get('/api/ai/overview', async (req, res) => {
  res.json(await buildAiOverview());
});

app.post('/api/ai/reason', async (req, res) => {
  const payload = {
    problem: String(req.body.problem || ''),
    reasoning_mode: String(req.body.reasoning_mode || 'forward_chain'),
    context: req.body.context || null,
  };

  if (!payload.problem) {
    return res.status(400).json({ error: 'Problem text is required' });
  }

  const remote = await tryAiPost('/api/reason', payload);
  if (remote) {
    return res.json({ live: true, result: remote });
  }

  return res.json({
    live: false,
    result: buildFallbackReasoning(payload.problem, payload.reasoning_mode),
  });
});

app.post('/api/ai/solve-design-problem', async (req, res) => {
  const payload = {
    problem_description: String(req.body.problem_description || ''),
    objectives: Array.isArray(req.body.objectives) ? req.body.objectives : ['design_validity'],
    constraints: req.body.constraints || null,
  };

  if (!payload.problem_description) {
    return res.status(400).json({ error: 'Problem description is required' });
  }

  const remote = await tryAiPost('/api/solve-design-problem', payload);
  if (remote) {
    return res.json({ live: true, result: remote });
  }

  return res.json({
    live: false,
    result: buildFallbackDesignSolution(payload.problem_description, payload.objectives),
  });
});

io.on('connection', (socket) => {
  socket.emit('stats:update', buildSystemStats());
  socket.emit('scene:update', buildScene());

  socket.on('job:subscribe', (jobId) => {
    socket.join(`job_${jobId}`);
  });
});

function advanceJobQueue() {
  jobQueue.forEach((job, jobId) => {
    if (job.status === 'pending') {
      job.status = 'running';
      job.progress = Math.max(job.progress, 8);
      jobQueue.set(jobId, normalizeJob(job));
      io.emit('job:started', normalizeJob(job));
      return;
    }

    if (job.status === 'running') {
      const increment = 8 + Math.random() * 12;
      job.progress = clamp(job.progress + increment, 0, 100);

      if (job.progress >= 100) {
        job.status = 'completed';
        job.progress = 100;
        job.result = buildJobResult(job);
        resultCache.set(jobId, job.result);
        io.emit('job:completed', { jobId, result: job.result });
      } else {
        io.to(`job_${jobId}`).emit('job:progress', {
          jobId,
          progress: round(job.progress, 1),
        });
      }

      jobQueue.set(jobId, normalizeJob(job));
    }
  });

  io.emit('stats:update', buildSystemStats());
}

let jobWorker = null;

function startJobWorker(intervalMs = 1200) {
  if (!jobWorker) {
    jobWorker = setInterval(advanceJobQueue, intervalMs);
  }

  return jobWorker;
}

function stopJobWorker() {
  if (jobWorker) {
    clearInterval(jobWorker);
    jobWorker = null;
  }
}

function startServer(port = PORT) {
  startJobWorker();
  return server.listen(port, () => {
    console.log(`SZM Forge backend listening on port ${port}`);
    console.log(`WebSocket enabled on ws://localhost:${port}`);
  });
}

if (require.main === module) {
  startServer();
}

module.exports = {
  app,
  io,
  startServer,
  stopJobWorker,
  advanceJobQueue,
  resetState,
};
