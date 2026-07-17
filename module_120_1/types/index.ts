export type Vector3 = { readonly x: number; readonly y: number; readonly z: number };

export type ExtrusionCreateConfig = {
  readonly id?: string;
  readonly profileFaceId: string;
  readonly direction: Vector3;
  readonly distance: number;
};

export type ExtrusionOperation = {
  readonly id: string;
  readonly profileFaceId: string;
  readonly direction: Vector3;
  readonly distance: number;
  readonly status: "active" | "error";
};

export type ExtrusionConfig = {
  readonly maxOperations: number;
  readonly minDistance: number;
};

export type ExtrusionEvent =
  | { readonly type: "EXTRUSION_CREATED"; readonly payload: ExtrusionOperation }
  | { readonly type: "EXTRUSION_UPDATED"; readonly payload: { readonly id: string; readonly distance: number } }
  | { readonly type: "EXTRUSION_REMOVED"; readonly payload: { readonly id: string } };

export interface EventBus {
  emit(event: ExtrusionEvent): void;
  subscribe(handler: (event: ExtrusionEvent) => void): void;
}
