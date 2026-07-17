import { EventBus, GraphicsEvent } from "../types";

export class GraphicsEventDispatcher {
private bus: EventBus | null = null;
private debug: boolean = false;

public connect(bus: EventBus): void {
this.bus = bus;
}

public setDebug(enabled: boolean): void {
this.debug = enabled;
}

public dispatch(event: GraphicsEvent): void {
if (!this.bus) return;
if (this.debug) {
console.log([GraphicsEventDispatcher] Emitting: ${event.type}, event.payload);
}
this.bus.emit(event);
}
}

