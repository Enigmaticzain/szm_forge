import { GraphicsConfig, SwapchainConfig, Swapchain, EventBus, Resolution } from "../types";
import { Result, success, failure } from "../utils/Result";
import { GraphicsEventDispatcher } from "../systems/GraphicsEventDispatcher";

export class GraphicsManager {
private stateMap: Map<string, Swapchain> = new Map();
private priorityList: string[] = [];
private config: GraphicsConfig | null = null;
private dispatcher: GraphicsEventDispatcher = new GraphicsEventDispatcher();
private isInitialized: boolean = false;
private debugMode: boolean = false;

public initialize(config: GraphicsConfig, eventBus: EventBus): Result<void> {
if (this.isInitialized) return failure("GraphicsManager already initialized.");
this.config = config;
this.dispatcher.connect(eventBus);
this.isInitialized = true;

this.dispatcher.dispatch({ 
  type: "CONTEXT_INITIALIZED", 
  payload: { accelerated: config.hardwareAcceleration } 
});

return success(undefined);
}

public enableDebug(enabled: boolean): void {
this.debugMode = enabled;
this.dispatcher.setDebug(enabled);
}

public createSwapchain(config: SwapchainConfig): Result<Swapchain> {
if (!this.isInitialized || !this.config) return failure("System not initialized");
if (this.stateMap.size >= this.config.maxSwapchains) return failure("Max swapchains limit reached");
if (this.stateMap.has(config.id)) return failure("Swapchain ID already exists");
if (!this.isValidResolution(config.resolution)) return failure("Invalid resolution: must be > 0");

const newSwapchain: Swapchain = {
  id: config.id,
  resolution: { ...config.resolution },
  vsync: config.vsync,
  state: "ready"
};

this.stateMap.set(config.id, newSwapchain);
this.priorityList.push(config.id);
this.dispatcher.dispatch({ type: "SWAPCHAIN_CREATED", payload: newSwapchain });

return success(newSwapchain);
}

public destroySwapchain(id: string): Result<void> {
if (!this.isInitialized) return failure("System not initialized");
if (!this.stateMap.has(id)) return failure("Swapchain ID not found");

this.stateMap.delete(id);
this.priorityList = this.priorityList.filter(sId => sId !== id);
this.dispatcher.dispatch({ type: "SWAPCHAIN_DESTROYED", payload: { id } });

return success(undefined);
}

public resizeSwapchain(id: string, width: number, height: number): Result<void> {
if (!this.isInitialized) return failure("System not initialized");
const current = this.stateMap.get(id);
if (!current) return failure("Swapchain ID not found");

const newRes: Resolution = { width, height };
if (!this.isValidResolution(newRes)) return failure("Invalid resolution: must be > 0");

const updatedSwapchain: Swapchain = { ...current, resolution: newRes };
this.stateMap.set(id, updatedSwapchain);
this.dispatcher.dispatch({ type: "SWAPCHAIN_RESIZED", payload: { id, resolution: newRes } });

return success(undefined);
}

public presentFrame(id: string): Result<void> {
if (!this.isInitialized) return failure("System not initialized");
const current = this.stateMap.get(id);
if (!current) return failure("Swapchain ID not found");
if (current.state === "lost") return failure("Swapchain is in lost state");

const presentingSwapchain: Swapchain = { ...current, state: "presenting" };
this.stateMap.set(id, presentingSwapchain);

this.dispatcher.dispatch({ type: "FRAME_PRESENTED", payload: { id } });

const readySwapchain: Swapchain = { ...presentingSwapchain, state: "ready" };
this.stateMap.set(id, readySwapchain);

return success(undefined);
}

public getSwapchainSnapshot(): Swapchain[] {
const snapshot: Swapchain[] = [];
for (let i = 0; i < this.priorityList.length; i++) {
const swp = this.stateMap.get(this.priorityList[i]);
if (swp) snapshot.push(swp);
}
return snapshot;
}

private isValidResolution(res: Resolution): boolean {
return res.width > 0 && res.height > 0;
}
}

