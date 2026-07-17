import { Result } from "../utils/Result";
import {
Command,
CommandConfig,
HistoryConfig,
EventBus,
HistoryEvent
} from "../types";

export class HistoryManager {
private isInitialized: boolean = false;
private isDebug: boolean = false;
private config: HistoryConfig | null = null;
private eventBus: EventBus | null = null;

private commands: Map<string, Command> = new Map();
private timeline: string[] = [];
private currentIndex: number = -1;

public initialize(config: HistoryConfig, eventBus: EventBus): Result<void> {
if (this.isInitialized) {
return { success: false, error: "System already initialized" };
}
this.config = Object.freeze({ ...config });
this.eventBus = eventBus;
this.isInitialized = true;
return { success: true, data: undefined };
}

public enableDebug(enabled: boolean): void {
this.isDebug = enabled;
}

public pushCommand(config: CommandConfig): Result<string> {
const check = this.validateState();
if (!check.success) return check;

const id = config.id || this.generateId();

if (this.commands.has(id)) {
  return { success: false, error: "Duplicate command ID" };
}

const newCommand: Command = Object.freeze({
  id,
  type: config.type,
  payload: Object.freeze({ ...config.payload }),
  timestamp: Date.now()
});

const nextCommands = new Map(this.commands);
let nextTimeline = [...this.timeline];

// Drop future branching if pointer isn't at the end
if (this.currentIndex < nextTimeline.length - 1) {
  const droppedIds = nextTimeline.slice(this.currentIndex + 1);
  for (const droppedId of droppedIds) {
    nextCommands.delete(droppedId);
  }
  nextTimeline = nextTimeline.slice(0, this.currentIndex + 1);
}

// Append new command
nextCommands.set(id, newCommand);
nextTimeline = [...nextTimeline, id];

// Prune max size
if (nextTimeline.length > this.config!.maxHistory) {
  const excess = nextTimeline.length - this.config!.maxHistory;
  const prunedIds = nextTimeline.slice(0, excess);
  for (const prunedId of prunedIds) {
    nextCommands.delete(prunedId);
  }
  nextTimeline = nextTimeline.slice(excess);
}

// Atomic Apply
this.commands = nextCommands;
this.timeline = nextTimeline;
this.currentIndex = this.timeline.length - 1;

this.dispatch({ type: "COMMAND_PUSHED", payload: newCommand });
return { success: true, data: id };
}

public undo(): Result<Command> {
const check = this.validateState();
if (!check.success) return check;

if (this.currentIndex < 0) {
  return { success: false, error: "Cannot undo: at beginning of history" };
}

const targetId = this.timeline[this.currentIndex];
const targetCommand = this.commands.get(targetId);

if (!targetCommand) {
  return { success: false, error: "Critical Invariant Failure: Command missing in Map" };
}

// Atomic update
this.currentIndex -= 1;

this.dispatch({ type: "COMMAND_UNDONE", payload: targetCommand });
return { success: true, data: targetCommand };
}

public redo(): Result<Command> {
const check = this.validateState();
if (!check.success) return check;

if (this.currentIndex >= this.timeline.length - 1) {
  return { success: false, error: "Cannot redo: at end of history" };
}

// Atomic update
this.currentIndex += 1;

const targetId = this.timeline[this.currentIndex];
const targetCommand = this.commands.get(targetId);

if (!targetCommand) {
  // Revert pointer on failure (should never happen)
  this.currentIndex -= 1;
  return { success: false, error: "Critical Invariant Failure: Command missing in Map" };
}

this.dispatch({ type: "COMMAND_REDONE", payload: targetCommand });
return { success: true, data: targetCommand };
}

public clear(): Result<void> {
const check = this.validateState();
if (!check.success) return check;

// Atomic flush
this.commands = new Map();
this.timeline = [];
this.currentIndex = -1;

this.dispatch({ type: "HISTORY_CLEARED", payload: null });
return { success: true, data: undefined };
}

public getStateSnapshot(): Command[] {
if (!this.isInitialized) return [];

const activeTimeline = this.timeline.slice(0, this.currentIndex + 1);

return Object.freeze(
  activeTimeline.map(id => this.commands.get(id) as Command)
) as unknown as Command[];
}

private validateState(): Result<void> {
if (!this.isInitialized || !this.config || !this.eventBus) {
return { success: false, error: "Manager not initialized" };
}
return { success: true, data: undefined };
}

private generateId(): string {
return cmd_${Date.now()}_${Math.random().toString(36).substring(2, 9)};
}

private dispatch(event: HistoryEvent): void {
if (this.isDebug) {
console.log([HistoryManager] ${event.type}, event.payload);
}
this.eventBus!.emit(event);
}
}

