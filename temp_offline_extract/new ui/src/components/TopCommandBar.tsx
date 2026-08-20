import React from 'react';
import { WorkspaceMode, ViewportRenderMode } from '../types';
import { useTooltips } from '../store/TooltipContext';
import { tooltips } from '../data/tooltips';
import {
  Cpu, Layers, Box, Factory, Brain, Radio,
  Play, Pause, RotateCcw, Settings, Maximize2,
  Eye, Thermometer, Zap, Activity, Grid3x3,
  ChevronDown, Hexagon, Shield, Bell, PenTool
} from 'lucide-react';

interface Props {
  workspace: WorkspaceMode;
  setWorkspace: (m: WorkspaceMode) => void;
  renderMode: ViewportRenderMode;
  setRenderMode: (m: ViewportRenderMode) => void;
  solverProgress: number;
  simulationRunning: boolean;
  onOpenNotifications?: () => void;
  onOpenSettings?: () => void;
  onRunSimulation?: () => void;
  onPauseSimulation?: () => void;
  onResumeSimulation?: () => void;
  onResetSimulation?: () => void;
  onToggleAIPanel?: () => void;
  showAIPanel?: boolean;
  simulationPaused?: boolean;
  notificationCount?: number;
}

const workspaceTabs: { id: WorkspaceMode; label: string; icon: React.ReactNode; tipKey: keyof typeof tooltips.workspace }[] = [
  { id: 'engineering', label: 'ENGINEERING', icon: <Box size={13} />, tipKey: 'engineering' },
  { id: 'designing', label: 'DESIGNING', icon: <PenTool size={13} />, tipKey: 'designing' },
  { id: 'simulation', label: 'SIMULATION', icon: <Cpu size={13} />, tipKey: 'simulation' },
  { id: 'physics', label: 'REAL PHYSICS', icon: <Zap size={13} />, tipKey: 'simulation' },
  { id: 'digital-twin', label: 'DIGITAL TWIN', icon: <Layers size={13} />, tipKey: 'digitalTwin' },
  { id: 'manufacturing', label: 'MANUFACTURING', icon: <Factory size={13} />, tipKey: 'manufacturing' },
  { id: 'ai-assistant', label: 'AI ENGINE', icon: <Brain size={13} />, tipKey: 'aiAssistant' },
  { id: 'fleet-command', label: 'FLEET CMD', icon: <Radio size={13} />, tipKey: 'fleetCommand' },
];

const renderModes: { id: ViewportRenderMode; label: string; icon: React.ReactNode; tipKey: keyof typeof tooltips.viewport }[] = [
  { id: 'shaded', label: 'Shaded', icon: <Eye size={12} />, tipKey: 'shaded' },
  { id: 'wireframe', label: 'Wire', icon: <Grid3x3 size={12} />, tipKey: 'wireframe' },
  { id: 'xray', label: 'X-Ray', icon: <Maximize2 size={12} />, tipKey: 'xray' },
  { id: 'thermal', label: 'Thermal', icon: <Thermometer size={12} />, tipKey: 'thermal' },
  { id: 'stress', label: 'Stress', icon: <Activity size={12} />, tipKey: 'stress' },
  { id: 'electrical', label: 'Electric', icon: <Zap size={12} />, tipKey: 'electrical' },
];

export const TopCommandBar: React.FC<Props> = ({
  workspace, setWorkspace, renderMode, setRenderMode, solverProgress, simulationRunning,
  onOpenNotifications,
  onOpenSettings,
  onRunSimulation,
  onPauseSimulation,
  onResumeSimulation,
  onResetSimulation,
  onToggleAIPanel,
  showAIPanel = true,
  simulationPaused = false,
  notificationCount = 0,
}) => {
  const { t } = useTooltips();

  return (
    <div className="flex flex-col border-b border-forge-border/30 bg-forge-dark/80 backdrop-blur-xl relative z-50 shadow-lg">
      
      {/* Top row - App Logo and Global Status */}
      <div className="h-[40px] flex items-center px-4 border-b border-forge-border/50">
        <div className="flex items-center gap-2 mr-6">
          <div className="relative">
            <Hexagon size={22} className="text-forge-accent" strokeWidth={2.5} />
            <div className="absolute inset-0 flex items-center justify-center">
              <div className="w-1.5 h-1.5 bg-forge-accent rounded-full" />
            </div>
          </div>
          <div className="flex flex-col">
            <span className="text-xs font-bold tracking-[0.25em] text-forge-accent text-glow-accent leading-none">
              SZM FORGE
            </span>
            <span className="text-[10px] tracking-[0.15em] text-forge-text-muted font-mono leading-none mt-1">
              INDUSTRIAL INTELLIGENCE
            </span>
          </div>
        </div>

        <div className="flex items-center gap-6 ml-auto">
          {/* Status indicators */}
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse" />
            <span className="text-[10px] font-mono text-forge-text-dim">SOLVER ONLINE</span>
          </div>
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-accent" />
            <span className="text-[10px] font-mono text-forge-text-dim">GPU 94%</span>
          </div>
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green" />
            <span className="text-[10px] font-mono text-forge-text-dim">32 CORES</span>
          </div>
          <div className="flex items-center gap-1.5">
            <Shield size={12} className="text-forge-green" />
            <span className="text-[10px] font-mono text-forge-text-dim">SYNC</span>
          </div>
        </div>
      </div>

      {/* Bottom row - Workspace tabs + Controls */}
      <div className="h-[48px] flex items-center px-2">
        {/* Workspace Tabs */}
        <div className="flex items-center gap-0.5">
          {workspaceTabs.map(tab => (
            <button
              key={tab.id}
              onClick={() => setWorkspace(tab.id)}
              title={t(tooltips.workspace[tab.tipKey])}
              aria-label={t(tooltips.workspace[tab.tipKey])}
              className={`
                flex items-center gap-1.5 px-4 py-2 rounded-md text-[11px] font-semibold tracking-wider
                transition-all duration-300 relative
                ${workspace === tab.id
                  ? 'bg-forge-accent/15 text-forge-accent border border-forge-accent/40 shadow-[0_0_15px_rgba(0,212,255,0.15)]'
                  : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
                }
              `}
            >
              {tab.icon}
              {tab.label}
              {workspace === tab.id && (
                <div className="absolute bottom-0 left-1/4 right-1/4 h-[1px] bg-forge-accent" />
              )}
            </button>
          ))}
        </div>

        <div className="w-px h-6 bg-forge-border mx-3" />

        {/* Render Mode */}
        <div className="flex items-center gap-0.5">
          {renderModes.map(mode => (
            <button
              key={mode.id}
              onClick={() => setRenderMode(mode.id)}
              title={t(tooltips.viewport[mode.tipKey])}
              aria-label={t(tooltips.viewport[mode.tipKey])}
              className={`
                flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono tracking-wider
                transition-all duration-200
                ${renderMode === mode.id
                  ? 'bg-forge-purple/15 text-forge-purple border border-forge-purple/30'
                  : 'text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface border border-transparent'
                }
              `}
            >
              {mode.icon}
              {mode.label}
            </button>
          ))}
        </div>

        <div className="w-px h-6 bg-forge-border mx-3" />

        {/* Simulation Controls */}
        <div className="flex items-center gap-1">
          <button
            title={t(tooltips.simulation.run)}
            aria-label={t(tooltips.simulation.run)}
            onClick={onRunSimulation}
            className="flex items-center gap-1 px-2.5 py-1 rounded text-[9px] font-mono tracking-wider bg-forge-green/10 text-forge-green border border-forge-green/20 hover:bg-forge-green/20 transition-all"
          >
            <Play size={10} />
            RUN
          </button>
          <button
            type="button"
            title={simulationPaused ? 'Resume' : t(tooltips.simulation.pause)}
            aria-label={simulationPaused ? 'Resume' : t(tooltips.simulation.pause)}
            onClick={simulationPaused ? onResumeSimulation : onPauseSimulation}
            className="flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono tracking-wider text-forge-text-muted hover:text-forge-yellow hover:bg-forge-yellow/10 transition-all"
          >
            <Pause size={10} />
          </button>
          <button
            type="button"
            title={t(tooltips.simulation.reset)}
            aria-label={t(tooltips.simulation.reset)}
            onClick={onResetSimulation}
            className="flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono tracking-wider text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface transition-all"
          >
            <RotateCcw size={10} />
          </button>
        </div>

        {/* Solver Progress */}
        {simulationRunning && (
          <div className="flex items-center gap-2 ml-3">
            <div className="w-24 h-1.5 bg-forge-surface rounded-full overflow-hidden">
              <div
                className="h-full bg-gradient-to-r from-forge-accent to-forge-cyan rounded-full transition-all duration-500"
                style={{ width: `${solverProgress}%` }}
              />
            </div>
            <span className="text-[9px] font-mono text-forge-accent">{solverProgress}%</span>
          </div>
        )}

        {/* Right side controls */}
        <div className="ml-auto flex items-center gap-2">
          <button 
            onClick={onOpenNotifications}
            title="View notifications"
            aria-label="Notifications"
            className="relative p-1.5 rounded text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface transition-all"
          >
            <Bell size={13} />
            {notificationCount > 0 && (
              <span className="absolute -top-0.5 -right-0.5 w-3.5 h-3.5 rounded-full bg-forge-red text-[7px] font-bold flex items-center justify-center text-white">
                {notificationCount > 9 ? '9+' : notificationCount}
              </span>
            )}
          </button>
          <button
            type="button"
            onClick={onToggleAIPanel}
            title={showAIPanel ? 'Hide analysis panel' : 'Show analysis panel'}
            className={`px-2 py-1 rounded text-[9px] font-mono border transition-all ${
              showAIPanel ? 'border-forge-accent/40 text-forge-accent' : 'border-transparent text-forge-text-muted'
            }`}
          >
            PANEL
          </button>
          <button
            type="button"
            onClick={onOpenSettings}
            title="Open settings & preferences"
            aria-label="Settings"
            className="flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface transition-all"
          >
            <Settings size={11} />
            <ChevronDown size={8} />
          </button>
        </div>
      </div>
    </div>
  );
};
