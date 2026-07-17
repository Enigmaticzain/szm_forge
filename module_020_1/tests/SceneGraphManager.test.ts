import { SceneGraphManager } from "../core/SceneGraphManager";
import { SceneConfig, EventBus, SceneEvent } from "../types";

export function runTests() {
  const manager = new SceneGraphManager();
  const mockBus: EventBus = {
    emit: (e: SceneEvent) => {},
    subscribe: () => {}
  };

  const config: SceneConfig = {
    maxNodes: 1000,
    maxHierarchyDepth: 10
  };

  manager.initialize(config, mockBus);
}\

