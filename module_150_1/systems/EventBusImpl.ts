import { EventBus, CameraEvent } from "../types";

export class EventBusImpl implements EventBus {
private handlers: Array<(event: CameraEvent) => void> = [];

public emit(event: CameraEvent): void {
const currentHandlers = [...this.handlers];
for (const handler of currentHandlers) {
try {
handler(event);
} catch (e) {
console.error("Event handler failed", e);
}
}
}

public subscribe(handler: (event: CameraEvent) => void): void {
this.handlers.push(handler);
}
}

