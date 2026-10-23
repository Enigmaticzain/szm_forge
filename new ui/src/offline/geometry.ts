import * as THREE from 'three';
import { STLExporter } from 'three/examples/jsm/exporters/STLExporter.js';

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
  if (lower.includes('cylinder') || lower.includes('shaft') || lower.includes('pipe')) shape = 'cylinder';
  else if (lower.includes('sphere') || lower.includes('ball')) shape = 'sphere';
  else if (lower.includes('cone')) shape = 'cone';
  else if (lower.includes('torus') || lower.includes('ring')) shape = 'torus';
  else if (lower.includes('bracket')) shape = 'box';

  const length = numFrom(lower, [/(\d+(?:\.\d+)?)\s*mm\s*(?:long|length)/, /length\s*(\d+)/]) ?? 100;
  const width = numFrom(lower, [/(\d+(?:\.\d+)?)\s*mm\s*(?:wide|width)/, /width\s*(\d+)/, /×\s*(\d+)/]) ?? 60;
  const height = numFrom(lower, [/(\d+(?:\.\d+)?)\s*mm\s*(?:tall|high|height|thick)/, /height\s*(\d+)/]) ?? 15;
  const radius = numFrom(lower, [/radius\s*(\d+(?:\.\d+)?)/, /(\d+(?:\.\d+)?)\s*mm\s*radius/]) ?? 25;

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
  const blob = new Blob([stl as ArrayBuffer], { type: 'application/octet-stream' });
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
}

export function solveSketchOffline(sketch: {
  entities: SketchEntity[];
}): { solved: boolean; dof: number; entities: SketchEntity[] } {
  return { solved: true, dof: Math.max(0, sketch.entities.length * 2 - 4), entities: sketch.entities };
}

export function extrudeSketchOffline(
  sketch: { entities: SketchEntity[] },
  distanceMm: number,
): { ok: boolean; volume_m3: number; vertex_count: number; face_count: number } {
  const shape = new THREE.Shape();
  const lines = sketch.entities.filter(e => e.type === 'line' && e.p2);
  if (lines.length >= 3) {
    const [first] = lines;
    shape.moveTo(first.p1[0], first.p1[1]);
    for (const line of lines) {
      if (line.p2) shape.lineTo(line.p2[0], line.p2[1]);
    }
    shape.closePath();
  } else {
    shape.moveTo(-20, -20);
    shape.lineTo(20, -20);
    shape.lineTo(20, 20);
    shape.lineTo(-20, 20);
    shape.closePath();
  }

  const geo = new THREE.ExtrudeGeometry(shape, { depth: distanceMm, bevelEnabled: false });
  geo.computeBoundingBox();
  const box = geo.boundingBox!;
  const vol = (box.max.x - box.min.x) * (box.max.y - box.min.y) * (box.max.z - box.min.z) / 1e9;

  return {
    ok: true,
    volume_m3: vol,
    vertex_count: geo.attributes.position.count,
    face_count: geo.index ? geo.index.count / 3 : 0,
  };
}
