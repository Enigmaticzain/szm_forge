import { EventBus, ExtrusionEvent } from "../types";

export class ExtrusionEventDispatcher {
  private bus: EventBus | null = null;
  private debug: boolean = false;

  public connect(bus: EventBus): void {
    this.bus = bus;
  }

  public setDebug(enabled: boolean): void {
    this.debug = enabled;
  }

  public dispatch(event: ExtrusionEvent): void {
    if (!this.bus) return;
    if (this.debug) {
      console.log(`[ExtrusionEventDispatcher] Emitting: ${event.type}`, event.payload);
    }
    this.bus.emit(event);
  }
}
