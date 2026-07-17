export type ModuleMetadata = {
module: "150.1";
version: "1.0.0";
};

export const METADATA: ModuleMetadata = {
module: "150.1",
version: "1.0.0",
};

export type Vector3 = {
readonly x: number;
readonly y: number;
readonly z: number;
};

export type Camera = {
readonly id: string;
readonly position: Vector3;
readonly target: Vector3;
readonly fov: number;
readonly up: Vector3;
readonly state: "active" | "inactive";
};

export type CameraConfig = {
readonly id?: string;
readonly position: Vector3;
readonly target: Vector3;
readonly fov: number;
};

export type ViewportConfig = {
readonly maxCameras: number;
readonly fovLimits: { readonly min: number; readonly max: number };
};

export type CameraEvent =
| { type: "CAMERA_CREATED"; payload: Camera }
| { type: "CAMERA_MOVED"; payload: { id: string; position: Vector3 } }
| { type: "CAMERA_TARGET_UPDATED"; payload: { id: string; target: Vector3 } }
| { type: "CAMERA_FOV_CHANGED"; payload: { id: string; fov: number } }
| { type: "CAMERA_DESTROYED"; payload: { id: string } };

export interface EventBus {
emit(event: CameraEvent): void;
subscribe(handler: (event: CameraEvent) => void): void;
}

