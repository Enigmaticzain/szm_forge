import { EventBus, HistoryEvent } from "../types";

export class EventBusImpl implements EventBus {
private handlers: Array<(event: HistoryEvent) => void> = [];

public emit(event: HistoryEvent): void {
const currentHandlers = [...this.handlers];
for (const handler of currentHandlers) {
try {
handler(event);
} catch (e) {
console.error("Event handler failed", e);
}
}
}

public subscribe(handler: (event: HistoryEvent) => void): void {
this.handlers.push(handler);
}
}

