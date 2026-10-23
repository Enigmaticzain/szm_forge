/**
 * Structural solver — runs entirely in-browser (offline).
 */

import type { StructuralMember } from '../workshop/structuralAssembly';
import type { FurniturePart } from '../workshop/furnitureEngine';
import { solveStructural } from '../offline/structuralSolver';

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
  loadKg: number,
): Promise<StructuralSolveResult | null> {
  const result = solveStructural(members, materialId, loadKg);
  return {
    ok: result.ok,
    parts: result.parts,
    memberCount: members.length,
    loadKg,
    materialId,
    solver: result.solver,
    error: result.error,
  };
}
