// @vitest-environment jsdom
import { render, screen, cleanup } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, describe, expect, it } from 'vitest';
import { CommandCenterWorkspace } from './CommandCenterWorkspace';
import { ForgeStoreProvider } from '../store/ForgeStoreContext';
import { BackendProvider } from '../store/BackendContext';
import { ProjectProvider } from '../store/ProjectContext';

afterEach(() => {
  cleanup();
});

function renderWithProviders(ui: React.ReactElement) {
  // Mirrors the provider tree in main.tsx so contexts resolve honestly.
  return render(
    <ForgeStoreProvider>
      <BackendProvider>
        <ProjectProvider>{ui}</ProjectProvider>
      </BackendProvider>
    </ForgeStoreProvider>,
  );
}

describe('CommandCenterWorkspace', () => {
  it('renders the SZM Forge command center overview', () => {
    renderWithProviders(<CommandCenterWorkspace />);

    expect(screen.getByText('Engineering Command Center')).toBeInTheDocument();
    expect(screen.getByText('SZM AI memory')).toBeInTheDocument();
    expect(screen.getByText('Launch critical workflows')).toBeInTheDocument();
  });

  it('reports honest health: no model loaded and offline backend bridge', () => {
    renderWithProviders(<CommandCenterWorkspace />);

    // With nothing loaded, the app must say NO MODEL — never a fake readiness %.
    expect(screen.getByText('NO MODEL')).toBeInTheDocument();
    // Default test env has VITE_ENABLE_BACKEND unset → offline-only mode.
    expect(screen.getByText('OFFLINE MODE')).toBeInTheDocument();
    // The local engine must never claim remote connectivity.
    expect(screen.queryByText('CONNECTED')).not.toBeInTheDocument();
  });
});
