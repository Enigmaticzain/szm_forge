import { SceneConfig, NodeConfig, SceneNode, Transform, EventBus } from "../types";
import { Result, success, failure } from "../utils/Result";
import { SceneEventDispatcher } from "../systems/SceneEventDispatcher";

const DEFAULT_TRANSFORM: Transform = {
  position: { x: 0, y: 0, z: 0 },
  rotation: { x: 0, y: 0, z: 0 },
  scale: { x: 1, y: 1, z: 1 }
};

export class SceneGraphManager {
  private stateMap: Map<string, SceneNode> = new Map();
  private config: SceneConfig | null = null;
  private dispatcher: SceneEventDispatcher = new SceneEventDispatcher();
  private isInitialized: boolean = false;

  public initialize(config: SceneConfig, eventBus: EventBus): Result<void> {
    if (this.isInitialized) return failure("Already initialized");
    this.config = config;
    this.dispatcher.connect(eventBus);
    this.isInitialized = true;
    return success(undefined);
  }

  public enableDebug(enabled: boolean): void {
    this.dispatcher.setDebug(enabled);
  }

  public createNode(config: NodeConfig): Result<SceneNode> {
    if (!this.isInitialized || !this.config) return failure("Not initialized");
    if (this.stateMap.size >= this.config.maxNodes) return failure("Max nodes reached");

    const id = config.id || this.generateId();
    if (this.stateMap.has(id)) return failure("Duplicate ID");

    const parentId = config.parentId || null;
    if (parentId !== null && !this.stateMap.has(parentId)) return failure("Parent not found");
    if (parentId !== null && this.calculateDepth(parentId) >= this.config.maxHierarchyDepth) {
      return failure("Max hierarchy depth exceeded");
    }

    const newNode: SceneNode = {
      id,
      parentId,
      children: [],
      transform: this.mergeTransform(DEFAULT_TRANSFORM, config.transform),
      nodeType: config.nodeType || "default"
    };

    const updates = new Map<string, SceneNode>();
    updates.set(id, newNode);

    if (parentId !== null) {
      const parent = this.stateMap.get(parentId)!;
      updates.set(parentId, { ...parent, children: [...parent.children, id] });
    }

    this.applyUpdates(updates);
    this.dispatcher.dispatch({ type: "NODE_CREATED", payload: newNode });

    return success(newNode);
  }

  public removeNode(id: string): Result<void> {
    if (!this.isInitialized) return failure("Not initialized");
    if (!this.stateMap.has(id)) return failure("Node not found");

    const nodeToDelete = this.stateMap.get(id)!;
    const updates = new Map<string, SceneNode>();
    const removals = new Set<string>();

    this.collectDescendants(id, removals);
    removals.add(id);

    if (nodeToDelete.parentId !== null) {
      const parent = this.stateMap.get(nodeToDelete.parentId);
      if (parent && !removals.has(parent.id)) {
        updates.set(parent.id, {
          ...parent,
          children: parent.children.filter(c => c !== id)
        });
      }
    }

    this.applyDeletions(updates, removals);
    
    // Dispatch individual removal events to maintain contract
    for (const removedId of removals) {
      this.dispatcher.dispatch({ type: "NODE_REMOVED", payload: { id: removedId } });
    }

    return success(undefined);
  }

  public reparentNode(id: string, newParentId: string | null): Result<void> {
    if (!this.isInitialized || !this.config) return failure("Not initialized");
    if (!this.stateMap.has(id)) return failure("Node not found");
    
    const node = this.stateMap.get(id)!;
    if (node.parentId === newParentId) return success(undefined);

    if (newParentId !== null) {
      if (!this.stateMap.has(newParentId)) return failure("New parent not found");
      if (this.wouldCreateCycle(id, newParentId)) return failure("Cyclical reparenting detected");
      
      const newParentDepth = this.calculateDepth(newParentId);
      const subtreeDepth = this.calculateSubtreeDepth(id);
      if (newParentDepth + subtreeDepth >= this.config.maxHierarchyDepth) {
        return failure("Max hierarchy depth exceeded");
      }
    }

    const updates = new Map<string, SceneNode>();
    updates.set(id, { ...node, parentId: newParentId });

    if (node.parentId !== null) {
      const oldParent = this.stateMap.get(node.parentId)!;
      updates.set(oldParent.id, {
        ...oldParent,
        children: oldParent.children.filter(c => c !== id)
      });
    }

    if (newParentId !== null) {
      const newParent = updates.get(newParentId) || this.stateMap.get(newParentId)!;
      updates.set(newParentId, {
        ...newParent,
        children: [...newParent.children, id]
      });
    }

    this.applyUpdates(updates);
    this.dispatcher.dispatch({
      type: "NODE_REPARENTED",
      payload: { id, oldParentId: node.parentId, newParentId }
    });

    return success(undefined);
  }

  public updateTransform(id: string, partialTransform: Partial<Transform>): Result<void> {
    if (!this.isInitialized) return failure("Not initialized");
    const node = this.stateMap.get(id);
    if (!node) return failure("Node not found");

    const newTransform = this.mergeTransform(node.transform, partialTransform);
    const updatedNode: SceneNode = { ...node, transform: newTransform };
    
    this.stateMap.set(id, updatedNode);
    this.dispatcher.dispatch({ type: "TRANSFORM_UPDATED", payload: { id, transform: newTransform } });

    return success(undefined);
  }

  public serializeScene(): Result<string> {
    if (!this.isInitialized) return failure("Not initialized");
    try {
      const nodes = Array.from(this.stateMap.values());
      return success(JSON.stringify(nodes));
    } catch (e) {
      return failure("Serialization failed");
    }
  }

  public deserializeScene(data: string): Result<void> {
    if (!this.isInitialized || !this.config) return failure("Not initialized");
    try {
      const parsed: SceneNode[] = JSON.parse(data);
      if (!Array.isArray(parsed) || parsed.length > this.config.maxNodes) {
        return failure("Invalid data or exceeds max nodes");
      }

      const newMap = new Map<string, SceneNode>();
      for (const node of parsed) {
        if (!node.id || node.parentId === undefined || !Array.isArray(node.children)) {
          return failure("Malformed node structure detected");
        }
        newMap.set(node.id, node);
      }

      this.stateMap = newMap;
      this.dispatcher.dispatch({ type: "SCENE_DESERIALIZED", payload: { count: this.stateMap.size } });
      return success(undefined);
    } catch (e) {
      return failure("Deserialization failed");
    }
  }

  public getStateSnapshot(): SceneNode[] {
    return Array.from(this.stateMap.values());
  }

  private mergeTransform(base: Transform, partial?: Partial<Transform>): Transform {
    if (!partial) return base;
    return {
      position: { ...base.position, ...partial.position },
      rotation: { ...base.rotation, ...partial.rotation },
      scale: { ...base.scale, ...partial.scale }
    };
  }

  private generateId(): string {
    return `node_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
  }

  private wouldCreateCycle(nodeId: string, targetParentId: string): boolean {
    let currentId: string | null = targetParentId;
    while (currentId !== null) {
      if (currentId === nodeId) return true;
      const parent = this.stateMap.get(currentId);
      currentId = parent ? parent.parentId : null;
    }
    return false;
  }

  private calculateDepth(nodeId: string): number {
    let depth = 0;
    let currentId: string | null = nodeId;
    while (currentId !== null) {
      depth++;
      const node = this.stateMap.get(currentId);
      currentId = node ? node.parentId : null;
    }
    return depth;
  }

  private calculateSubtreeDepth(nodeId: string): number {
    const node = this.stateMap.get(nodeId);
    if (!node || node.children.length === 0) return 1;
    let maxChildDepth = 0;
    for (const childId of node.children) {
      maxChildDepth = Math.max(maxChildDepth, this.calculateSubtreeDepth(childId));
    }
    return 1 + maxChildDepth;
  }

  private collectDescendants(nodeId: string, removals: Set<string>): void {
    const node = this.stateMap.get(nodeId);
    if (!node) return;
    for (const childId of node.children) {
      removals.add(childId);
      this.collectDescendants(childId, removals);
    }
  }

  private applyUpdates(updates: Map<string, SceneNode>): void {
    updates.forEach((node, id) => {
      this.stateMap.set(id, node);
    });
  }

  private applyDeletions(updates: Map<string, SceneNode>, removals: Set<string>): void {
    removals.forEach(id => this.stateMap.delete(id));
    this.applyUpdates(updates);
  }
}
