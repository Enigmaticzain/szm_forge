import { EventBus, SceneEvent } from "../types";

export class SceneEventDispatcher {
  private bus: EventBus | null = null;
  private debug: boolean = false;

  public connect(bus: EventBus): void {
    this.bus = bus;
  }

  public setDebug(enabled: boolean): void {
    this.debug = enabled;
  }

  public dispatch(event: SceneEvent): void {
    if (!this.bus) return;
    if (this.debug) {
      console.log(`[SceneEventDispatcher] ${event.type}`, event.payload);
    }
    this.bus.emit(event);
  }
}
