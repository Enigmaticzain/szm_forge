// @vitest-environment jsdom
import React from 'react';
import { render, screen, fireEvent, cleanup } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { describe, expect, it, beforeEach, afterEach } from 'vitest';
import { IsometricGearViewer } from './IsometricGearViewer';
import { DEFAULT_GEAR_TRAIN } from './gearTrainModel';

afterEach(() => {
  cleanup();
});

// Mock matchMedia for JSDOM
let mockMatches = false;
beforeEach(() => {
  mockMatches = false;
  window.matchMedia = window.matchMedia || function() {
    return {
      matches: mockMatches,
      addListener: function() {},
      removeListener: function() {},
      addEventListener: function() {},
      removeEventListener: function() {},
      dispatchEvent: function() { return false; },
    };
  };
});

describe('IsometricGearViewer Component', () => {
  it('renders the isometric gear train assembly view with fig label and title', () => {
    render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);

    expect(screen.getAllByText(/COMPOUND GEAR TRAIN/i).length).toBeGreaterThan(0);
    expect(screen.getAllByText(/6:1 REDUCTION/i).length).toBeGreaterThan(0);
    expect(screen.getByText(/Fig 1/i)).toBeInTheDocument();
    expect(screen.getByText('Compound Gear Train')).toBeInTheDocument();
  });

  it('renders all 5 gear labels in the SVG figure', () => {
    render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);

    // Check all unique labels G1..G5
    for (const gear of DEFAULT_GEAR_TRAIN.gears) {
      expect(screen.getByText(gear.label)).toBeInTheDocument();
    }
    // Check tooth counts: G3 and G5 both have 16T
    expect(screen.getAllByText('16T').length).toBe(2);
    expect(screen.getByText('20T')).toBeInTheDocument();
    expect(screen.getByText('40T')).toBeInTheDocument();
    expect(screen.getByText('48T')).toBeInTheDocument();
  });

  it('allows selecting a gear to inspect mechanical properties including tier', () => {
    render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);

    // Click on Gear 2 (the compound junction gear)
    const g2Button = screen.getByRole('button', { name: /Gear G2/i });
    expect(g2Button).toBeInTheDocument();
    fireEvent.click(g2Button);

    // Inspector overlay should appear showing teeth, radius, tier, and compound note
    expect(screen.getByText(/Pitch Radius/i)).toBeInTheDocument();
    expect(screen.getByText(/80 mm/i)).toBeInTheDocument();
    expect(screen.getByText(/Tier #1/i)).toBeInTheDocument();
    expect(screen.getByText(/Compound junction gear/i)).toBeInTheDocument();
  });

  it('allows keyboard selection with Enter and Space on gear buttons', () => {
    render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);

    const g1Button = screen.getByRole('button', { name: /Gear G1/i });
    fireEvent.keyDown(g1Button, { key: 'Enter' });
    expect(screen.getByText(/40 mm/i)).toBeInTheDocument();

    // Toggle off with Space
    fireEvent.keyDown(g1Button, { key: ' ' });
    expect(screen.queryByText(/40 mm/i)).not.toBeInTheDocument();
  });

  it('toggles pause and play state on click', () => {
    render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);

    const pauseBtn = screen.getByTitle(/Pause/i);
    expect(pauseBtn).toBeInTheDocument();

    fireEvent.click(pauseBtn);
    expect(screen.getByTitle(/Play/i)).toBeInTheDocument();
    expect(screen.getByText(/PAUSED/i)).toBeInTheDocument();
  });

  it('toggles explode view mode and renders shaft axis guides', () => {
    const { container } = render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);

    const explodeBtn = screen.getByTitle(/Explode assembly/i);
    expect(explodeBtn).toBeInTheDocument();

    fireEvent.click(explodeBtn);
    expect(screen.getByTitle(/Collapse assembly/i)).toBeInTheDocument();

    // Axis centerlines are drawn in exploded mode
    const axisLines = container.querySelectorAll('line[stroke-dasharray="4 3"]');
    expect(axisLines.length).toBeGreaterThanOrEqual(3);
  });

  it('renders machine bed base plate with machined bearing mounts', () => {
    const { container } = render(<IsometricGearViewer model={DEFAULT_GEAR_TRAIN} />);
    const machineBed = container.querySelector('g[aria-label="Machine bed"]');
    expect(machineBed).toBeInTheDocument();

    const bearingMounts = container.querySelectorAll('g[aria-label*="bearing mount"]');
    expect(bearingMounts.length).toBe(3);
  });
});
