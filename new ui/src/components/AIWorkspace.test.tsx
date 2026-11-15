// @vitest-environment jsdom
import React from 'react';
import { render, screen, cleanup, within } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, beforeAll, describe, expect, it } from 'vitest';
import { AIWorkspace } from './AIWorkspace';
import { ForgeStoreProvider } from '../store/ForgeStoreContext';
import { BackendProvider } from '../store/BackendContext';
import { ProjectProvider } from '../store/ProjectContext';

beforeAll(() => {
  // jsdom does not implement scrollIntoView; the chat panel calls it on mount.
  Element.prototype.scrollIntoView = function () {};
});

afterEach(() => {
  cleanup();
});

function renderWithProviders(ui: React.ReactElement) {
  return render(
    <ForgeStoreProvider>
      <BackendProvider>
        <ProjectProvider>{ui}</ProjectProvider>
      </BackendProvider>
    </ForgeStoreProvider>,
  );
}

describe('AIWorkspace truthfulness', () => {
  it('never presents sample turbine metrics as live when no model is loaded', () => {
    renderWithProviders(<AIWorkspace />);

    // No fabricated model metrics may leak into the header.
    expect(screen.getByText('NONE')).toBeInTheDocument();
    expect(screen.getAllByText('OFFLINE').length).toBeGreaterThan(0);

    // Sample insights stay hidden behind an explicit, labeled toggle.
    expect(screen.queryByText('Thermal Stress Concentration Detected')).not.toBeInTheDocument();
    expect(screen.getByText('PREVIEW SAMPLE INSIGHTS (DEMO)')).toBeInTheDocument();

    // Fabricated optimization numbers (847 MPa etc.) are gone.
    expect(screen.queryByText('847 MPa')).not.toBeInTheDocument();
    expect(screen.queryByText('$47.2K')).not.toBeInTheDocument();

    // Honest empty state for optimization targets.
    expect(
      screen.getByText(/No solved parts yet/i),
    ).toBeInTheDocument();
  });

  it('shows sample insights only behind the demo toggle, marked SAMPLE', async () => {
    renderWithProviders(<AIWorkspace />);

    const toggle = screen.getByText('PREVIEW SAMPLE INSIGHTS (DEMO)');
    toggle.click();

    const demoBanner = await screen.findByText(/DEMO DATA/i);
    expect(demoBanner).toBeInTheDocument();
    expect(screen.getByText('Thermal Stress Concentration Detected')).toBeInTheDocument();

    // Every rendered sample insight carries a SAMPLE badge.
    const insightsCard = demoBanner.parentElement as HTMLElement;
    expect(within(insightsCard).getAllByText('SAMPLE').length).toBeGreaterThan(0);
  });
});
