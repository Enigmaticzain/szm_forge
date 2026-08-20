import { getApiBaseUrl } from './forgeApi';
import type { FurnitureType } from '../workshop/furnitureEngine';

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

export async function fetchProject() {
  return getJson<{
    projectName: string;
    hasFurniture?: boolean;
    hasTable?: boolean;
    furnitureType?: string;
    materialId: string;
    material: string;
    loadKg: number;
    aiImproved: boolean;
  }>('/api/project');
}

export async function createNewProject(name: string) {
  return postJson('/api/project/new', { name });
}

export async function createFurniture(type: FurnitureType, materialId: string, loadKg: number) {
  if (type === 'table') {
    return postJson<{ parts: unknown[] }>('/api/project/table', { materialId, loadKg });
  }
  return postJson<{ parts: unknown[] }>('/api/project/furniture', { type, materialId, loadKg });
}

export async function setLoad(loadKg: number) {
  return postJson<{ parts: unknown[] }>('/api/project/load', { loadKg });
}

export async function runLimitSweep() {
  return postJson<{
    results: unknown[];
    maxSafeLoadKg?: number;
    firstFailureLoadKg?: number;
  }>('/api/project/limits', {});
}

export async function runAIImprove() {
  return postJson<{
    parts: unknown[];
    insights: unknown[];
    materialId?: string;
    material?: string;
  }>('/api/ai/improve', {});
}
