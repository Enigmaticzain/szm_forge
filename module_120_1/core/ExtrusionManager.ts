import { ExtrusionConfig, ExtrusionCreateConfig, ExtrusionOperation, EventBus, Vector3 } from "../types";
import { Result, success, failure } from "../utils/Result";
import { ExtrusionEventDispatcher } from "../systems/ExtrusionEventDispatcher";

export class ExtrusionManager {
  private stateMap: Map<string, ExtrusionOperation> = new Map();
  private executionOrder: string[] = [];
  private config: ExtrusionConfig | null = null;
  private dispatcher: ExtrusionEventDispatcher = new ExtrusionEventDispatcher();
  private isInitialized: boolean = false;

  public initialize(config: ExtrusionConfig, eventBus: EventBus): Result<void> {
    if (this.isInitialized) return failure("Manager already initialized.");
    this.config = config;
    this.dispatcher.connect(eventBus);
    this.isInitialized = true;
    return success(undefined);
  }

  public enableDebug(enabled: boolean): void {
    this.dispatcher.setDebug(enabled);
  }

  public createExtrusion(config: ExtrusionCreateConfig): Result<ExtrusionOperation> {
    if (!this.isInitialized || !this.config) return failure("System not initialized");
    if (this.stateMap.size >= this.config.maxOperations) return failure("Max operations limit reached");

    const id = config.id || this.generateId();
    if (this.stateMap.has(id)) return failure("Extrusion ID already exists");

    if (config.distance < this.config.minDistance) return failure("Distance below minimum threshold");
    if (!this.isValidDirection(config.direction)) return failure("Direction vector magnitude must be > 0");

    const newExtrusion: ExtrusionOperation = {
      id,
      profileFaceId: config.profileFaceId,
      direction: { ...config.direction },
      distance: config.distance,
      status: "active"
    };

    this.stateMap.set(id, newExtrusion);
    this.executionOrder.push(id);
    this.dispatcher.dispatch({ type: "EXTRUSION_CREATED", payload: newExtrusion });

    return success(newExtrusion);
  }

  public updateExtrusion(id: string, distance: number): Result<void> {
    if (!this.isInitialized || !this.config) return failure("System not initialized");
    const op = this.stateMap.get(id);
    if (!op) return failure("Extrusion ID not found");

    if (distance < this.config.minDistance) return failure("Distance below minimum threshold");

    const updatedExtrusion: ExtrusionOperation = { ...op, distance };
    this.stateMap.set(id, updatedExtrusion);
    
    this.dispatcher.dispatch({ type: "EXTRUSION_UPDATED", payload: { id, distance } });
    return success(undefined);
  }

  public removeExtrusion(id: string): Result<void> {
    if (!this.isInitialized) return failure("System not initialized");
    if (!this.stateMap.has(id)) return failure("Extrusion ID not found");

    this.stateMap.delete(id);
    this.executionOrder = this.executionOrder.filter(opId => opId !== id);
    
    this.dispatcher.dispatch({ type: "EXTRUSION_REMOVED", payload: { id } });
    return success(undefined);
  }

  public getStateSnapshot(): ExtrusionOperation[] {
    const snapshot: ExtrusionOperation[] = [];
    for (let i = 0; i < this.executionOrder.length; i++) {
      const op = this.stateMap.get(this.executionOrder[i]);
      if (op) snapshot.push(op);
    }
    return snapshot;
  }

  private generateId(): string {
    return `extrude_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
  }

  private isValidDirection(dir: Vector3): boolean {
    const magSquared = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
    return magSquared > 0;
  }
}
