// @vitest-environment node
import { describe, expect, it } from 'vitest';
import * as THREE from 'three';
import { geometryToMeshArrays, sanitizePrimName, sceneToUsda } from './usdExport';
import type { SceneObject } from '../components/BodyDesign/InteractivePartEditor';

function makeBox(id = 'b1', name = 'Bracket A'): SceneObject {
  return {
    id, name, type: 'box',
    position: { x: 0.5, y: 0, z: -0.25 },
    size: { x: 0.2, y: 0.1, z: 0.05 },
  };
}

describe('sanitizePrimName', () => {
  it('produces legal USD identifiers', () => {
    const used = new Set<string>();
    expect(sanitizePrimName('Sketch Extrusion 1', 'x', used)).toBe('Sketch_Extrusion_1');
    expect(sanitizePrimName('42nd part', 'x', used)).toBe('_42nd_part');
    expect(sanitizePrimName('!!!', 'fallback', used)).toBe('fallback');
  });
  it('dedupes collisions', () => {
    const used = new Set<string>();
    sanitizePrimName('Hub', 'x', used);
    expect(sanitizePrimName('Hub', 'x', used)).toBe('Hub_2');
  });
});

describe('geometryToMeshArrays', () => {
  it('keeps indexed meshes compact (positions shared)', () => {
    const box = new THREE.BoxGeometry(1, 1, 1); // indexed: 24 verts, 36 idx
    const m = geometryToMeshArrays(box);
    expect(m.positions.length).toBe(24 * 3);
    expect(m.indices.length).toBe(36);
    expect(m.normals).not.toBeNull();
    expect(m.extentMax[0]).toBeCloseTo(0.5);
  });
  it('expands non-indexed geometry to sequential indices', () => {
    const g = new THREE.BufferGeometry();
    g.setAttribute('position', new THREE.BufferAttribute(new Float32Array([0,0,0, 1,0,0, 0,1,0]), 3));
    const m = geometryToMeshArrays(g);
    expect(Array.from(m.indices)).toEqual([0, 1, 2]);
  });
  it('throws on empty geometry', () => {
    expect(() => geometryToMeshArrays(new THREE.BufferGeometry())).toThrow(/no vertices/i);
  });
});

describe('sceneToUsda', () => {
  it('writes a complete USDA: header, xform translate, mesh arrays, extent', () => {
    const { usda, partCount, triangleCount, fileName } = sceneToUsda([makeBox()], { projectName: 'My Rig' });
    expect(fileName).toBe('my_rig.usda');
    expect(partCount).toBe(1);
    expect(triangleCount).toBe(12); // closed box = 12 triangles
    expect(usda.startsWith('#usda 1.0')).toBe(true);
    expect(usda).toContain('def Xform "Scene"');
    expect(usda).toContain('def Xform "Bracket_A"');
    expect(usda).toContain('float3 xformOp:translate = (0.5, 0, -0.25)');
    expect(usda).toContain('def Mesh "Geom"');
    expect(usda).toContain('int[] faceVertexCounts = [3');
    expect(usda).toContain('point3f[] points = [');
    expect(usda).toContain('normal3f[] normals = [');
    expect(usda).toContain('float3[] extent =');
    expect(usda).toContain('metersPerUnit = 1');
  });

  it('bends export unbent and SAY SO in warnings', () => {
    const bent = { ...makeBox('b2', 'Plate B'), type: 'plate' as const, bendAngle: 1.2 };
    const result = sceneToUsda([bent]);
    expect(result.warnings.some(w => /unbent/i.test(w))).toBe(true);
  });

  it('exports custom imported geometry verbatim (STEP round-trip mesh)', () => {
    const custom: SceneObject = {
      id: 's1', name: 'STEP solid', type: 'custom',
      position: { x: 0, y: 0, z: 0 },
      size: { x: 0.1, y: 0.1, z: 0.1 },
      customGeometry: new THREE.BoxGeometry(0.1, 0.1, 0.1),
    };
    const { usda, triangleCount } = sceneToUsda([custom]);
    expect(triangleCount).toBe(12);
    expect(usda).toContain('def Xform "STEP_solid"');
    expect(usda).toContain('float3[] extent = [(-0.05, -0.05, -0.05), (0.05, 0.05, 0.05)]');
  });

  it('throws on an empty scene instead of emitting a fake file', () => {
    expect(() => sceneToUsda([])).toThrow(/nothing to export/i);
  });

  it('marks mm sources with metersPerUnit = 0.001', () => {
    const { usda } = sceneToUsda([makeBox()], { units: 'mm' });
    expect(usda).toContain('metersPerUnit = 0.001');
  });

  it('skips degenerate parts but keeps the rest, with a named warning', () => {
    const bad: SceneObject = { id: 'bad', name: 'Empty Mesh', type: 'custom', position: { x: 0, y: 0, z: 0 }, size: { x: 1, y: 1, z: 1 }, customGeometry: new THREE.BufferGeometry() };
    const result = sceneToUsda([bad, makeBox()]);
    expect(result.partCount).toBe(1);
    expect(result.warnings.some(w => w.includes('Empty Mesh'))).toBe(true);
    expect(result.usda).toContain('def Xform "Bracket_A"');
  });
});
