import { TopologyManager } from "../core/TopologyManager";
import { TopologyConfig, EventBus, TopologyEvent } from "../types";

export function runTests() {
  const manager = new TopologyManager();
  const mockBus: EventBus = {
    emit: (e: TopologyEvent) => {},
    subscribe: () => {}
  };

  const config: TopologyConfig = {
    maxEntities: 10000,
    tolerance: 0.0001
  };

  manager.initialize(config, mockBus);
}

