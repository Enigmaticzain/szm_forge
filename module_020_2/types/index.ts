export type ModuleMetadata = {
module: "020.2";
version: "1.0.0";
};

export const METADATA: ModuleMetadata = {
module: "020.2",
version: "1.0.0",
};

export type Command = {
readonly id: string;
readonly type: string;
readonly payload: Record<string, unknown>;
readonly timestamp: number;
};

export type CommandConfig = {
readonly id?: string;
readonly type: string;
readonly payload: Record<string, unknown>;
};

export type HistoryConfig = {
readonly maxHistory: number;
};

export type HistoryEvent =
| { type: "COMMAND_PUSHED"; payload: Command }
| { type: "COMMAND_UNDONE"; payload: Command }
| { type: "COMMAND_REDONE"; payload: Command }
| { type: "HISTORY_CLEARED"; payload: null };

export interface EventBus {
emit(event: HistoryEvent): void;
subscribe(handler: (event: HistoryEvent) => void): void;
}

