export type WorkspaceMode = 'command-center' | 'engineering' | 'simulation' | 'digital-twin' | 'manufacturing' | 'ai-assistant' | 'fleet-command' | 'designing' | 'physics' | 'chemistry-lab' | 'circuit-design';

export type ViewportRenderMode = 'shaded' | 'wireframe' | 'xray' | 'thermal' | 'stress' | 'pressure' | 'electrical';

export type SimulationStatus = 'idle' | 'configuring' | 'meshing' | 'solving' | 'post-processing' | 'complete' | 'error';

export type SeverityLevel = 'info' | 'warning' | 'critical' | 'success';

export interface TreeNode {
  id: string;
  label: string;
  type: 'assembly' | 'part' | 'constraint' | 'material' | 'simulation' | 'sensor' | 'agent' | 'twin' | 'group';
  children?: TreeNode[];
  status?: 'active' | 'warning' | 'error' | 'processing' | 'complete';
  icon?: string;
  expanded?: boolean;
}

export interface TelemetryData {
  id: string;
  label: string;
  value: number;
  unit: string;
  trend: 'up' | 'down' | 'stable';
  status: SeverityLevel;
  history: number[];
}

export interface SimulationResult {
  metric: string;
  value: string;
  status: SeverityLevel;
  detail?: string;
}

export interface AIInsight {
  id: string;
  type: 'optimization' | 'warning' | 'recommendation' | 'diagnostic';
  title: string;
  description: string;
  confidence: number;
  severity: SeverityLevel;
  timestamp: string;
}

export interface MachineUnit {
  id: string;
  name: string;
  type: string;
  status: 'online' | 'degraded' | 'maintenance' | 'offline' | 'critical';
  health: number;
  uptime: number;
  temperature: number;
  load: number;
  efficiency: number;
  location: string;
  lastMaintenance: string;
  nextMaintenance: string;
  telemetry: TelemetryData[];
}

export interface TimelineEvent {
  id: string;
  timestamp: string;
  type: 'simulation' | 'optimization' | 'maintenance' | 'alert' | 'ai-decision' | 'lifecycle';
  title: string;
  severity: SeverityLevel;
}
