export type Resolution = {
readonly width: number;
readonly height: number;
};

export type SwapchainConfig = {
readonly id: string;
readonly resolution: Resolution;
readonly vsync: boolean;
};

export type Swapchain = {
readonly id: string;
readonly resolution: Resolution;
readonly vsync: boolean;
readonly state: "ready" | "presenting" | "lost";
};

export type GraphicsConfig = {
readonly maxSwapchains: number;
readonly hardwareAcceleration: boolean;
};

export type GraphicsEvent =
| { readonly type: "CONTEXT_INITIALIZED"; readonly payload: { readonly accelerated: boolean } }
| { readonly type: "SWAPCHAIN_CREATED"; readonly payload: Swapchain }
| { readonly type: "SWAPCHAIN_RESIZED"; readonly payload: { readonly id: string; readonly resolution: Resolution } }
| { readonly type: "FRAME_PRESENTED"; readonly payload: { readonly id: string } }
| { readonly type: "SWAPCHAIN_DESTROYED"; readonly payload: { readonly id: string } };

export interface EventBus {
emit(event: GraphicsEvent): void;
subscribe(handler: (event: GraphicsEvent) => void): void;
}

