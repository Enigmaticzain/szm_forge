import { EventBus, MeshEvent } from "../types";

export class EventBusImpl implements EventBus {
  private handlers: Array<(event: MeshEvent) => void> = [];

  public emit(event: MeshEvent): void {
    const currentHandlers = [...this.handlers];
    for (const handler of currentHandlers) {
      try {
        handler(event);
      } catch (e) {
        console.error("Event handler failed", e);
      }
    }
  }

  public subscribe(handler: (event: MeshEvent) => void): void {
    this.handlers.push(handler);
  }
}


