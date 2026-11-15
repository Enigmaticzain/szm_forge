// @vitest-environment node
import { describe, expect, it } from 'vitest';
import { DxfImportError, parseDxfBytes, parseDxfText, profileToSketchSegments } from './dxfImport';

/** Assemble an ASCII DXF from raw (code, value) rows. */
function dxf(rows: [number | string, string][]): string {
  return rows.map(([c, v]) => `${String(typeof c === 'number' ? c : c.toString()).padStart(3, ' ')}\n${v}`).join('\n') + '\n  0\nEOF\n';
}

const CLOSED_SQUARE: [number | string, string][] = [
  ['  9', '$ACADVER'], ['  1', 'AC1009'],
  ['  9', '$INSUNITS'], [' 70', '4'], // mm
  ['  0', 'SECTION'], ['  2', 'ENTITIES'],
  ['  0', 'LWPOLYLINE'],
  [' 90', '4'], [' 70', '1'], // closed
  [' 10', '10'], [' 20', '20'],
  [' 10', '110'], [' 20', '20'],
  [' 10', '110'], [' 20', '120'],
  [' 10', '10'], [' 20', '120'],
  ['  0', 'ENDSEC'],
];

describe('parseDxfText', () => {
  it('parses a closed LWPOLYLINE in mm', () => {
    const r = parseDxfText(dxf(CLOSED_SQUARE));
    expect(r.units).toBe('mm');
    expect(r.profiles).toHaveLength(1);
    expect(r.profiles[0].closed).toBe(true);
    expect(r.profiles[0].points[0]).toEqual([10, 20]);
    expect(r.warnings).toEqual([]);
  });

  it('scales $INSUNITS=1 (inches) into millimetres', () => {
    const rows = CLOSED_SQUARE.map(([c, v]) =>
      c === 70 && v === '4' && String(c) === ' 70' ? [c, v] : [c, v],
    ) as [number | string, string][];
    const inchRows: [number | string, string][] = rows.map(([c, v], i) =>
      i === 3 ? [' 70', '1'] : [c, v],
    );
    const r = parseDxfText(dxf(inchRows));
    expect(r.profiles[0].points[0][0]).toBeCloseTo(10 * 25.4, 5);
    expect(r.profiles[0].points[0][1]).toBeCloseTo(20 * 25.4, 5);
  });

  it('warns when $INSUNITS is absent and assumes mm', () => {
    const rows = [...CLOSED_SQUARE.slice(0, 2), ...CLOSED_SQUARE.slice(4)] as [number | string, string][];
    const r = parseDxfText(dxf(rows));
    expect(r.profiles[0].points[0]).toEqual([10, 20]);
    expect(r.warnings.some(w => /assuming millimetres/i.test(w))).toBe(true);
  });

  it('parses classic POLYLINE/VERTEX/SEQEND (open)', () => {
    const rows: [number | string, string][] = [
      ['  9', '$INSUNITS'], [' 70', '4'],
      ['  0', 'SECTION'], ['  2', 'ENTITIES'],
      ['  0', 'POLYLINE'], [' 70', '0'],
      ['  0', 'VERTEX'], [' 10', '0'], [' 20', '0'],
      ['  0', 'VERTEX'], [' 10', '50'], [' 20', '0'],
      ['  0', 'VERTEX'], [' 10', '50'], [' 20', '30'],
      ['  0', 'SEQEND'],
      ['  0', 'ENDSEC'],
    ];
    const r = parseDxfText(dxf(rows));
    expect(r.profiles[0].closed).toBe(false);
    expect(r.profiles[0].points).toEqual([[0, 0], [50, 0], [50, 30]]);
  });

  it('throws an honest error for a file with no polylines', () => {
    const rows: [number | string, string][] = [
      ['  0', 'SECTION'], ['  2', 'ENTITIES'],
      ['  0', 'CIRCLE'], [' 10', '0'], [' 20', '0'], [' 40', '25'],
      ['  0', 'ENDSEC'],
    ];
    expect(() => parseDxfText(dxf(rows))).toThrowError(/no planar polylines/i);
  });

  it('warns when unsupported curve entities are skipped', () => {
    const rows: [number | string, string][] = [
      ...CLOSED_SQUARE.slice(0, 6), // header + SECTION ENTITIES
      ['  0', 'SPLINE'], [' 70', '8'],
      ...CLOSED_SQUARE.slice(6),
    ];
    const r = parseDxfText(dxf(rows as [number | string, string][]));
    expect(r.warnings.some(w => /spline/i.test(w))).toBe(true);
    expect(r.profiles).toHaveLength(1);
  });

  it('orders largest profile first and warns about extras', () => {
    const small: [number | string, string][] = [
      ['  0', 'LWPOLYLINE'], [' 70', '1'],
      [' 10', '0'], [' 20', '0'], [' 10', '1'], [' 20', '0'], [' 10', '1'], [' 20', '1'],
    ];
    const rows = [...CLOSED_SQUARE.slice(0, 6), ...small, ...CLOSED_SQUARE.slice(6)] as [number | string, string][];
    const r = parseDxfText(dxf(rows));
    expect(r.profiles).toHaveLength(2);
    expect(r.profiles[0].points[0]).toEqual([10, 20]); // the big profile comes first
    expect(r.warnings.some(w => /2 profiles/i.test(w))).toBe(true);
  });
});

describe('parseDxfBytes', () => {
  it('rejects binary DXF honestly', () => {
    const bytes = new TextEncoder().encode('AutoCAD Binary DXF\r\n\x1a\x00garbage');
    expect(() => parseDxfBytes(bytes, 'bin.dxf')).toThrowError(DxfImportError);
    expect(() => parseDxfBytes(bytes, 'bin.dxf')).toThrowError(/Binary DXF/);
  });

  it('decodes ascii text bytes', () => {
    const r = parseDxfBytes(new TextEncoder().encode(dxf(CLOSED_SQUARE)));
    expect(r.profiles).toHaveLength(1);
  });
});

describe('profileToSketchSegments', () => {
  it('recentres, keeps mm scale ratio, closes the loop, preserves handedness', () => {
    const r = parseDxfText(dxf(CLOSED_SQUARE));
    const segs = profileToSketchSegments(r.profiles[0], 360, 240); // 0.7*240/100 = 1.68 scale
    expect(segs).toHaveLength(4);
    // square is 100x100mm centred at (60,70) → span s=(0.7*240/100)=1.68 → 168x168mm centred 0,0
    const xs = segs.flatMap(s => [s.p1[0], s.p2[0]]);
    const ys = segs.flatMap(s => [s.p1[1], s.p2[1]]);
    expect(Math.max(...xs)).toBeCloseTo(84, 3);
    expect(Math.min(...xs)).toBeCloseTo(-84, 3);
    expect(Math.max(...ys)).toBeCloseTo(84, 3);
    expect(Math.min(...ys)).toBeCloseTo(-84, 3);
    // closed loop: last segment returns to first point
    expect(segs[3].p2).toEqual(segs[0].p1);
    // no y-flip: first DXF point (10,20) → model (−84,−84) — bottom-left stays bottom-left
    expect(segs[0].p1[0]).toBeCloseTo(-84, 3);
    expect(segs[0].p1[1]).toBeCloseTo(-84, 3);
  });

  it('leaves open polylines without a closing segment', () => {
    const r = parseDxfText(dxf([
      ['  9', '$INSUNITS'], [' 70', '4'],
      ['  0', 'SECTION'], ['  2', 'ENTITIES'],
      ['  0', 'LWPOLYLINE'], [' 70', '0'],
      [' 10', '0'], [' 20', '0'], [' 10', '40'], [' 20', '0'], [' 10', '40'], [' 20', '30'],
      ['  0', 'ENDSEC'],
    ]));
    const segs = profileToSketchSegments(r.profiles[0], 360, 240);
    expect(segs).toHaveLength(2);
  });
});
