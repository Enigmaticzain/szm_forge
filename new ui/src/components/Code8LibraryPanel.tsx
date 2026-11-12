import React, { useEffect, useState } from 'react';
import { BookOpen, Loader2, Wrench } from 'lucide-react';
import { fetchCode8Catalog, type Code8ExampleDto } from '../services/code8Api';
import { useForgeStore } from '../store/ForgeStoreContext';
import { useProject } from '../store/ProjectContext';

export const Code8LibraryPanel: React.FC = () => {
  const [loading, setLoading] = useState(true);
  const [examples, setExamples] = useState<Code8ExampleDto[]>([]);
  const [root, setRoot] = useState('');
  const [coreReady, setCoreReady] = useState(false);
  const [filter, setFilter] = useState('');
  const { showToast, setWorkspace } = useForgeStore();
  const { setBodyParts } = useProject();

  useEffect(() => {
    let cancelled = false;
    (async () => {
      setLoading(true);
      const cat = await fetchCode8Catalog();
      if (cancelled) return;
      if (cat?.ok) {
        setExamples(cat.examples ?? []);
        setRoot(cat.root);
        setCoreReady(cat.toolchain?.coreSolverReady ?? false);
      } else {
        showToast('Code8 catalog offline — start backend with codes/code 8');
      }
      setLoading(false);
    })();
    return () => {
      cancelled = true;
    };
  }, [showToast]);

  const filtered = examples.filter(
    e =>
      !filter ||
      e.name.toLowerCase().includes(filter.toLowerCase()) ||
      e.category.toLowerCase().includes(filter.toLowerCase())
  );

  const categories = [...new Set(filtered.map(e => e.category))].sort();

  const loadExampleIntoScene = (ex: Code8ExampleDto) => {
    const lower = `${ex.name} ${ex.category} ${ex.summary}`.toLowerCase();
    const isShell = lower.includes('shell') || lower.includes('plate');
    const isBeam = lower.includes('beam') || lower.includes('cantilever');
    const size = isShell
      ? { x: 120, y: 4, z: 80 }
      : isBeam
        ? { x: 200, y: 20, z: 20 }
        : { x: 60, y: 40, z: 40 };

    const id = `code8_${Date.now()}`;
    setBodyParts(prev => [
      ...prev,
      {
        id,
        name: ex.name,
        type: isBeam ? 'cylinder' : 'box',
        position: { x: (prev.length % 4) * 70, y: 0, z: Math.floor(prev.length / 4) * 70 },
        size,
        bendAngle: 0,
        materialId: 'steel',
      },
    ]);
    setWorkspace('geometry');
    showToast(`Loaded ${ex.name} into viewport (${ex.inpCount} .inp ref)`);
  };

  return (
    <div className="flex flex-col gap-2 text-[8px] font-mono">
      <div className="flex items-center gap-2 text-forge-text-dim">
        <BookOpen size={11} className="text-forge-purple" />
        <span className="tracking-widest">CALCULIX · codes/code 8</span>
        {loading && <Loader2 size={10} className="animate-spin ml-auto text-forge-accent" />}
      </div>

      <p className="text-forge-text-muted leading-relaxed">
        Parametric FEA examples (beams, shells, contact, buckling). Click a card to drop a stand-in mesh into the scene.
        {root ? ` Root: ${root}` : ''}
      </p>

      <div className="flex items-center gap-2">
        <Wrench size={10} className={coreReady ? 'text-forge-green' : 'text-forge-yellow'} />
        <span className={coreReady ? 'text-forge-green' : 'text-forge-yellow'}>
          {coreReady ? 'CCX+CGX toolchain detected' : 'Install CalculiX / CGX for full runs'}
        </span>
      </div>

      <input
        type="search"
        placeholder="Filter examples…"
        value={filter}
        onChange={e => setFilter(e.target.value)}
        className="w-full bg-forge-dark border border-forge-border rounded px-2 py-1 text-[9px]"
      />

      <div className="max-h-[140px] overflow-y-auto custom-scrollbar space-y-2">
        {categories.map(cat => (
          <div key={cat}>
            <div className="text-[7px] text-forge-purple mb-1">{cat}</div>
            {filtered
              .filter(e => e.category === cat)
              .slice(0, 8)
              .map(ex => (
                <button
                  key={ex.name + ex.directory}
                  type="button"
                  title={`${ex.directory} — click to load into viewport`}
                  onClick={() => loadExampleIntoScene(ex)}
                  className="w-full text-left px-2 py-1 rounded border border-forge-border/60 hover:border-forge-accent/40 mb-0.5"
                >
                  <div className="text-forge-text truncate">{ex.name}</div>
                  {ex.summary && (
                    <div className="text-forge-text-muted truncate text-[7px]">{ex.summary}</div>
                  )}
                </button>
              ))}
          </div>
        ))}
        {!loading && filtered.length === 0 && (
          <p className="text-forge-text-muted">No examples — check SZM_CODE8_DIR or codes/code 8 path.</p>
        )}
      </div>
      <p className="text-[7px] text-forge-text-muted">{filtered.length} of {examples.length} examples</p>
    </div>
  );
};
