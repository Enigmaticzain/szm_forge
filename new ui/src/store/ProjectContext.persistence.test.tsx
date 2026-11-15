// @vitest-environment jsdom
import React from 'react';
import { render, screen, cleanup, act } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, beforeEach, describe, expect, it } from 'vitest';
import { ProjectProvider, useProject } from './ProjectContext';
import { persistProject } from '../offline/projectPersistence';
import type { SceneObject } from '../components/BodyDesign/InteractivePartEditor';

const extrudedPart: SceneObject = {
  id: 'obj-ext',
  name: 'Sketch Extrusion 1',
  type: 'custom',
  position: { x: 0, y: 0.05, z: 0 },
  size: { x: 0.1, y: 0.1, z: 0.01 },
  materialId: 'm1',
};

const Probe: React.FC = () => {
  const { bodyParts, persistStatus, newProject } = useProject();
  return (
    <div>
      <div data-testid="parts">{bodyParts.map(p => p.name).join(',') || 'EMPTY'}</div>
      <div data-testid="restored">{persistStatus.restored ? 'RESTORED' : 'FRESH'}</div>
      <button onClick={() => void newProject('Wipe Test')}>NEW</button>
    </div>
  );
};

beforeEach(() => localStorage.clear());
afterEach(() => {
  cleanup();
  localStorage.clear();
});

describe('ProjectContext persistence', () => {
  it('restores saved session parts on mount (no more lost work on reload)', () => {
    persistProject({
      project: {
        projectName: 'Reload Study',
        hasFurniture: false,
        furnitureType: 'table',
        materialId: 'MAT',
        material: 'Steel',
        loadKg: 100,
        aiImproved: false,
      },
      members: [],
      bodyParts: [extrudedPart],
      bodyJoints: [],
      parts: [],
      customMaterials: [],
    });

    render(
      <ProjectProvider>
        <Probe />
      </ProjectProvider>,
    );

    expect(screen.getByTestId('parts').textContent).toContain('Sketch Extrusion 1');
    expect(screen.getByTestId('restored').textContent).toBe('RESTORED');
  });

  it('starting a NEW project wipes the stored snapshot instead of resurrecting it', async () => {
    persistProject({
      project: null, members: [], bodyParts: [extrudedPart], bodyJoints: [], parts: [], customMaterials: [],
    });

    render(
      <ProjectProvider>
        <Probe />
      </ProjectProvider>,
    );
    expect(screen.getByTestId('parts').textContent).toContain('Sketch Extrusion 1');

    await act(async () => {
      screen.getByText('NEW').click();
    });

    // After reset, the extruded part is gone from state…
    expect(screen.getByTestId('parts').textContent).toBe('EMPTY');
    // …and any stored snapshot belongs to the NEW empty project, not the old one.
    const raw = localStorage.getItem('szm_forge_project_v1');
    if (raw) {
      expect(raw).not.toContain('Sketch Extrusion 1');
    }
  });
});
