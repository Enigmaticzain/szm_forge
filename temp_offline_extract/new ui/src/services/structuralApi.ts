/**
 * Backend structural kit solver (uses codes/ MaterialDatabase + beam mechanics).
 */

import type { StructuralMember } from '../workshop/structuralAssembly';
import type { FurniturePart } from '../workshop/furnitureEngine';

const API_BASE = (
  (import.meta.env.VITE_FORGE_API_URL as string | undefined)?.replace(/\/$/, '') ??
  (import.meta.env.DEV ? '' : 'http://127.0.0.1:8000')
);

export interface StructuralSolveResult {
  ok: boolean;
  parts?: FurniturePart[];
  memberCount?: number;
  loadKg?: number;
  materialId?: string;
  solver?: string;
  error?: string;
}

export async function solveStructuralBackend(
  members: StructuralMember[],
  materialId: string,
  loadKg: number
): Promise<StructuralSolveResult | null> {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 8000);
  try {
    const res = await fetch(`${API_BASE}/api/structural/solve`, {
      method: 'POST',
      signal: controller.signal,
      headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
      body: JSON.stringify({ members, materialId, loadKg }),
    });
    if (!res.ok) return null;
    return (await res.json()) as StructuralSolveResult;
  } catch {
    return null;
  } finally {
    clearTimeout(timer);
  }
}
