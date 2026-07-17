import { useState, useCallback } from 'react';
import { WorkspaceMode, ViewportRenderMode, SimulationStatus } from '../types';

export interface ForgeState {
  workspace: WorkspaceMode;
  setWorkspace: (mode: WorkspaceMode) => void;
  renderMode: ViewportRenderMode;
  setRenderMode: (mode: ViewportRenderMode) => void;
  simulationStatus: SimulationStatus;
  setSimulationStatus: (status: SimulationStatus) => void;
  selectedNode: string | null;
  setSelectedNode: (id: string | null) => void;
  showAIPanel: boolean;
  toggleAIPanel: () => void;
  showTimeline: boolean;
  toggleTimeline: () => void;
  leftPanelWidth: number;
  rightPanelWidth: number;
  solverProgress: number;
}

export function useForgeStore(): ForgeState {
  const [workspace, setWorkspace] = useState<WorkspaceMode>('engineering');
  const [renderMode, setRenderMode] = useState<ViewportRenderMode>('shaded');
  const [simulationStatus, setSimulationStatus] = useState<SimulationStatus>('complete');
  const [selectedNode, setSelectedNode] = useState<string | null>('prt-1');
  const [showAIPanel, setShowAIPanel] = useState(true);
  const [showTimeline, setShowTimeline] = useState(true);
  const [solverProgress] = useState(87);

  const toggleAIPanel = useCallback(() => setShowAIPanel(v => !v), []);
  const toggleTimeline = useCallback(() => setShowTimeline(v => !v), []);

  return {
    workspace, setWorkspace,
    renderMode, setRenderMode,
    simulationStatus, setSimulationStatus,
    selectedNode, setSelectedNode,
    showAIPanel, toggleAIPanel,
    showTimeline, toggleTimeline,
    leftPanelWidth: 280,
    rightPanelWidth: 320,
    solverProgress,
  };
}
