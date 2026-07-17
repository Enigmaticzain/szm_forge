/**
 * In-browser furniture workshop (used when user clicks UI controls).
 * Syncs to C++ backend when SZM_Forge is running on :8000.
 */

export type FurnitureType = 'table' | 'chair' | 'ladder';

export interface FurniturePart {
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
  parts?: FurniturePart[];
}

export interface AIInsightDto {
  title: string;
  description: string;
  confidence: number;
  actions?: string[];
  type: string;
}

export interface ProjectState {
  projectName: string;
  hasFurniture: boolean;
  furnitureType: FurnitureType | null;
  materialId: string;
  material: string;
  loadKg: number;
  aiImproved: boolean;
}

const MATERIALS: Record<string, { label: string; yieldMPa: number }> = {
  'MAT-STEEL-STRUCT': { label: 'Structural Steel', yieldMPa: 250 },
  'MAT-AL-6061-T6': { label: '6061-T6 Aluminum', yieldMPa: 275 },
  'MAT-TI-6AL4V': { label: 'Ti-6Al-4V Titanium', yieldMPa: 880 },
  'MAT-CU-C110': { label: 'C110 Copper', yieldMPa: 70 },
};

// Fallback math logic removed — we strictly use the C++ backend now.

export function materialOptions() {
  return Object.entries(MATERIALS).map(([id, m]) => ({ id, label: m.label }));
}
