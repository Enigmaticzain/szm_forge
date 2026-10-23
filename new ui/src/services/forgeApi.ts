/**
 * HTTP bridge to SZM Forge C++ backend (WebAPI on port 8000).
 * Used by both Theme 1 (Classic) and Theme 2 (Modern).
 */

/** In dev, Vite proxies /api → :8000 (same origin, no CORS). */
const API_BASE = (
  (import.meta.env.VITE_FORGE_API_URL as string | undefined)?.replace(/\/$/, '') ??
  (import.meta.env.DEV ? '' : 'http://127.0.0.1:8000')
);

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
  const status = await fetchJson<ForgeStatus>('/api/status');
  return status?.status === 'running';
}

export async function fetchSimulationStatus(): Promise<SimulationStatusDto | null> {
  return fetchJson<SimulationStatusDto>('/api/simulation/status');
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
  return fetchJson<MachineConfigDto>(`/api/machine/config?id=${id}`);
}

export async function saveMachineConfig(cfg: MachineConfigDto): Promise<boolean> {
  try {
    const res = await fetch(`${API_BASE}/api/machine/config`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(cfg),
    });
    const data = await res.json();
    return data?.ok === true;
  } catch {
    return false;
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
  const res = await fetchJson<MaterialsResponse>('/api/materials', 5000);
  return res?.materials ?? [];
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
  const res = await fetchJson<{ ok: boolean; domains: KbDomainDto[] }>('/api/kb/domains', 5000);
  return res?.domains ?? [];
}

export async function fetchKbDomain(domain: string): Promise<Record<string, unknown> | null> {
  const res = await fetchJson<{ ok: boolean; data: Record<string, unknown> }>(
    `/api/kb/${encodeURIComponent(domain)}`,
    5000,
  );
  return res?.data ?? null;
}

export async function searchKnowledge(query: string, domain = ''): Promise<KbSearchHit[]> {
  const params = new URLSearchParams({ q: query });
  if (domain) params.set('domain', domain);
  const res = await fetchJson<KbSearchResponse>(`/api/kb/search?${params}`, 5000);
  return res?.results ?? [];
}

// ======================== Machine Learning Pipeline ========================

export async function mlIngestData(): Promise<number> {
  const res = await fetch(`${API_BASE}/api/ml/ingest`, { method: 'POST' });
  const data = await res.json();
  return data?.count ?? 0;
}

export async function mlLearnPatterns(): Promise<number> {
  const res = await fetch(`${API_BASE}/api/ml/learn`, { method: 'POST' });
  const data = await res.json();
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
  const res = await fetch(`${API_BASE}/api/ml/generate`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ machine_type, count, parameters }),
  });
  const data = await res.json();
  return data?.generated ?? [];
}

// ======================== GraphRAG & Reasoning ========================

export async function queryGraphRAG(query: string): Promise<any> {
  const res = await fetch(`${API_BASE}/api/graphrag/query`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ query }),
  });
  return res.json();
}

export async function queryReasoning(problem: string, reasoning_mode = 'forward_chain'): Promise<any> {
  const res = await fetch(`${API_BASE}/api/reason`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ problem, reasoning_mode }),
  });
  return res.json();
}

// ======================== Manufacturing Endpoints ========================

export async function validateDFM(component_name: string, parameters: any, process = 'CNC'): Promise<any> {
  const res = await fetch(`${API_BASE}/api/manufacturability/validate`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_name, parameters, process }),
  });
  return res.json();
}

export async function exportGCode(component_name: string, bounds: any): Promise<any> {
  const res = await fetch(`${API_BASE}/api/manufacturability/export-gcode`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_name, bounds }),
  });
  return res.json();
}

export async function generateCNCToolpath(component_name: string, bounds: any, options: any = {}): Promise<any> {
  const res = await fetch(`${API_BASE}/api/manufacturability/cnc-toolpath`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_name, bounds, ...options }),
  });
  return res.json();
}

// ======================== Simulation & Batching ========================

export async function runBatchSimulation(num_variations = 50): Promise<any> {
  const res = await fetch(`${API_BASE}/api/batch-simulate`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ num_variations }),
  });
  return res.json();
  return res.json();
}

// ======================== Data Crawlers ========================

export async function searchArxiv(query: string, limit = 5): Promise<any> {
  const res = await fetch(`${API_BASE}/data/search/arxiv?q=${encodeURIComponent(query)}&limit=${limit}`);
  return res.json();
}

export async function searchPatents(query: string, limit = 5): Promise<any> {
  const res = await fetch(`${API_BASE}/data/search/patents?q=${encodeURIComponent(query)}&limit=${limit}`);
  return res.json();
}

export async function searchNasa(query: string, limit = 5): Promise<any> {
  const res = await fetch(`${API_BASE}/data/search/nasa?q=${encodeURIComponent(query)}&limit=${limit}`);
  return res.json();
}

// ======================== Robotics ========================

export async function parseUrdf(content: string): Promise<any> {
  const res = await fetch(`${API_BASE}/robotics/urdf/parse`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ content }),
  });
  return res.json();
}

export async function runPybulletSimulation(steps = 240): Promise<any> {
  const res = await fetch(`${API_BASE}/robotics/simulate`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ steps }),
  });
  return res.json();
}

// ======================== Surrogates (Physics ML) ========================

export async function trainSurrogate(name: string, backend = "torch", epochs = 100): Promise<any> {
  const res = await fetch(`${API_BASE}/simulation/surrogate/train`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ name, backend, epochs }),
  });
  return res.json();
}

export async function predictSurrogate(name: string, inputs: Record<string, number>): Promise<any> {
  const res = await fetch(`${API_BASE}/simulation/surrogate/predict`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ name, inputs }),
  });
  return res.json();
}

// ======================== IoT ========================

export async function startMqtt(broker = "localhost", port = 1883, topic = "szm/telemetry/#"): Promise<any> {
  const res = await fetch(`${API_BASE}/iot/mqtt/start`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ broker, port, topic }),
  });
  return res.json();
}

export async function readOpcUa(endpoint: string, node_id: string): Promise<any> {
  const res = await fetch(`${API_BASE}/iot/opcua/read`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ endpoint, node_id }),
  });
  return res.json();
}

// ======================== Multiphysics Solvers ========================

async function handleResponse(res: Response) {
  if (!res.ok) {
    const errorData = await res.json().catch(() => ({}));
    if (errorData.detail) {
      // Pydantic validation error parsing
      const msgs = Array.isArray(errorData.detail) 
        ? errorData.detail.map((e: any) => `${e.loc.join('.')}: ${e.msg}`).join(', ')
        : errorData.detail;
      throw new Error(`API Error (${res.status}): ${msgs}`);
    }
    throw new Error(`HTTP Error ${res.status}: ${res.statusText}`);
  }
  return res.json();
}

export async function runFea(component_id: number, material_id: string, force_n: number, area_m2: number, thickness_m: number): Promise<any> {
  const res = await fetch(`${API_BASE}/simulation/fea/run`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_id, material_id, force_n, area_m2, thickness_m }),
  });
  return handleResponse(res);
}

export async function runThermal(component_id: number, temp_c: number): Promise<any> {
  const res = await fetch(`${API_BASE}/simulation/thermal/run`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_id, temp_c }),
  });
  return handleResponse(res);
}

export async function runModal(component_id: number, modes: number = 6): Promise<any> {
  const res = await fetch(`${API_BASE}/simulation/modal/run`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_id, modes }),
  });
  return handleResponse(res);
}

export async function runCfd(component_id: number, velocity_m_s: number): Promise<any> {
  const res = await fetch(`${API_BASE}/simulation/cfd/run`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_id, velocity_m_s }),
  });
  return handleResponse(res);
}

// ======================== Pipelines & Automation ========================

export async function runPipeline(job_name: string, tasks: string[]): Promise<any> {
  const res = await fetch(`${API_BASE}/pipeline/run`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ job_name, tasks }),
  });
  return handleResponse(res);
}

// ======================== CAD & Export ========================

export async function exportUsd(component_id: string): Promise<any> {
  const res = await fetch(`${API_BASE}/export/usd`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ component_id }),
  });
  return handleResponse(res);
}
