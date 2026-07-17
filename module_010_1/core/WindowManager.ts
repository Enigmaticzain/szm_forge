import { OSConfig, WindowConfig, Window, EventBus } from "../types";
import { Result, success, failure } from "../utils/Result";
import { EventDispatcher } from "../systems/EventDispatcher";

export class WindowManager {
  private stateMap: Map<string, Window> = new Map();
  private zOrder: string[] = [];
  private config: OSConfig | null = null;
  private dispatcher: EventDispatcher = new EventDispatcher();
  private isInitialized: boolean = false;
  private debugMode: boolean = false;

  public initialize(config: OSConfig, eventBus: EventBus): Result<void> {
    if (this.isInitialized) return failure("WindowManager already initialized.");
    this.config = config;
    this.dispatcher.connect(eventBus);
    this.isInitialized = true;
    return success(undefined);
  }

  public enableDebug(enabled: boolean): void {
    this.debugMode = enabled;
    this.dispatcher.setDebug(enabled);
  }

  public createWindow(config: WindowConfig): Result<Window> {
    if (!this.isInitialized || !this.config) return failure("System not initialized");
    if (this.stateMap.size >= this.config.maxWindows) return failure("Max windows limit reached");

    const id = config.id || this.generateId();
    if (this.stateMap.has(id)) return failure("Window ID already exists");

    if (!this.isValidSize(config.size)) return failure("Window size violates constraints");
    if (!this.isValidPosition(config.position, config.size)) return failure("Window bounds violate constraints");

    const newWindow: Window = {
      id,
      position: { ...config.position },
      size: { ...config.size },
      state: "active"
    };

    this.stateMap.set(id, newWindow);
    this.zOrder.push(id);
    this.dispatcher.dispatch({ type: "WINDOW_CREATED", payload: newWindow });

    return success(newWindow);
  }

  public destroyWindow(id: string): Result<void> {
    if (!this.isInitialized) return failure("System not initialized");
    if (!this.stateMap.has(id)) return failure("Window ID not found");

    this.stateMap.delete(id);
    this.zOrder = this.zOrder.filter(wId => wId !== id);
    this.dispatcher.dispatch({ type: "WINDOW_DESTROYED", payload: { id } });

    return success(undefined);
  }

  public resizeWindow(id: string, width: number, height: number): Result<void> {
    if (!this.isInitialized || !this.config) return failure("System not initialized");
    const win = this.stateMap.get(id);
    if (!win) return failure("Window ID not found");

    const newSize = { x: width, y: height };
    if (!this.isValidSize(newSize)) return failure("Window size violates constraints");
    if (!this.isValidPosition(win.position, newSize)) return failure("Window bounds violate constraints");

    const updatedWindow: Window = { ...win, size: newSize };
    this.stateMap.set(id, updatedWindow);
    this.dispatcher.dispatch({ type: "WINDOW_RESIZED", payload: { id, size: newSize } });

    return success(undefined);
  }

  public moveWindow(id: string, x: number, y: number): Result<void> {
    if (!this.isInitialized || !this.config) return failure("System not initialized");
    const win = this.stateMap.get(id);
    if (!win) return failure("Window ID not found");

    const newPos = { x, y };
    if (!this.isValidPosition(newPos, win.size)) return failure("Window bounds violate constraints");

    const updatedWindow: Window = { ...win, position: newPos };
    this.stateMap.set(id, updatedWindow);
    this.dispatcher.dispatch({ type: "WINDOW_MOVED", payload: { id, position: newPos } });

    return success(undefined);
  }

  public focusWindow(id: string): Result<void> {
    if (!this.isInitialized) return failure("System not initialized");
    if (!this.stateMap.has(id)) return failure("Window ID not found");

    this.zOrder = this.zOrder.filter(wId => wId !== id);
    this.zOrder.push(id);
    
    this.dispatcher.dispatch({ type: "WINDOW_FOCUSED", payload: { id } });
    return success(undefined);
  }

  public getStateSnapshot(): Window[] {
    const snapshot: Window[] = [];
    for (let i = 0; i < this.zOrder.length; i++) {
      const win = this.stateMap.get(this.zOrder[i]);
      if (win) snapshot.push(win);
    }
    return snapshot;
  }

  private generateId(): string {
    return `win_${Date.now()}_${Math.random().toString(36).substring(2, 9)}`;
  }

  private isValidSize(size: { x: number; y: number }): boolean {
    if (!this.config) return false;
    return size.x >= this.config.minWindowSize.x && size.y >= this.config.minWindowSize.y;
  }

  private isValidPosition(pos: { x: number; y: number }, size: { x: number; y: number }): boolean {
    if (!this.config) return false;
    return pos.x >= 0 && pos.y >= 0 && 
           pos.x + size.x <= this.config.desktopBounds.width && 
           pos.y + size.y <= this.config.desktopBounds.height;
  }
}
