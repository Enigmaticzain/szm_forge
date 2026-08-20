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

export function getApiBaseUrl(): string {
  return API_BASE;
}
