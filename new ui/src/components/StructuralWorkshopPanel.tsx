import React, { useEffect, useState } from 'react';
import {
  Box, Scale, Brain, Play, FolderPlus, Loader2, Link2, Plus, Trash2,
  Columns, Minus, Square,
} from 'lucide-react';
import { useProject } from '../store/ProjectContext';
import { useTooltips } from '../store/TooltipContext';
import { HelpHint } from './ui/HelpHint';
import type { MemberKind, AttachEnd } from '../workshop/structuralAssembly';
import { materialCatalog } from '../workshop/structuralAssembly';
import { Code8LibraryPanel } from './Code8LibraryPanel';

const KINDS: { id: MemberKind; label: string; icon: React.ReactNode }[] = [
  { id: 'beam', label: 'Beam', icon: <Columns size={11} /> },
  { id: 'rod', label: 'Rod', icon: <Minus size={11} /> },
  { id: 'plate', label: 'Plate', icon: <Square size={11} /> },
  { id: 'block', label: 'Block', icon: <Box size={11} /> },
];

const PRESETS = ['table', 'chair', 'ladder'] as const;

export const StructuralWorkshopPanel: React.FC<{ onBuilt?: () => void }> = ({ onBuilt }) => {
  const {
    project,
    parts,
    limits,
    aiInsights,
    loading,
    error,
    lastAction,
    members,
    selectedMemberId,
    setSelectedMemberId,
    newProject,
    loadPreset,
    addMember,
    removeMember,
    updateSelectedMember,
    solveAssembly,
    applyLoad,
    measureLimits,
    improveWithAI,
    previewLoad,
  } = useProject();
  const { t } = useTooltips();

  const [tab, setTab] = useState<'kit' | 'presets' | 'code8'>('kit');
  const [materialId, setMaterialId] = useState('MAT-STEEL-STRUCT');
  const [loadKg, setLoadKg] = useState(200);
  const [kind, setKind] = useState<MemberKind>('beam');
  const [length, setLength] = useState(0.75);
  const [width, setWidth] = useState(0.08);
  const [height, setHeight] = useState(0.08);
  const [attachTo, setAttachTo] = useState<string>('');
  const [attachEnd, setAttachEnd] = useState<AttachEnd>('end');

  const selected = members.find(m => m.id === selectedMemberId);

  useEffect(() => {
    if (project?.materialId) setMaterialId(project.materialId);
    if (project?.loadKg) setLoadKg(project.loadKg);
  }, [project?.materialId, project?.loadKg]);

  useEffect(() => {
    if (selected) {
      setKind(selected.kind);
      setLength(selected.length);
      setWidth(selected.width);
      setHeight(selected.height);
      setAttachTo(selected.attachTo ?? '');
      setAttachEnd(selected.attachEnd);
    }
  }, [selectedMemberId, selected]);

  const statusColor = (s: string) =>
    s === 'FAIL' ? 'text-forge-red' : s === 'WARN' ? 'text-forge-yellow' : 'text-forge-green';

  return (
    <div className="border-t border-forge-border bg-forge-panel/80 p-3 flex flex-col gap-2 max-h-[420px] overflow-y-auto custom-scrollbar">
      <div className="flex items-center gap-2">
        <Box size={12} className="text-forge-accent" />
        <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">
          STRUCTURAL KIT
        </span>
        <HelpHint text="Add beams, rods, plates, blocks — resize, attach end-to-end, solve with beam bending + von Mises stress." />
        {loading && <Loader2 size={12} className="animate-spin text-forge-accent ml-auto" />}
      </div>

      <div className="flex gap-1">
        {(['kit', 'presets', 'code8'] as const).map(tid => (
          <button
            key={tid}
            type="button"
            onClick={() => setTab(tid)}
            className={`flex-1 py-1 rounded text-[8px] font-mono border ${
              tab === tid ? 'border-forge-accent text-forge-accent bg-forge-accent/10' : 'border-forge-border text-forge-text-muted'
            }`}
          >
            {tid === 'kit' ? 'KIT' : tid === 'presets' ? 'PRESETS' : 'FEA LIB'}
          </button>
        ))}
      </div>

      {lastAction && (
        <p className="text-[8px] font-mono text-forge-accent/80 border-l-2 border-forge-accent pl-2">{lastAction}</p>
      )}
      {error && <p className="text-[8px] font-mono text-forge-yellow">{error}</p>}

      {tab === 'code8' ? (
        <Code8LibraryPanel />
      ) : tab === 'presets' ? (
        <div className="flex gap-1 flex-wrap">
          {PRESETS.map(p => (
            <button
              key={p}
              type="button"
              disabled={loading}
              onClick={async () => {
                await loadPreset(p, materialId, loadKg);
                onBuilt?.();
              }}
              className="flex-1 min-w-[70px] py-1.5 rounded text-[8px] font-mono border border-forge-border hover:border-forge-accent/40 capitalize"
            >
              {p}
            </button>
          ))}
        </div>
      ) : (
        <>
          <div className="flex gap-1 flex-wrap">
            {KINDS.map(k => (
              <button
                key={k.id}
                type="button"
                onClick={() => setKind(k.id)}
                className={`flex items-center gap-1 px-2 py-1 rounded text-[8px] font-mono border ${
                  kind === k.id ? 'border-forge-accent bg-forge-accent/15 text-forge-accent' : 'border-forge-border text-forge-text-muted'
                }`}
              >
                {k.icon}
                {k.label}
              </button>
            ))}
          </div>

          <div className="grid grid-cols-3 gap-1">
            <label className="text-[7px] font-mono text-forge-text-muted">
              L (m)
              <input type="number" step={0.01} min={0.01} value={length} onChange={e => setLength(+e.target.value)} className="w-full mt-0.5 bg-forge-dark border border-forge-border rounded px-1 py-0.5 text-[9px]" />
            </label>
            <label className="text-[7px] font-mono text-forge-text-muted">
              W (m)
              <input type="number" step={0.01} min={0.01} value={width} onChange={e => setWidth(+e.target.value)} className="w-full mt-0.5 bg-forge-dark border border-forge-border rounded px-1 py-0.5 text-[9px]" />
            </label>
            <label className="text-[7px] font-mono text-forge-text-muted">
              {kind === 'rod' ? 'Ø' : 'H'} (m)
              <input type="number" step={0.01} min={0.01} value={height} onChange={e => setHeight(+e.target.value)} className="w-full mt-0.5 bg-forge-dark border border-forge-border rounded px-1 py-0.5 text-[9px]" />
            </label>
          </div>

          <div className="flex gap-1">
            <select
              className="flex-1 bg-forge-dark border border-forge-border rounded px-2 py-1 text-[8px] font-mono"
              value={attachTo}
              onChange={e => setAttachTo(e.target.value)}
            >
              <option value="">— root (ground) —</option>
              {members.map(m => (
                <option key={m.id} value={m.id}>
                  Attach to {m.name}
                </option>
              ))}
            </select>
            <select
              className="w-20 bg-forge-dark border border-forge-border rounded px-1 py-1 text-[8px] font-mono"
              value={attachEnd}
              onChange={e => setAttachEnd(e.target.value as AttachEnd)}
            >
              <option value="end">end</option>
              <option value="start">start</option>
              <option value="top">top</option>
              <option value="side">side</option>
            </select>
          </div>

          <div className="flex gap-1">
            <button
              type="button"
              disabled={loading}
              onClick={() => {
                addMember(kind, { length, width, height: kind === 'rod' ? width : height }, materialId, attachTo || null, attachEnd);
                onBuilt?.();
              }}
              className="flex-1 flex items-center justify-center gap-1 py-1.5 rounded text-[8px] font-mono border border-forge-accent/40 text-forge-accent"
            >
              <Plus size={10} /> ADD
            </button>
            {selected && (
              <>
                <button
                  type="button"
                  onClick={() =>
                    updateSelectedMember({ length, width, height: kind === 'rod' ? width : height, kind, materialId })
                  }
                  className="flex-1 py-1.5 rounded text-[8px] font-mono border border-forge-border"
                >
                  UPDATE
                </button>
                <button
                  type="button"
                  onClick={() => removeMember(selected.id)}
                  className="px-2 py-1.5 rounded text-[8px] border border-forge-red/40 text-forge-red"
                >
                  <Trash2 size={10} />
                </button>
              </>
            )}
          </div>

          {members.length > 0 && (
            <ul className="max-h-16 overflow-y-auto space-y-0.5 border border-forge-border/40 rounded p-1">
              {members.map(m => (
                <li key={m.id}>
                  <button
                    type="button"
                    onClick={() => setSelectedMemberId(m.id)}
                    className={`w-full text-left text-[8px] font-mono px-1 py-0.5 rounded ${
                      selectedMemberId === m.id ? 'bg-forge-accent/20 text-forge-accent' : 'text-forge-text-muted hover:bg-forge-surface'
                    }`}
                  >
                    <Link2 size={8} className="inline mr-1" />
                    {m.name} ({m.kind}) {m.length.toFixed(2)}m
                  </button>
                </li>
              ))}
            </ul>
          )}
        </>
      )}

      <select
        className="w-full bg-forge-dark border border-forge-border rounded px-2 py-1 text-[9px] font-mono"
        value={materialId}
        onChange={e => setMaterialId(e.target.value)}
      >
        {materialCatalog().map(m => (
          <option key={m.id} value={m.id}>
            {m.label}
          </option>
        ))}
      </select>

      <label className="text-[8px] font-mono text-forge-text-muted">
        Load — {loadKg} kg
      </label>
      <input
        type="range"
        min={10}
        max={800}
        step={10}
        value={loadKg}
        onChange={e => {
          const v = Number(e.target.value);
          setLoadKg(v);
          if (members.length) previewLoad(v);
        }}
        className="w-full"
      />

      <div className="grid grid-cols-2 gap-1">
        <button
          type="button"
          disabled={loading}
          onClick={() => newProject('Structural Assembly')}
          className="flex items-center justify-center gap-1 py-1.5 rounded text-[8px] font-mono border border-forge-border"
        >
          <FolderPlus size={10} /> NEW
        </button>
        <button
          type="button"
          disabled={loading || !members.length}
          onClick={async () => {
            await solveAssembly(materialId, loadKg);
            onBuilt?.();
          }}
          className="flex items-center justify-center gap-1 py-1.5 rounded text-[8px] font-mono border border-forge-border"
        >
          <Box size={10} /> SOLVE
        </button>
        <button
          type="button"
          disabled={loading || !members.length}
          onClick={() => applyLoad(loadKg)}
          className="flex items-center justify-center gap-1 py-1.5 rounded text-[8px] font-mono border border-forge-border"
        >
          <Scale size={10} /> LOAD
        </button>
        <button
          type="button"
          disabled={loading || !members.length}
          onClick={() => measureLimits()}
          className="flex items-center justify-center gap-1 py-1.5 rounded text-[8px] font-mono border border-forge-yellow/40 text-forge-yellow"
        >
          <Play size={10} /> LIMITS
        </button>
      </div>

      <button
        type="button"
        disabled={loading || !members.length}
        onClick={() => improveWithAI()}
        className="w-full flex items-center justify-center gap-1 py-2 rounded text-[9px] font-mono bg-forge-purple/20 border border-forge-purple/40 text-forge-purple"
      >
        <Brain size={11} /> AI IMPROVE
      </button>

      {parts.length > 0 && (
        <div className="space-y-0.5 max-h-20 overflow-y-auto">
          {parts.map(p => (
            <div key={p.name} className="flex justify-between text-[8px] font-mono">
              <span className="text-forge-text-dim">{p.name}</span>
              <span className={statusColor(p.status)}>{(p.stressRatio * 100).toFixed(0)}%</span>
            </div>
          ))}
        </div>
      )}

      {limits.length > 0 && (
        <div className="border-t border-forge-border/40 pt-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Limit sweep</span>
          {limits.slice(0, 3).map(row => (
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
