// @vitest-environment jsdom
import { fireEvent, render, screen, cleanup, waitFor } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { TestingLabWorkspace } from './TestingLabWorkspace';

const mocks = vi.hoisted(() => ({
  bodyParts: [] as Array<{
    id: string;
    name: string;
    materialId?: string;
    type: 'box';
    position: { x: number; y: number; z: number };
    size: { x: number; y: number; z: number };
  }>,
  showToast: vi.fn(),
  setImportDialogOpen: vi.fn(),
  runFea: vi.fn(),
  runThermal: vi.fn(),
}));

vi.mock('../store/ForgeStoreContext', () => ({
  useForgeStore: () => ({
    showToast: mocks.showToast,
    setImportDialogOpen: mocks.setImportDialogOpen,
  }),
}));

vi.mock('../store/ProjectContext', () => ({
  useProject: () => ({ bodyParts: mocks.bodyParts }),
}));

vi.mock('../services/forgeApi', () => ({
  runFea: mocks.runFea,
  runThermal: mocks.runThermal,
}));

afterEach(cleanup);

beforeEach(() => {
  mocks.bodyParts = [];
  mocks.showToast.mockReset();
  mocks.setImportDialogOpen.mockReset();
  mocks.runFea.mockReset();
  mocks.runThermal.mockReset();
});

describe('TestingLabWorkspace', () => {
  it('requires a loaded component before enabling stress or thermal tests', () => {
    render(<TestingLabWorkspace />);

    expect(screen.getByRole('button', { name: 'RUN STRESS' })).toBeDisabled();
    expect(screen.getByRole('button', { name: 'RUN THERMAL' })).toBeDisabled();
    expect(screen.getByText(/Load and select a component before testing/)).toBeInTheDocument();
    expect(mocks.runFea).not.toHaveBeenCalled();
    expect(mocks.runThermal).not.toHaveBeenCalled();
  });

  it('reports solver failures instead of substituting fabricated offline results', async () => {
    mocks.bodyParts = [{
      id: 'plate-1',
      name: 'Plate',
      materialId: 'al-6061',
      type: 'box',
      position: { x: 0, y: 0, z: 0 },
      size: { x: 100, y: 50, z: 5 },
    }];
    mocks.runFea.mockRejectedValue(new Error('FEA solver unavailable'));

    render(<TestingLabWorkspace />);
    fireEvent.change(screen.getByLabelText('Cross-section area (m²)'), { target: { value: '0.0005' } });
    fireEvent.change(screen.getByLabelText('Thickness (m)'), { target: { value: '0.005' } });
    fireEvent.click(screen.getByRole('button', { name: 'RUN STRESS' }));

    await waitFor(() => {
      expect(screen.getByText(/Stress test failed · Plate · FEA solver unavailable/)).toBeInTheDocument();
    });
    expect(mocks.runFea).toHaveBeenCalledWith(1, 'al-6061', 1000, 0.0005, 0.005);
    expect(screen.queryByText(/σ≈/)).not.toBeInTheDocument();
    expect(mocks.showToast).toHaveBeenCalledWith(expect.stringContaining('Stress test failed'));
  });
});
