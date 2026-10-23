/**
 * In-browser furniture workshop — fully offline.
 */

import { getAllMaterials } from '../offline/materials';

export type FurnitureType = 'table' | 'chair' | 'ladder' | 'motor';

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

export function materialOptions() {
  return getAllMaterials().map(m => ({ id: m.id, label: m.name }));
}
