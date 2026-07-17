import { Result } from "../utils/Result";
import {
Constraint,
ConstraintConfig,
SolverConfig,
EventBus,
SolverEvent
} from "../types";

export class SolverManager {
private isInitialized: boolean = false;
private isDebug: boolean = false;
private config: SolverConfig | null = null;
private eventBus: EventBus | null = null;

private constraints: Map<string, Constraint> = new Map();
private executionOrder: string[] = [];

public initialize(config: SolverConfig, eventBus: EventBus): Result<void> {
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

public createConstraint(config: ConstraintConfig): Result<string> {
const check = this.validateState();
if (!check.success) return check;

const id = config.id || this.generateId();

if (this.constraints.has(id)) {
  return { success: false, error: "Duplicate constraint ID" };
}

if (this.constraints.size >= this.config!.maxConstraints) {
  return { success: false, error: "Max constraints exceeded" };
}

const rulesCheck = this.validateRules(config.targetA, config.targetB, config.type, config.value);
if (!rulesCheck.success) return rulesCheck;

const newConstraint: Constraint = Object.freeze({
  id,
  type: config.type,
  targetA: config.targetA,
  targetB: config.targetB,
  value: config.value,
  state: "active"
});

const nextConstraints = new Map(this.constraints);
nextConstraints.set(id, newConstraint);

this.constraints = nextConstraints;
this.executionOrder = [...this.executionOrder, id];

this.dispatch({ type: "CONSTRAINT_CREATED", payload: newConstraint });
return { success: true, data: id };
}

public destroyConstraint(id: string): Result<void> {
const check = this.validateState();
if (!check.success) return check;

if (!this.constraints.has(id)) {
  return { success: false, error: "Invalid constraint id" };
}

const nextConstraints = new Map(this.constraints);
nextConstraints.delete(id);
this.constraints = nextConstraints;
this.executionOrder = this.executionOrder.filter(cId => cId !== id);

this.dispatch({ type: "CONSTRAINT_DESTROYED", payload: { id } });
return { success: true, data: undefined };
}

public updateConstraintValue(id: string, value: number): Result<void> {
const check = this.validateState();
if (!check.success) return check;

const currentConstraint = this.constraints.get(id);
if (!currentConstraint) return { success: false, error: "Invalid constraint id" };

const rulesCheck = this.validateRules(
  currentConstraint.targetA, 
  currentConstraint.targetB, 
  currentConstraint.type, 
  value
);
if (!rulesCheck.success) return rulesCheck;

const updatedConstraint: Constraint = Object.freeze({
  ...currentConstraint,
  value
});

const nextConstraints = new Map(this.constraints);
nextConstraints.set(id, updatedConstraint);
this.constraints = nextConstraints;

this.dispatch({ type: "CONSTRAINT_UPDATED", payload: { id, value: updatedConstraint.value } });
return { success: true, data: undefined };
}

public getStateSnapshot(): Constraint[] {
if (!this.isInitialized) return [];

return Object.freeze(
  this.executionOrder.map(id => this.constraints.get(id) as Constraint)
) as unknown as Constraint[];
}

private validateState(): Result<void> {
if (!this.isInitialized || !this.config || !this.eventBus) {
return { success: false, error: "Manager not initialized" };
}
return { success: true, data: undefined };
}

private validateRules(targetA: string, targetB: string, type: string, value: number): Result<void> {
if (targetA === targetB) {
return { success: false, error: "Targets cannot be identical" };
}
if (type === "distance" && value < 0) {
return { success: false, error: "Distance constraint value cannot be negative" };
}
return { success: true, data: undefined };
}

private generateId(): string {
return cst_${Date.now()}_${Math.random().toString(36).substring(2, 9)};
}

private dispatch(event: SolverEvent): void {
if (this.isDebug) {
console.log([SolverManager] ${event.type}, event.payload);
}
this.eventBus!.emit(event);
}
}

