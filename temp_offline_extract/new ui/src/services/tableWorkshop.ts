/**
 * Four-leg table workshop — talks to C++ backend on :8000
 */

import { getApiBaseUrl } from './forgeApi';

const base = () => getApiBaseUrl();

async function postJson<T>(path: string, body: Record<string, unknown> = {}): Promise<T | null> {
  try {
    const res = await fetch(`${base()}${path}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
      body: JSON.stringify(body),
    });
    if (!res.ok) return null;
    return (await res.json()) as T;
  } catch {
    return null;
  }
}

async function getJson<T>(path: string): Promise<T | null> {
  try {
    const res = await fetch(`${base()}${path}`, { headers: { Accept: 'application/json' } });
    if (!res.ok) return null;
    return (await res.json()) as T;
  } catch {
    return null;
  }
}

export interface ProjectState {
  projectName: string;
  hasTable: boolean;
  materialId: string;
  material: string;
  loadKg: number;
  aiImproved: boolean;
}

export interface TablePart {
  name: string;
  stress_MPa: number;
  yield_MPa: number;
  stressRatio: number;
  status: string;
  force_N?: number;
  material?: string;
}

export interface LimitSweepRow {
  loadKg: number;
  worstUtilization: number;
  criticalPart: string;
  status: string;
  parts?: TablePart[];
}

export interface AIInsightDto {
  title: string;
  description: string;
  confidence: number;
  actions?: string[];
  type: string;
}

export interface BootstrapResult {
  ok: boolean;
  project: ProjectState;
  table: { parts: TablePart[]; material: string; loadKg: number };
  limits: { maxSafeLoadKg: number; firstFailureLoadKg?: number; results: LimitSweepRow[] };
  ai: { insights: AIInsightDto[]; improved: boolean; material: string };
}

export const MATERIALS = [
  { id: 'MAT-STEEL-STRUCT', label: 'Structural Steel (AISI 1020)' },
  { id: 'MAT-AL-6061-T6', label: '6061-T6 Aluminum' },
  { id: 'MAT-TI-6AL4V', label: 'Ti-6Al-4V Titanium' },
  { id: 'MAT-CU-C110', label: 'C110 Copper' },
] as const;

export async function fetchProject(): Promise<ProjectState | null> {
  return getJson<ProjectState>('/api/project');
}

export async function createNewProject(name: string) {
  return postJson<{ ok: boolean; projectName: string }>('/api/project/new', { name });
}

export async function createTable(materialId: string, loadKg: number) {
  return postJson<{ ok: boolean; parts: TablePart[]; material: string }>('/api/project/table', {
    materialId,
    loadKg,
  });
}

export async function setLoad(loadKg: number) {
  return postJson<{ ok: boolean; parts: TablePart[] }>('/api/project/load', { loadKg });
}

export async function runLimitSweep() {
  return postJson<{
    ok: boolean;
    maxSafeLoadKg: number;
    firstFailureLoadKg?: number;
    results: LimitSweepRow[];
  }>('/api/project/limits', {});
}

export async function runAIImprove() {
  return postJson<{ ok: boolean; insights: AIInsightDto[]; improved: boolean; parts: TablePart[] }>(
    '/api/ai/improve',
    {}
  );
}

export async function bootstrapTableWorkshop(
  materialId = 'MAT-STEEL-STRUCT',
  loadKg = 200,
  name = 'Four-Leg Table Study'
): Promise<BootstrapResult | null> {
  return postJson<BootstrapResult>('/api/workshop/bootstrap', { name, materialId, loadKg });
}

export async function fetchAIInsights() {
  return getJson<{ insights: AIInsightDto[] }>('/api/ai/insights');
}

export async function fetchLimits() {
  return getJson<{ results: LimitSweepRow[] }>('/api/project/limits');
}
