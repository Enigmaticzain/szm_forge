import { Result } from "../utils/Result";
import {
  MeshNode,
  TessellationConfig,
  RenderConfig,
  EventBus,
  MeshEvent,
  PBRMaterial
} from "../types";

export class TessellationManager {
  private isInitialized: boolean = false;
  private isDebug: boolean = false;
  private config: RenderConfig | null = null;
  private eventBus: EventBus | null = null;

  private meshes: Map<string, MeshNode> = new Map();
  private renderQueue: string[] = []; 

  public initialize(config: RenderConfig, eventBus: EventBus): Result<void> {
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

  public tessellateBody(config: TessellationConfig): Result<string> {
    const check = this.validateState();
    if (!check.success) return check;

    const id = config.id || this.generateId();

    if (this.meshes.has(id)) {
      return { success: false, error: "Duplicate mesh ID" };
    }

    if (this.meshes.size >= this.config!.maxMeshes) {
      return { success: false, error: "Max meshes exceeded" };
    }

    const geometryCheck = this.validateGeometry(config.vertexCount, config.indexCount);
    if (!geometryCheck.success) return geometryCheck;

    const pbrCheck = this.validateMaterial(config.material);
    if (!pbrCheck.success) return pbrCheck;

    const newMesh: MeshNode = Object.freeze({
      id,
      sourceBodyId: config.sourceBodyId,
      vertexCount: config.vertexCount,
      indexCount: config.indexCount,
      material: Object.freeze({ ...config.material }),
      state: "active"
    });

    const nextMeshes = new Map(this.meshes);
    nextMeshes.set(id, newMesh);
    
    this.meshes = nextMeshes;
    this.recalculateRenderQueue();

    this.dispatch({ type: "MESH_GENERATED", payload: newMesh });
    return { success: true, data: id };
  }

  public destroyMesh(id: string): Result<void> {
    const check = this.validateState();
    if (!check.success) return check;

    if (!this.meshes.has(id)) {
      return { success: false, error: "Invalid mesh id" };
    }

    const nextMeshes = new Map(this.meshes);
    nextMeshes.delete(id);
    this.meshes = nextMeshes;
    this.recalculateRenderQueue();

    this.dispatch({ type: "MESH_DESTROYED", payload: { id } });
    return { success: true, data: undefined };
  }

  public updateMaterial(id: string, material: PBRMaterial): Result<void> {
    const check = this.validateState();
    if (!check.success) return check;

    const currentMesh = this.meshes.get(id);
    if (!currentMesh) return { success: false, error: "Invalid mesh id" };

    const pbrCheck = this.validateMaterial(material);
    if (!pbrCheck.success) return pbrCheck;

    const updatedMesh: MeshNode = Object.freeze({
      ...currentMesh,
      material: Object.freeze({ ...material })
    });

    const nextMeshes = new Map(this.meshes);
    nextMeshes.set(id, updatedMesh);
    this.meshes = nextMeshes;
    this.recalculateRenderQueue();

    this.dispatch({ type: "MATERIAL_UPDATED", payload: { id, material: updatedMesh.material } });
    return { success: true, data: undefined };
  }

  public getStateSnapshot(): MeshNode[] {
    if (!this.isInitialized) return [];
    
    return Object.freeze(
      this.renderQueue.map(id => this.meshes.get(id) as MeshNode)
    ) as unknown as MeshNode[];
  }

  private validateState(): Result<void> {
    if (!this.isInitialized || !this.config || !this.eventBus) {
      return { success: false, error: "Manager not initialized" };
    }
    return { success: true, data: undefined };
  }

  private validateGeometry(vertexCount: number, indexCount: number): Result<void> {
    if (vertexCount <= 0 || indexCount <= 0) {
      return { success: false, error: "Vertex and Index counts must be > 0 invariant violation" };
    }
    if (vertexCount > this.config!.maxVerticesPerMesh) {
      return { success: false, error: "Mesh exceeds max vertex limit" };
    }
    return { success: true, data: undefined };
  }

  private validateMaterial(material: PBRMaterial): Result<void> {
    if (material.metallic < 0.0 || material.metallic > 1.0) {
      return { success: false, error: "PBR Metallic must be between 0.0 and 1.0" };
    }
    if (material.roughness < 0.0 || material.roughness > 1.0) {
      return { success: false, error: "PBR Roughness must be between 0.0 and 1.0" };
    }
    return { success: true, data: undefined };
  }

  private recalculateRenderQueue(): void {
    // Deterministic state derivation: Sort by material albedo as a simple batching mechanism
    this.renderQueue = Array.from(this.meshes.values())
      .sort((a, b) => a.material.albedo - b.material.albedo)
      .map(mesh => mesh.id);
  }

  private generateId(): string {
    return `mesh_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
  }

  private dispatch(event: MeshEvent): void {
    if (this.isDebug) {
      console.log(`[TessellationManager] ${event.type}`, event.payload);
    }
    this.eventBus!.emit(event);
  }
}


