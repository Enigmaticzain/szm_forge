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
