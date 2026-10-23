import type { FurnitureType } from '../workshop/furnitureEngine';
import {
  createProject,
  loadProject,
  createFurnitureOffline,
  setLoadOffline,
  runLimitSweepOffline,
  runAIImproveOffline,
} from '../offline/furnitureWorkshop';
import { presetChair, presetLadder, presetTable } from '../workshop/structuralAssembly';

export async function fetchProject() {
  return loadProject();
}

export async function createNewProject(name: string) {
  const project = createProject(name);
  return { ok: true, project };
}

export async function createFurniture(type: FurnitureType, materialId: string, loadKg: number) {
  const { project, members, parts } = createFurnitureOffline(type, materialId, loadKg);
  return { ok: true, project, members, parts };
}

export async function setLoad(
  loadKg: number,
  members = presetTable('MAT-STEEL-STRUCT'),
  materialId = 'MAT-STEEL-STRUCT',
) {
  const parts = setLoadOffline(members, materialId, loadKg);
  return { ok: true, parts, loadKg };
}

export async function runLimitSweep(
  members = presetTable('MAT-STEEL-STRUCT'),
  materialId = 'MAT-STEEL-STRUCT',
) {
  const results = runLimitSweepOffline(members, materialId);
  const maxSafe = [...results].reverse().find(r => r.status !== 'FAIL')?.loadKg ?? 0;
  const firstFailure = results.find(r => r.status === 'FAIL')?.loadKg;
  return { ok: true, results, maxSafeLoadKg: maxSafe, firstFailureLoadKg: firstFailure };
}

export async function runAIImprove(
  members = presetTable('MAT-STEEL-STRUCT'),
  materialId = 'MAT-STEEL-STRUCT',
  loadKg = 200,
) {
  const { parts, insights, materialId: improvedMat } = runAIImproveOffline(members, materialId, loadKg);
  return { ok: true, parts, insights, materialId: improvedMat };
}

export function getPresetMembers(preset: 'table' | 'chair' | 'ladder', materialId: string) {
  if (preset === 'chair') return presetChair(materialId);
  if (preset === 'ladder') return presetLadder(materialId);
  return presetTable(materialId);
}
