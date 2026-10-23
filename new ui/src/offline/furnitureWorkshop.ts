import type { FurniturePart, FurnitureType, LimitSweepRow, ProjectState, AIInsightDto } from '../workshop/furnitureEngine';
import type { StructuralMember } from '../workshop/structuralAssembly';
import { presetChair, presetLadder, presetTable } from '../workshop/structuralAssembly';
import { solveStructural } from './structuralSolver';
import { getMaterialLabel } from './materials';
import { loadJson, saveJson } from './storage';

const PROJECT_KEY = 'project';

export function loadProject(): ProjectState | null {
  return loadJson<ProjectState | null>(PROJECT_KEY, null);
}

export function saveProject(state: ProjectState): void {
  saveJson(PROJECT_KEY, state);
}

export function createProject(name: string): ProjectState {
  const state: ProjectState = {
    projectName: name,
    hasFurniture: false,
    furnitureType: null,
    materialId: 'MAT-STEEL-STRUCT',
    material: getMaterialLabel('MAT-STEEL-STRUCT'),
    loadKg: 200,
    aiImproved: false,
  };
  saveProject(state);
  return state;
}

export function createFurnitureOffline(
  type: FurnitureType,
  materialId: string,
  loadKg: number,
): { project: ProjectState; members: StructuralMember[]; parts: FurniturePart[] } {
  const members =
    type === 'table' ? presetTable(materialId)
    : type === 'chair' ? presetChair(materialId)
    : type === 'ladder' ? presetLadder(materialId)
    : presetTable(materialId);

  const solved = solveStructural(members, materialId, loadKg);
  const project: ProjectState = {
    projectName: `${type.charAt(0).toUpperCase()}${type.slice(1)} Study`,
    hasFurniture: true,
    furnitureType: type,
    materialId,
    material: getMaterialLabel(materialId),
    loadKg,
    aiImproved: false,
  };
  saveProject(project);
  return { project, members, parts: solved.parts };
}

export function setLoadOffline(
  members: StructuralMember[],
  materialId: string,
  loadKg: number,
): FurniturePart[] {
  return solveStructural(members, materialId, loadKg).parts;
}

export function runLimitSweepOffline(
  members: StructuralMember[],
  materialId: string,
): LimitSweepRow[] {
  const loads = [50, 100, 150, 200, 250, 300, 400, 500];
  const rows: LimitSweepRow[] = [];
  let firstFailure: number | undefined;
  let maxSafe = 0;

  for (const loadKg of loads) {
    const { parts } = solveStructural(members, materialId, loadKg);
    const worst = parts.reduce((max, p) => Math.max(max, p.stressRatio), 0);
    const critical = parts.find(p => p.stressRatio === worst);
    const status = worst >= 1 ? 'FAIL' : worst >= 0.8 ? 'WARN' : 'SAFE';
    if (status === 'FAIL' && firstFailure === undefined) firstFailure = loadKg;
    if (status !== 'FAIL') maxSafe = loadKg;
    rows.push({
      loadKg,
      worstUtilization: worst,
      criticalPart: critical?.name ?? '—',
      status,
      parts,
    });
  }

  return rows;
}

export function runAIImproveOffline(
  members: StructuralMember[],
  materialId: string,
  loadKg: number,
): { parts: FurniturePart[]; insights: AIInsightDto[]; materialId: string } {
  const base = solveStructural(members, materialId, loadKg);
  const worst = base.parts.reduce((max, p) => (p.stressRatio > max.stressRatio ? p : max), base.parts[0]);

  const insights: AIInsightDto[] = [];
  if (worst && worst.stressRatio > 0.8) {
    insights.push({
      title: 'Increase section or upgrade material',
      description: `${worst.name} is at ${(worst.stressRatio * 100).toFixed(0)}% utilization. Consider thicker ${worst.name} or Ti-6Al-4V.`,
      confidence: 88,
      type: 'warning',
      actions: ['Increase width/height 15%', 'Switch to MAT-TI-6AL4V'],
    });
  } else {
    insights.push({
      title: 'Design within safe limits',
      description: 'All members below 80% yield. Geometry is structurally sound for current load.',
      confidence: 92,
      type: 'success',
    });
  }

  let improvedMaterial = materialId;
  if (worst && worst.stressRatio > 1 && materialId === 'MAT-STEEL-STRUCT') {
    improvedMaterial = 'MAT-TI-6AL4V';
    insights.push({
      title: 'Auto material upgrade suggested',
      description: 'Switched recommendation to Ti-6Al-4V for higher yield strength.',
      confidence: 85,
      type: 'info',
    });
  }

  const improved = solveStructural(members, improvedMaterial, loadKg);
  return { parts: improved.parts, insights, materialId: improvedMaterial };
}
