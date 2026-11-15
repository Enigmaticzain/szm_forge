/**
 * USD (.usda) export of the ACTUAL current scene — no backend round-trip,
 * no hard-coded sample plate. Writes Pixar's plain-text USDA format with one
 * Xform+Mesh prim per part, so exports open in usdview / Blender / Omniverse.
 *
 * Design rules (same as the rest of the offline engine):
 *  - Export exactly what the viewport shows: custom meshes as authored,
 *    primitives baked to simple closed meshes.
 *  - Bend deformations are a viewport effect; bent parts export UNBENT and
 *    the result reports that in `warnings`.
 *  - Throw on empty scenes — never emit a file for nothing.
 */

import * as THREE from 'three';
import type { SceneObject } from '../components/BodyDesign/InteractivePartEditor';

export interface UsdExportResult {
  usda: string;
  fileName: string;
  partCount: number;
  triangleCount: number;
  warnings: string[];
}

/** USD prim identifiers: [A-Za-z_][A-Za-z0-9_]* — sanitize, never collide. */
export function sanitizePrimName(raw: string, fallback: string, used: Set<string>): string {
  let name = raw.replace(/[^A-Za-z0-9_]/g, '_').replace(/^_+|_+$/g, '');
  if (/^[0-9]/.test(name)) name = `_${name}`;
  if (!name) name = fallback;
  let candidate = name;
  let n = 2;
  while (used.has(candidate)) candidate = `${name}_${n++}`;
  used.add(candidate);
  return candidate;
}

interface MeshArrays {
  positions: THREE.TypedArray;
  indices: ArrayLike<number>;
  normals: THREE.TypedArray | null;
  extentMin: [number, number, number];
  extentMax: [number, number, number];
}

/** Extract triangleSoup arrays from any BufferGeometry (mutates `geometry`'s normals/bounds cache only). */
export function geometryToMeshArrays(geometry: THREE.BufferGeometry): MeshArrays {
  const positions = geometry.getAttribute('position');
  if (!positions || positions.count === 0) {
    throw new Error('Geometry has no vertices');
  }
  // Indexed form is compact exactly as USD wants; expand non-indexed to sequential indices.
  let indices: ArrayLike<number>;
  if (geometry.index) {
    indices = geometry.index.array;
  } else {
    const sequential = new Uint32Array(positions.count);
    for (let i = 0; i < positions.count; i++) sequential[i] = i;
    indices = sequential;
  }
  if (!geometry.getAttribute('normal')) geometry.computeVertexNormals();
  geometry.computeBoundingBox();
  const bb = geometry.boundingBox!;
  return {
    positions: positions.array as THREE.TypedArray,
    indices,
    normals: geometry.getAttribute('normal')?.array as THREE.TypedArray | null,
    extentMin: [bb.min.x, bb.min.y, bb.min.z],
    extentMax: [bb.max.x, bb.max.y, bb.max.z],
  };
}

/** Real geometry for a part: authored mesh or a low-segment primitive at true size. */
function partToGeometry(obj: SceneObject): THREE.BufferGeometry {
  if (obj.customGeometry) return obj.customGeometry;
  switch (obj.type) {
    case 'cylinder': {
      const g = new THREE.CylinderGeometry(0.5, 0.5, 1, 32);
      g.scale(obj.size.x, obj.size.y, obj.size.z);
      return g;
    }
    case 'plate':
    case 'box':
    case 'custom':
    default: {
      const g = new THREE.BoxGeometry(obj.size.x, obj.size.y, obj.size.z);
      return g;
    }
  }
}

function fmt(n: number): string {
  const v = Number(n.toPrecision(6));
  return Number.isFinite(v) ? String(v) : '0';
}

function vec3(attr: THREE.TypedArray, i: number): string {
  return `(${fmt(Number(attr[i * 3]))}, ${fmt(Number(attr[i * 3 + 1]))}, ${fmt(Number(attr[i * 3 + 2]))})`;
}

export function sceneToUsda(
  parts: SceneObject[],
  opts: { projectName?: string; fileName?: string; units?: 'm' | 'mm' } = {},
): UsdExportResult {
  if (!parts.length) {
    throw new Error('Nothing to export — the scene has no parts.');
  }
  const projectName = opts.projectName ?? 'SZM Forge Scene';
  const units = opts.units ?? 'm';
  const metersPerUnit = units === 'mm' ? 0.001 : 1;
  const warnings: string[] = [];
  const usedNames = new Set<string>();
  const chunks: string[] = [];
  let triangleCount = 0;

  chunks.push(
    '#usda 1.0',
    '(',
    '    defaultPrim = "Scene"',
    '    doc = "Exported from SZM Forge — actual scene contents"',
    `    metersPerUnit = ${metersPerUnit}`,
    '    upAxis = "Y"',
    ')',
    '',
    'def Xform "Scene"',
    '{',
  );

  parts.forEach((part, i) => {
    const primName = sanitizePrimName(part.name, `Part_${i + 1}`, usedNames);
    const geometry = partToGeometry(part);
    let arrays: MeshArrays;
    try {
      arrays = geometryToMeshArrays(geometry);
    } catch (e) {
      warnings.push(`"${part.name}" skipped: ${e instanceof Error ? e.message : String(e)}`);
      return;
    }
    if ((part.bendAngle ?? 0) > 0) {
      warnings.push(`"${part.name}" has an active bend — exported unbent (viewport-only deformation).`);
    }

    const triCount = Math.floor(arrays.indices.length / 3);
    triangleCount += triCount;

    const faceCounts = new Array<string>(triCount).fill('3').join(', ');
    const faceIndices = Array.from(arrays.indices).join(', ');
    const points: string[] = [];
    for (let v = 0; v < arrays.positions.length / 3; v++) points.push(vec3(arrays.positions, v));
    const normals: string[] = [];
    if (arrays.normals) {
      for (let v = 0; v < arrays.normals.length / 3; v++) normals.push(vec3(arrays.normals, v));
    }

    chunks.push(
      `    def Xform "${primName}"`,
      '    (',
      `        doc = "type=${part.type}${part.materialId ? ` material=${part.materialId}` : ''}"`,
      '    )',
      '    {',
      `        float3 xformOp:translate = (${fmt(part.position.x)}, ${fmt(part.position.y)}, ${fmt(part.position.z)})`,
      '        uniform token[] xformOpOrder = ["xformOp:translate"]',
      '',
      '        def Mesh "Geom"',
      '        {',
      `            int[] faceVertexCounts = [${faceCounts}]`,
      `            int[] faceVertexIndices = [${faceIndices}]`,
      `            point3f[] points = [${points.join(', ')}]`,
      ...(arrays.normals
        ? [`            normal3f[] normals = [${normals.join(', ')}] (`, '                interpolation = "vertex"', '            )']
        : []),
      `            float3[] extent = [(${arrays.extentMin.map(fmt).join(', ')}), (${arrays.extentMax.map(fmt).join(', ')})]`,
      '        }',
      '    }',
      '',
    );
  });

  chunks.push('}', '');

  const baseFile = sanitizePrimName(projectName, 'scene', new Set()).toLowerCase();
  return {
    usda: chunks.join('\n'),
    fileName: opts.fileName ?? `${baseFile}.usda`,
    partCount: parts.length - warnings.filter(w => w.includes('skipped')).length,
    triangleCount,
    warnings,
  };
}

/** Trigger a browser download for a USDA payload. */
export function downloadUsda(result: UsdExportResult): void {
  const blob = new Blob([result.usda], { type: 'text/plain' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = result.fileName;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}
