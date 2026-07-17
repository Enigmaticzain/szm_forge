export type Vector3 = { readonly x: number; readonly y: number; readonly z: number };

export type TopoVertex = { readonly id: string; readonly type: "vertex"; readonly position: Vector3 };
export type TopoEdge = { readonly id: string; readonly type: "edge"; readonly startVertexId: string; readonly endVertexId: string };
export type TopoFace = { readonly id: string; readonly type: "face"; readonly edgeIds: readonly string[] };
export type TopoSolid = { readonly id: string; readonly type: "solid"; readonly faceIds: readonly string[] };

export type TopoEntity = TopoVertex | TopoEdge | TopoFace | TopoSolid;

export type VertexConfig = { readonly id?: string; readonly position: Vector3 };
export type EdgeConfig = { readonly id?: string; readonly startVertexId: string; readonly endVertexId: string };
export type FaceConfig = { readonly id?: string; readonly edgeIds: string[] };
export type SolidConfig = { readonly id?: string; readonly faceIds: string[] };

export type TopologyConfig = {
  readonly maxEntities: number;
  readonly tolerance: number;
};

export type TopologyEvent =
  | { readonly type: "ENTITY_CREATED"; readonly payload: TopoEntity }
  | { readonly type: "ENTITY_REMOVED"; readonly payload: { readonly id: string; readonly entityType: string } }
  | { readonly type: "TOPOLOGY_UPDATED"; readonly payload: { readonly timestamp: number } };

export interface EventBus {
  emit(event: TopologyEvent): void;
  subscribe(handler: (event: TopologyEvent) => void): void;
}
