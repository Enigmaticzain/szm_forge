import React, { createContext, useCallback, useContext, useState } from 'react';
import { WorkspaceMode, ViewportRenderMode, SimulationStatus } from '../types';

export interface ForgeState {
  workspace: WorkspaceMode;
  setWorkspace: (mode: WorkspaceMode) => void;
  renderMode: ViewportRenderMode;
  setRenderMode: (mode: ViewportRenderMode) => void;
  simulationStatus: SimulationStatus;
  setSimulationStatus: (status: SimulationStatus) => void;
  simulationPaused: boolean;
  pauseSimulation: () => void;
  resumeSimulation: () => void;
  resetSimulation: () => void;
  selectedNode: string | null;
  setSelectedNode: (id: string | null) => void;
  showAIPanel: boolean;
  toggleAIPanel: () => void;
  showTimeline: boolean;
  toggleTimeline: () => void;
  leftPanelWidth: number;
  setLeftPanelWidth: React.Dispatch<React.SetStateAction<number>>;
  rightPanelWidth: number;
  setRightPanelWidth: React.Dispatch<React.SetStateAction<number>>;
  timelineHeight: number;
  setTimelineHeight: React.Dispatch<React.SetStateAction<number>>;
  solverProgress: number;
  setSolverProgress: (n: number) => void;
  viewportZoom: number;
  setViewportZoom: React.Dispatch<React.SetStateAction<number>>;
  showViewportGrid: boolean;
  toggleViewportGrid: () => void;
  toast: string | null;
  showToast: (msg: string) => void;
  activeTheme: 'theme1' | 'theme2';
  setTheme: (theme: 'theme1' | 'theme2') => void;
}

const ForgeStoreContext = createContext<ForgeState | null>(null);

export const ForgeStoreProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [workspace, setWorkspace] = useState<WorkspaceMode>('engineering');
  const [renderMode, setRenderMode] = useState<ViewportRenderMode>('shaded');
  const [simulationStatus, setSimulationStatus] = useState<SimulationStatus>('complete');
  const [simulationPaused, setSimulationPaused] = useState(false);
  const [selectedNode, setSelectedNode] = useState<string | null>('prt-1');
  const [showAIPanel, setShowAIPanel] = useState(true);
  const [showTimeline, setShowTimeline] = useState(true);
  const [leftPanelWidth, setLeftPanelWidth] = useState(280);
  const [rightPanelWidth, setRightPanelWidth] = useState(320);
  const [timelineHeight, setTimelineHeight] = useState(130);
  const [solverProgress, setSolverProgress] = useState(87);
  const [viewportZoom, setViewportZoom] = useState(1);
  const [showViewportGrid, setShowViewportGrid] = useState(true);
  const [toast, setToast] = useState<string | null>(null);
  const [activeTheme, setActiveThemeState] = useState<'theme1' | 'theme2'>(() => {
    return (localStorage.getItem('szm-theme') as 'theme1' | 'theme2') || 'theme2';
  });

  const showToast = useCallback((msg: string) => {
    setToast(msg);
    setTimeout(() => setToast(null), 2800);
  }, []);

  const setTheme = useCallback((theme: 'theme1' | 'theme2') => {
    localStorage.setItem('szm-theme', theme);
    setActiveThemeState(theme);
  }, []);

  const pauseSimulation = useCallback(() => {
    setSimulationPaused(true);
    setSimulationStatus('paused');
    showToast('Simulation paused');
  }, [showToast]);

  const resumeSimulation = useCallback(() => {
    setSimulationPaused(false);
    setSimulationStatus('solving');
    showToast('Simulation resumed');
  }, [showToast]);

  const resetSimulation = useCallback(() => {
    setSimulationPaused(false);
    setSimulationStatus('idle');
    setSolverProgress(0);
    showToast('Simulation reset — press RUN to solve');
  }, [showToast]);

  const toggleAIPanel = useCallback(() => setShowAIPanel(v => !v), []);
  const toggleTimeline = useCallback(() => setShowTimeline(v => !v), []);
  const toggleViewportGrid = useCallback(() => setShowViewportGrid(v => !v), []);

  const value: ForgeState = {
    workspace,
    setWorkspace,
    renderMode,
    setRenderMode,
    simulationStatus,
    setSimulationStatus,
    simulationPaused,
    pauseSimulation,
    resumeSimulation,
    resetSimulation,
    selectedNode,
    setSelectedNode,
    showAIPanel,
    toggleAIPanel,
    showTimeline,
    toggleTimeline,
    leftPanelWidth,
    setLeftPanelWidth,
    rightPanelWidth,
    setRightPanelWidth,
    timelineHeight,
    setTimelineHeight,
    solverProgress,
    setSolverProgress,
    viewportZoom,
    setViewportZoom,
    showViewportGrid,
    toggleViewportGrid,
    toast,
    showToast,
    activeTheme,
    setTheme,
  };

  return <ForgeStoreContext.Provider value={value}>{children}</ForgeStoreContext.Provider>;
};

export function useForgeStore(): ForgeState {
  const ctx = useContext(ForgeStoreContext);
  if (!ctx) throw new Error('useForgeStore must be used within ForgeStoreProvider');
  return ctx;
}

/** @deprecated import useForgeStore from ForgeStoreContext */
export { useForgeStore as useForgeStoreHook };
