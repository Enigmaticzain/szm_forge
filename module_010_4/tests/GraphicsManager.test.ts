import { GraphicsManager } from "../core/GraphicsManager";
import { GraphicsConfig, EventBus, GraphicsEvent } from "../types";

export function runTests() {
const manager = new GraphicsManager();
const mockBus: EventBus = {
emit: (e: GraphicsEvent) => {},
subscribe: () => {}
};

const config: GraphicsConfig = {
maxSwapchains: 4,
hardwareAcceleration: true
};

manager.initialize(config, mockBus);
}

