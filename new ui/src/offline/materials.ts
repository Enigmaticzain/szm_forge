import materialsData from '@kb/materials.json';
import type { MaterialDto } from '../services/forgeApi';

interface MaterialEntry {
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

function toDto(e: MaterialEntry): MaterialDto {
  return {
    id: e.id,
    name: e.name,
    category: e.category,
    tags: e.tags,
    notes: e.notes,
    youngsModulus_GPa: e.youngsModulus_GPa,
    poissonsRatio: e.poissonsRatio,
    yieldStrength_MPa: e.yieldStrength_MPa,
    ultimateStrength_MPa: e.ultimateStrength_MPa,
    density_kg_m3: e.density_kg_m3,
    thermalExpansion_1_K: e.thermalExpansion_1_K,
    thermalConductivity_W_mK: e.thermalConductivity_W_mK,
  };
}

const BUNDLED = ((materialsData as { entries: MaterialEntry[] }).entries ?? []).map(toDto);

export function getAllMaterials(): MaterialDto[] {
  return BUNDLED;
}

export function getMaterialById(id: string): MaterialDto | undefined {
  return BUNDLED.find(m => m.id === id);
}

export function getYieldMPa(materialId: string): number {
  return getMaterialById(materialId)?.yieldStrength_MPa ?? 250;
}

export function getYoungsGPa(materialId: string): number {
  return getMaterialById(materialId)?.youngsModulus_GPa ?? 200;
}

export function getMaterialLabel(materialId: string): string {
  return getMaterialById(materialId)?.name ?? materialId;
}
