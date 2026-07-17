export type ModuleMetadata = {
module: "010.3";
version: "1.0.0";
};

export const METADATA: ModuleMetadata = {
module: "010.3",
version: "1.0.0",
};

export type Vector2 = {
readonly x: number;
readonly y: number;
};

export type VirtualInputRegion = {
readonly id: string;
readonly position: Vector2;
readonly size: Vector2;
readonly state: "active" | "minimized";
};

export type RegionConfig = {
readonly id?: string;
readonly position: Vector2;
readonly size: Vector2;
};

export type OSConfig = {
readonly maxRegions: number;
readonly desktopBounds: Vector2;
readonly minSize: Vector2;
};

export type InputEvent =
| { type: "REGION_CREATED"; payload: VirtualInputRegion }
| { type: "REGION_MOVED"; payload: { id: string; position: Vector2 } }
| { type: "REGION_RESIZED"; payload: { id: string; size: Vector2 } }
| { type: "REGION_DESTROYED"; payload: { id: string } }
| { type: "REGION_FOCUSED"; payload: { id: string } };

export interface EventBus {
emit(event: InputEvent): void;
subscribe(handler: (event: InputEvent) => void): void;
}

