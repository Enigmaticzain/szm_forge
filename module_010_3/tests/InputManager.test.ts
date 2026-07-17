import { InputManager } from "../core/InputManager";
import { EventBusImpl } from "../systems/EventBusImpl";
import { OSConfig } from "../types";

export function runTests() {
const manager = new InputManager();
const eventBus = new EventBusImpl();

const config: OSConfig = {
maxRegions: 5,
desktopBounds: { x: 1920, y: 1080 },
minSize: { x: 100, y: 100 }
};

const initRes = manager.initialize(config, eventBus);
if (!initRes.success) throw new Error("Init failed");

const createRes = manager.createRegion({
id: "test_1",
position: { x: 50, y: 50 },
size: { x: 200, y: 200 }
});
if (!createRes.success) throw new Error("Create failed");

const moveRes = manager.moveRegion("test_1", 100, 100);
if (!moveRes.success) throw new Error("Move failed");

const resizeFailRes = manager.resizeRegion("test_1", 50, 50);
if (resizeFailRes.success) throw new Error("Invariant failure missed");

const destroyRes = manager.destroyRegion("test_1");
if (!destroyRes.success) throw new Error("Destroy failed");

const snapshot = manager.getStateSnapshot();
if (snapshot.length !== 0) throw new Error("Snapshot invalid state");

console.log("All invariant constraints & atomic tests passed.");
}

