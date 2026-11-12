// @vitest-environment jsdom
import { render, screen, cleanup } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, describe, expect, it } from 'vitest';
import { CommandCenterWorkspace } from './CommandCenterWorkspace';
import { ForgeStoreProvider } from '../store/ForgeStoreContext';

afterEach(() => {
  cleanup();
});

describe('CommandCenterWorkspace', () => {
  it('renders the SZM Forge command center overview', () => {
    render(
      <ForgeStoreProvider>
        <CommandCenterWorkspace />
      </ForgeStoreProvider>
    );

    expect(screen.getByText('Engineering Command Center')).toBeInTheDocument();
    expect(screen.getByText('SZM AI memory')).toBeInTheDocument();
    expect(screen.getByText('Launch critical workflows')).toBeInTheDocument();
  });
});
