import { TreeNode, TelemetryData, SimulationResult, AIInsight, MachineUnit, TimelineEvent } from '../types';

export const projectTree: TreeNode[] = [
  {
    id: 'asm-1',
    label: 'Turbine Assembly v4.2',
    type: 'assembly',
    status: 'active',
    expanded: true,
    children: [
      {
        id: 'grp-blades',
        label: 'Blade System',
        type: 'group',
        expanded: true,
        children: [
          { id: 'prt-1', label: 'Primary Blade [Ti-6Al-4V]', type: 'part', status: 'active' },
          { id: 'prt-2', label: 'Secondary Blade [IN718]', type: 'part', status: 'warning' },
          { id: 'prt-3', label: 'Blade Root Attachment', type: 'part', status: 'active' },
          { id: 'cst-1', label: 'Radial Constraint', type: 'constraint', status: 'active' },
          { id: 'cst-2', label: 'Angular Constraint (15°)', type: 'constraint', status: 'active' },
        ]
      },
      {
        id: 'grp-hub',
        label: 'Hub Assembly',
        type: 'group',
        children: [
          { id: 'prt-4', label: 'Central Hub [Steel 4340]', type: 'part', status: 'active' },
          { id: 'prt-5', label: 'Bearing Housing', type: 'part', status: 'active' },
          { id: 'prt-6', label: 'Shaft Coupling', type: 'part', status: 'processing' },
        ]
      },
      {
        id: 'grp-housing',
        label: 'Housing & Nacelle',
        type: 'group',
        children: [
          { id: 'prt-7', label: 'Outer Nacelle [CFRP]', type: 'part', status: 'active' },
          { id: 'prt-8', label: 'Inner Shroud', type: 'part', status: 'active' },
        ]
      },
    ]
  },
  {
    id: 'mat-lib',
    label: 'Material Library',
    type: 'group',
    children: [
      { id: 'mat-1', label: 'Ti-6Al-4V (Grade 5)', type: 'material', status: 'active' },
      { id: 'mat-2', label: 'Inconel 718', type: 'material', status: 'active' },
      { id: 'mat-3', label: 'AISI 4340 Steel', type: 'material', status: 'active' },
      { id: 'mat-4', label: 'CFRP Composite', type: 'material', status: 'warning' },
    ]
  },
  {
    id: 'sim-group',
    label: 'Simulations',
    type: 'group',
    children: [
      { id: 'sim-1', label: 'Static Structural FEA', type: 'simulation', status: 'complete' },
      { id: 'sim-2', label: 'Thermal Steady-State', type: 'simulation', status: 'processing' },
      { id: 'sim-3', label: 'Modal Analysis', type: 'simulation', status: 'active' },
      { id: 'sim-4', label: 'Fatigue Life Prediction', type: 'simulation', status: 'warning' },
    ]
  },
  {
    id: 'twin-group',
    label: 'Digital Twins',
    type: 'group',
    children: [
      { id: 'tw-1', label: 'Turbine Unit Alpha', type: 'twin', status: 'active' },
      { id: 'tw-2', label: 'Turbine Unit Beta', type: 'twin', status: 'warning' },
    ]
  },
  {
    id: 'ai-group',
    label: 'AI Agents',
    type: 'group',
    children: [
      { id: 'ai-1', label: 'Topology Optimizer', type: 'agent', status: 'active' },
      { id: 'ai-2', label: 'Failure Predictor', type: 'agent', status: 'active' },
      { id: 'ai-3', label: 'Material Selector', type: 'agent', status: 'processing' },
    ]
  },
];

export const telemetryData: TelemetryData[] = [
  { id: 't1', label: 'Von Mises Stress', value: 847.3, unit: 'MPa', trend: 'up', status: 'warning', history: [720, 750, 780, 810, 830, 847] },
  { id: 't2', label: 'Max Deformation', value: 0.342, unit: 'mm', trend: 'stable', status: 'success', history: [0.31, 0.33, 0.34, 0.34, 0.34, 0.34] },
  { id: 't3', label: 'Temperature Peak', value: 1247, unit: '°C', trend: 'up', status: 'critical', history: [1100, 1150, 1180, 1210, 1230, 1247] },
  { id: 't4', label: 'Safety Factor', value: 1.82, unit: 'x', trend: 'down', status: 'warning', history: [2.1, 2.0, 1.95, 1.90, 1.85, 1.82] },
  { id: 't5', label: 'Mesh Quality', value: 98.4, unit: '%', trend: 'stable', status: 'success', history: [97, 97.5, 98, 98.2, 98.3, 98.4] },
  { id: 't6', label: 'Fatigue Cycles', value: 1.2e6, unit: 'cycles', trend: 'down', status: 'info', history: [1.5e6, 1.4e6, 1.35e6, 1.3e6, 1.25e6, 1.2e6] },
  { id: 't7', label: 'Vibration Freq', value: 142.8, unit: 'Hz', trend: 'stable', status: 'success', history: [140, 141, 142, 142.5, 142.7, 142.8] },
  { id: 't8', label: 'Pressure Delta', value: 34.7, unit: 'kPa', trend: 'up', status: 'info', history: [28, 30, 32, 33, 34, 34.7] },
];

export const simulationResults: SimulationResult[] = [
  { metric: 'Max Principal Stress', value: '847.3 MPa', status: 'warning', detail: 'Approaching yield at blade root' },
  { metric: 'Total Deformation', value: '0.342 mm', status: 'success', detail: 'Within tolerance ±0.5mm' },
  { metric: 'Thermal Gradient', value: '312 °C/mm', status: 'critical', detail: 'Exceeds recommended limit' },
  { metric: 'Natural Frequency (1st)', value: '142.8 Hz', status: 'success', detail: 'Away from operational range' },
  { metric: 'Fatigue Safety Factor', value: '1.82', status: 'warning', detail: 'Below 2.0 threshold' },
  { metric: 'Convergence', value: '99.7%', status: 'success', detail: 'Fully converged in 847 iterations' },
  { metric: 'Element Quality', value: '98.4%', status: 'success', detail: '0.2% elements below threshold' },
  { metric: 'Contact Penetration', value: '0.003 mm', status: 'success', detail: 'Negligible penetration' },
];

export const aiInsights: AIInsight[] = [
  {
    id: 'ai-1',
    type: 'warning',
    title: 'Thermal Stress Concentration Detected',
    description: 'Blade root fillet shows 23% higher thermal gradient than specification. Recommend fillet radius increase from 2mm to 3.5mm.',
    confidence: 94,
    severity: 'critical',
    timestamp: '2 min ago'
  },
  {
    id: 'ai-2',
    type: 'optimization',
    title: 'Topology Optimization Available',
    description: 'Hub geometry can be optimized to reduce mass by 12.4% while maintaining safety factor > 2.0. Estimated material savings: 2.3 kg per unit.',
    confidence: 89,
    severity: 'info',
    timestamp: '5 min ago'
  },
  {
    id: 'ai-3',
    type: 'recommendation',
    title: 'Material Substitution Opportunity',
    description: 'IN718 on secondary blade can be replaced with Waspaloy for 8% better creep resistance at operating temperature.',
    confidence: 76,
    severity: 'info',
    timestamp: '12 min ago'
  },
  {
    id: 'ai-4',
    type: 'diagnostic',
    title: 'Mesh Refinement Suggestion',
    description: 'Contact region between blade root and hub shows mesh-dependent results. Recommend local refinement to 0.5mm element size.',
    confidence: 91,
    severity: 'warning',
    timestamp: '18 min ago'
  },
  {
    id: 'ai-5',
    type: 'warning',
    title: 'Fatigue Life Below Target',
    description: 'Current design shows 1.2M cycles vs. 2M cycle requirement. Critical location: blade trailing edge.',
    confidence: 88,
    severity: 'warning',
    timestamp: '24 min ago'
  },
];

export const machineFleet: MachineUnit[] = [
  {
    id: 'm1', name: 'Turbine Alpha-01', type: 'Gas Turbine', status: 'online',
    health: 94, uptime: 99.2, temperature: 892, load: 87, efficiency: 96.3,
    location: 'Bay A-1', lastMaintenance: '2025-12-15', nextMaintenance: '2026-03-15',
    telemetry: []
  },
  {
    id: 'm2', name: 'Turbine Alpha-02', type: 'Gas Turbine', status: 'online',
    health: 91, uptime: 98.7, temperature: 905, load: 92, efficiency: 95.1,
    location: 'Bay A-2', lastMaintenance: '2025-11-20', nextMaintenance: '2026-02-20',
    telemetry: []
  },
  {
    id: 'm3', name: 'Turbine Beta-01', type: 'Steam Turbine', status: 'degraded',
    health: 72, uptime: 94.1, temperature: 547, load: 65, efficiency: 88.4,
    location: 'Bay B-1', lastMaintenance: '2025-10-01', nextMaintenance: '2026-01-01',
    telemetry: []
  },
  {
    id: 'm4', name: 'Turbine Beta-02', type: 'Steam Turbine', status: 'maintenance',
    health: 45, uptime: 0, temperature: 24, load: 0, efficiency: 0,
    location: 'Bay B-2', lastMaintenance: '2026-01-05', nextMaintenance: '2026-04-05',
    telemetry: []
  },
  {
    id: 'm5', name: 'Compressor Unit C-01', type: 'Axial Compressor', status: 'online',
    health: 97, uptime: 99.8, temperature: 312, load: 78, efficiency: 97.8,
    location: 'Bay C-1', lastMaintenance: '2025-12-20', nextMaintenance: '2026-03-20',
    telemetry: []
  },
  {
    id: 'm6', name: 'Generator G-01', type: 'Synchronous Gen', status: 'online',
    health: 88, uptime: 99.1, temperature: 185, load: 91, efficiency: 98.2,
    location: 'Bay D-1', lastMaintenance: '2025-11-10', nextMaintenance: '2026-02-10',
    telemetry: []
  },
  {
    id: 'm7', name: 'Pump Station P-01', type: 'Centrifugal Pump', status: 'critical',
    health: 31, uptime: 87.3, temperature: 95, load: 42, efficiency: 71.2,
    location: 'Bay E-1', lastMaintenance: '2025-09-15', nextMaintenance: 'OVERDUE',
    telemetry: []
  },
  {
    id: 'm8', name: 'CNC Mill M-01', type: '5-Axis CNC', status: 'online',
    health: 96, uptime: 92.4, temperature: 42, load: 83, efficiency: 94.7,
    location: 'Bay F-1', lastMaintenance: '2025-12-28', nextMaintenance: '2026-03-28',
    telemetry: []
  },
];

export const timelineEvents: TimelineEvent[] = [
  { id: 'ev1', timestamp: '14:32:18', type: 'simulation', title: 'FEA Solver converged — 847 iterations', severity: 'success' },
  { id: 'ev2', timestamp: '14:30:05', type: 'ai-decision', title: 'AI detected thermal anomaly on blade root', severity: 'critical' },
  { id: 'ev3', timestamp: '14:28:42', type: 'optimization', title: 'Topology optimization cycle 12/20 complete', severity: 'info' },
  { id: 'ev4', timestamp: '14:25:11', type: 'alert', title: 'Pump P-01 vibration exceeding threshold', severity: 'warning' },
  { id: 'ev5', timestamp: '14:22:30', type: 'simulation', title: 'Thermal steady-state solver started', severity: 'info' },
  { id: 'ev6', timestamp: '14:18:55', type: 'maintenance', title: 'Turbine Beta-02 entered maintenance mode', severity: 'warning' },
  { id: 'ev7', timestamp: '14:15:20', type: 'ai-decision', title: 'Material recommendation generated for IN718 replacement', severity: 'info' },
  { id: 'ev8', timestamp: '14:12:08', type: 'lifecycle', title: 'Fatigue life prediction updated — 1.2M cycles', severity: 'warning' },
  { id: 'ev9', timestamp: '14:08:44', type: 'simulation', title: 'Modal analysis queued — 6 modes requested', severity: 'info' },
  { id: 'ev10', timestamp: '14:05:30', type: 'alert', title: 'Mesh quality check passed — 98.4% elements OK', severity: 'success' },
];

export const solverMetrics = {
  iterations: 847,
  convergence: 99.7,
  residual: 1.2e-6,
  wallTime: '4m 32s',
  cpuCores: 32,
  gpuUtil: 94,
  memUsage: 12.4,
  meshNodes: 2847293,
  meshElements: 8412847,
  dofs: 8541879,
};
