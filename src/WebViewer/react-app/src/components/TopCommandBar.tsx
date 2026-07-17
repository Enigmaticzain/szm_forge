import React from 'react';
import { WorkspaceMode, ViewportRenderMode } from '../types';
import {
  Cpu, Layers, Box, Factory, Brain, Radio,
  Play, Pause, RotateCcw, Settings, Maximize2,
  Eye, Thermometer, Zap, Activity, Grid3x3,
  ChevronDown, Hexagon, Shield, Bell
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
  notificationCount?: number;
}

const workspaceTabs: { id: WorkspaceMode; label: string; icon: React.ReactNode }[] = [
  { id: 'engineering', label: 'ENGINEERING', icon: <Box size={13} /> },
  { id: 'simulation', label: 'SIMULATION', icon: <Cpu size={13} /> },
  { id: 'digital-twin', label: 'DIGITAL TWIN', icon: <Layers size={13} /> },
  { id: 'manufacturing', label: 'MANUFACTURING', icon: <Factory size={13} /> },
  { id: 'ai-assistant', label: 'AI ENGINE', icon: <Brain size={13} /> },
  { id: 'fleet-command', label: 'FLEET CMD', icon: <Radio size={13} /> },
];

const renderModes: { id: ViewportRenderMode; label: string; icon: React.ReactNode }[] = [
  { id: 'shaded', label: 'Shaded', icon: <Eye size={12} /> },
  { id: 'wireframe', label: 'Wire', icon: <Grid3x3 size={12} /> },
  { id: 'xray', label: 'X-Ray', icon: <Maximize2 size={12} /> },
  { id: 'thermal', label: 'Thermal', icon: <Thermometer size={12} /> },
  { id: 'stress', label: 'Stress', icon: <Activity size={12} /> },
  { id: 'electrical', label: 'Electric', icon: <Zap size={12} /> },
];

export const TopCommandBar: React.FC<Props> = ({
  workspace, setWorkspace, renderMode, setRenderMode, solverProgress, simulationRunning,
  onOpenNotifications, onOpenSettings, notificationCount = 0
}) => {
  return (
    <div className="h-[88px] flex flex-col border-b border-forge-border bg-forge-dark relative z-50">
      {/* Top row - Brand + System Status */}
      <div className="h-[40px] flex items-center px-3 border-b border-forge-border/50">
        {/* Logo */}
        <div className="flex items-center gap-2 mr-6">
          <div className="relative">
            <Hexagon size={22} className="text-forge-accent" strokeWidth={2.5} />
            <div className="absolute inset-0 flex items-center justify-center">
              <div className="w-1.5 h-1.5 bg-forge-accent rounded-full" />
            </div>
          </div>
          <div className="flex flex-col">
            <span className="text-[11px] font-bold tracking-[0.25em] text-forge-accent text-glow-accent leading-none">
              SZM FORGE
            </span>
            <span className="text-[8px] tracking-[0.15em] text-forge-text-muted font-mono leading-none mt-0.5">
              INDUSTRIAL INTELLIGENCE
            </span>
          </div>
        </div>

        {/* System Status Indicators */}
        <div className="flex items-center gap-4 ml-auto">
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse" />
            <span className="text-[9px] font-mono text-forge-text-dim">SOLVER ONLINE</span>
          </div>
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-accent" />
            <span className="text-[9px] font-mono text-forge-text-dim">GPU 94%</span>
          </div>
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green" />
            <span className="text-[9px] font-mono text-forge-text-dim">32 CORES</span>
          </div>
          <div className="flex items-center gap-1.5">
            <Shield size={10} className="text-forge-green" />
            <span className="text-[9px] font-mono text-forge-text-dim">SYNC</span>
          </div>
          <div className="text-[9px] font-mono text-forge-text-muted">
            MEM 12.4 / 64 GB
          </div>
          <div className="text-[9px] font-mono text-forge-text-muted">
            v4.2.1-alpha
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
              className={`
                flex items-center gap-1.5 px-3 py-1.5 rounded text-[10px] font-semibold tracking-wider
                transition-all duration-200 relative
                ${workspace === tab.id
                  ? 'bg-forge-accent/10 text-forge-accent border border-forge-accent/30'
                  : 'text-forge-text-dim hover:text-forge-text hover:bg-forge-surface border border-transparent'
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
          <button className="flex items-center gap-1 px-2.5 py-1 rounded text-[9px] font-mono tracking-wider bg-forge-green/10 text-forge-green border border-forge-green/20 hover:bg-forge-green/20 transition-all">
            <Play size={10} />
            RUN
          </button>
          <button className="flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono tracking-wider text-forge-text-muted hover:text-forge-yellow hover:bg-forge-yellow/10 transition-all">
            <Pause size={10} />
          </button>
          <button className="flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono tracking-wider text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface transition-all">
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
            onClick={onOpenSettings}
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
