import { EventBus, TopologyEvent } from "../types";

export class TopologyEventDispatcher {
  private bus: EventBus | null = null;
  private debug: boolean = false;

  public connect(bus: EventBus): void {
    this.bus = bus;
  }

  public setDebug(enabled: boolean): void {
    this.debug = enabled;
  }

  public dispatch(event: TopologyEvent): void {
    if (!this.bus) return;
    if (this.debug) {
      console.log(`[TopologyEventDispatcher] Emitting: ${event.type}`, event.payload);
    }
    this.bus.emit(event);
  }
}
