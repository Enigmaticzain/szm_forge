import { afterEach, describe, expect, it } from 'vitest';
import * as THREE from 'three';
import { createSketchExtrusion } from './geometry';

const createdGeometries: Array<{ dispose: () => void }> = [];

afterEach(() => {
  createdGeometries.forEach(geometry => geometry.dispose());
  createdGeometries.length = 0;
});

describe('createSketchExtrusion', () => {
  it('creates geometry and accurate volume for a closed rectangular profile', () => {
    const extrusion = createSketchExtrusion({
      entities: [
        { id: 1, type: 'line', p1: [0, 0], p2: [100, 0] },
        { id: 2, type: 'line', p1: [100, 0], p2: [100, 100] },
        { id: 3, type: 'line', p1: [100, 100], p2: [0, 100] },
        { id: 4, type: 'line', p1: [0, 100], p2: [0, 0] },
      ],
    }, 10);

    expect(extrusion).not.toBeNull();
    createdGeometries.push(extrusion!.geometry);
    extrusion!.geometry.computeBoundingBox();
    expect(extrusion!.volumeM3).toBeCloseTo(0.0001);
    expect(extrusion!.geometry.boundingBox?.getSize(new THREE.Vector3()).toArray()).toEqual([100, 100, 10]);
  });

  it('rejects open profiles and non-positive extrusion distances', () => {
    const openProfile = createSketchExtrusion({
      entities: [
        { id: 1, type: 'line', p1: [0, 0], p2: [100, 0] },
        { id: 2, type: 'line', p1: [100, 0], p2: [100, 100] },
        { id: 3, type: 'line', p1: [100, 100], p2: [0, 100] },
      ],
    }, 10);

    expect(openProfile).toBeNull();
    expect(createSketchExtrusion({ entities: [] }, 0)).toBeNull();
  });
});
