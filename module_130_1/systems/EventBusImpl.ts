import { EventBus, SolverEvent } from "../types";

export class EventBusImpl implements EventBus {
private handlers: Array<(event: SolverEvent) => void> = [];

public emit(event: SolverEvent): void {
const currentHandlers = [...this.handlers];
for (const handler of currentHandlers) {
try {
handler(event);
} catch (e) {
console.error("Event handler failed", e);
}
}
}

public subscribe(handler: (event: SolverEvent) => void): void {
this.handlers.push(handler);
}
}

