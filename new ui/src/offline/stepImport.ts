/**
 * STEP (.step/.stp) import via occt-import-js — the OpenCascade geometry kernel
 * compiled to WebAssembly. This replaces the old placeholder-box import with
 * real B-Rep → triangulation (mirror of how desktop OCCT apps do it).
 *
 * Units: STEP output is requested in millimeters and scaled to the app's
 * scene units (meters), matching how furniture/extrusion parts are modeled.
 *
 * The WASM↔three.js conversion is a pure function (meshJsonToGeometry) so it is
 * unit-testable without loading the WASM module.
 */

import * as THREE from 'three';

/** Scene units = meters; OCCT outputs millimeters → factor 1/1000. */
export const MM_TO_SCENE = 0.001;

// ── Minimal structural types for occt-import-js results (package ships no .d.ts)
export interface OcctMeshJson {
  name?: string;
  color?: [number, number, number];
  attributes: {
    position: { array: ArrayLike<number> };
    normal?: { array: ArrayLike<number> };
  };
  index: { array: ArrayLike<number> };
}

export interface OcctNodeJson {
  name: string;
  meshes: number[];
  children: OcctNodeJson[];
}

export interface OcctResultJson {
  success: boolean;
  meshes: OcctMeshJson[];
  root: OcctNodeJson;
}

export interface ImportedMeshPart {
  name: string;
  geometry: THREE.BufferGeometry;
  color?: [number, number, number];
}

export interface StepImportSummary {
  parts: ImportedMeshPart[];
  totalTriangles: number;
  warnings: string[];
}

/** Resolve human-readable part names from the product tree onto mesh indices. */
export function collectMeshNames(root: OcctNodeJson): Map<number, string> {
  const names = new Map<number, string>();
  const walk = (node: OcctNodeJson) => {
    for (const idx of node.meshes ?? []) {
      if (node.name && !names.has(idx)) names.set(idx, node.name);
    }
    for (const child of node.children ?? []) walk(child);
  };
  walk(root);
  return names;
}

/** Convert one occt-import-js mesh into a THREE.BufferGeometry (pure). */
export function meshJsonToGeometry(
  mesh: OcctMeshJson,
  scale = MM_TO_SCENE,
): THREE.BufferGeometry {
  const geo = new THREE.BufferGeometry();
  const positions = Float32Array.from(mesh.attributes.position.array as ArrayLike<number>);
  if (positions.length === 0 || positions.length % 3 !== 0) {
    throw new Error('Mesh has invalid position data');
  }
  if (scale !== 1) {
    for (let i = 0; i < positions.length; i++) positions[i] *= scale;
  }
  geo.setAttribute('position', new THREE.BufferAttribute(positions, 3));

  const indices = Uint32Array.from(mesh.index.array as ArrayLike<number>);
  if (indices.length > 0) {
    geo.setIndex(new THREE.BufferAttribute(indices, 1));
  }

  if (mesh.attributes.normal?.array && mesh.attributes.normal.array.length === positions.length) {
    geo.setAttribute(
      'normal',
      new THREE.BufferAttribute(Float32Array.from(mesh.attributes.normal.array), 3),
    );
  } else {
    geo.computeVertexNormals();
  }
  geo.computeBoundingBox();
  geo.computeBoundingSphere();
  return geo;
}

/** Turn a full OCCT read result into importable parts, with honest accounting. */
export function occtResultToParts(result: OcctResultJson, baseName: string): StepImportSummary {
  const warnings: string[] = [];
  if (!result.success || !Array.isArray(result.meshes) || result.meshes.length === 0) {
    return { parts: [], totalTriangles: 0, warnings: ['The STEP file contained no readable solids.'] };
  }
  const names = result.root ? collectMeshNames(result.root) : new Map<number, string>();
  const parts: ImportedMeshPart[] = [];
  let totalTriangles = 0;

  result.meshes.forEach((mesh, i) => {
    try {
      const geometry = meshJsonToGeometry(mesh);
      totalTriangles += geometry.getIndex() ? geometry.getIndex()!.count / 3 : geometry.getAttribute('position').count / 3;
      parts.push({
        name: names.get(i) || mesh.name || `${baseName} ${result.meshes.length > 1 ? `part ${i + 1}` : ''}`.trim(),
        geometry,
        color: mesh.color,
      });
    } catch (e) {
      warnings.push(`Mesh ${i + 1} skipped: ${e instanceof Error ? e.message : String(e)}`);
    }
  });

  if (parts.length === 0) warnings.push('All meshes failed conversion.');
  return { parts, totalTriangles, warnings };
}

// ── WASM loader (lazy singleton) ────────────────────────────────────────────

type OcctModule = { ReadStepFile: (content: Uint8Array, params: unknown) => OcctResultJson };

let occtPromise: Promise<OcctModule> | null = null;

async function loadOcct(): Promise<OcctModule> {
  if (!occtPromise) {
    occtPromise = (async () => {
      const init = (await import('occt-import-js')).default;
      // Node (tests/SSR): emscripten finds the wasm next to the wrapper via fs.
      if (typeof process !== 'undefined' && process.versions?.node) {
        return init() as Promise<OcctModule>;
      }
      // Browser: route the wasm through the bundler so it works in dev and in
      // the single-file production build (asset is inlined/emitted by Vite).
      const { default: wasmUrl } = await import('occt-import-js/dist/occt-import-js.wasm?url');
      return init({ locateFile: () => wasmUrl }) as Promise<OcctModule>;
    })();
    occtPromise.catch(() => { occtPromise = null; }); // allow retry after a failure
  }
  return occtPromise;
}

/**
 * Parse a STEP file's bytes into scene-ready mesh parts.
 * Throws with a human-readable reason; never returns fake geometry.
 */
export async function parseStepBytes(content: Uint8Array, baseName: string): Promise<StepImportSummary> {
  if (content.byteLength === 0) throw new Error('The file is empty.');
  const occt = await loadOcct();
  const result = occt.ReadStepFile(content, { linearUnit: 'millimeter' });
  return occtResultToParts(result, baseName);
}

/** Determine whether the reader succeeded in producing parseable content at all. */
export function looksLikeStep(content: Uint8Array): boolean {
  const head = new TextDecoder().decode(content.subarray(0, Math.min(content.byteLength, 256)));
  return head.includes('ISO-10303-21');
}
