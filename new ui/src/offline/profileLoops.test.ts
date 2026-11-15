// @vitest-environment node
import { describe, expect, it } from 'vitest';
import { assembleLoops, type Segment2 } from './profileLoops';
import { createSketchExtrusion, createSketchRevolution, revolveSketchOffline, type SketchEntity } from './geometry';

const square = (size = 100, ox = 0, oy = 0): Segment2[] => ([
  { p1: [ox, oy], p2: [ox + size, oy] },
  { p1: [ox + size, oy], p2: [ox + size, oy + size] },
  { p1: [ox + size, oy + size], p2: [ox, oy + size] },
  { p1: [ox, oy + size], p2: [ox, oy] },
]);

function shuffled<T>(arr: T[]): T[] {
  const a = [...arr];
  for (let i = a.length - 1; i > 0; i--) {
    const j = (i * 7 + 3) % (i + 1); // deterministic shuffle
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

describe('assembleLoops', () => {
  it('assembles a closed loop from unordered segments', () => {
    const r = assembleLoops(shuffled(square(50)));
    expect(r.loops).toHaveLength(1);
    expect(r.loops[0]).toHaveLength(4);
    expect(r.openChains).toBe(0);
  });

  it('snaps tiny endpoint gaps (≤ tolerance) without bridging larger ones', () => {
    const segs = square(50);
    // introduce 10µm mismatch on one corner
    segs[0] = { p1: [0, 0], p2: [50.00001, 0] };
    segs[1] = { p1: [50, 0], p2: [50, 50] };
    const r = assembleLoops(segs, 0.05);
    expect(r.loops).toHaveLength(1);

    // 100µm mismatch (bigger than default tolerance) → open chain, no loop
    const segs2 = square(50);
    segs2[0] = { p1: [0, 0], p2: [50.1, 0] };
    const r2 = assembleLoops(segs2, 0.05);
    expect(r2.loops).toHaveLength(0);
    expect(r2.openChains).toBeGreaterThan(0);
  });

  it('keeps multiple loops, largest first', () => {
    const r = assembleLoops([...square(10, 200), ...square(80)]);
    expect(r.loops).toHaveLength(2);
    const area0 = Math.abs(r.loops[0].length && areaOf(r.loops[0]));
    const area1 = Math.abs(areaOf(r.loops[1]));
    expect(area0).toBeGreaterThan(area1);
  });

  it('reports open chains instead of inventing closure', () => {
    const r = assembleLoops([
      { p1: [0, 0], p2: [10, 0] },
      { p1: [10, 0], p2: [20, 0] },
    ]);
    expect(r.loops).toHaveLength(0);
    expect(r.openChains).toBe(1);
    expect(r.unclosedComponents).toBe(1);
  });
});

function areaOf(loop: [number, number][]): number {
  let a = 0;
  for (let i = 0; i < loop.length; i++) {
    const [x1, y1] = loop[i];
    const [x2, y2] = loop[(i + 1) % loop.length];
    a += x1 * y2 - x2 * y1;
  }
  return a / 2;
}

const toEntities = (segs: Segment2[]): SketchEntity[] =>
  segs.map((s, i) => ({ id: i + 1, type: 'line' as const, p1: s.p1, p2: s.p2 }));

describe('createSketchExtrusion — order-agnostic path', () => {
  it('extrudes a 50×50 mm unordered square to 2 mm: 50·50·2 = 5000 mm³', () => {
    const ex = createSketchExtrusion({ entities: toEntities(shuffled(square(50))) }, 2);
    expect(ex).not.toBeNull();
    expect(ex!.volumeM3).toBeCloseTo(5000 / 1e9, 12);
    ex!.geometry.dispose();
  });

  it('fails honestly for an open profile', () => {
    const ex = createSketchExtrusion({
      entities: toEntities([
        { p1: [0, 0], p2: [50, 0] },
        { p1: [50, 0], p2: [50, 50] },
        { p1: [50, 50], p2: [0, 50] },
      ]),
    }, 2);
    expect(ex).toBeNull();
  });
});

describe('createSketchRevolution', () => {
  it('revolves a 20×50 mm rectangle at u∈[5,25] into an annular cylinder — Pappus check', () => {
    const rect: Segment2[] = [
      { p1: [5, 0], p2: [25, 0] },
      { p1: [25, 0], p2: [25, 50] },
      { p1: [25, 50], p2: [5, 50] },
      { p1: [5, 50], p2: [5, 0] },
    ];
    const rev = createSketchRevolution({ entities: toEntities(shuffled(rect)) }, 16);
    expect(rev).not.toBeNull();
    // V = 2π · Rc · A = 2π · 15 · 1000 mm³
    expect(rev!.volumeM3).toBeCloseTo((2 * Math.PI * 15 * 1000) / 1e9, 12);
    expect(rev!.warnings).toEqual([]);
    rev!.geometry.dispose();
  });

  it('warns when shifting a profile that crosses the axis instead of silently mirroring', () => {
    const crossing: Segment2[] = [
      { p1: [-10, 0], p2: [10, 0] },
      { p1: [10, 0], p2: [10, 20] },
      { p1: [10, 20], p2: [-10, 20] },
      { p1: [-10, 20], p2: [-10, 0] },
    ];
    const rev = createSketchRevolution({ entities: toEntities(crossing) }, 16);
    expect(rev).not.toBeNull();
    expect(rev!.warnings.some(w => /crossed the axis/.test(w))).toBe(true);
    rev!.geometry.dispose();
  });

  it('revolveSketchOffline fails honestly on circles-only sketches', () => {
    const r = revolveSketchOffline({
      entities: [{ id: 1, type: 'circle', p1: [0, 0], p2: [10, 0] }],
    });
    expect(r.ok).toBe(false);
    expect(r.error).toMatch(/closed line profile/i);
  });
});
