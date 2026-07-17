export type Vector2 = { readonly x: number; readonly y: number };

export type WindowConfig = {
  readonly id?: string;
  readonly position: Vector2;
  readonly size: Vector2;
};

export type Window = {
  readonly id: string;
  readonly position: Vector2;
  readonly size: Vector2;
  readonly state: "active" | "minimized";
};

export type OSConfig = {
  readonly maxWindows: number;
  readonly minWindowSize: Vector2;
  readonly desktopBounds: { readonly width: number; readonly height: number };
};

export type WindowEvent =
  | { readonly type: "WINDOW_CREATED"; readonly payload: Window }
  | { readonly type: "WINDOW_MOVED"; readonly payload: { readonly id: string; readonly position: Vector2 } }
  | { readonly type: "WINDOW_RESIZED"; readonly payload: { readonly id: string; readonly size: Vector2 } }
  | { readonly type: "WINDOW_DESTROYED"; readonly payload: { readonly id: string } }
  | { readonly type: "WINDOW_FOCUSED"; readonly payload: { readonly id: string } };

export interface EventBus {
  emit(event: WindowEvent): void;
  subscribe(handler: (event: WindowEvent) => void): void;
}
