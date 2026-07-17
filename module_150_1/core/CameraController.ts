import { Result } from "../utils/Result";
import {
Camera,
CameraConfig,
ViewportConfig,
EventBus,
CameraEvent,
Vector3
} from "../types";

export class CameraController {
private isInitialized: boolean = false;
private isDebug: boolean = false;
private config: ViewportConfig | null = null;
private eventBus: EventBus | null = null;

private cameras: Map<string, Camera> = new Map();
private activeCameraIds: string[] = [];

public initialize(config: ViewportConfig, eventBus: EventBus): Result<void> {
if (this.isInitialized) {
return { success: false, error: "System already initialized" };
}
this.config = Object.freeze({ ...config });
this.eventBus = eventBus;
this.isInitialized = true;
return { success: true, data: undefined };
}

public enableDebug(enabled: boolean): void {
this.isDebug = enabled;
}

public createCamera(config: CameraConfig): Result<string> {
const check = this.validateState();
if (!check.success) return check;

const id = config.id || this.generateId();

if (this.cameras.has(id)) {
  return { success: false, error: "Duplicate camera ID" };
}

if (this.cameras.size >= this.config!.maxCameras) {
  return { success: false, error: "Max cameras exceeded" };
}

const distCheck = this.validateDistance(config.position, config.target);
if (!distCheck.success) return distCheck;

const fovCheck = this.validateFov(config.fov);
if (!fovCheck.success) return fovCheck;

const newCamera: Camera = Object.freeze({
  id,
  position: Object.freeze({ ...config.position }),
  target: Object.freeze({ ...config.target }),
  fov: config.fov,
  up: Object.freeze({ x: 0, y: 1, z: 0 }),
  state: "active"
});

const nextCameras = new Map(this.cameras);
nextCameras.set(id, newCamera);

this.cameras = nextCameras;
this.activeCameraIds = [...this.activeCameraIds, id];

this.dispatch({ type: "CAMERA_CREATED", payload: newCamera });
return { success: true, data: id };
}

public destroyCamera(id: string): Result<void> {
const check = this.validateState();
if (!check.success) return check;

if (!this.cameras.has(id)) {
  return { success: false, error: "Invalid camera id" };
}

const nextCameras = new Map(this.cameras);
nextCameras.delete(id);
this.cameras = nextCameras;
this.activeCameraIds = this.activeCameraIds.filter(camId => camId !== id);

this.dispatch({ type: "CAMERA_DESTROYED", payload: { id } });
return { success: true, data: undefined };
}

public updatePosition(id: string, position: Vector3): Result<void> {
const check = this.validateState();
if (!check.success) return check;

const currentCamera = this.cameras.get(id);
if (!currentCamera) return { success: false, error: "Invalid camera id" };

const distCheck = this.validateDistance(position, currentCamera.target);
if (!distCheck.success) return distCheck;

const updatedCamera: Camera = Object.freeze({
  ...currentCamera,
  position: Object.freeze({ ...position })
});

const nextCameras = new Map(this.cameras);
nextCameras.set(id, updatedCamera);
this.cameras = nextCameras;

this.dispatch({ type: "CAMERA_MOVED", payload: { id, position: updatedCamera.position } });
return { success: true, data: undefined };
}

public updateTarget(id: string, target: Vector3): Result<void> {
const check = this.validateState();
if (!check.success) return check;

const currentCamera = this.cameras.get(id);
if (!currentCamera) return { success: false, error: "Invalid camera id" };

const distCheck = this.validateDistance(currentCamera.position, target);
if (!distCheck.success) return distCheck;

const updatedCamera: Camera = Object.freeze({
  ...currentCamera,
  target: Object.freeze({ ...target })
});

const nextCameras = new Map(this.cameras);
nextCameras.set(id, updatedCamera);
this.cameras = nextCameras;

this.dispatch({ type: "CAMERA_TARGET_UPDATED", payload: { id, target: updatedCamera.target } });
return { success: true, data: undefined };
}

public updateFov(id: string, fov: number): Result<void> {
const check = this.validateState();
if (!check.success) return check;

const currentCamera = this.cameras.get(id);
if (!currentCamera) return { success: false, error: "Invalid camera id" };

const fovCheck = this.validateFov(fov);
if (!fovCheck.success) return fovCheck;

const updatedCamera: Camera = Object.freeze({
  ...currentCamera,
  fov
});

const nextCameras = new Map(this.cameras);
nextCameras.set(id, updatedCamera);
this.cameras = nextCameras;

this.dispatch({ type: "CAMERA_FOV_CHANGED", payload: { id, fov: updatedCamera.fov } });
return { success: true, data: undefined };
}

public getStateSnapshot(): Camera[] {
if (!this.isInitialized) return [];

return Object.freeze(
  this.activeCameraIds.map(id => this.cameras.get(id) as Camera)
) as unknown as Camera[];
}

private validateState(): Result<void> {
if (!this.isInitialized || !this.config || !this.eventBus) {
return { success: false, error: "Controller not initialized" };
}
return { success: true, data: undefined };
}

private validateDistance(p1: Vector3, p2: Vector3): Result<void> {
const dx = p1.x - p2.x;
const dy = p1.y - p2.y;
const dz = p1.z - p2.z;
const distSq = dx * dx + dy * dy + dz * dz;

if (distSq < 0.0001) {
  return { success: false, error: "Position and target overlap invariant violation" };
}
return { success: true, data: undefined };
}

private validateFov(fov: number): Result<void> {
const cfg = this.config!;
if (fov < cfg.fovLimits.min || fov > cfg.fovLimits.max) {
return { success: false, error: "FOV out of bounds invariant violation" };
}
return { success: true, data: undefined };
}

private generateId(): string {
return cam_${Date.now()}_${Math.random().toString(36).substring(2, 9)};
}

private dispatch(event: CameraEvent): void {
if (this.isDebug) {
console.log([CameraController] ${event.type}, event.payload);
}
this.eventBus!.emit(event);
}
}

