import { CameraController } from "../core/CameraController";
import { EventBusImpl } from "../systems/EventBusImpl";
import { ViewportConfig } from "../types";

export function runTests() {
const controller = new CameraController();
const eventBus = new EventBusImpl();

const config: ViewportConfig = {
maxCameras: 2,
fovLimits: { min: 10, max: 120 }
};

const initRes = controller.initialize(config, eventBus);
if (!initRes.success) throw new Error("Init failed");

const createRes = controller.createCamera({
id: "main_cam",
position: { x: 0, y: 0, z: 10 },
target: { x: 0, y: 0, z: 0 },
fov: 60
});
if (!createRes.success) throw new Error("Create failed");

const posRes = controller.updatePosition("main_cam", { x: 5, y: 5, z: 5 });
if (!posRes.success) throw new Error("Position update failed");

const targetFailRes = controller.updateTarget("main_cam", { x: 5, y: 5, z: 5 });
if (targetFailRes.success) throw new Error("Distance invariant failure missed");

const fovFailRes = controller.updateFov("main_cam", 150);
if (fovFailRes.success) throw new Error("FOV limit invariant failure missed");

const destroyRes = controller.destroyCamera("main_cam");
if (!destroyRes.success) throw new Error("Destroy failed");

const snapshot = controller.getStateSnapshot();
if (snapshot.length !== 0) throw new Error("Snapshot invalid state");

console.log("All invariant constraints & atomic tests passed.");
}

