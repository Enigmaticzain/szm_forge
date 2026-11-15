// @vitest-environment jsdom
import { describe, expect, it, beforeEach, afterEach } from 'vitest';
import * as THREE from 'three';
import {
  clearPersistedProject,
  loadPersistedProject,
  persistProject,
} from './projectPersistence';
import type { SceneObject } from '../components/BodyDesign/InteractivePartEditor';

const baseObject: SceneObject = {
  id: 'obj-1',
  name: 'Sketch Extrusion 1',
  type: 'box',
  position: { x: 1, y: 2, z: 3 },
  size: { x: 0.1, y: 0.1, z: 0.05 },
  materialId: 'm1',
};

const emptyInput = {
  project: null,
  members: [],
  bodyParts: [] as SceneObject[],
  bodyJoints: [],
  parts: [],
  customMaterials: [],
};

beforeEach(() => localStorage.clear());
afterEach(() => localStorage.clear());

describe('projectPersistence', () => {
  it('returns null when nothing was saved', () => {
    const { data, warnings } = loadPersistedProject();
    expect(data).toBeNull();
    expect(warnings).toEqual([]);
  });

  it('round-trips plain scene objects', () => {
    const save = persistProject({ ...emptyInput, bodyParts: [baseObject] });
    expect(save.ok).toBe(true);
    expect(save.droppedGeometry).toEqual([]);

    const { data, warnings } = loadPersistedProject();
    expect(warnings).toEqual([]);
    expect(data?.bodyParts).toHaveLength(1);
    const restored = data!.bodyParts[0];
    expect(restored.id).toBe('obj-1');
    expect(restored.position).toEqual({ x: 1, y: 2, z: 3 });
    expect(restored.customGeometry).toBeUndefined();
  });

  it('round-trips custom THREE.BufferGeometry (extrusion survives reload)', () => {
    const custom: SceneObject = {
      ...baseObject,
      id: 'obj-geo',
      name: 'CSG Result',
      customGeometry: new THREE.BoxGeometry(0.2, 0.1, 0.05),
    };
    const save = persistProject({ ...emptyInput, bodyParts: [custom] });
    expect(save.ok).toBe(true);

    const { data, warnings } = loadPersistedProject();
    expect(warnings).toEqual([]);
    const restored = data!.bodyParts[0];
    expect(restored.customGeometry).toBeInstanceOf(THREE.BufferGeometry);
    const attr = restored.customGeometry!.getAttribute('position');
    expect(attr.count).toBeGreaterThan(0);
  });

  it('ignores corrupted payloads instead of crashing', () => {
    localStorage.setItem('szm_forge_project_v1', '{not json');
    const { data, warnings } = loadPersistedProject();
    expect(data).toBeNull();
    expect(warnings[0]).toMatch(/corrupted/i);
  });

  it('rejects unsupported schema versions', () => {
    localStorage.setItem('szm_forge_project_v1', JSON.stringify({ version: 999 }));
    const { data, warnings } = loadPersistedProject();
    expect(data).toBeNull();
    expect(warnings[0]).toMatch(/unsupported/i);
  });

  it('degrades honestly when the quota is exceeded: custom meshes dropped, part kept', () => {
    const custom: SceneObject = {
      ...baseObject,
      customGeometry: new THREE.BoxGeometry(1, 1, 1),
    };
    const realSet = Storage.prototype.setItem;
    let calls = 0;
    // Fail the first write (full payload), allow the retry (stripped payload).
    Storage.prototype.setItem = function (...args) {
      calls += 1;
      if (calls === 1) throw new DOMException('quota', 'QuotaExceededError');
      return realSet.apply(this, args as [string, string]);
    };
    try {
      const save = persistProject({ ...emptyInput, bodyParts: [custom] });
      expect(save.ok).toBe(true);
      expect(save.degraded).toBe(true);
      expect(save.droppedGeometry).toContain(baseObject.name);

      const { data, warnings } = loadPersistedProject();
      const restored = data!.bodyParts[0];
      expect(restored.customGeometry).toBeUndefined();
      expect(warnings.some(w => w.includes(baseObject.name))).toBe(true);
    } finally {
      Storage.prototype.setItem = realSet;
    }
  });

  it('clearPersistedProject removes the snapshot', () => {
    persistProject({ ...emptyInput, bodyParts: [baseObject] });
    clearPersistedProject();
    expect(loadPersistedProject().data).toBeNull();
  });
});
