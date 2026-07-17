import { EventBus, WindowEvent } from "../types";

export class EventDispatcher {
  private bus: EventBus | null = null;
  private debug: boolean = false;

  public connect(bus: EventBus): void {
    this.bus = bus;
  }

  public setDebug(enabled: boolean): void {
    this.debug = enabled;
  }

  public dispatch(event: WindowEvent): void {
    if (!this.bus) return;
    if (this.debug) {
      console.log(`[EventDispatcher] Emitting: ${event.type}`, event.payload);
    }
    this.bus.emit(event);
  }
}
