import { EventBus, InputEvent } from "../types";

export class EventBusImpl implements EventBus {
private handlers: Array<(event: InputEvent) => void> = [];

public emit(event: InputEvent): void {
const currentHandlers = [...this.handlers];
for (const handler of currentHandlers) {
try {
handler(event);
} catch (e) {
console.error("Event handler failed", e);
}
}
}

public subscribe(handler: (event: InputEvent) => void): void {
this.handlers.push(handler);
}
}

