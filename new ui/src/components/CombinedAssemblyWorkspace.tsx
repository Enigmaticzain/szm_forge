import React, { useState } from 'react';
import { useForgeStore } from '../store/ForgeStoreContext';
import { useProject } from '../store/ProjectContext';
import { Combine, Link2, Plus, Puzzle } from 'lucide-react';

type MateType = 'fixed' | 'revolute' | 'slider' | 'ball';

interface MateStub {
  id: string;
  type: MateType;
  partA: string;
  partB: string;
}

export const CombinedAssemblyWorkspace: React.FC = () => {
  const store = useForgeStore();
  const { bodyParts, bodyJoints, setBodyJoints } = useProject();
  const [mates, setMates] = useState<MateStub[]>([]);
  const [mateType, setMateType] = useState<MateType>('fixed');
  const [partA, setPartA] = useState('');
  const [partB, setPartB] = useState('');

  const addMate = () => {
    const a = partA || bodyParts[0]?.id;
    const b = partB || bodyParts[1]?.id;
    if (!a || !b || a === b) {
      store.showToast('Select two different parts to mate');
      return;
    }
    const id = `mate_${Date.now()}`;
    setMates(prev => [...prev, { id, type: mateType, partA: a, partB: b }]);
    setBodyJoints(prev => [
      ...prev,
      {
        id,
        partAId: a,
        partBId: b,
        type: mateType === 'revolute' ? 'hinge' : mateType === 'fixed' ? 'bolt' : 'glue',
        position: { x: 0, y: 0, z: 0 },
        size: 8,
      },
    ]);
    store.showToast(`Added ${mateType} mate`);
  };

  const partName = (id: string) => bodyParts.find(p => p.id === id)?.name ?? id;

  return (
    <div className="w-full h-full flex bg-forge-dark/50 relative overflow-hidden">
      <div className="w-[300px] border-r border-forge-border bg-forge-dark/90 p-4 flex flex-col gap-3 flex-shrink-0">
        <div className="flex items-center gap-2 text-forge-accent">
          <Puzzle size={14} />
          <span className="text-[10px] font-bold tracking-widest">ADD MATE</span>
        </div>

        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Mate type</span>
          <select value={mateType} onChange={e => setMateType(e.target.value as MateType)}
            className="bg-forge-black border border-forge-border rounded px-2 py-1.5 text-[10px] font-mono text-forge-text">
            <option value="fixed">Fixed</option>
            <option value="revolute">Revolute / hinge</option>
            <option value="slider">Slider</option>
            <option value="ball">Ball joint</option>
          </select>
        </label>

        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Part A</span>
          <select value={partA} onChange={e => setPartA(e.target.value)}
            className="bg-forge-black border border-forge-border rounded px-2 py-1.5 text-[10px] font-mono text-forge-text">
            <option value="">— select —</option>
            {bodyParts.map(p => <option key={p.id} value={p.id}>{p.name}</option>)}
          </select>
        </label>

        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Part B</span>
          <select value={partB} onChange={e => setPartB(e.target.value)}
            className="bg-forge-black border border-forge-border rounded px-2 py-1.5 text-[10px] font-mono text-forge-text">
            <option value="">— select —</option>
            {bodyParts.map(p => <option key={p.id} value={p.id}>{p.name}</option>)}
          </select>
        </label>

        <button onClick={addMate}
          className="flex items-center justify-center gap-2 py-2 rounded bg-forge-accent/20 text-forge-accent border border-forge-accent/50 text-[10px] font-bold font-mono">
          <Plus size={12} /> ADD MATE
        </button>
        <button onClick={() => store.setImportDialogOpen(true)}
          className="py-2 rounded border border-forge-green/40 text-forge-green text-[10px] font-bold font-mono hover:bg-forge-green/10">
          IMPORT SUB-ASSEMBLY
        </button>
      </div>

      <div className="flex-1 flex flex-col min-w-0 p-6 gap-4 overflow-auto">
        <div className="flex items-center gap-2">
          <Combine size={18} className="text-forge-accent" />
          <span className="text-forge-text font-mono text-sm tracking-wider">COMBINED ASSEMBLY</span>
        </div>

        <div className="rounded-xl border border-forge-border/60 bg-forge-panel/60 p-4">
          <div className="text-[9px] font-bold tracking-widest text-forge-text-muted mb-3">ASSEMBLY TREE</div>
          {bodyParts.length === 0 ? (
            <p className="text-[11px] text-forge-text-muted font-mono">No parts yet — import a sub-assembly or create geometry.</p>
          ) : (
            <ul className="space-y-1">
              {bodyParts.map(p => (
                <li key={p.id} className="flex items-center gap-2 text-[11px] font-mono text-forge-text">
                  <span className="text-forge-accent">▸</span>
                  {p.name}
                  <span className="text-forge-text-muted text-[9px]">{p.type}</span>
                </li>
              ))}
            </ul>
          )}
        </div>

        <div className="rounded-xl border border-forge-border/60 bg-forge-panel/60 p-4">
          <div className="text-[9px] font-bold tracking-widest text-forge-text-muted mb-3 flex items-center gap-2">
            <Link2 size={12} /> MATES / CONSTRAINTS
          </div>
          {mates.length === 0 && bodyJoints.length === 0 ? (
            <p className="text-[11px] text-forge-text-muted font-mono">No mates defined.</p>
          ) : (
            <ul className="space-y-2">
              {mates.map(m => (
                <li key={m.id} className="text-[11px] font-mono text-forge-text-dim border border-forge-border/40 rounded px-2 py-1.5">
                  <span className="text-forge-green uppercase">{m.type}</span>
                  {' · '}{partName(m.partA)} ↔ {partName(m.partB)}
                </li>
              ))}
              {bodyJoints.filter(j => !mates.some(m => m.id === j.id)).map(j => (
                <li key={j.id} className="text-[11px] font-mono text-forge-text-dim border border-forge-border/40 rounded px-2 py-1.5">
                  <span className="text-forge-purple uppercase">{j.type}</span>
                  {' · '}{partName(j.partAId)} ↔ {partName(j.partBId)}
                </li>
              ))}
            </ul>
          )}
        </div>
      </div>
    </div>
  );
};
