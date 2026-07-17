export type ModuleMetadata = {
  module: "150.2";
  version: "1.0.0";
};

export const METADATA: ModuleMetadata = {
  module: "150.2",
  version: "1.0.0",
};

export type Vector3 = {
  readonly x: number;
  readonly y: number;
  readonly z: number;
};

export type PBRMaterial = {
  readonly albedo: number;
  readonly metallic: number;
  readonly roughness: number;
};

export type MeshNode = {
  readonly id: string;
  readonly sourceBodyId: string;
  readonly vertexCount: number;
  readonly indexCount: number;
  readonly material: PBRMaterial;
  readonly state: "active" | "hidden";
};

export type TessellationConfig = {
  readonly id?: string;
  readonly sourceBodyId: string;
  readonly vertexCount: number;
  readonly indexCount: number;
  readonly material: PBRMaterial;
};

export type RenderConfig = {
  readonly maxMeshes: number;
  readonly maxVerticesPerMesh: number;
};

export type MeshEvent =
  | { type: "MESH_GENERATED"; payload: MeshNode }
  | { type: "MATERIAL_UPDATED"; payload: { id: string; material: PBRMaterial } }
  | { type: "MESH_DESTROYED"; payload: { id: string } };

export interface EventBus {
  emit(event: MeshEvent): void;
  subscribe(handler: (event: MeshEvent) => void): void;
}


