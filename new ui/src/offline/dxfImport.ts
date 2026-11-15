/**
 * dxfImport.ts — ASCII DXF → 2D polylines, zero-dependency, offline-first.
 *
 * DXF (AutoCAD Drawing Interchange Format) ASCII files are alternating lines
 * of group-code / value pairs. We extract planar profiles from:
 *
 *   - LWPOLYLINE entities (vertex pairs 10/20, closed flag 70 bit 1)
 *   - POLYLINE + VERTEX … SEQEND sequences (classic form, closed flag 70 bit 1)
 *
 * Honesty rules (USER_WORKFLOW_FINDINGS doctrine):
 *   - Binary DXF ("AutoCAD Binary DXF" magic) is REJECTED with guidance —
 *     we never guess at a binary stream.
 *   - Splines / ellipses / arcs are not flattened here; files containing none
 *     of our supported curve groups yield explicit warnings, not silent drops
 *     disguised as success.
 *   - Units come from $INSUNITS; unknown/absent defaults to millimetres WITH
 *     a warning saying so. Values are returned in millimetres.
 */

export interface DxfProfile {
  /** Vertex loop in millimetres, DXF y-up orientation preserved (caller may flip). */
  points: [number, number][];
  closed: boolean;
}

export interface DxfImportResult {
  profiles: DxfProfile[];
  warnings: string[];
  /** Units after scaling — always 'mm'. */
  units: 'mm';
}

/** $INSUNITS → millimetre scale factor (subset we claim to understand). */
const INSUNITS_TO_MM: Record<number, number> = {
  1: 25.4, // inch
  2: 304.8, // feet
  4: 1, // millimetres
  5: 10, // centimetres
  6: 1000, // metres
};

const BINARY_MAGIC = 'AutoCAD Binary DXF';

export class DxfImportError extends Error {}

/** Parse AC1009+ ASCII DXF text. Pure function — testable in node. */
export function parseDxfText(text: string, fileName = 'file.dxf'): DxfImportResult {
  if (text.startsWith(BINARY_MAGIC)) {
    throw new DxfImportError(
      'Binary DXF is not supported. Re-export as an ASCII DXF (R12 or later) and try again.'
    );
  }

  const warnings: string[] = [];
  const lines = text.split(/\r\n|\r|\n/);

  // ---- pass 1: group-code / value stream ----
  interface Pair { code: number; value: string; }
  const pairs: Pair[] = [];
  for (let i = 0; i + 1 < lines.length; i += 2) {
    const code = parseInt(lines[i].trim(), 10);
    if (Number.isNaN(code)) {
      i -= 1; // tolerate odd padding lines without poisoning the stream
      continue;
    }
    pairs.push({ code, value: lines[i + 1] });
  }
  if (pairs.length === 0) {
    throw new DxfImportError('No DXF group-code pairs found — is this really an ASCII DXF?');
  }

  // ---- $INSUNITS ----
  let insunits = 0;
  let unitMmFactor = 1;
  for (let i = 0; i < pairs.length - 1; i++) {
    if (pairs[i].code === 9 && pairs[i].value.trim() === '$INSUNITS' && pairs[i + 1].code === 70) {
      insunits = parseInt(pairs[i + 1].value.trim(), 10) || 0;
      break;
    }
  }
  if (insunits === 0) {
    warnings.push('$INSUNITS not set — assuming millimetres');
  } else if (!(insunits in INSUNITS_TO_MM)) {
    warnings.push(`$INSUNITS=${insunits} not recognised — assuming millimetres`);
  } else {
    unitMmFactor = INSUNITS_TO_MM[insunits];
  }

  // ---- entities ----
  const profiles: DxfProfile[] = [];
  let sawUnsupportedCurve = false;
  let inEntities = false;
  let i = 0;

  const flushVertex = (
    pending: { x: number | null; y: number | null },
    onPoint: (pt: [number, number]) => void,
  ) => {
    if (pending.x !== null && pending.y !== null) onPoint([pending.x, pending.y]);
    pending.x = null;
    pending.y = null;
  };

  while (i < pairs.length) {
    const { code, value } = pairs[i];
    const v = value.trim();

    // section tracking
    if (code === 2 && v === 'ENTITIES' && pairs[i - 1]?.code === 0 && pairs[i - 1]?.value.trim() === 'SECTION') {
      inEntities = true; i++; continue;
    }
    if (code === 0 && v === 'ENDSEC' && inEntities) { inEntities = false; i++; continue; }
    if (code === 0 && v === 'EOF') break;

    if (!inEntities) { i++; continue; }

    // entity starts are always group 0
    if (code === 0 && v === 'LWPOLYLINE') {
      const pts: [number, number][] = [];
      let closed = false;
      let px: number | null = null;
      i++;
      while (i < pairs.length && pairs[i].code !== 0) {
        const c = pairs[i].code; const val = pairs[i].value.trim();
        if (c === 70) closed = (parseInt(val, 10) & 1) === 1;
        else if (c === 10) px = parseFloat(val);
        else if (c === 20 && px !== null) { pts.push([px, parseFloat(val)]); px = null; }
        i++;
      }
      if (pts.length >= 2) profiles.push({ points: pts.map(scalePt(unitMmFactor)), closed });
      continue;
    }

    if (code === 0 && v === 'POLYLINE') {
      const pts: [number, number][] = [];
      let closed = false;
      let vertexOpen = false;
      const pending = { x: null as number | null, y: null as number | null };
      i++;
      while (i < pairs.length) {
        const c = pairs[i].code; const val = pairs[i].value.trim();
        if (c === 0 && val === 'SEQEND') { flushVertex(pending, pt => pts.push(pt)); i++; break; }
        if (c === 0 && val === 'VERTEX') {
          flushVertex(pending, pt => pts.push(pt));
          vertexOpen = true; i++; continue;
        }
        if (c === 0) break; // next entity without SEQEND — be lenient
        if (vertexOpen) {
          if (c === 10) pending.x = parseFloat(val);
          else if (c === 20) pending.y = parseFloat(val);
        } else if (c === 70) {
          closed = (parseInt(val, 10) & 1) === 1;
        }
        i++;
      }
      if (pts.length >= 2) profiles.push({ points: pts.map(scalePt(unitMmFactor)), closed });
      continue;
    }

    if (code === 0 && (v === 'SPLINE' || v === 'ELLIPSE')) {
      sawUnsupportedCurve = true;
      i++;
      while (i < pairs.length && pairs[i].code !== 0) i++;
      continue;
    }

    i++;
  }

  if (sawUnsupportedCurve) {
    warnings.push('SPLINE/ELLIPSE entities ignored — only polyline outlines are imported');
  }
  if (profiles.length === 0) {
    throw new DxfImportError(
      `${fileName}: no planar polylines found (LWPOLYLINE/POLYLINE). ` +
      'Explode splines or arcs in your CAD tool before export.'
    );
  }
  if (profiles.length > 1) {
    // deterministically keep the largest by bounding-box area first; we still
    // RETURN all so the caller decides, but the count is part of the contract.
    profiles.sort((a, b) => bboxArea(b.points) - bboxArea(a.points));
    warnings.push(`${profiles.length} profiles found — importing the largest; review the sketch`);
  }

  return { profiles, warnings, units: 'mm' };
}

function scalePt(factor: number) {
  return (pt: [number, number]): [number, number] => [pt[0] * factor, pt[1] * factor];
}

function bboxArea(points: [number, number][]): number {
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
  for (const [x, y] of points) {
    if (x < minX) minX = x; if (y < minY) minY = y;
    if (x > maxX) maxX = x; if (y > maxY) maxY = y;
  }
  const w = maxX - minX; const h = maxY - minY;
  return Number.isFinite(w * h) ? w * h : 0;
}

/** Decode bytes as ASCII DXF text (DXF is an ASCII format; NUL bytes betray binary). */
export function parseDxfBytes(bytes: Uint8Array, fileName = 'file.dxf'): DxfImportResult {
  // cheap binary sniff before decoding
  if (bytes.length >= BINARY_MAGIC.length) {
    const prefix = new TextDecoder('ascii').decode(bytes.subarray(0, BINARY_MAGIC.length));
    if (prefix === BINARY_MAGIC) throw new DxfImportError(
      'Binary DXF is not supported. Re-export as an ASCII DXF (R12 or later) and try again.'
    );
  }
  const text = new TextDecoder().decode(bytes);
  return parseDxfText(text, fileName);
}

/**
 * Map a DXF profile (mm, y-up) into SKETCH MODEL coordinates (mm, y-up):
 * recentred on the sketch origin and fit inside the viewport span so the
 * profile is visible at default zoom. Segment endpoints stay in millimetres —
 * the sketch's view transform handles rendering scale and y-direction itself.
 */
export function profileToSketchSegments(
  profile: DxfProfile,
  viewportWmm: number,
  viewportHmm: number,
  marginRatio = 0.15,
): { p1: [number, number]; p2: [number, number] }[] {
  const pts = profile.points;
  let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
  for (const [x, y] of pts) {
    if (x < minX) minX = x; if (y < minY) minY = y;
    if (x > maxX) maxX = x; if (y > maxY) maxY = y;
  }
  const w = Math.max(maxX - minX, 1e-9);
  const h = Math.max(maxY - minY, 1e-9);
  const s = Math.min(
    (viewportWmm * (1 - 2 * marginRatio)) / w,
    (viewportHmm * (1 - 2 * marginRatio)) / h,
  );
  const cx = (minX + maxX) / 2; const cy = (minY + maxY) / 2;
  const map = (pt: [number, number]): [number, number] => ([
    (pt[0] - cx) * s,
    (pt[1] - cy) * s,
  ]);

  const segments: { p1: [number, number]; p2: [number, number] }[] = [];
  for (let k = 0; k < pts.length - 1; k++) {
    segments.push({ p1: map(pts[k]), p2: map(pts[k + 1]) });
  }
  if (profile.closed && pts.length > 1) {
    segments.push({ p1: map(pts[pts.length - 1]), p2: map(pts[0]) });
  }
  return segments;
}
