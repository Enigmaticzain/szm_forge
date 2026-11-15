/**
 * ViewportGuard — WebGL containment for every 3D entry point.
 *
 * Why this exists (USER_WORKFLOW_FINDINGS P1 #5): a WebGLRenderer that fails to
 * construct previously crashed the React tree and blanked the whole app. This
 * guard does two honest things instead:
 *
 *  1. PREFLIGHT: check WebGL availability before mounting children at all —
 *     shows guidance instantly, no console spam, no broken half-mounted canvas.
 *  2. CONTAIN: if the renderer still throws during commit (driver loss, GPU
 *     reset), catch it and leave the rest of the app fully usable.
 *
 * The fallback states the feature name, the reason, what to do, and what stays
 * usable — never a blank page.
 */

import { Component, ReactNode } from 'react';
import { TriangleAlert } from 'lucide-react';

export interface WebGlSupport {
  supported: boolean;
  reason: string | null;
}

export function detectWebGLSupport(): WebGlSupport {
  if (typeof document === 'undefined') return { supported: true, reason: null }; // SSR: defer to mount
  try {
    const canvas = document.createElement('canvas');
    const gl = (canvas.getContext('webgl2') ?? canvas.getContext('webgl')) as unknown;
    if (gl) return { supported: true, reason: null };
    return { supported: false, reason: 'this browser or GPU driver exposes no WebGL context' };
  } catch (e) {
    return { supported: false, reason: e instanceof Error ? e.message : 'WebGL check failed' };
  }
}

const FALLBACK_GUIDANCE = [
  'Enable hardware acceleration in your browser settings and reload.',
  'On Linux under a VM, ensure GPU pass-through or software GL (SwiftShader/llvmpipe) is available.',
  'Non-3D workspaces (Sketch 2D, Analysis, Knowledge, Settings) remain fully usable.',
];

interface Props {
  /** Human-readable name of the 3D surface being guarded, e.g. "Physics viewport". */
  featureName: string;
  /** What the user clicked from, for context in the message (optional). */
  hint?: string;
  children: ReactNode;
}

interface State {
  /** Preflight failure reason, null when WebGL preflight passed. */
  unsupportedReason: string | null;
  /** True when a renderer crashed during mount despite preflight. */
  crashed: boolean;
}

export class ViewportGuard extends Component<Props, State> {
  constructor(props: Props) {
    super(props);
    const support = detectWebGLSupport();
    this.state = {
      unsupportedReason: support.supported ? null : (support.reason ?? 'WebGL unavailable'),
      crashed: false,
    };
  }

  static getDerivedStateFromError(): Partial<State> {
    return { crashed: true };
  }

  componentDidCatch(error: Error): void {
    // Surface once for debugging without spamming: the UI stays honest.
    console.error(`[ViewportGuard] ${this.props.featureName} crashed:`, error);
  }

  render() {
    const reason = this.state.unsupportedReason;
    if (reason === null && !this.state.crashed) {
      return this.props.children;
    }
    return (
      <div
        role="alert"
        className="h-full w-full flex items-center justify-center p-6 text-center bg-forge-black/60 border border-forge-yellow/20 rounded"
      >
        <div className="max-w-md space-y-3">
          <TriangleAlert size={20} className="text-forge-yellow mx-auto" />
          <div className="text-sm font-semibold text-forge-text">
            {this.props.featureName} needs WebGL — unavailable here.
          </div>
          <p className="text-xs leading-relaxed text-forge-text-muted">
            {this.state.crashed
              ? `${this.props.featureName} crashed while starting its 3D renderer.`
              : reason}
            {this.props.hint ? ` ${this.props.hint}` : ''}
          </p>
          <ul className="text-[11px] text-forge-text-dim text-left list-disc pl-4 space-y-1">
            {FALLBACK_GUIDANCE.map(g => (
              <li key={g}>{g}</li>
            ))}
          </ul>
        </div>
      </div>
    );
  }
}
