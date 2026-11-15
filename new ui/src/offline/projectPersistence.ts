/**
 * Project persistence — saves the user's session project to localStorage so
 * loaded/created geometry survives a reload (fixes USER_WORKFLOW_FINDINGS P1
 * "Created scene geometry is not persisted as a project").
 *
 * Design rules:
 *  - Honest degradation: if a geometry cannot be serialized or storage quota
 *    is exceeded, the caller is told exactly what was dropped. Nothing fails
 *    silently.
 *  - Versioned payload so future schema changes can migrate instead of
 *    corrupting state.
 */

import * as THREE from 'three';
import type { SceneObject } from '../components/BodyDesign/InteractivePartEditor';
import type { Joint } from '../components/BodyDesign/BodyDesignLayout';
import type { StructuralMember } from '../workshop/structuralAssembly';
import type { FurniturePart, ProjectState } from '../workshop/furnitureEngine';
import type { CustomMaterial } from '../store/ProjectContext';

const STORAGE_KEY = 'szm_forge_project_v1';
export const PROJECT_SCHEMA_VERSION = 1;

/** SceneObject with THREE.BufferGeometry converted to plain JSON. */
export interface PersistedSceneObject {
  id: string;
  name: string;
  type: SceneObject['type'];
  position: { x: number; y: number; z: number };
  size: { x: number; y: number; z: number };
  bendAngle?: number;
  bendAxis?: 'x' | 'y' | 'z';
  /** THREE.BufferGeometry.toJSON() output, if the object had custom geometry. */
  customGeometryJson?: Record<string, unknown>;
  /** Geometry that failed to round-trip is dropped; the part name lands here. */
  droppedCustomGeometry?: true;
  materialId?: string;
}

export interface PersistedProjectV1 {
  version: number;
  savedAt: number;
  project: ProjectState | null;
  members: StructuralMember[];
  bodyParts: PersistedSceneObject[];
  bodyJoints: Joint[];
  /** Last solver output so analysis panels show the last run immediately. */
  parts: FurniturePart[];
  customMaterials: CustomMaterial[];
}

export interface PersistInput {
  project: ProjectState | null;
  members: StructuralMember[];
  bodyParts: SceneObject[];
  bodyJoints: Joint[];
  parts: FurniturePart[];
  customMaterials: CustomMaterial[];
}

export interface PersistResult {
  ok: boolean;
  /** True when the save succeeded but custom meshes were dropped (quota). */
  degraded?: boolean;
  droppedGeometry: string[];
  error?: string;
  savedAt?: number;
}

export interface LoadResult {
  data: Omit<PersistedProjectV1, 'bodyParts'> & { bodyParts: SceneObject[] } | null;
  warnings: string[];
}

/**
 * THREE's typed primitives (BoxGeometry etc.) serialize as constructor params
 * which BufferGeometryLoader cannot rebuild. Normalize to attribute-level
 * BufferGeometry JSON — the same representation CSG/extrusion results have.
 */
function toAttributeGeometryJson(geo: THREE.BufferGeometry): Record<string, unknown> {
  const plain = new THREE.BufferGeometry();
  for (const [name, attr] of Object.entries(geo.attributes)) {
    plain.setAttribute(name, attr as THREE.BufferAttribute);
  }
  if (geo.index) plain.setIndex(geo.index.clone());
  return plain.toJSON() as unknown as Record<string, unknown>;
}

function serializeSceneObject(
  obj: SceneObject,
  dropped: string[],
  stripCustomGeometry = false,
): PersistedSceneObject {
  const out: PersistedSceneObject = {
    id: obj.id,
    name: obj.name,
    type: obj.type,
    position: { ...obj.position },
    size: { ...obj.size },
    materialId: obj.materialId,
  };
  if (obj.bendAngle !== undefined) out.bendAngle = obj.bendAngle;
  if (obj.bendAxis) out.bendAxis = obj.bendAxis;

  if (obj.customGeometry && !stripCustomGeometry) {
    try {
      out.customGeometryJson = toAttributeGeometryJson(obj.customGeometry);
    } catch {
      dropped.push(obj.name);
      out.droppedCustomGeometry = true;
    }
  } else if (obj.customGeometry && stripCustomGeometry) {
    dropped.push(obj.name);
    out.droppedCustomGeometry = true;
  }
  return out;
}

function deserializeSceneObject(
  p: PersistedSceneObject,
  warnings: string[],
): SceneObject {
  const obj: SceneObject = {
    id: p.id,
    name: p.name,
    type: p.type,
    position: p.position ?? { x: 0, y: 0, z: 0 },
    size: p.size ?? { x: 0.1, y: 0.1, z: 0.1 },
    bendAngle: p.bendAngle,
    bendAxis: p.bendAxis,
    materialId: p.materialId,
  };
  if (p.customGeometryJson) {
    try {
      obj.customGeometry = new THREE.BufferGeometryLoader().parse(p.customGeometryJson);
    } catch {
      warnings.push(`Geometry of "${p.name}" could not be restored; primitive shape was kept.`);
    }
  } else if (p.droppedCustomGeometry) {
    warnings.push(`Custom geometry of "${p.name}" was not saved (storage limits); primitive shape was kept.`);
  }
  return obj;
}

function buildPayload(input: PersistInput, stripCustomGeometry: boolean) {
  const droppedGeometry: string[] = [];
  const payload: PersistedProjectV1 = {
    version: PROJECT_SCHEMA_VERSION,
    savedAt: Date.now(),
    project: input.project,
    members: input.members,
    bodyParts: input.bodyParts.map(o => serializeSceneObject(o, droppedGeometry, stripCustomGeometry)),
    bodyJoints: input.bodyJoints,
    parts: input.parts,
    customMaterials: input.customMaterials,
  };
  return { payload, droppedGeometry };
}

export function persistProject(input: PersistInput): PersistResult {
  if (typeof localStorage === 'undefined') {
    return { ok: false, droppedGeometry: [], error: 'localStorage unavailable in this environment' };
  }

  // Attempt 1: full payload including custom meshes.
  let attempt = buildPayload(input, false);
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(attempt.payload));
    return {
      ok: true,
      degraded: attempt.droppedGeometry.length > 0,
      droppedGeometry: attempt.droppedGeometry,
      savedAt: attempt.payload.savedAt,
    };
  } catch {
    // Attempt 2 (quota / serialization size): retry without custom meshes and
    // report the degradation honestly to the caller.
    attempt = buildPayload(input, true);
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(attempt.payload));
      return {
        ok: true,
        degraded: true,
        droppedGeometry: attempt.droppedGeometry,
        error: attempt.droppedGeometry.length
          ? 'Storage limit hit — custom meshes were not saved'
          : 'Storage limit hit — saved without custom meshes',
        savedAt: attempt.payload.savedAt,
      };
    } catch (e) {
      return {
        ok: false,
        droppedGeometry: attempt.droppedGeometry,
        error: `Could not persist project: ${e instanceof Error ? e.message : String(e)}`,
      };
    }
  }
}

export function loadPersistedProject(): LoadResult {
  const warnings: string[] = [];
  if (typeof localStorage === 'undefined') return { data: null, warnings };

  let raw: string | null = null;
  try {
    raw = localStorage.getItem(STORAGE_KEY);
  } catch {
    return { data: null, warnings: ['Could not read saved project (storage blocked).'] };
  }
  if (!raw) return { data: null, warnings };

  let parsed: PersistedProjectV1;
  try {
    parsed = JSON.parse(raw) as PersistedProjectV1;
  } catch {
    return { data: null, warnings: ['Saved project was corrupted and has been ignored.'] };
  }
  if (parsed.version !== PROJECT_SCHEMA_VERSION) {
    return { data: null, warnings: [`Saved project schema v${parsed.version} unsupported — starting fresh.`] };
  }

  const bodyParts = (parsed.bodyParts ?? []).map(p => deserializeSceneObject(p, warnings));
  return {
    data: { ...parsed, bodyParts },
    warnings,
  };
}

export function clearPersistedProject(): void {
  try {
    localStorage.removeItem(STORAGE_KEY);
  } catch { /* storage blocked — nothing else we can do */ }
}
