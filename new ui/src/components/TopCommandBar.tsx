import React, { useState, useRef, useEffect } from 'react';
import { WorkspaceMode, ViewportRenderMode } from '../types';
import { useTooltips } from '../store/TooltipContext';
import { tooltips } from '../data/tooltips';
import { useForgeStore } from '../store/ForgeStoreContext';
import {
  Cpu, Layers, Box, Factory, Brain, Radio,
  Play, Pause, RotateCcw, Settings, Maximize2,
  Eye, Thermometer, Zap, Activity, Grid3x3,
  ChevronDown, Hexagon, Shield, Bell, PenTool, Microscope, CircuitBoard, LayoutGrid, Download, Aperture, BookMarked, GitMerge, Wand2,
  Component, Combine, TestTube, Lightbulb, Import
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
  onOpenWorkspaces?: () => void;
  onRunSimulation?: () => void;
  onPauseSimulation?: () => void;
  onResumeSimulation?: () => void;
  onResetSimulation?: () => void;
  onToggleAIPanel?: () => void;
  showAIPanel?: boolean;
  simulationPaused?: boolean;
  notificationCount?: number;
}

const DropdownMenu = ({ title, icon, items, currentWorkspace, setWorkspace }: any) => {
  const [isOpen, setIsOpen] = useState(false);
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (ref.current && !ref.current.contains(event.target as Node)) {
        setIsOpen(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const isActive = items.some((item: any) => item.id === currentWorkspace);

  return (
    <div className="relative" ref={ref}>
      <button
        onClick={() => setIsOpen(!isOpen)}
        className={`flex items-center gap-1.5 px-3 py-1.5 rounded-md text-[11px] font-semibold tracking-wider transition-all duration-300 ${
          isActive || isOpen
            ? 'bg-forge-accent/15 text-forge-accent border border-forge-accent/40 shadow-[0_0_10px_rgba(0,212,255,0.1)]'
            : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
        }`}
      >
        {icon}
        {title}
        <ChevronDown size={10} className={`transition-transform duration-200 ${isOpen ? 'rotate-180' : ''}`} />
      </button>

      {isOpen && (
        <div className="absolute top-full left-0 mt-1 w-48 bg-forge-dark/95 backdrop-blur-xl border border-forge-border/40 rounded-lg shadow-2xl py-1 z-50">
          {items.map((item: any) => (
            <button
              key={item.id}
              onClick={() => {
                setWorkspace(item.id);
                setIsOpen(false);
              }}
              className={`w-full flex items-center gap-2 px-3 py-2 text-[11px] font-mono tracking-wider transition-colors ${
                currentWorkspace === item.id
                  ? 'bg-forge-accent/20 text-forge-accent border-l-2 border-forge-accent'
                  : 'text-forge-text hover:bg-forge-surface hover:text-forge-accent border-l-2 border-transparent'
              }`}
            >
              {item.icon}
              {item.label}
            </button>
          ))}
        </div>
      )}
    </div>
  );
};

export const TopCommandBar: React.FC<Props> = ({
  workspace, setWorkspace, renderMode, setRenderMode, solverProgress, simulationRunning,
  onOpenNotifications,
  onOpenSettings,
  onOpenWorkspaces,
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
  const [isExporting, setIsExporting] = useState(false);
  const store = useForgeStore();

  const handleRenderModeChange = async (modeId: ViewportRenderMode) => {
    setRenderMode(modeId);
    if (modeId === 'raytraced') {
      try {
        await fetch('/api/render/optix', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({
            project_name: 'SZM_Forge_OptiX',
            resolution: [1920, 1080],
            samples: 256,
            components: [],
            materials: []
          })
        });
      } catch (e) {
        console.error('OptiX rendering dispatch failed:', e);
      }
    }
  };

  const handleExportUSD = async () => {
    setIsExporting(true);
    try {
      const res = await fetch('/api/export/usd', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          project_name: 'SZM_Forge_Export',
          components: [
            { name: 'Base_Plate', bbox: {w: 1, h: 0.1, d: 1}, position: [0, 0, 0], material_name: 'Aluminum' }
          ],
          materials: [
            { name: 'Aluminum', base_color: [0.9, 0.9, 0.9], roughness: 0.2, metallic: 1.0 }
          ]
        })
      });
      const data = await res.json();
      if (data.status === 'success') {
        const downloadFile = (filename: string, content: string) => {
          const blob = new Blob([content], { type: 'text/plain' });
          const url = URL.createObjectURL(blob);
          const a = document.createElement('a');
          a.href = url;
          a.download = filename;
          document.body.appendChild(a);
          a.click();
          document.body.removeChild(a);
          URL.revokeObjectURL(url);
        };
        downloadFile('scene.usda', data.usda_data);
        downloadFile('materials.mtlx', data.mtlx_data);
      }
    } catch (e) {
      console.error('Export failed:', e);
    }
    setIsExporting(false);
  };

  const creationModules = [
    { id: 'geometry', label: 'GEOMETRY', icon: <PenTool size={13} /> },
    { id: 'components', label: 'COMPONENTS', icon: <Box size={13} /> },
    { id: 'mechanical', label: 'MECHANICAL', icon: <Settings size={13} /> },
    { id: 'circuit-design', label: 'ELECTRICAL', icon: <CircuitBoard size={13} /> },
    { id: 'chemistry-lab', label: 'CHEMICAL', icon: <Microscope size={13} /> },
    { id: 'optics', label: 'OPTICS', icon: <Lightbulb size={13} /> },
    { id: 'combined-assembly', label: 'ASSEMBLY', icon: <Combine size={13} /> },
  ];

  const executionModules = [
    { id: 'testing-lab', label: 'TESTING LAB', icon: <TestTube size={13} /> },
    { id: 'simulation', label: 'SIMULATION', icon: <Cpu size={13} /> },
    { id: 'digital-twin', label: 'DIGITAL TWIN', icon: <Layers size={13} /> },
    { id: 'manufacturing', label: 'MANUFACTURING', icon: <Factory size={13} /> },
  ];

  const helperModules = [
    { id: 'command-center', label: 'CMD CENTER', icon: <LayoutGrid size={13} /> },
    { id: 'ai-assistant', label: 'SZM AI', icon: <Brain size={13} /> },
    { id: 'knowledge-base', label: 'KNOWLEDGE', icon: <BookMarked size={13} /> },
    { id: 'node-programming', label: 'NODE LOGIC', icon: <GitMerge size={13} /> },
    { id: 'text-to-cad', label: 'TEXT → CAD', icon: <Wand2 size={13} /> },
    { id: 'fleet-command', label: 'FLEET CMD', icon: <Radio size={13} /> },
  ];

  const renderModes: { id: ViewportRenderMode; label: string; icon: React.ReactNode; tipKey: keyof typeof tooltips.viewport }[] = [
    { id: 'shaded', label: 'Shaded', icon: <Eye size={12} />, tipKey: 'shaded' },
    { id: 'wireframe', label: 'Wire', icon: <Grid3x3 size={12} />, tipKey: 'wireframe' },
    { id: 'xray', label: 'X-Ray', icon: <Maximize2 size={12} />, tipKey: 'xray' },
    { id: 'thermal', label: 'Thermal', icon: <Thermometer size={12} />, tipKey: 'thermal' },
    { id: 'stress', label: 'Stress', icon: <Activity size={12} />, tipKey: 'stress' },
    { id: 'electrical', label: 'Electric', icon: <Zap size={12} />, tipKey: 'electrical' },
    { id: 'raytraced', label: 'Raytraced', icon: <Aperture size={12} />, tipKey: 'shaded' },
  ];

  return (
    <div className="flex flex-col border-b border-forge-border/30 bg-forge-dark/80 backdrop-blur-xl relative z-50 shadow-lg">
      <div className="h-[44px] flex items-center px-4">
        {/* Logo */}
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
          </div>
        </div>

        <div className="w-px h-6 bg-forge-border mx-3" />

        {/* Global Import Action */}
        <button
          onClick={() => store.setImportDialogOpen(true)}
          className="flex items-center gap-1.5 px-3 py-1.5 rounded-md text-[11px] font-semibold tracking-wider text-forge-green hover:bg-forge-green/10 border border-forge-green/30 transition-all mr-2"
        >
          <Import size={12} />
          IMPORT / ADD
        </button>

        {/* Dropdown Navigation */}
        <div className="flex items-center gap-2">
          <DropdownMenu 
            title="CREATION MODULES" 
            icon={<Component size={13} />} 
            items={creationModules} 
            currentWorkspace={workspace} 
            setWorkspace={setWorkspace} 
          />
          <DropdownMenu 
            title="EXECUTION" 
            icon={<Activity size={13} />} 
            items={executionModules} 
            currentWorkspace={workspace} 
            setWorkspace={setWorkspace} 
          />
          <DropdownMenu 
            title="TOOLS & AI" 
            icon={<Brain size={13} />} 
            items={helperModules} 
            currentWorkspace={workspace} 
            setWorkspace={setWorkspace} 
          />
        </div>

        <div className="w-px h-6 bg-forge-border mx-4" />

        {/* Render Modes */}
        <div className="flex items-center gap-0.5">
          {renderModes.map(mode => (
            <button
              key={mode.id}
              onClick={() => handleRenderModeChange(mode.id)}
              title={t(tooltips.viewport[mode.tipKey])}
              className={`flex items-center gap-1 px-2 py-1 rounded text-[9px] font-mono tracking-wider transition-all duration-200 ${
                renderMode === mode.id
                  ? 'bg-forge-purple/15 text-forge-purple border border-forge-purple/30'
                  : 'text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface border border-transparent'
              }`}
            >
              {mode.icon}
              {mode.label}
            </button>
          ))}
        </div>

        <div className="w-px h-6 bg-forge-border mx-4" />

        {/* Simulation Controls */}
        <div className="flex items-center gap-1">
          <button onClick={onRunSimulation} className="flex items-center gap-1 px-2.5 py-1 rounded text-[9px] font-mono tracking-wider bg-forge-green/10 text-forge-green border border-forge-green/20 hover:bg-forge-green/20">
            <Play size={10} /> RUN
          </button>
          <button onClick={simulationPaused ? onResumeSimulation : onPauseSimulation} className="px-2 py-1 rounded text-forge-text-muted hover:text-forge-yellow hover:bg-forge-yellow/10">
            <Pause size={10} />
          </button>
          <button onClick={onResetSimulation} className="px-2 py-1 rounded text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface">
            <RotateCcw size={10} />
          </button>
        </div>

        {/* Status & Export */}
        <div className="ml-auto flex items-center gap-3">
          <button
            onClick={handleExportUSD}
            disabled={isExporting}
            className="flex items-center gap-1.5 px-3 py-1 rounded text-[10px] font-bold tracking-wider bg-forge-accent/20 text-forge-accent border border-forge-accent/40 hover:bg-forge-accent/40 disabled:opacity-50"
          >
            <Download size={11} /> {isExporting ? 'EXPORTING...' : 'EXPORT USD'}
          </button>
          <button onClick={onOpenNotifications} className="relative p-1.5 rounded text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface">
            <Bell size={13} />
            {notificationCount > 0 && (
              <span className="absolute -top-0.5 -right-0.5 w-3.5 h-3.5 rounded-full bg-forge-red text-[7px] font-bold flex items-center justify-center text-white">
                {notificationCount}
              </span>
            )}
          </button>
          <button onClick={onToggleAIPanel} className={`px-2 py-1 rounded text-[9px] font-mono border ${showAIPanel ? 'border-forge-accent/40 text-forge-accent' : 'border-transparent text-forge-text-muted'}`}>PANEL</button>
          <button onClick={onOpenSettings} className="p-1.5 rounded text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface">
            <Settings size={13} />
          </button>
        </div>
      </div>
    </div>
  );
};
