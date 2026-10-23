import React, { useCallback, useEffect, useRef, useState } from 'react';
import {
  Circle, Minus, MousePointer2, Play, Square, Layers,
  Ruler, AlignHorizontalJustifyCenter, AlignVerticalJustifyCenter,
} from 'lucide-react';
import {
  extrudeSketch, solveSketch, SketchDto, SketchEntityDto, SketchSolveResult,
} from '../../services/forgeApi';

type SketchTool = 'select' | 'line' | 'rectangle' | 'circle';

const CANVAS_W = 720;
const CANVAS_H = 480;
const GRID = 20;
const SCALE = 2; // pixels per mm

function snap(v: number) {
  return Math.round(v / GRID) * GRID;
}

function screenToSketch(sx: number, sy: number): [number, number] {
  const cx = CANVAS_W / 2;
  const cy = CANVAS_H / 2;
  return [(sx - cx) / SCALE, -(sy - cy) / SCALE];
}

function sketchToScreen(u: number, v: number): [number, number] {
  const cx = CANVAS_W / 2;
  const cy = CANVAS_H / 2;
  return [cx + u * SCALE, cy - v * SCALE];
}

let nextId = 1;

export const SketchEditor: React.FC = () => {
  const [tool, setTool] = useState<SketchTool>('line');
  const [entities, setEntities] = useState<SketchEntityDto[]>([]);
  const [draft, setDraft] = useState<{ start: [number, number]; end: [number, number] } | null>(null);
  const [solveResult, setSolveResult] = useState<SketchSolveResult | null>(null);
  const [extrudeDist, setExtrudeDist] = useState(10);
  const [extrudeResult, setExtrudeResult] = useState<Record<string, unknown> | null>(null);
  const [busy, setBusy] = useState(false);
  const svgRef = useRef<SVGSVGElement>(null);

  const buildSketch = useCallback((): SketchDto => ({
    plane: { origin: [0, 0, 0], normal: [0, 0, 1] },
    entities,
    constraints: [],
    dimensions: [],
  }), [entities]);

  const handleCanvasClick = (e: React.MouseEvent<SVGSVGElement>) => {
    const rect = svgRef.current?.getBoundingClientRect();
    if (!rect) return;
    const sx = e.clientX - rect.left;
    const sy = e.clientY - rect.top;
    const pt = screenToSketch(snap(sx), snap(sy));

    if (tool === 'line') {
      if (!draft) {
        setDraft({ start: pt, end: pt });
      } else {
        const id = nextId++;
        setEntities(prev => [...prev, {
          id, type: 'line', p1: draft.start, p2: pt,
        }]);
        setDraft(null);
      }
      return;
    }

    if (tool === 'rectangle') {
      if (!draft) {
        setDraft({ start: pt, end: pt });
      } else {
        const [x1, y1] = draft.start;
        const [x2, y2] = pt;
        const lines: SketchEntityDto[] = [
          { id: nextId++, type: 'line', p1: [x1, y1], p2: [x2, y1] },
          { id: nextId++, type: 'line', p1: [x2, y1], p2: [x2, y2] },
          { id: nextId++, type: 'line', p1: [x2, y2], p2: [x1, y2] },
          { id: nextId++, type: 'line', p1: [x1, y2], p2: [x1, y1] },
        ];
        setEntities(prev => [...prev, ...lines]);
        setDraft(null);
      }
      return;
    }

    if (tool === 'circle') {
      if (!draft) {
        setDraft({ start: pt, end: pt });
      } else {
        const dx = pt[0] - draft.start[0];
        const dy = pt[1] - draft.start[1];
        const r = Math.sqrt(dx * dx + dy * dy);
        if (r > 0.5) {
          setEntities(prev => [...prev, {
            id: nextId++,
            type: 'circle',
            p1: draft.start,
            p2: [draft.start[0] + r, draft.start[1]],
          }]);
        }
        setDraft(null);
      }
    }
  };

  const handleMouseMove = (e: React.MouseEvent<SVGSVGElement>) => {
    if (!draft) return;
    const rect = svgRef.current?.getBoundingClientRect();
    if (!rect) return;
    const sx = e.clientX - rect.left;
    const sy = e.clientY - rect.top;
    setDraft(d => d ? { ...d, end: screenToSketch(snap(sx), snap(sy)) } : null);
  };

  const addConstraint = (type: string) => {
    if (entities.length < 1) return;
    const sketch = buildSketch();
    sketch.constraints = [...(sketch.constraints ?? []), {
      type,
      entity1: entities[entities.length - 1].id,
      entity2: entities.length > 1 ? entities[entities.length - 2].id : entities[0].id,
    }];
    setSolveResult(null);
  };

  const runSolve = async () => {
    setBusy(true);
    const result = await solveSketch(buildSketch());
    if (result?.entities) {
      setEntities(result.entities);
    }
    setSolveResult(result);
    setBusy(false);
  };

  const runExtrude = async () => {
    setBusy(true);
    const result = await extrudeSketch(buildSketch(), extrudeDist / 1000);
    setExtrudeResult(result);
    setBusy(false);
  };

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.key === 'Escape') setDraft(null);
    };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, []);

  const renderEntity = (ent: SketchEntityDto) => {
    if (ent.type === 'line' && ent.p2) {
      const [x1, y1] = sketchToScreen(ent.p1[0], ent.p1[1]);
      const [x2, y2] = sketchToScreen(ent.p2[0], ent.p2[1]);
      return (
        <line key={ent.id} x1={x1} y1={y1} x2={x2} y2={y2}
          stroke="#00d4ff" strokeWidth={2} />
      );
    }
    if (ent.type === 'circle' && ent.p2) {
      const [cx, cy] = sketchToScreen(ent.p1[0], ent.p1[1]);
      const dx = ent.p2[0] - ent.p1[0];
      const dy = ent.p2[1] - ent.p1[1];
      const r = Math.sqrt(dx * dx + dy * dy) * SCALE;
      return <circle key={ent.id} cx={cx} cy={cy} r={r} fill="none" stroke="#9d4edd" strokeWidth={2} />;
    }
    return null;
  };

  return (
    <div className="h-full flex bg-forge-black">
      <div className="w-[200px] border-r border-forge-border bg-forge-dark/90 p-3 flex flex-col gap-3 flex-shrink-0">
        <div className="text-[9px] font-bold tracking-widest text-forge-accent">SKETCH TOOLS</div>
        <div className="grid grid-cols-2 gap-1.5">
          {([
            ['select', MousePointer2, 'Select'],
            ['line', Minus, 'Line'],
            ['rectangle', Square, 'Rect'],
            ['circle', Circle, 'Circle'],
          ] as const).map(([id, Icon, label]) => (
            <button
              key={id}
              onClick={() => { setTool(id); setDraft(null); }}
              className={`flex flex-col items-center gap-0.5 p-2 rounded border text-[8px] font-mono transition-colors
                ${tool === id
                  ? 'bg-forge-accent/20 text-forge-accent border-forge-accent/50'
                  : 'bg-forge-surface/30 text-forge-text-muted border-forge-border/50 hover:text-forge-text'}`}
            >
              <Icon size={14} />
              {label}
            </button>
          ))}
        </div>

        <div className="text-[9px] font-bold tracking-widest text-forge-text-muted mt-2">CONSTRAINTS</div>
        <div className="flex flex-col gap-1">
          <button onClick={() => addConstraint('horizontal')}
            className="flex items-center gap-1.5 px-2 py-1 rounded text-[8px] font-mono bg-forge-surface/30 border border-forge-border/50 hover:border-forge-accent/40">
            <AlignHorizontalJustifyCenter size={10} /> Horizontal
          </button>
          <button onClick={() => addConstraint('vertical')}
            className="flex items-center gap-1.5 px-2 py-1 rounded text-[8px] font-mono bg-forge-surface/30 border border-forge-border/50 hover:border-forge-accent/40">
            <AlignVerticalJustifyCenter size={10} /> Vertical
          </button>
          <button onClick={() => addConstraint('coincident')}
            className="flex items-center gap-1.5 px-2 py-1 rounded text-[8px] font-mono bg-forge-surface/30 border border-forge-border/50 hover:border-forge-accent/40">
            <Ruler size={10} /> Coincident
          </button>
        </div>

        <div className="mt-auto flex flex-col gap-2">
          <button onClick={runSolve} disabled={busy || entities.length === 0}
            className="flex items-center justify-center gap-1.5 py-2 rounded bg-forge-accent/20 text-forge-accent border border-forge-accent/50 text-[9px] font-bold font-mono disabled:opacity-40">
            <Play size={12} /> SOLVE SKETCH
          </button>
          <div className="flex items-center gap-2">
            <label className="text-[8px] font-mono text-forge-text-muted">Extrude mm</label>
            <input type="number" value={extrudeDist} onChange={e => setExtrudeDist(+e.target.value)}
              className="flex-1 bg-forge-black border border-forge-border rounded px-1.5 py-0.5 text-[9px] font-mono text-forge-text" />
          </div>
          <button onClick={runExtrude} disabled={busy || entities.length === 0}
            className="flex items-center justify-center gap-1.5 py-2 rounded bg-forge-purple/20 text-forge-purple border border-forge-purple/50 text-[9px] font-bold font-mono disabled:opacity-40">
            <Layers size={12} /> EXTRUDE
          </button>
        </div>
      </div>

      <div className="flex-1 flex flex-col min-w-0">
        <div className="h-8 border-b border-forge-border bg-forge-dark/60 px-3 flex items-center gap-4 text-[8px] font-mono text-forge-text-muted">
          <span>Entities: {entities.length}</span>
          <span>DOF: {solveResult?.dof ?? '—'}</span>
          <span>Solved: {solveResult?.solved != null ? (solveResult.solved ? 'yes' : 'no') : '—'}</span>
          {extrudeResult?.volume_m3 != null && (
            <span className="text-forge-green">Volume: {(extrudeResult.volume_m3 as number * 1e9).toFixed(1)} mm³</span>
          )}
        </div>
        <div className="flex-1 flex items-center justify-center overflow-auto p-4 bg-[#0a0e14]">
          <svg
            ref={svgRef}
            width={CANVAS_W}
            height={CANVAS_H}
            className="border border-forge-border/50 rounded bg-forge-black cursor-crosshair"
            onClick={handleCanvasClick}
            onMouseMove={handleMouseMove}
          >
            {Array.from({ length: CANVAS_W / GRID + 1 }).map((_, i) => (
              <line key={`v${i}`} x1={i * GRID} y1={0} x2={i * GRID} y2={CANVAS_H}
                stroke="#1a2332" strokeWidth={0.5} />
            ))}
            {Array.from({ length: CANVAS_H / GRID + 1 }).map((_, i) => (
              <line key={`h${i}`} x1={0} y1={i * GRID} x2={CANVAS_W} y2={i * GRID}
                stroke="#1a2332" strokeWidth={0.5} />
            ))}
            <line x1={0} y1={CANVAS_H / 2} x2={CANVAS_W} y2={CANVAS_H / 2} stroke="#334155" strokeWidth={1} />
            <line x1={CANVAS_W / 2} y1={0} x2={CANVAS_W / 2} y2={CANVAS_H} stroke="#334155" strokeWidth={1} />
            {entities.map(renderEntity)}
            {draft && tool === 'line' && (
              <line
                x1={sketchToScreen(draft.start[0], draft.start[1])[0]}
                y1={sketchToScreen(draft.start[0], draft.start[1])[1]}
                x2={sketchToScreen(draft.end[0], draft.end[1])[0]}
                y2={sketchToScreen(draft.end[0], draft.end[1])[1]}
                stroke="#00d4ff" strokeWidth={1} strokeDasharray="4 4" opacity={0.6}
              />
            )}
            {draft && tool === 'rectangle' && (
              <rect
                x={Math.min(sketchToScreen(draft.start[0], draft.start[1])[0], sketchToScreen(draft.end[0], draft.end[1])[0])}
                y={Math.min(sketchToScreen(draft.start[0], draft.start[1])[1], sketchToScreen(draft.end[0], draft.end[1])[1])}
                width={Math.abs(sketchToScreen(draft.end[0], draft.end[1])[0] - sketchToScreen(draft.start[0], draft.start[1])[0])}
                height={Math.abs(sketchToScreen(draft.end[0], draft.end[1])[1] - sketchToScreen(draft.start[0], draft.start[1])[1])}
                fill="none" stroke="#00d4ff" strokeWidth={1} strokeDasharray="4 4" opacity={0.6}
              />
            )}
            {draft && tool === 'circle' && (() => {
              const [cx, cy] = sketchToScreen(draft.start[0], draft.start[1]);
              const dx = draft.end[0] - draft.start[0];
              const dy = draft.end[1] - draft.start[1];
              const r = Math.sqrt(dx * dx + dy * dy) * SCALE;
              return <circle cx={cx} cy={cy} r={r} fill="none" stroke="#9d4edd" strokeWidth={1} strokeDasharray="4 4" opacity={0.6} />;
            })()}
          </svg>
        </div>
      </div>
    </div>
  );
};
