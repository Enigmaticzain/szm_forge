/**
 * occt-import-js ships no TypeScript declarations. This mirrors the documented
 * result shape (README "Processing the result") with full typing for the parts
 * we consume.
 */
declare module 'occt-import-js' {
  export interface OcctMeshAttributes {
    position: { array: ArrayLike<number> };
    normal?: { array: ArrayLike<number> };
  }
  export interface OcctMesh {
    name?: string;
    color?: [number, number, number];
    brep_faces?: Array<{ first: number; last: number; color: [number, number, number] | null }>;
    attributes: OcctMeshAttributes;
    index: { array: ArrayLike<number> };
  }
  export interface OcctNode {
    name: string;
    meshes: number[];
    children: OcctNode[];
  }
  export interface OcctResult {
    success: boolean;
    root: OcctNode;
    meshes: OcctMesh[];
  }
  export interface TriangulationParams {
    linearUnit?: 'millimeter' | 'centimeter' | 'meter' | 'inch' | 'foot';
    linearDeflectionType?: 'bounding_box_ratio' | 'absolute_value';
    linearDeflection?: number;
    angularDeflection?: number;
  }
  export interface OcctInstance {
    ReadStepFile(content: Uint8Array, params: TriangulationParams | null): OcctResult;
    ReadIgesFile(content: Uint8Array, params: TriangulationParams | null): OcctResult;
    ReadBrepFile(content: Uint8Array, params: TriangulationParams | null): OcctResult;
    ReadFile(format: string, content: Uint8Array, params: TriangulationParams | null): OcctResult;
  }
  export interface OcctInitOptions {
    locateFile?: (file: string) => string;
  }
  export default function initOcct(options?: OcctInitOptions): Promise<OcctInstance>;
}

declare module 'occt-import-js/dist/occt-import-js.wasm?url' {
  const wasmUrl: string;
  export default wasmUrl;
}
