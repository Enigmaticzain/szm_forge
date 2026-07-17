export type ModuleMetadata = {
module: "130.1";
version: "1.0.0";
};

export const METADATA: ModuleMetadata = {
module: "130.1",
version: "1.0.0",
};

export type Constraint = {
readonly id: string;
readonly type: "distance" | "angle" | "locked";
readonly targetA: string;
readonly targetB: string;
readonly value: number;
readonly state: "active" | "violated";
};

export type ConstraintConfig = {
readonly id?: string;
readonly type: "distance" | "angle" | "locked";
readonly targetA: string;
readonly targetB: string;
readonly value: number;
};

export type SolverConfig = {
readonly maxConstraints: number;
readonly tolerance: number;
};

export type SolverEvent =
| { type: "CONSTRAINT_CREATED"; payload: Constraint }
| { type: "CONSTRAINT_UPDATED"; payload: { id: string; value: number } }
| { type: "CONSTRAINT_DESTROYED"; payload: { id: string } }
| { type: "CONSTRAINT_VIOLATED"; payload: { id: string } };

export interface EventBus {
emit(event: SolverEvent): void;
subscribe(handler: (event: SolverEvent) => void): void;
}

