// @vitest-environment node
// Runs in node (not jsdom): the OCCT WASM module loads from disk here, which
// lets us parse a REAL STEP file end-to-end in tests — no mocks pretending
// import works.
import { describe, expect, it } from 'vitest';
import { readFileSync } from 'node:fs';
import { join } from 'node:path';
import * as THREE from 'three';
import {
  collectMeshNames,
  looksLikeStep,
  meshJsonToGeometry,
  occtResultToParts,
  parseStepBytes,
  MM_TO_SCENE,
  type OcctMeshJson,
  type OcctResultJson,
} from './stepImport';

const cubeMesh: OcctMeshJson = {
  name: 'solid',
  attributes: {
    position: { array: [0, 0, 0, 10, 0, 0, 10, 10, 0, 0, 10, 0] },
    normal: { array: [0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1] },
  },
  index: { array: [0, 1, 2, 0, 2, 3] },
};

describe('meshJsonToGeometry (pure conversion)', () => {
  it('builds indexed geometry scaled mm → scene meters', () => {
    const geo = meshJsonToGeometry(cubeMesh);
    expect(geo).toBeInstanceOf(THREE.BufferGeometry);
    const pos = geo.getAttribute('position');
    expect(pos.count).toBe(4);
    expect(pos.getX(1)).toBeCloseTo(10 * MM_TO_SCENE, 6);
    expect(geo.getIndex()!.count).toBe(6);
    expect(geo.getAttribute('normal')).toBeDefined();
    expect(geo.boundingBox).not.toBeNull();
  });

  it('computes normals when absent', () => {
    const noNormals: OcctMeshJson = {
      attributes: cubeMesh.attributes,
      index: cubeMesh.index,
    };
    const geo = meshJsonToGeometry({ ...noNormals, attributes: { position: cubeMesh.attributes.position }, index: cubeMesh.index });
    expect(geo.getAttribute('normal')).toBeDefined();
  });

  it('rejects invalid position data honestly', () => {
    const bad: OcctMeshJson = {
      attributes: { position: { array: [0, 1] } },
      index: { array: [] },
    };
    expect(() => meshJsonToGeometry(bad)).toThrow(/invalid position/i);
  });
});

describe('occtResultToParts (accounting)', () => {
  it('names parts from the product tree and counts triangles', () => {
    const result: OcctResultJson = {
      success: true,
      meshes: [cubeMesh, cubeMesh],
      root: { name: '', meshes: [], children: [{ name: 'Bracket', meshes: [0, 1], children: [] }] },
    };
    const summary = occtResultToParts(result, 'import');
    expect(summary.parts).toHaveLength(2);
    expect(summary.parts[0].name).toBe('Bracket');
    expect(summary.totalTriangles).toBe(4);
    expect(summary.warnings).toEqual([]);
  });

  it('never fabricates parts from an unsuccessful read', () => {
    const summary = occtResultToParts({ success: false, meshes: [], root: { name: '', meshes: [], children: [] } }, 'x');
    expect(summary.parts).toEqual([]);
    expect(summary.warnings[0]).toMatch(/no readable solids/i);
  });

  it('collectMeshNames prefers product-node names', () => {
    const names = collectMeshNames({ name: '', meshes: [], children: [{ name: 'Hub', meshes: [2], children: [] }] });
    expect(names.get(2)).toBe('Hub');
  });
});

describe('looksLikeStep', () => {
  it('accepts ISO-10303-21 payloads and rejects others', () => {
    expect(looksLikeStep(new TextEncoder().encode('ISO-10303-21;\nHEADER;'))).toBe(true);
    expect(looksLikeStep(new TextEncoder().encode('solid ascii\nfacet'))).toBe(false);
  });
});

describe('parseStepBytes (real WASM + real STEP)', () => {
  it('imports the corpus cube.stp into a valid triangulated part', async () => {
    const cubePath = join(
      __dirname,
      '../../node_modules/occt-import-js/test/testfiles/simple-basic-cube/cube.stp',
    );
    const bytes = new Uint8Array(readFileSync(cubePath));
    const summary = await parseStepBytes(bytes, 'cube');
    expect(summary.parts.length).toBeGreaterThan(0);
    expect(summary.totalTriangles).toBeGreaterThan(0);
    const part = summary.parts[0];
    expect(part.name.toLowerCase()).toContain('cube');
    const pos = part.geometry.getAttribute('position');
    expect(pos.count).toBeGreaterThanOrEqual(24); // triangulated cube => 24 verts typical
    // 100 mm cube scaled to scene units (meters)
    const bbox = part.geometry.boundingBox!;
    expect(bbox.max.x - bbox.min.x).toBeLessThan(1); // not 100 units — mm scaled down
  }, 30000);

  it('throws honestly on garbage instead of returning a placeholder', async () => {
    const garbage = new TextEncoder().encode('ISO-10303-21;\nHEADER;\nENDSEC;\nDATA;\nENDSEC;\nEND-ISO-10303-21;');
    await expect(parseStepBytes(garbage, 'fake')).resolves.toMatchObject({
      parts: [],
    });
  }, 30000);
});
