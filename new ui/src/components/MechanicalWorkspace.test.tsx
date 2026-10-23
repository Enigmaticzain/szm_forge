// @vitest-environment jsdom
import React from 'react';
import { render, screen, cleanup } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { describe, expect, it, beforeEach, afterEach } from 'vitest';

afterEach(() => {
  cleanup();
});
import { MechanicalWorkspace } from './MechanicalWorkspace';
import { ForgeStoreProvider } from '../store/ForgeStoreContext';

beforeEach(() => {
  window.matchMedia = window.matchMedia || function() {
    return {
      matches: false,
      addListener: function() {},
      removeListener: function() {},
      addEventListener: function() {},
      removeEventListener: function() {},
      dispatchEvent: function() { return false; },
    };
  };
});

describe('MechanicalWorkspace Integration', () => {
  it('renders the interactive isometric mechanical assembly viewer', () => {
    render(
      <ForgeStoreProvider>
        <MechanicalWorkspace />
      </ForgeStoreProvider>
    );

    expect(screen.getByText('DROP IN COMPONENT')).toBeInTheDocument();
    expect(screen.getAllByText(/COMPOUND GEAR TRAIN/i).length).toBeGreaterThan(0);
    expect(screen.getByText(/Fig 1/i)).toBeInTheDocument();
  });
});
