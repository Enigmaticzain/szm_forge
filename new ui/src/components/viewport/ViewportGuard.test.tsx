// @vitest-environment jsdom
import React, { useEffect } from 'react';
import { render, screen, cleanup, act } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { ViewportGuard, detectWebGLSupport } from './ViewportGuard';

const realGetContext = HTMLCanvasElement.prototype.getContext;

beforeEach(() => {
  // jsdom lacks WebGL; make getContext quietly deterministic per test.
  HTMLCanvasElement.prototype.getContext = vi.fn().mockReturnValue(null) as never;
});

afterEach(() => {
  cleanup();
  HTMLCanvasElement.prototype.getContext = realGetContext;
});

describe('detectWebGLSupport', () => {
  it('reports unsupported with a reason when no context exists', () => {
    const s = detectWebGLSupport();
    expect(s.supported).toBe(false);
    expect(s.reason).toBeTruthy();
  });
});

describe('ViewportGuard', () => {
  it('shows an honest fallback instead of mounting children when WebGL is unavailable', () => {
    render(
      <ViewportGuard featureName="Physics viewport">
        <div data-testid="live-3d">live 3d</div>
      </ViewportGuard>,
    );
    expect(screen.queryByTestId('live-3d')).not.toBeInTheDocument();
    const alert = screen.getByRole('alert');
    expect(alert).toHaveTextContent('Physics viewport needs WebGL');
    expect(alert).toHaveTextContent(/no WebGL context/i);
    expect(alert).toHaveTextContent(/hardware acceleration/i);
    expect(alert).toHaveTextContent(/remain fully usable/i);
  });

  it('renders children normally when WebGL is present', () => {
    HTMLCanvasElement.prototype.getContext = vi.fn().mockReturnValue({}) as never;
    render(
      <ViewportGuard featureName="Physics viewport">
        <div data-testid="live-3d">live 3d</div>
      </ViewportGuard>,
    );
    expect(screen.getByTestId('live-3d')).toBeInTheDocument();
  });

  it('contains a renderer crash during commit and keeps the app chrome alive', () => {
    // WebGL "available" so children mount…
    HTMLCanvasElement.prototype.getContext = vi.fn().mockReturnValue({}) as never;
    // …then the renderer throws during effect commit (GPU reset / driver loss).
    const Boom: React.FC = () => {
      useEffect(() => {
        throw new Error('THREE.WebGLRenderer: Error creating WebGL context');
      }, []);
      return null;
    };
    const consoleSpy = vi.spyOn(console, 'error').mockImplementation(() => {});

    render(
      <div>
        <ViewportGuard featureName="Assembly editor" hint="Part lists keep working.">
          <Boom />
        </ViewportGuard>
        <div data-testid="sibling-panel">sibling UI</div>
      </div>,
    );

    act(() => {}); // flush passive effects (where the throw lands)

    expect(screen.getByRole('alert')).toHaveTextContent(/Assembly editor needs WebGL/i);
    expect(screen.getByTestId('sibling-panel')).toBeInTheDocument();
    consoleSpy.mockRestore();
  });
});
