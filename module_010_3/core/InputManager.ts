import { Result } from "../utils/Result";
import {
VirtualInputRegion,
RegionConfig,
OSConfig,
EventBus,
InputEvent,
Vector2
} from "../types";

export class InputManager {
private isInitialized: boolean = false;
private isDebug: boolean = false;
private config: OSConfig | null = null;
private eventBus: EventBus | null = null;

private regions: Map<string, VirtualInputRegion> = new Map();
private priorityList: string[] = [];

public initialize(config: OSConfig, eventBus: EventBus): Result<void> {
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

public createRegion(config: RegionConfig): Result<string> {
const check = this.validateState();
if (!check.success) return check;

const id = config.id || this.generateId();

if (this.regions.has(id)) {
  return { success: false, error: "Duplicate region ID" };
}

if (this.regions.size >= this.config!.maxRegions) {
  return { success: false, error: "Max regions exceeded" };
}

const boundsCheck = this.validateBounds(config.position, config.size);
if (!boundsCheck.success) return boundsCheck;

const newRegion: VirtualInputRegion = Object.freeze({
  id,
  position: Object.freeze({ ...config.position }),
  size: Object.freeze({ ...config.size }),
  state: "active"
});

const nextRegions = new Map(this.regions);
nextRegions.set(id, newRegion);

this.regions = nextRegions;
this.priorityList = [...this.priorityList, id];

this.dispatch({ type: "REGION_CREATED", payload: newRegion });
return { success: true, data: id };
}

public destroyRegion(id: string): Result<void> {
const check = this.validateState();
if (!check.success) return check;

if (!this.regions.has(id)) {
  return { success: false, error: "Invalid region id" };
}

const nextRegions = new Map(this.regions);
nextRegions.delete(id);
this.regions = nextRegions;
this.priorityList = this.priorityList.filter(regId => regId !== id);

this.dispatch({ type: "REGION_DESTROYED", payload: { id } });
return { success: true, data: undefined };
}

public resizeRegion(id: string, width: number, height: number): Result<void> {
const check = this.validateState();
if (!check.success) return check;

const currentRegion = this.regions.get(id);
if (!currentRegion) return { success: false, error: "Invalid region id" };

const newSize: Vector2 = { x: width, y: height };
const boundsCheck = this.validateBounds(currentRegion.position, newSize);
if (!boundsCheck.success) return boundsCheck;

const updatedRegion: VirtualInputRegion = Object.freeze({
  ...currentRegion,
  size: Object.freeze(newSize)
});

const nextRegions = new Map(this.regions);
nextRegions.set(id, updatedRegion);
this.regions = nextRegions;

this.dispatch({ type: "REGION_RESIZED", payload: { id, size: updatedRegion.size } });
return { success: true, data: undefined };
}

public moveRegion(id: string, x: number, y: number): Result<void> {
const check = this.validateState();
if (!check.success) return check;

const currentRegion = this.regions.get(id);
if (!currentRegion) return { success: false, error: "Invalid region id" };

const newPosition: Vector2 = { x, y };
const boundsCheck = this.validateBounds(newPosition, currentRegion.size);
if (!boundsCheck.success) return boundsCheck;

const updatedRegion: VirtualInputRegion = Object.freeze({
  ...currentRegion,
  position: Object.freeze(newPosition)
});

const nextRegions = new Map(this.regions);
nextRegions.set(id, updatedRegion);
this.regions = nextRegions;

this.dispatch({ type: "REGION_MOVED", payload: { id, position: updatedRegion.position } });
return { success: true, data: undefined };
}

public getStateSnapshot(): VirtualInputRegion[] {
if (!this.isInitialized) return [];

return Object.freeze(
  this.priorityList.map(id => this.regions.get(id) as VirtualInputRegion)
) as unknown as VirtualInputRegion[];
}

private validateState(): Result<void> {
if (!this.isInitialized || !this.config || !this.eventBus) {
return { success: false, error: "Manager not initialized" };
}
return { success: true, data: undefined };
}

private validateBounds(pos: Vector2, size: Vector2): Result<void> {
const cfg = this.config!;

if (size.x < cfg.minSize.x || size.y < cfg.minSize.y) {
  return { success: false, error: "Size below minSize invariant" };
}
if (pos.x < 0 || pos.y < 0) {
  return { success: false, error: "Position out of desktop bounds" };
}
if (pos.x + size.x > cfg.desktopBounds.x || pos.y + size.y > cfg.desktopBounds.y) {
  return { success: false, error: "Region exceeds desktop bounds" };
}

return { success: true, data: undefined };
}

private generateId(): string {
return region_${Date.now()}_${Math.random().toString(36).substring(2, 9)};
}

private dispatch(event: InputEvent): void {
if (this.isDebug) {
console.log([InputManager] ${event.type}, event.payload);
}
this.eventBus!.emit(event);
}
}

