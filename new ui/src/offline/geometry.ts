import * as THREE from 'three';
import { STLExporter } from 'three/examples/jsm/exporters/STLExporter.js';
import { assembleLoops } from './profileLoops';

/** Endpoint snap tolerance when assembling unordered sketch segments (mm). */
const SKETCH_SNAP_TOLERANCE_MM = 0.05;

export interface CadSpec {
  shape: string;
  material: string;
  features: Record<string, boolean>;
  length?: number;
  width?: number;
  height?: number;
  radius?: number;
  radius1?: number;
  radius2?: number;
  major_radius?: number;
  minor_radius?: number;
}

export interface TextToCadResult {
  status: string;
  prompt: string;
  spec: CadSpec;
  step_file: null;
  step_available: false;
  geometry: Record<string, unknown>;
  engine: string;
}

function numFrom(text: string, patterns: RegExp[]): number | undefined {
  for (const p of patterns) {
    const m = text.match(p);
    if (m) return parseFloat(m[1]);
  }
  return undefined;
}

export function parseTextToCad(prompt: string): TextToCadResult {
  const lower = prompt.toLowerCase();
  const material =
    lower.includes('titanium') || lower.includes('ti-6') ? 'titanium'
    : lower.includes('aluminum') || lower.includes('aluminium') || lower.includes('al ') ? 'aluminum'
    : lower.includes('copper') ? 'copper'
    : lower.includes('steel') ? 'steel'
    : 'steel';

  let shape = 'box';
  if (lower.includes('cylinder') || lower.includes('shaft') || lower.includes('pipe') || lower.includes('rod') || lower.includes('pin')) shape = 'cylinder';
  else if (lower.includes('sphere') || lower.includes('ball')) shape = 'sphere';
  else if (lower.includes('cone')) shape = 'cone';
  else if (lower.includes('torus') || lower.includes('ring') || lower.includes('washer')) shape = 'torus';
  else if (lower.includes('hex')) shape = 'cylinder';
  else if (
    lower.includes('cube') || lower.includes('block') || lower.includes('bracket')
    || lower.includes('plate')
  ) shape = 'box';

  // ASCII "100x50x20" or unicode "100×50×20"
  const xyz = lower.match(/(\d+(?:\.\d+)?)\s*[x×]\s*(\d+(?:\.\d+)?)\s*[x×]\s*(\d+(?:\.\d+)?)/);
  const xy = !xyz ? lower.match(/(\d+(?:\.\d+)?)\s*[x×]\s*(\d+(?:\.\d+)?)/) : null;

  const length = numFrom(lower, [/(\d+(?:\.\d+)?)\s*mm\s*(?:long|length)/, /length\s*(\d+)/])
    ?? (xyz ? parseFloat(xyz[1]) : undefined)
    ?? (xy ? parseFloat(xy[1]) : undefined)
    ?? (lower.includes('cube') ? 50 : 100);
  const width = numFrom(lower, [/(\d+(?:\.\d+)?)\s*mm\s*(?:wide|width)/, /width\s*(\d+)/, /[x×]\s*(\d+(?:\.\d+)?)/])
    ?? (xyz ? parseFloat(xyz[2]) : undefined)
    ?? (xy ? parseFloat(xy[2]) : undefined)
    ?? (lower.includes('cube') ? length : 60);
  const height = numFrom(lower, [/(\d+(?:\.\d+)?)\s*mm\s*(?:tall|high|height|thick)/, /height\s*(\d+)/])
    ?? (xyz ? parseFloat(xyz[3]) : undefined)
    ?? (lower.includes('cube') ? length : lower.includes('plate') ? 5 : 15);
  const radius = numFrom(lower, [/radius\s*(\d+(?:\.\d+)?)/, /(\d+(?:\.\d+)?)\s*mm\s*radius/])
    ?? (lower.includes('washer') ? 20 : lower.includes('pin') || lower.includes('rod') ? 5 : 25);

  const spec: CadSpec = {
    shape,
    material,
    features: {
      fillets: lower.includes('fillet'),
      chamfer: lower.includes('chamfer'),
      hollow: lower.includes('hollow') || lower.includes('pipe'),
    },
    length,
    width,
    height,
    radius,
    major_radius: numFrom(lower, [/major\s*radius\s*(\d+)/]) ?? 50,
    minor_radius: numFrom(lower, [/minor\s*radius\s*(\d+)/, /tube\s*(\d+)/]) ?? 10,
  };

  const geometry = buildGeometryDescriptor(spec);

  return {
    status: 'ok',
    prompt,
    spec,
    step_file: null,
    step_available: false,
    geometry,
    engine: 'SZM Offline CAD Parser',
  };
}

export function buildGeometryDescriptor(spec: CadSpec): Record<string, unknown> {
  switch (spec.shape) {
    case 'cylinder':
      return {
        type: 'cylinder',
        radiusTop: spec.radius ?? 25,
        radiusBottom: spec.radius ?? 25,
        height: spec.length ?? spec.height ?? 100,
        radialSegments: 32,
      };
    case 'sphere':
      return { type: 'sphere', radius: spec.radius ?? 40, widthSegments: 32, heightSegments: 16 };
    case 'cone':
      return {
        type: 'cone',
        radiusTop: 0,
        radiusBottom: spec.radius ?? 30,
        height: spec.height ?? 150,
      };
    case 'torus':
      return {
        type: 'torus',
        radius: spec.major_radius ?? 50,
        tube: spec.minor_radius ?? 10,
        radialSegments: 16,
        tubularSegments: 48,
      };
    default:
      return {
        type: 'box',
        width: spec.width ?? 60,
        height: spec.height ?? 15,
        depth: spec.length ?? 120,
      };
  }
}

export function geometryToThree(spec: Record<string, unknown>): THREE.BufferGeometry {
  const type = spec.type as string;
  if (type === 'cylinder') {
    return new THREE.CylinderGeometry(
      spec.radiusTop as number, spec.radiusBottom as number,
      spec.height as number, (spec.radialSegments as number) ?? 32,
    );
  }
  if (type === 'sphere') {
    return new THREE.SphereGeometry(spec.radius as number, 32, 16);
  }
  if (type === 'cone') {
    return new THREE.CylinderGeometry(
      spec.radiusTop as number, spec.radiusBottom as number,
      spec.height as number, 32,
    );
  }
  if (type === 'torus') {
    return new THREE.TorusGeometry(
      spec.radius as number, spec.tube as number,
      (spec.radialSegments as number) ?? 16, (spec.tubularSegments as number) ?? 48,
    );
  }
  return new THREE.BoxGeometry(
    spec.width as number, spec.height as number, spec.depth as number,
  );
}

export function exportGeometryStlBlob(spec: Record<string, unknown>): string {
  const geo = geometryToThree(spec);
  const mesh = new THREE.Mesh(geo, new THREE.MeshStandardMaterial());
  const exporter = new STLExporter();
  const stl = exporter.parse(mesh, { binary: true });
  const bytes = new Uint8Array(stl.buffer as ArrayBuffer, stl.byteOffset, stl.byteLength);
  const blob = new Blob([bytes], { type: 'application/octet-stream' });
  return URL.createObjectURL(blob);
}

export function createParametricPart(
  partType: string,
  parameters: Record<string, unknown>,
): { render_url: string; geometry: Record<string, unknown> } {
  let geo: Record<string, unknown>;

  switch (partType) {
    case 'shaft':
      geo = {
        type: 'cylinder',
        radiusTop: (parameters.diameter as number ?? 20) / 2,
        radiusBottom: (parameters.diameter as number ?? 20) / 2,
        height: parameters.length ?? 100,
      };
      break;
    case 'flange':
      geo = {
        type: 'cylinder',
        radiusTop: (parameters.outer_d as number ?? 80) / 2,
        radiusBottom: (parameters.outer_d as number ?? 80) / 2,
        height: parameters.thickness ?? 10,
      };
      break;
    case 'gear':
    case 'housing':
    case 'bracket':
    case 'plate':
    default: {
      const L = parameters.length ?? parameters.outer_d ?? 80;
      const W = parameters.width ?? parameters.inner_d ?? 40;
      const H = parameters.thickness ?? parameters.height ?? 5;
      geo = { type: 'box', width: W, height: H, depth: L };
      break;
    }
  }

  const render_url = exportGeometryStlBlob(geo);
  return { render_url, geometry: geo };
}

export function analyzePart(
  partType: string,
  parameters: Record<string, unknown>,
): Record<string, unknown> {
  const L = (parameters.length as number) ?? 80;
  const W = (parameters.width as number) ?? 40;
  const T = (parameters.thickness as number) ?? 5;
  return {
    ok: true,
    part_type: partType,
    bounding_box: { x: L, y: W, z: T },
    volume_mm3: L * W * T,
    manufacturability: {
      warnings: T < 2 ? ['Wall thickness below 2mm — may be difficult for CNC'] : [],
      score: T >= 2 ? 0.92 : 0.65,
    },
    engine: 'SZM Offline Geometry',
  };
}

export interface SketchEntity {
  id: number;
  type: 'point' | 'line' | 'circle' | 'arc';
  p1: [number, number];
  p2?: [number, number];
  p3?: [number, number];
}

export interface SketchExtrusion {
  geometry: THREE.ExtrudeGeometry;
  volumeM3: number;
}

export function createSketchExtrusion(
  sketch: { entities: SketchEntity[] },
  distanceMm: number,
): SketchExtrusion | null {
  if (!Number.isFinite(distanceMm) || distanceMm <= 0) return null;

  const shape = new THREE.Shape();
  const circles = sketch.entities.filter(e => e.type === 'circle' && e.p2);
  const lines = sketch.entities.filter(e => e.type === 'line' && e.p2);
  let areaMm2: number;

  if (circles.length === 1 && lines.length === 0 && sketch.entities.length === 1) {
    const circle = circles[0];
    const radius = Math.hypot(circle.p2![0] - circle.p1[0], circle.p2![1] - circle.p1[1]);
    if (radius <= 0) return null;
    shape.absarc(circle.p1[0], circle.p1[1], radius, 0, Math.PI * 2, false);
    areaMm2 = Math.PI * radius * radius;
  } else if (lines.length >= 3 && lines.length === sketch.entities.length) {
    // Fast path: entities already in draw order.
    const first = lines[0];
    const points: THREE.Vector2[] = [new THREE.Vector2(...first.p1)];
    let previous = first.p1;
    const closeEnough = (a: [number, number], b: [number, number]) =>
      Math.hypot(a[0] - b[0], a[1] - b[1]) <= 1e-3;
    let ordered = true;

    for (const line of lines) {
      if (!line.p2 || !closeEnough(line.p1, previous)) { ordered = false; break; }
      points.push(new THREE.Vector2(...line.p2));
      previous = line.p2;
    }

    if (ordered && closeEnough(previous, first.p1)) {
      shape.moveTo(first.p1[0], first.p1[1]);
      for (const line of lines) shape.lineTo(line.p2![0], line.p2![1]);
      shape.closePath();
      areaMm2 = Math.abs(THREE.ShapeUtils.area(points));
      if (areaMm2 <= 0) return null;
    } else {
      // Order-agnostic path: snap endpoints and assemble closed loops
      // (handles DXF imports and multi-stroke sketches).
      const assembly = assembleLoops(
        lines.map(l => ({ p1: l.p1, p2: l.p2! })),
        SKETCH_SNAP_TOLERANCE_MM,
      );
      const loop = assembly.loops[0];
      if (!loop) return null;
      shape.moveTo(loop[0][0], loop[0][1]);
      for (const pt of loop.slice(1)) shape.lineTo(pt[0], pt[1]);
      shape.closePath();
      areaMm2 = Math.abs(THREE.ShapeUtils.area(loop.map(p => new THREE.Vector2(p[0], p[1]))));
      if (areaMm2 <= 0) return null;
    }
  } else {
    return null;
  }

  const geometry = new THREE.ExtrudeGeometry(shape, { depth: distanceMm, bevelEnabled: false });
  return { geometry, volumeM3: areaMm2 * distanceMm / 1e9 };
}

export interface SketchRevolution {
  geometry: THREE.LatheGeometry;
  /** Approximate spun volume (m³) via Pappus on the largest profile loop. */
  volumeM3: number;
  warnings: string[];
}

/**
 * Spin the sketch's largest closed loop around the sketch's vertical axis
 * (v-axis through the origin). Points must sit at u >= 0; negatives are clamped
 * to the axis with an explicit warning (never silently mirrored).
 */
export function createSketchRevolution(
  sketch: { entities: SketchEntity[] },
  segments = 64,
): SketchRevolution | null {
  const warnings: string[] = [];
  const lines = sketch.entities.filter(e => e.type === 'line' && e.p2);
  if (lines.length < 3 || lines.length !== sketch.entities.length) return null;

  const assembly = assembleLoops(
    lines.map(l => ({ p1: l.p1, p2: l.p2! })),
    SKETCH_SNAP_TOLERANCE_MM,
  );
  const loop = assembly.loops[0];
  if (!loop || loop.length < 3) return null;
  if (assembly.loops.length > 1) {
    warnings.push(`${assembly.loops.length} loops found — revolving only the largest`);
  }
  if (assembly.openChains > 0) {
    warnings.push(`${assembly.openChains} open chain(s) ignored`);
  }

  // Pappus centroidal theorem: V = 2π · Rc · A (valid when the whole loop is
  // on one side of the axis).
  const area = Math.abs(THREE.ShapeUtils.area(loop.map(p => new THREE.Vector2(p[0], p[1]))));
  const centroidU = loop.reduce((acc, p) => acc + p[0], 0) / loop.length;
  const minU = loop.reduce((acc, p) => Math.min(acc, p[0]), Infinity);

  let uShift = 0;
  if (minU < -SKETCH_SNAP_TOLERANCE_MM) {
    uShift = -minU;
    warnings.push('Profile crossed the axis — shifted so it starts at the axis');
  } else if (minU < 0) {
    uShift = -minU;
  }

  const pts = loop.map(p => new THREE.Vector2(Math.max(0, p[0] + uShift), p[1]));
  pts.push(pts[0].clone()); // Lathe wants the profile outline closed for caps
  const geometry = new THREE.LatheGeometry(pts, segments);
  const volumeM3 = (2 * Math.PI * Math.max(0, centroidU + uShift) * area) / 1e9;
  return { geometry, volumeM3, warnings };
}

export function solveSketchOffline(sketch: {
  entities: SketchEntity[];
}): { solved: boolean; dof: number; entities: SketchEntity[] } {
  return { solved: true, dof: Math.max(0, sketch.entities.length * 2 - 4), entities: sketch.entities };
}

export function revolveSketchOffline(
  sketch: { entities: SketchEntity[] },
  segments = 64,
): { ok: boolean; volume_m3: number; vertex_count: number; face_count: number; warnings: string[]; error?: string } {
  const rev = createSketchRevolution(sketch, segments);
  if (!rev) {
    return {
      ok: false,
      volume_m3: 0,
      vertex_count: 0,
      face_count: 0,
      warnings: [],
      error: 'Revolve needs a closed line profile (draw right of the vertical axis through origin).',
    };
  }
  const vertexCount = rev.geometry.attributes.position.count;
  const faceCount = rev.geometry.index ? rev.geometry.index.count / 3 : vertexCount / 3;
  rev.geometry.dispose();
  return {
    ok: true,
    volume_m3: rev.volumeM3,
    vertex_count: vertexCount,
    face_count: faceCount,
    warnings: rev.warnings,
  };
}

export function extrudeSketchOffline(
  sketch: { entities: SketchEntity[] },
  distanceMm: number,
): { ok: boolean; volume_m3: number; vertex_count: number; face_count: number; error?: string } {
  const extrusion = createSketchExtrusion(sketch, distanceMm);
  if (!extrusion) {
    return {
      ok: false,
      volume_m3: 0,
      vertex_count: 0,
      face_count: 0,
      error: 'Draw a closed line profile or a circle before extruding.',
    };
  }
  const vertexCount = extrusion.geometry.attributes.position.count;
  const faceCount = extrusion.geometry.index
    ? extrusion.geometry.index.count / 3
    : vertexCount / 3;
  extrusion.geometry.dispose();
  return {
    ok: true,
    volume_m3: extrusion.volumeM3,
    vertex_count: vertexCount,
    face_count: faceCount,
  };
}
