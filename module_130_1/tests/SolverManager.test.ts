import { SolverManager } from "../core/SolverManager";
import { EventBusImpl } from "../systems/EventBusImpl";
import { SolverConfig } from "../types";

export function runTests() {
const manager = new SolverManager();
const eventBus = new EventBusImpl();

const config: SolverConfig = {
maxConstraints: 10,
tolerance: 0.001
};

const initRes = manager.initialize(config, eventBus);
if (!initRes.success) throw new Error("Init failed");

const createRes = manager.createConstraint({
id: "test_dist_1",
type: "distance",
targetA: "node_1",
targetB: "node_2",
value: 10.5
});
if (!createRes.success) throw new Error("Create failed");

const duplicateNodeRes = manager.createConstraint({
id: "test_fail_1",
type: "distance",
targetA: "node_1",
targetB: "node_1",
value: 5.0
});
if (duplicateNodeRes.success) throw new Error("Self-reference invariant failure missed");

const updateRes = manager.updateConstraintValue("test_dist_1", 15.0);
if (!updateRes.success) throw new Error("Update failed");

const negativeDistRes = manager.updateConstraintValue("test_dist_1", -5.0);
if (negativeDistRes.success) throw new Error("Negative distance invariant failure missed");

const destroyRes = manager.destroyConstraint("test_dist_1");
if (!destroyRes.success) throw new Error("Destroy failed");

const snapshot = manager.getStateSnapshot();
if (snapshot.length !== 0) throw new Error("Snapshot invalid state");

console.log("All invariant constraints & atomic tests passed.");
}

