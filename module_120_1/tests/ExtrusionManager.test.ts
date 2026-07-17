import { ExtrusionManager } from "../core/ExtrusionManager";
import { ExtrusionConfig, EventBus, ExtrusionEvent } from "../types";

export function runTests() {
  const manager = new ExtrusionManager();
  const mockBus: EventBus = {
    emit: (e: ExtrusionEvent) => {},
    subscribe: () => {}
  };

  const config: ExtrusionConfig = {
    maxOperations: 100,
    minDistance: 0.001
  };

  manager.initialize(config, mockBus);
  // Extrusion testing logic goes here
}

