import React, { useEffect, useState } from 'react';
import {
  Box, Scale, Brain, Play, FolderPlus, Loader2, Armchair, GripVertical,
} from 'lucide-react';
import { useProject } from '../store/ProjectContext';
import { useTooltips } from '../store/TooltipContext';
import { HelpHint } from './ui/HelpHint';
import type { FurnitureType } from '../workshop/furnitureEngine';
import { materialOptions } from '../workshop/furnitureEngine';
const FURNITURE_TYPES: { id: FurnitureType; label: string; icon: React.ReactNode }[] = [
  { id: 'table', label: 'Table', icon: <Box size={11} /> },
  { id: 'chair', label: 'Chair', icon: <Armchair size={11} /> },
  { id: 'ladder', label: 'Ladder', icon: <GripVertical size={11} /> },
];

/**
 * Step-by-step workshop — each button is one user action (no hidden bootstrap).
 */
interface FurnitureWorkshopPanelProps {
  onFurnitureBuilt?: () => void;
}

export const FurnitureWorkshopPanel: React.FC<FurnitureWorkshopPanelProps> = ({ onFurnitureBuilt }) => {
  const {
    project,
    parts,
    limits,
    aiInsights,
    loading,
    error,
    furnitureType,
    setFurnitureType,
    newProject,
    buildFurniture,
    applyLoad,
    measureLimits,
    improveWithAI,
    previewLoad,
    lastAction,
  } = useProject();
  const { t } = useTooltips();

  const [materialId, setMaterialId] = useState('MAT-STEEL-STRUCT');
  const [loadKg, setLoadKg] = useState(200);
  const [stepDone, setStepDone] = useState(0);

  useEffect(() => {
    if (project?.materialId) setMaterialId(project.materialId);
    if (project?.loadKg) setLoadKg(project.loadKg);
  }, [project?.materialId, project?.loadKg]);

  const defaultLoad = (type: FurnitureType) =>
    type === 'ladder' ? 100 : type === 'chair' ? 90 : 200;

  const onPickType = (type: FurnitureType) => {
    setFurnitureType(type);
    setLoadKg(defaultLoad(type));
  };

  const statusColor = (s: string) =>
    s === 'FAIL' ? 'text-forge-red' : s === 'WARN' ? 'text-forge-yellow' : 'text-forge-green';

  return (
    <div className="border-t border-forge-border bg-forge-panel/80 p-3 flex flex-col gap-2 max-h-[360px] overflow-y-auto custom-scrollbar">
      <div className="flex items-center gap-2">
        <Box size={12} className="text-forge-accent" />
        <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">
          FURNITURE WORKSHOP
        </span>
        <HelpHint text="Work like in CAD: new project → pick furniture → material → build → load → RUN limits → AI improve. Use toolbar RUN to refresh live results." />
        {loading && <Loader2 size={12} className="animate-spin text-forge-accent ml-auto" />}
      </div>

      {lastAction && (
        <p className="text-[8px] font-mono text-forge-accent/80 border-l-2 border-forge-accent pl-2">
          {lastAction}
        </p>
      )}
      {error && <p className="text-[8px] font-mono text-forge-yellow">{error}</p>}

      <div className="flex gap-1">
        {FURNITURE_TYPES.map(ft => (
          <button
            key={ft.id}
            type="button"
            onClick={() => onPickType(ft.id)}
            className={`flex-1 flex items-center justify-center gap-1 py-1.5 rounded text-[8px] font-mono border ${
              furnitureType === ft.id
                ? 'border-forge-accent bg-forge-accent/15 text-forge-accent'
                : 'border-forge-border text-forge-text-muted hover:border-forge-accent/30'
            }`}
          >
            {ft.icon}
            {ft.label}
          </button>
        ))}
      </div>

      <select
        className="w-full bg-forge-dark border border-forge-border rounded px-2 py-1 text-[9px] font-mono text-forge-text"
        value={materialId}
        onChange={e => setMaterialId(e.target.value)}
        title={t('Material for all structural members')}
      >
        {materialOptions().map(m => (
          <option key={m.id} value={m.id}>
            {m.label}
          </option>
        ))}
      </select>

      <label className="text-[8px] font-mono text-forge-text-muted">
        Load ({furnitureType === 'ladder' ? 'climber' : 'on top'}) — {loadKg} kg
      </label>
      <input
        type="range"
        min={furnitureType === 'ladder' ? 40 : 50}
        max={furnitureType === 'ladder' ? 200 : 700}
        step={10}
        value={loadKg}
        onChange={e => {
          const v = Number(e.target.value);
          setLoadKg(v);
          if (project?.hasFurniture) previewLoad(v);
        }}
        className="w-full"
      />
      {project?.hasFurniture && (
        <p className="text-[7px] font-mono text-forge-text-muted">
          Drag load to see stress update live in the 3D view (Stress mode recommended).
        </p>
      )}

      <div className="grid grid-cols-2 gap-1">
        <button
          type="button"
          disabled={loading}
          onClick={async () => {
            await newProject(`${furnitureType.charAt(0).toUpperCase()}${furnitureType.slice(1)} Study`);
            setStepDone(1);
          }}
          className="flex items-center justify-center gap-1 px-2 py-1.5 rounded text-[8px] font-mono border border-forge-border hover:border-forge-accent/40"
        >
          <FolderPlus size={10} /> 1. NEW PROJECT
        </button>
        <button
          type="button"
          disabled={loading}
          onClick={async () => {
            await buildFurniture(materialId, loadKg);
            onFurnitureBuilt?.();
            setStepDone(2);
          }}
          className="flex items-center justify-center gap-1 px-2 py-1.5 rounded text-[8px] font-mono border border-forge-border hover:border-forge-accent/40"
        >
          <Box size={10} /> 2. BUILD
        </button>
        <button
          type="button"
          disabled={loading || !project?.hasFurniture}
          onClick={async () => {
            await applyLoad(loadKg);
            setStepDone(3);
          }}
          className="flex items-center justify-center gap-1 px-2 py-1.5 rounded text-[8px] font-mono border border-forge-border hover:border-forge-accent/40"
        >
          <Scale size={10} /> 3. APPLY LOAD
        </button>
        <button
          type="button"
          disabled={loading || !project?.hasFurniture}
          onClick={async () => {
            await measureLimits();
            setStepDone(4);
          }}
          className="flex items-center justify-center gap-1 px-2 py-1.5 rounded text-[8px] font-mono border border-forge-yellow/40 text-forge-yellow"
        >
          <Play size={10} /> 4. LIMITS
        </button>
      </div>

      <button
        type="button"
        disabled={loading || !project?.hasFurniture}
        onClick={async () => {
          await improveWithAI();
          setStepDone(5);
        }}
        className="w-full flex items-center justify-center gap-1 py-2 rounded text-[9px] font-mono bg-forge-purple/20 border border-forge-purple/40 text-forge-purple"
      >
        <Brain size={11} /> 5. AI IMPROVE — then open AI ENGINE tab
      </button>

      {stepDone > 0 && (
        <p className="text-[7px] font-mono text-forge-text-muted">
          Tip: switch render mode to <strong className="text-forge-accent">Stress</strong> in the toolbar; press{' '}
          <strong className="text-forge-accent">RUN</strong> after each step when backend is live.
        </p>
      )}

      {project?.hasFurniture && parts.length > 0 && (
        <div className="space-y-0.5">
          {parts.map(p => (
            <div key={p.name} className="flex justify-between text-[8px] font-mono">
              <span className="text-forge-text-dim">{p.name}</span>
              <span className={statusColor(p.status)}>
                {(p.stressRatio * 100).toFixed(0)}% {p.status}
              </span>
            </div>
          ))}
        </div>
      )}

      {limits.length > 0 && (
        <div className="border-t border-forge-border/40 pt-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Limit sweep</span>
          {limits.slice(0, 4).map(row => (
            <div key={row.loadKg} className="flex justify-between text-[8px] font-mono">
              <span>{row.loadKg} kg</span>
              <span className={statusColor(row.status)}>{row.worstUtilization.toFixed(0)}%</span>
            </div>
          ))}
        </div>
      )}

      {aiInsights.length > 0 && (
        <p className="text-[8px] text-forge-purple line-clamp-2" data-testid="ai-insight">
          AI: {aiInsights[0].title}
        </p>
      )}
    </div>
  );
};
