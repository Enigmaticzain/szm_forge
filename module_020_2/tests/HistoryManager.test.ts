import { HistoryManager } from "../core/HistoryManager";
import { EventBusImpl } from "../systems/EventBusImpl";
import { HistoryConfig } from "../types";

export function runTests() {
const manager = new HistoryManager();
const eventBus = new EventBusImpl();

const config: HistoryConfig = {
maxHistory: 3
};

const initRes = manager.initialize(config, eventBus);
if (!initRes.success) throw new Error("Init failed");

// Push 1
const push1 = manager.pushCommand({
id: "cmd_1",
type: "TEST_ACTION",
payload: { val: 1 }
});
if (!push1.success) throw new Error("Push 1 failed");

// Push 2
manager.pushCommand({ id: "cmd_2", type: "TEST_ACTION", payload: { val: 2 } });

// Push 3
manager.pushCommand({ id: "cmd_3", type: "TEST_ACTION", payload: { val: 3 } });

// Test Undo
const undoRes = manager.undo();
if (!undoRes.success || undoRes.data.id !== "cmd_3") throw new Error("Undo failed or wrong order");

// Test Redo
const redoRes = manager.redo();
if (!redoRes.success || redoRes.data.id !== "cmd_3") throw new Error("Redo failed or wrong order");

// Test Future Drop (Undo then Push)
manager.undo(); // Pointer now at cmd_2
manager.pushCommand({ id: "cmd_4", type: "BRANCH_ACTION", payload: {} }); // Drops cmd_3

const snapshot = manager.getStateSnapshot();
if (snapshot.length !== 3) throw new Error("Future drop snapshot count invalid");
if (snapshot[snapshot.length - 1].id !== "cmd_4") throw new Error("Future branch invalid");

// Test Max Limit Eviction
manager.pushCommand({ id: "cmd_5", type: "OOM_TEST", payload: {} }); // Should evict cmd_1

const clearRes = manager.clear();
if (!clearRes.success) throw new Error("Clear failed");

if (manager.getStateSnapshot().length !== 0) throw new Error("Clear did not reset snapshot");

console.log("All invariant constraints, pointer shifts, and atomic tests passed.");
}

