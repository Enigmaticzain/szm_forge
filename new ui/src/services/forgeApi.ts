/**
 * SZM Forge API — offline-first. All core data and solvers run in-browser.
 * Optional C++ backend (:8000) enhances results when VITE_ENABLE_BACKEND=true.
 */

import {
  getAllMaterials,
  listKbDomains,
  getKbDomain,
  searchKb,
  solveSketchOffline,
  extrudeSketchOffline,
  createParametricPart,
  analyzePart,
  parseTextToCad,
  loadMachineConfig,
  saveMachineConfigLocal,
  OFFLINE_ENGINE_NAME,
  OFFLINE_ENGINE_VERSION,
} from '../offline';
import type { TextToCadResult } from '../offline/geometry';

/** Network backend only when explicitly enabled */
const NETWORK_ENABLED = import.meta.env.VITE_ENABLE_BACKEND === 'true';

/** In dev with network enabled, Vite proxies /api → :8000 */
const API_BASE = (
  (import.meta.env.VITE_FORGE_API_URL as string | undefined)?.replace(/\/$/, '') ??
  (import.meta.env.DEV && NETWORK_ENABLED ? '' : '')
);

const CAN_USE_BACKEND = NETWORK_ENABLED && (Boolean(API_BASE) || import.meta.env.DEV);

function requireBackend(feature: string): void {
  if (!CAN_USE_BACKEND) {
    throw new Error(`${feature} requires backend connectivity (set VITE_ENABLE_BACKEND=true)`);
  }
}

async function handleResponse<T = any>(res: Response): Promise<T> {
  if (!res.ok) {
    const errorData = await res.json().catch(() => ({}));
    if (errorData.detail) {
      // Pydantic validation error parsing
      const msgs = Array.isArray(errorData.detail)
        ? errorData.detail.map((e: any) => `${Array.isArray(e.loc) ? e.loc.join('.') : e.loc}: ${e.msg}`).join(', ')
        : errorData.detail;
      throw new Error(`API Error (${res.status}): ${msgs}`);
    }
    throw new Error(`HTTP Error ${res.status}: ${res.statusText}`);
  }
  return res.json() as Promise<T>;
}

async function getBackendJson<T = any>(feature: string, path: string): Promise<T> {
  requireBackend(feature);
  const res = await fetch(`${API_BASE}${path}`);
  return handleResponse<T>(res);
}

async function postBackendJson<T = any>(feature: string, path: string, body?: unknown): Promise<T> {
  requireBackend(feature);
  const init: RequestInit = body === undefined
    ? { method: 'POST' }
    : {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
      };
  const res = await fetch(`${API_BASE}${path}`, init);
  return handleResponse<T>(res);
}

export interface ForgeStatus {
  status: string;
  version: string;
  engine: string;
  modules?: string[];
}

export interface SimulationComponentDto {
  name: string;
  stress_MPa: number;
  temperature_C: number;
  stressRatio: number;
  tempRatio: number;
  status: string;
}

export interface SimulationStatusDto {
  simTime: number;
  componentCount: number;
  components: SimulationComponentDto[];
}

export interface AssemblyNodeDto {
  name: string;
  type?: string;
  mass?: number;
  children?: AssemblyNodeDto[];
}

export async function fetchJson<T>(path: string, timeoutMs = 2500): Promise<T | null> {
  if (!CAN_USE_BACKEND) return null;
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const res = await fetch(`${API_BASE}${path}`, {
      signal: controller.signal,
      headers: { Accept: 'application/json' },
    });
    if (!res.ok) return null;
    return (await res.json()) as T;
  } catch {
    return null;
  } finally {
    clearTimeout(timer);
  }
}

export async function pingBackend(): Promise<boolean> {
  if (NETWORK_ENABLED) {
    const status = await fetchJson<ForgeStatus>('/api/status');
    if (status?.status === 'running') return true;
  }
  return true; // offline engine always available
}

/**
 * Honest health probe for the OPTIONAL remote backend (:8000 / Python service).
 * Unlike pingBackend() — which answers "is any engine usable?" — this reports
 * the remote service state only, so panels never show "connected" on failure.
 */
export interface RemoteBackendHealth {
  /** VITE_ENABLE_BACKEND=true? When false the app is offline-only by design. */
  enabled: boolean;
  /** /api/status answered with a valid payload. */
  reachable: boolean;
  /** HTTP status observed on failure, if any. */
  statusCode: number | null;
  /** Human-readable detail, e.g. "HTTP 500" or "timed out". */
  detail: string;
}

export async function checkRemoteBackendHealth(timeoutMs = 2500): Promise<RemoteBackendHealth> {
  if (!CAN_USE_BACKEND) {
    return {
      enabled: false,
      reachable: false,
      statusCode: null,
      detail: 'Remote backend disabled — using the in-browser engine',
    };
  }
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const res = await fetch(`${API_BASE}/api/status`, {
      signal: controller.signal,
      headers: { Accept: 'application/json' },
    });
    if (!res.ok) {
      return {
        enabled: true,
        reachable: false,
        statusCode: res.status,
        detail: `Backend answered HTTP ${res.status}${res.statusText ? ` ${res.statusText}` : ''}`,
      };
    }
    const data = (await res.json().catch(() => null)) as ForgeStatus | null;
    return {
      enabled: true,
      reachable: true,
      statusCode: res.status,
      detail: data?.engine ? `Responding — ${data.engine}` : 'Responding',
    };
  } catch (e) {
    const timedOut = e instanceof DOMException && e.name === 'AbortError';
    return {
      enabled: true,
      reachable: false,
      statusCode: null,
      detail: timedOut ? 'Backend timed out' : 'Backend unreachable',
    };
  } finally {
    clearTimeout(timer);
  }
}

export function isOfflineMode(): boolean {
  return !NETWORK_ENABLED;
}

export function getEngineInfo(): { name: string; version: string; mode: 'offline' | 'hybrid' } {
  return {
    name: OFFLINE_ENGINE_NAME,
    version: OFFLINE_ENGINE_VERSION,
    mode: NETWORK_ENABLED ? 'hybrid' : 'offline',
  };
}

export async function fetchSimulationStatus(): Promise<SimulationStatusDto | null> {
  const remote = await fetchJson<SimulationStatusDto>('/api/simulation/status');
  if (remote) return remote;
  return { simTime: 0, componentCount: 0, components: [] };
}

export async function fetchAssembly(): Promise<AssemblyNodeDto | null> {
  return fetchJson<AssemblyNodeDto>('/api/assembly');
}

export async function fetchSceneJson(): Promise<unknown | null> {
  return fetchJson<unknown>('/api/scene.json');
}

export interface AxisLimit {
  travel_mm: number;
  feed_mm_min: number;
  accel_mm_s2: number;
}

export interface MachineConfigDto {
  id: string;
  name: string;
  is_5axis: boolean;
  linear: AxisLimit[];
  rotary: AxisLimit[];
}

export function getApiBaseUrl(): string {
  return API_BASE;
}

export async function fetchMachineConfig(id = 'default'): Promise<MachineConfigDto | null> {
  const remote = await fetchJson<MachineConfigDto>(`/api/machine/config?id=${id}`);
  return remote ?? loadMachineConfig(id);
}

export async function saveMachineConfig(cfg: MachineConfigDto): Promise<boolean> {
  const local = saveMachineConfigLocal(cfg);
  if (!CAN_USE_BACKEND) return local;
  try {
    const res = await fetch(`${API_BASE}/api/machine/config`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(cfg),
    });
    const data = await res.json();
    return data?.ok === true || local;
  } catch {
    return local;
  }
}

export interface MaterialDto {
  id: string;
  name: string;
  category?: string;
  tags?: string[];
  notes?: string;
  youngsModulus_GPa: number;
  poissonsRatio?: number;
  yieldStrength_MPa: number;
  ultimateStrength_MPa?: number;
  density_kg_m3: number;
  thermalExpansion_1_K?: number;
  thermalConductivity_W_mK?: number;
}

export interface MaterialsResponse {
  ok: boolean;
  materials: MaterialDto[];
}

export async function fetchMaterials(): Promise<MaterialDto[]> {
  const remote = await fetchJson<MaterialsResponse>('/api/materials', 5000);
  if (remote?.materials?.length) return remote.materials;
  return getAllMaterials();
}

export interface KbDomainDto {
  id: string;
  name: string;
  file?: string;
  entryCount?: number;
}

export interface KbSearchHit {
  domain: string;
  entry: Record<string, unknown>;
}

export interface KbSearchResponse {
  ok: boolean;
  query: string;
  count: number;
  results: KbSearchHit[];
}

export async function fetchKbDomains(): Promise<KbDomainDto[]> {
  const remote = await fetchJson<{ ok: boolean; domains: KbDomainDto[] }>('/api/kb/domains', 5000);
  if (remote?.domains?.length) return remote.domains;
  return listKbDomains();
}

export async function fetchKbDomain(domain: string): Promise<Record<string, unknown> | null> {
  const remote = await fetchJson<{ ok: boolean; data: Record<string, unknown> }>(
    `/api/kb/${encodeURIComponent(domain)}`,
    5000,
  );
  if (remote?.data) return remote.data;
  return getKbDomain(domain);
}

export async function searchKnowledge(query: string, domain = ''): Promise<KbSearchHit[]> {
  if (NETWORK_ENABLED) {
    const params = new URLSearchParams({ q: query });
    if (domain) params.set('domain', domain);
    const res = await fetchJson<KbSearchResponse>(`/api/kb/search?${params}`, 5000);
    if (res?.results?.length) return res.results;
  }
  return searchKb(query, domain);
}

// ======================== Machine Learning Pipeline ========================

export async function mlIngestData(): Promise<number> {
  const data = await postBackendJson<{ count?: number }>('ML data ingestion', '/api/ml/ingest');
  return data?.count ?? 0;
}

export async function mlLearnPatterns(): Promise<number> {
  const data = await postBackendJson<{ patterns_learned?: number }>('ML pattern learning', '/api/ml/learn');
  return data?.patterns_learned ?? 0;
}

export interface GeneratedMachineCode {
  id: string;
  name: string;
  machine_type: string;
  confidence: number;
  header: string;
  source: string;
  cmake: string;
}

export async function mlGenerateMachine(
  machine_type: string,
  count = 1,
  parameters?: Record<string, number>
): Promise<GeneratedMachineCode[]> {
  const data = await postBackendJson<{ generated?: GeneratedMachineCode[] }>(
    'ML machine generation',
    '/api/ml/generate',
    { machine_type, count, parameters },
  );
  return data?.generated ?? [];
}

// ======================== GraphRAG & Reasoning ========================

export async function queryGraphRAG(query: string): Promise<any> {
  return postBackendJson('GraphRAG query', '/api/graphrag/query', { query });
}

export async function queryReasoning(problem: string, reasoning_mode = 'forward_chain'): Promise<any> {
  return postBackendJson('Reasoning query', '/api/reason', { problem, reasoning_mode });
}

// ======================== Manufacturing Endpoints ========================

export async function validateDFM(component_name: string, parameters: any, process = 'CNC'): Promise<any> {
  return postBackendJson('DFM validation', '/api/manufacturability/validate', {
    component_name,
    parameters,
    process,
  });
}

export async function exportGCode(component_name: string, bounds: any): Promise<any> {
  return postBackendJson('G-Code export', '/api/manufacturability/export-gcode', { component_name, bounds });
}

export async function generateCNCToolpath(component_name: string, bounds: any, options: any = {}): Promise<any> {
  return postBackendJson('CNC toolpath generation', '/api/manufacturability/cnc-toolpath', {
    component_name,
    bounds,
    ...options,
  });
}

// ======================== Simulation & Batching ========================

export async function runBatchSimulation(num_variations = 50): Promise<any> {
  return postBackendJson('Batch simulation', '/api/batch-simulate', { num_variations });
}

// ======================== Data Crawlers ========================

export async function searchArxiv(query: string, limit = 5): Promise<any> {
  return getBackendJson('arXiv search', `/data/search/arxiv?q=${encodeURIComponent(query)}&limit=${limit}`);
}

export async function searchPatents(query: string, limit = 5): Promise<any> {
  return getBackendJson('Patent search', `/data/search/patents?q=${encodeURIComponent(query)}&limit=${limit}`);
}

export async function searchNasa(query: string, limit = 5): Promise<any> {
  return getBackendJson('NASA search', `/data/search/nasa?q=${encodeURIComponent(query)}&limit=${limit}`);
}

// ======================== Robotics ========================

export async function parseUrdf(content: string): Promise<any> {
  return postBackendJson('URDF parser', '/api/robotics/urdf/parse', { content });
}

export async function runPybulletSimulation(steps = 240): Promise<any> {
  return postBackendJson('PyBullet simulation', '/api/robotics/simulate', { steps });
}

// ======================== Surrogates (Physics ML) ========================

export async function trainSurrogate(name: string, backend = "torch", epochs = 100): Promise<any> {
  return postBackendJson('Surrogate training', '/api/simulation/surrogate/train', { name, backend, epochs });
}

export async function predictSurrogate(name: string, inputs: Record<string, number>): Promise<any> {
  return postBackendJson('Surrogate prediction', '/api/simulation/surrogate/predict', { name, inputs });
}

// ======================== IoT ========================

export async function startMqtt(broker = "localhost", port = 1883, topic = "szm/telemetry/#"): Promise<any> {
  return postBackendJson('MQTT client', '/iot/mqtt/start', { broker, port, topic });
}

export async function readOpcUa(endpoint: string, node_id: string): Promise<any> {
  return postBackendJson('OPC-UA client', '/iot/opcua/read', { endpoint, node_id });
}

// ======================== Multiphysics Solvers ========================

export async function runFea(component_id: number, material_id: string, force_n: number, area_m2: number, thickness_m: number): Promise<any> {
  return postBackendJson('FEA solver', '/api/simulation/fea/run', {
    component_id,
    material_id,
    force_n,
    area_m2,
    thickness_m,
  });
}

export async function runThermal(component_id: number, temp_c: number): Promise<any> {
  return postBackendJson('Thermal solver', '/api/simulation/thermal/run', { component_id, temp_c });
}

export async function runModal(component_id: number, modes: number = 6): Promise<any> {
  return postBackendJson('Modal solver', '/api/simulation/modal/run', { component_id, modes });
}

export async function runCfd(component_id: number, velocity_m_s: number): Promise<any> {
  return postBackendJson('CFD solver', '/api/simulation/cfd/run', { component_id, velocity_m_s });
}

// ======================== Pipelines & Automation ========================

export async function runPipeline(job_name: string, tasks: string[]): Promise<any> {
  return postBackendJson('Automation pipeline', '/api/pipeline/run', { job_name, tasks });
}

// ======================== Geometry (offline Three.js + optional backend) ========================

export interface SketchEntityDto {
  id: number;
  type: 'point' | 'line' | 'circle' | 'arc';
  construction?: boolean;
  p1: [number, number];
  p2?: [number, number];
  p3?: [number, number];
}

export interface SketchDto {
  plane?: { origin: [number, number, number]; normal: [number, number, number] };
  entities: SketchEntityDto[];
  constraints?: Array<Record<string, unknown>>;
  dimensions?: Array<Record<string, unknown>>;
}

export interface SketchSolveResult extends SketchDto {
  solved?: boolean;
  dof?: number;
}

export async function solveSketch(sketch: SketchDto): Promise<SketchSolveResult | null> {
  if (CAN_USE_BACKEND) {
    try {
      const res = await fetch(`${API_BASE}/api/sketch/solve`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(sketch),
      });
      if (res.ok) return res.json();
    } catch { /* fall through */ }
  }
  return solveSketchOffline(sketch);
}

export async function extrudeSketch(
  sketch: SketchDto,
  distance = 0.01,
): Promise<Record<string, unknown> | null> {
  if (CAN_USE_BACKEND) {
    try {
      const res = await fetch(`${API_BASE}/api/sketch/extrude`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ sketch, distance }),
      });
      if (res.ok) return res.json();
    } catch { /* fall through */ }
  }
  return extrudeSketchOffline(sketch, distance * 1000);
}

export type CsgOperation = 'union' | 'difference' | 'intersection';

export async function csgBoolean(
  operation: CsgOperation,
  bodyA: Record<string, unknown>,
  bodyB: Record<string, unknown>,
): Promise<Record<string, unknown> | null> {
  if (!CAN_USE_BACKEND) {
    return { ok: true, operation, engine: 'client-three-bvh-csg', note: 'Use mesh mode boolean tools' };
  }
  const res = await fetch(`${API_BASE}/api/csg/boolean`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ operation, body_a: bodyA, body_b: bodyB }),
  });
  if (!res.ok) return null;
  return res.json();
}

export async function freecadParametric(script: string): Promise<Record<string, unknown> | null> {
  if (!CAN_USE_BACKEND) {
    return { ok: false, error: 'FreeCAD scripting requires VITE_ENABLE_BACKEND=true' };
  }
  const res = await fetch(`${API_BASE}/api/freecad/parametric`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ script }),
  });
  if (!res.ok) return null;
  return res.json();
}

export async function createGeometryPart(
  partType: string,
  parameters: Record<string, unknown>,
  _format: 'stl' | 'step' = 'stl',
): Promise<Record<string, unknown> | null> {
  return createParametricPart(partType, parameters);
}

export async function analyzeGeometryPart(
  partType: string,
  parameters: Record<string, unknown>,
): Promise<Record<string, unknown> | null> {
  return analyzePart(partType, parameters);
}

export async function textToCad(prompt: string): Promise<TextToCadResult | null> {
  return parseTextToCad(prompt);
}

export interface DesignCadSpecDto {
  shape: string;
  material: string;
  features: Record<string, boolean>;
  length?: number;
  width?: number;
  height?: number;
  radius?: number;
  radius1?: number;
  radius2?: number;
  major_radius?: number;
  minor_radius?: number;
}

export interface DesignWorkflowDto {
  design_id: string;
  status: string;
  design: {
    status: string;
    spec: DesignCadSpecDto;
    geometry: Record<string, unknown>;
    engine: string | null;
    step_available: boolean;
    step_download_url: string | null;
  };
  analysis: {
    status: string;
    reason?: string;
    nominal_volume_mm3?: number | null;
    structural_analysis?: { status: string; reason: string };
  };
  manufacturing: {
    status: string;
    dfm_status: string;
    toolpath_status?: string;
    toolpath_message?: string;
    toolpath_preview: { gcode: string; warnings?: string[] } | null;
    limitations?: string[];
  };
}

const AI_WORKFLOW_BASE = (
  (import.meta.env.VITE_AI_WORKFLOW_URL as string | undefined)?.replace(/\/$/, '') ??
  'http://127.0.0.1:8001'
);

export async function runDesignWorkflow(
  prompt: string,
  includeToolpathPreview = false,
): Promise<DesignWorkflowDto> {
  if (!NETWORK_ENABLED) {
    throw new Error('Design workflow service requires backend connectivity (set VITE_ENABLE_BACKEND=true)');
  }
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 120_000);
  try {
    const response = await fetch(`${AI_WORKFLOW_BASE}/api/workflow/design-to-manufacture`, {
      method: 'POST',
      signal: controller.signal,
      headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
      body: JSON.stringify({
        prompt,
        include_toolpath_preview: includeToolpathPreview,
      }),
    });
    if (!response.ok) {
      const detail = await response.text();
      throw new Error(`Design workflow service returned ${response.status}: ${detail}`);
    }
    return await response.json() as DesignWorkflowDto;
  } finally {
    clearTimeout(timeout);
  }
}

export function getAiWorkflowUrl(path: string): string {
  return `${AI_WORKFLOW_BASE}${path}`;
}

export function geometryBridgeUrl(path: string): string {
  if (path.startsWith('blob:')) return path;
  return path;
}

export { validateMachineConfig } from '../offline/machineConfig';

// ======================== CAD & Export ========================

export async function exportUsd(component_id: string): Promise<any> {
  return postBackendJson('USD export', '/export/usd', { component_id });
}
