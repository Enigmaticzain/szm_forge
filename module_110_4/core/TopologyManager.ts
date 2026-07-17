import { 
  TopologyConfig, VertexConfig, EdgeConfig, FaceConfig, SolidConfig, 
  TopoEntity, TopoVertex, TopoEdge, TopoFace, TopoSolid, EventBus 
} from "../types";
import { Result, success, failure } from "../utils/Result";
import { TopologyEventDispatcher } from "../systems/TopologyEventDispatcher";

export class TopologyManager {
  private entityMap: Map<string, TopoEntity> = new Map();
  private entityOrder: string[] = [];
  private config: TopologyConfig | null = null;
  private dispatcher: TopologyEventDispatcher = new TopologyEventDispatcher();
  private isInitialized: boolean = false;

  public initialize(config: TopologyConfig, eventBus: EventBus): Result<void> {
    if (this.isInitialized) return failure("TopologyManager already initialized");
    this.config = config;
    this.dispatcher.connect(eventBus);
    this.isInitialized = true;
    return success(undefined);
  }

  public enableDebug(enabled: boolean): void {
    this.dispatcher.setDebug(enabled);
  }

  public createVertex(config: VertexConfig): Result<TopoVertex> {
    if (!this.canAddEntity()) return failure("Max entities reached or uninitialized");
    const id = config.id || this.generateId("vertex");
    if (this.entityMap.has(id)) return failure("ID already exists");

    const vertex: TopoVertex = { id, type: "vertex", position: { ...config.position } };
    this.insertEntity(vertex);
    return success(vertex);
  }

  public createEdge(config: EdgeConfig): Result<TopoEdge> {
    if (!this.canAddEntity()) return failure("Max entities reached or uninitialized");
    const id = config.id || this.generateId("edge");
    if (this.entityMap.has(id)) return failure("ID already exists");

    if (!this.validateRef(config.startVertexId, "vertex") || !this.validateRef(config.endVertexId, "vertex")) {
      return failure("Invalid vertex references for edge");
    }

    const edge: TopoEdge = { id, type: "edge", startVertexId: config.startVertexId, endVertexId: config.endVertexId };
    this.insertEntity(edge);
    return success(edge);
  }

  public createFace(config: FaceConfig): Result<TopoFace> {
    if (!this.canAddEntity()) return failure("Max entities reached or uninitialized");
    const id = config.id || this.generateId("face");
    if (this.entityMap.has(id)) return failure("ID already exists");

    if (config.edgeIds.length < 3) return failure("Face requires at least 3 edges");
    for (const edgeId of config.edgeIds) {
      if (!this.validateRef(edgeId, "edge")) return failure(`Invalid edge reference: ${edgeId}`);
    }

    const face: TopoFace = { id, type: "face", edgeIds: [...config.edgeIds] };
    this.insertEntity(face);
    return success(face);
  }

  public createSolid(config: SolidConfig): Result<TopoSolid> {
    if (!this.canAddEntity()) return failure("Max entities reached or uninitialized");
    const id = config.id || this.generateId("solid");
    if (this.entityMap.has(id)) return failure("ID already exists");

    if (config.faceIds.length < 4) return failure("Solid requires at least 4 faces");
    for (const faceId of config.faceIds) {
      if (!this.validateRef(faceId, "face")) return failure(`Invalid face reference: ${faceId}`);
    }

    const solid: TopoSolid = { id, type: "solid", faceIds: [...config.faceIds] };
    this.insertEntity(solid);
    return success(solid);
  }

  public removeEntity(id: string): Result<void> {
    if (!this.isInitialized) return failure("System not initialized");
    if (!this.entityMap.has(id)) return failure("Entity not found");

    const removals = new Set<string>();
    this.cascadeRemove(id, removals);

    const updatedOrder: string[] = [];
    for (const currentId of this.entityOrder) {
      if (!removals.has(currentId)) {
        updatedOrder.push(currentId);
      } else {
        const entity = this.entityMap.get(currentId)!;
        this.dispatcher.dispatch({ type: "ENTITY_REMOVED", payload: { id: currentId, entityType: entity.type } });
        this.entityMap.delete(currentId);
      }
    }
    
    this.entityOrder = updatedOrder;
    this.dispatcher.dispatch({ type: "TOPOLOGY_UPDATED", payload: { timestamp: Date.now() } });

    return success(undefined);
  }

  public getStateSnapshot(): TopoEntity[] {
    const snapshot: TopoEntity[] = [];
    for (let i = 0; i < this.entityOrder.length; i++) {
      const entity = this.entityMap.get(this.entityOrder[i]);
      if (entity) snapshot.push(entity);
    }
    return snapshot;
  }

  private insertEntity(entity: TopoEntity): void {
    this.entityMap.set(entity.id, entity);
    this.entityOrder.push(entity.id);
    this.dispatcher.dispatch({ type: "ENTITY_CREATED", payload: entity });
    this.dispatcher.dispatch({ type: "TOPOLOGY_UPDATED", payload: { timestamp: Date.now() } });
  }

  private cascadeRemove(targetId: string, removals: Set<string>): void {
    if (removals.has(targetId)) return;
    removals.add(targetId);

    for (const [id, entity] of this.entityMap) {
      if (removals.has(id)) continue;

      if (entity.type === "edge" && (entity.startVertexId === targetId || entity.endVertexId === targetId)) {
        this.cascadeRemove(id, removals);
      } else if (entity.type === "face" && entity.edgeIds.includes(targetId)) {
        this.cascadeRemove(id, removals);
      } else if (entity.type === "solid" && entity.faceIds.includes(targetId)) {
        this.cascadeRemove(id, removals);
      }
    }
  }

  private canAddEntity(): boolean {
    return this.isInitialized && this.config !== null && this.entityMap.size < this.config.maxEntities;
  }

  private validateRef(id: string, expectedType: string): boolean {
    const entity = this.entityMap.get(id);
    return entity !== undefined && entity.type === expectedType;
  }

  private generateId(prefix: string): string {
    return `${prefix}_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
  }
}
