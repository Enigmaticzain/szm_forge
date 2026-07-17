export type Vector3 = {
  readonly x: number;
  readonly y: number;
  readonly z: number;
};

export type Transform = {
  readonly position: Vector3;
  readonly rotation: Vector3;
  readonly scale: Vector3;
};

export type NodeConfig = {
  readonly id?: string;
  readonly parentId?: string | null;
  readonly transform?: Partial<Transform>;
  readonly nodeType?: string;
};

export type SceneNode = {
  readonly id: string;
  readonly parentId: string | null;
  readonly children: readonly string[];
  readonly transform: Transform;
  readonly nodeType: string;
};

export type SceneConfig = {
  readonly maxNodes: number;
  readonly maxHierarchyDepth: number;
};

export type SceneEvent =
  | { readonly type: "NODE_CREATED"; readonly payload: SceneNode }
  | { readonly type: "NODE_REMOVED"; readonly payload: { readonly id: string } }
  | { readonly type: "NODE_REPARENTED"; readonly payload: { readonly id: string; readonly oldParentId: string | null; readonly newParentId: string | null } }
  | { readonly type: "TRANSFORM_UPDATED"; readonly payload: { readonly id: string; readonly transform: Transform } }
  | { readonly type: "SCENE_DESERIALIZED"; readonly payload: { readonly count: number } };

export interface EventBus {
  emit(event: SceneEvent): void;
  subscribe(handler: (event: SceneEvent) => void): void;
}
