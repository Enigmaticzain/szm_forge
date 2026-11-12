import React, { useCallback, useEffect, useRef, useState } from 'react';
import * as THREE from 'three';
import {
  Circle, Minus, MousePointer2, Play, Square, Layers,
  Ruler, AlignHorizontalJustifyCenter, AlignVerticalJustifyCenter,
  Spline, Scissors, CircleDot, Move,
} from 'lucide-react';
import {
  extrudeSketch, solveSketch, SketchDto, SketchEntityDto, SketchSolveResult,
} from '../../services/forgeApi';
import { createSketchExtrusion } from '../../offline/geometry';
import type { SceneObject } from '../BodyDesign/InteractivePartEditor';
import { useProject } from '../../store/ProjectContext';

type SketchTool =
  | 'select'
  | 'line'
  | 'rectangle'
  | 'circle'
  | 'arc'
  | 'polyline'
  | 'dimension'
  | 'trim'
  | 'pan';

const BASE_W = 720;
const BASE_H = 480;
const GRID = 20;
const SCALE = 2; // pixels per mm at zoom=1

function snap(v: number) {
  return Math.round(v / GRID) * GRID;
}

function dist(a: [number, number], b: [number, number]) {
  const dx = a[0] - b[0];
  const dy = a[1] - b[1];
  return Math.sqrt(dx * dx + dy * dy);
}

function pointToSegDist(p: [number, number], a: [number, number], b: [number, number]) {
  const dx = b[0] - a[0];
  const dy = b[1] - a[1];
  const len2 = dx * dx + dy * dy || 1;
  let t = ((p[0] - a[0]) * dx + (p[1] - a[1]) * dy) / len2;
  t = Math.max(0, Math.min(1, t));
  return dist(p, [a[0] + t * dx, a[1] + t * dy]);
}

let nextId = 1;

export const SketchEditor: React.FC = () => {
  const { bodyParts, setBodyParts } = useProject();
  const [tool, setTool] = useState<SketchTool>('line');
  const [entities, setEntities] = useState<SketchEntityDto[]>([]);
  const [draft, setDraft] = useState<{
    start: [number, number];
    mid?: [number, number];
    end: [number, number];
    points?: [number, number][];
    stage?: number;
  } | null>(null);
  const [solveResult, setSolveResult] = useState<SketchSolveResult | null>(null);
  const [extrudeDist, setExtrudeDist] = useState(10);
  const [extrudeResult, setExtrudeResult] = useState<Record<string, unknown> | null>(null);
  const [busy, setBusy] = useState(false);
  const [viewBox, setViewBox] = useState({ x: 0, y: 0, w: BASE_W, h: BASE_H });
  const [spaceDown, setSpaceDown] = useState(false);
  const [dimensionText, setDimensionText] = useState<string | null>(null);
  const [dimensions, setDimensions] = useState<Array<{
    id: number;
    p1: [number, number];
    p2: [number, number];
    length: number;
  }>>([]);
  const svgRef = useRef<SVGSVGElement>(null);
  const panRef = useRef<{ sx: number; sy: number; vx: number; vy: number } | null>(null);

  const screenToSvg = useCallback((clientX: number, clientY: number): [number, number] => {
    const svg = svgRef.current;
    if (!svg) return [0, 0];
    const pt = svg.createSVGPoint();
    pt.x = clientX;
    pt.y = clientY;
    const ctm = svg.getScreenCTM();
    if (!ctm) return [0, 0];
    const local = pt.matrixTransform(ctm.inverse());
    return [local.x, local.y];
  }, []);

  const svgToSketch = useCallback((sx: number, sy: number): [number, number] => {
    const cx = BASE_W / 2;
    const cy = BASE_H / 2;
    return [(sx - cx) / SCALE, -(sy - cy) / SCALE];
  }, []);

  const sketchToSvg = useCallback((u: number, v: number): [number, number] => {
    const cx = BASE_W / 2;
    const cy = BASE_H / 2;
    return [cx + u * SCALE, cy - v * SCALE];
  }, []);

  const buildSketch = useCallback((): SketchDto => ({
    plane: { origin: [0, 0, 0], normal: [0, 0, 1] },
    entities,
    constraints: [],
    dimensions: dimensions.map(d => ({
      type: 'distance',
      p1: d.p1,
      p2: d.p2,
      value: d.length,
    })),
  }), [entities, dimensions]);

  const pointerToSketch = (e: React.MouseEvent | MouseEvent): [number, number] => {
    const [sx, sy] = screenToSvg(e.clientX, e.clientY);
    const [u, v] = svgToSketch(sx, sy);
    return [snap(u * SCALE) / SCALE, snap(v * SCALE) / SCALE];
  };

  const handleWheel = (e: React.WheelEvent<SVGSVGElement>) => {
    e.preventDefault();
    const factor = e.deltaY > 0 ? 1.12 : 1 / 1.12;
    const [mx, my] = screenToSvg(e.clientX, e.clientY);
    setViewBox(vb => {
      const nw = Math.min(BASE_W * 8, Math.max(BASE_W * 0.2, vb.w * factor));
      const nh = (nw / vb.w) * vb.h;
      const rx = (mx - vb.x) / vb.w;
      const ry = (my - vb.y) / vb.h;
      return {
        x: mx - rx * nw,
        y: my - ry * nh,
        w: nw,
        h: nh,
      };
    });
  };

  const beginPan = (e: React.MouseEvent) => {
    panRef.current = { sx: e.clientX, sy: e.clientY, vx: viewBox.x, vy: viewBox.y };
  };

  const handleMouseDown = (e: React.MouseEvent<SVGSVGElement>) => {
    const panTool = tool === 'pan' || spaceDown || e.button === 1 || e.button === 2;
    if (panTool) {
      e.preventDefault();
      beginPan(e);
    }
  };

  const handleMouseMove = (e: React.MouseEvent<SVGSVGElement>) => {
    if (panRef.current) {
      const svg = svgRef.current;
      if (!svg) return;
      const rect = svg.getBoundingClientRect();
      const scaleX = viewBox.w / rect.width;
      const scaleY = viewBox.h / rect.height;
      const dx = (e.clientX - panRef.current.sx) * scaleX;
      const dy = (e.clientY - panRef.current.sy) * scaleY;
      setViewBox(vb => ({
        ...vb,
        x: panRef.current!.vx - dx,
        y: panRef.current!.vy - dy,
      }));
      return;
    }

    if (!draft) return;
    const pt = pointerToSketch(e);
    setDraft(d => (d ? { ...d, end: pt } : null));
    if (tool === 'dimension' || tool === 'line' || tool === 'polyline') {
      const start = draft.start;
      setDimensionText(`${dist(start, pt).toFixed(1)} mm`);
    }
  };

  const handleMouseUp = () => {
    panRef.current = null;
  };

  const handleCanvasClick = (e: React.MouseEvent<SVGSVGElement>) => {
    if (panRef.current || e.button !== 0 || spaceDown || tool === 'pan') return;
    const pt = pointerToSketch(e);

    if (tool === 'trim') {
      let bestId: number | null = null;
      let bestDist = 4; // mm
      for (const ent of entities) {
        if (ent.type === 'line' && ent.p2) {
          const d = pointToSegDist(pt, ent.p1, ent.p2);
          if (d < bestDist) {
            bestDist = d;
            bestId = ent.id;
          }
        } else if (ent.type === 'circle' && ent.p2) {
          const r = dist(ent.p1, ent.p2);
          const d = Math.abs(dist(pt, ent.p1) - r);
          if (d < bestDist) {
            bestDist = d;
            bestId = ent.id;
          }
        } else if (ent.type === 'arc' && ent.p2 && ent.p3) {
          const r = dist(ent.p1, ent.p2);
          const d = Math.abs(dist(pt, ent.p1) - r);
          if (d < bestDist) {
            bestDist = d;
            bestId = ent.id;
          }
        }
      }
      if (bestId != null) {
        setEntities(prev => prev.filter(ent => ent.id !== bestId));
        setDimensionText('Segment trimmed');
      }
      return;
    }

    if (tool === 'dimension') {
      if (!draft) {
        setDraft({ start: pt, end: pt });
      } else {
        const length = dist(draft.start, pt);
        setDimensions(prev => [
          ...prev,
          { id: nextId++, p1: draft.start, p2: pt, length },
        ]);
        setDimensionText(`${length.toFixed(1)} mm`);
        setDraft(null);
      }
      return;
    }

    if (tool === 'line') {
      if (!draft) {
        setDraft({ start: pt, end: pt });
      } else {
        setEntities(prev => [...prev, { id: nextId++, type: 'line', p1: draft.start, p2: pt }]);
        setDimensionText(`${dist(draft.start, pt).toFixed(1)} mm`);
        setDraft(null);
      }
      return;
    }

    if (tool === 'polyline') {
      if (!draft) {
        setDraft({ start: pt, end: pt, points: [pt] });
      } else {
        const points = [...(draft.points ?? [draft.start]), pt];
        if (e.detail === 2 || (points.length > 1 && dist(pt, points[0]) < 2)) {
          const segs: SketchEntityDto[] = [];
          for (let i = 0; i < points.length - 1; i++) {
            segs.push({ id: nextId++, type: 'line', p1: points[i], p2: points[i + 1] });
          }
          if (dist(pt, points[0]) < 2 && points.length > 2) {
            segs.push({ id: nextId++, type: 'line', p1: points[points.length - 1], p2: points[0] });
          }
          setEntities(prev => [...prev, ...segs]);
          setDraft(null);
          setDimensionText(`Polyline ${segs.length} segs`);
        } else {
          setDraft({ start: pt, end: pt, points });
        }
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
        const r = dist(draft.start, pt);
        if (r > 0.5) {
          setEntities(prev => [...prev, {
            id: nextId++,
            type: 'circle',
            p1: draft.start,
            p2: [draft.start[0] + r, draft.start[1]],
          }]);
          setDimensionText(`⌀ ${(r * 2).toFixed(1)} mm`);
        }
        setDraft(null);
      }
      return;
    }

    if (tool === 'arc') {
      if (!draft) {
        setDraft({ start: pt, end: pt, stage: 0 });
      } else if ((draft.stage ?? 0) === 0) {
        setDraft({ start: draft.start, mid: pt, end: pt, stage: 1 });
      } else {
        const center = draft.start;
        const p2 = draft.mid ?? draft.end;
        setEntities(prev => [...prev, {
          id: nextId++,
          type: 'arc',
          p1: center,
          p2,
          p3: pt,
        }]);
        setDimensionText(`R ${dist(center, p2).toFixed(1)} mm`);
        setDraft(null);
      }
    }
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
    if (result?.entities) setEntities(result.entities);
    setSolveResult(result);
    setBusy(false);
  };

  const runExtrude = async () => {
    const extrusion = createSketchExtrusion(buildSketch(), extrudeDist);
    if (!extrusion) {
      setExtrudeResult({ error: 'Draw a closed line profile or a circle before extruding.' });
      return;
    }

    setBusy(true);
    try {
      const result = await extrudeSketch(buildSketch(), extrudeDist / 1000);
      if (!result || result.ok === false) {
        extrusion.geometry.dispose();
        setExtrudeResult({
          error: typeof result?.error === 'string' ? result.error : 'The extrusion could not be created.',
        });
        return;
      }

      extrusion.geometry.computeBoundingBox();
      const bounds = extrusion.geometry.boundingBox;
      if (!bounds) {
        extrusion.geometry.dispose();
        setExtrudeResult({ error: 'The extrusion did not produce usable geometry.' });
        return;
      }

      const size = new THREE.Vector3();
      bounds.getSize(size);
      const name = `Sketch Extrusion ${bodyParts.length + 1}`;
      const part: SceneObject = {
        id: `sketch_extrusion_${Date.now()}`,
        name,
        type: 'custom',
        position: { x: 0, y: 0, z: 0 },
        size: { x: size.x, y: size.y, z: size.z },
        customGeometry: extrusion.geometry,
      };
      setBodyParts(previous => [...previous, part]);
      setExtrudeResult({
        ...result,
        volume_m3: extrusion.volumeM3,
        createdPart: name,
      });
    } catch (error) {
      extrusion.geometry.dispose();
      setExtrudeResult({
        error: error instanceof Error ? error.message : 'The extrusion could not be created.',
      });
    } finally {
      setBusy(false);
    }
  };

  useEffect(() => {
    const onKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'Escape') {
        setDraft(null);
        setDimensionText(null);
      }
      if (e.code === 'Space') {
        e.preventDefault();
        setSpaceDown(true);
      }
      if (e.key === 'Enter' && tool === 'polyline' && draft?.points && draft.points.length > 1) {
        const points = draft.points;
        const segs: SketchEntityDto[] = [];
        for (let i = 0; i < points.length - 1; i++) {
          segs.push({ id: nextId++, type: 'line', p1: points[i], p2: points[i + 1] });
        }
        setEntities(prev => [...prev, ...segs]);
        setDraft(null);
      }
    };
    const onKeyUp = (e: KeyboardEvent) => {
      if (e.code === 'Space') setSpaceDown(false);
    };
    window.addEventListener('keydown', onKeyDown);
    window.addEventListener('keyup', onKeyUp);
    return () => {
      window.removeEventListener('keydown', onKeyDown);
      window.removeEventListener('keyup', onKeyUp);
    };
  }, [tool, draft]);

  const arcPath = (center: [number, number], pStart: [number, number], pEnd: [number, number]) => {
    const r = dist(center, pStart);
    const [sx, sy] = sketchToSvg(pStart[0], pStart[1]);
    const [ex, ey] = sketchToSvg(pEnd[0], pEnd[1]);
    const a0 = Math.atan2(pStart[1] - center[1], pStart[0] - center[0]);
    const a1 = Math.atan2(pEnd[1] - center[1], pEnd[0] - center[0]);
    let delta = a1 - a0;
    while (delta <= -Math.PI) delta += Math.PI * 2;
    while (delta > Math.PI) delta -= Math.PI * 2;
    const large = Math.abs(delta) > Math.PI ? 1 : 0;
    const sweep = delta < 0 ? 1 : 0; // SVG y is flipped via sketch mapping
    return `M ${sx} ${sy} A ${r * SCALE} ${r * SCALE} 0 ${large} ${sweep} ${ex} ${ey}`;
  };

  const renderEntity = (ent: SketchEntityDto) => {
    if (ent.type === 'line' && ent.p2) {
      const [x1, y1] = sketchToSvg(ent.p1[0], ent.p1[1]);
      const [x2, y2] = sketchToSvg(ent.p2[0], ent.p2[1]);
      return (
        <line key={ent.id} x1={x1} y1={y1} x2={x2} y2={y2}
          stroke="#00d4ff" strokeWidth={2} vectorEffect="non-scaling-stroke" />
      );
    }
    if (ent.type === 'circle' && ent.p2) {
      const [cx, cy] = sketchToSvg(ent.p1[0], ent.p1[1]);
      const r = dist(ent.p1, ent.p2) * SCALE;
      return (
        <circle key={ent.id} cx={cx} cy={cy} r={r} fill="none" stroke="#9d4edd"
          strokeWidth={2} vectorEffect="non-scaling-stroke" />
      );
    }
    if (ent.type === 'arc' && ent.p2 && ent.p3) {
      return (
        <path key={ent.id} d={arcPath(ent.p1, ent.p2, ent.p3)} fill="none"
          stroke="#f59e0b" strokeWidth={2} vectorEffect="non-scaling-stroke" />
      );
    }
    return null;
  };

  const tools: Array<[SketchTool, React.ComponentType<{ size?: number }>, string]> = [
    ['select', MousePointer2, 'Select'],
    ['line', Minus, 'Line'],
    ['polyline', Spline, 'Poly'],
    ['rectangle', Square, 'Rect'],
    ['circle', Circle, 'Circle'],
    ['arc', CircleDot, 'Arc'],
    ['dimension', Ruler, 'Dim'],
    ['trim', Scissors, 'Trim'],
    ['pan', Move, 'Pan'],
  ];

  return (
    <div className="h-full flex bg-forge-black">
      <div className="w-[200px] border-r border-forge-border bg-forge-dark/90 p-3 flex flex-col gap-3 flex-shrink-0">
        <div className="text-[9px] font-bold tracking-widest text-forge-accent">SKETCH TOOLS</div>
        <div className="grid grid-cols-2 gap-1.5">
          {tools.map(([id, Icon, label]) => (
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
          {dimensionText && <span className="text-forge-accent">Dim: {dimensionText}</span>}
          {extrudeResult?.volume_m3 != null && (
            <span className="text-forge-green">Volume: {(extrudeResult.volume_m3 as number * 1e9).toFixed(1)} mm³</span>
          )}
          {typeof extrudeResult?.createdPart === 'string' && (
            <span className="text-forge-green">Created: {extrudeResult.createdPart}</span>
          )}
          {typeof extrudeResult?.error === 'string' && (
            <span role="alert" className="text-forge-red">{extrudeResult.error}</span>
          )}
          <span className="ml-auto">Wheel zoom · MMB/RMB/Space+drag pan · Enter finish poly</span>
        </div>
        <div className="flex-1 overflow-hidden p-2 bg-[#0a0e14]">
          <svg
            ref={svgRef}
            viewBox={`${viewBox.x} ${viewBox.y} ${viewBox.w} ${viewBox.h}`}
            className={`w-full h-full border border-forge-border/50 rounded bg-forge-black ${
              tool === 'pan' || spaceDown ? 'cursor-grab' : 'cursor-crosshair'
            }`}
            onClick={handleCanvasClick}
            onMouseDown={handleMouseDown}
            onMouseMove={handleMouseMove}
            onMouseUp={handleMouseUp}
            onMouseLeave={handleMouseUp}
            onWheel={handleWheel}
            onContextMenu={e => e.preventDefault()}
          >
            {Array.from({ length: Math.ceil(BASE_W / GRID) + 1 }).map((_, i) => (
              <line key={`v${i}`} x1={i * GRID} y1={0} x2={i * GRID} y2={BASE_H}
                stroke="#1a2332" strokeWidth={0.5} />
            ))}
            {Array.from({ length: Math.ceil(BASE_H / GRID) + 1 }).map((_, i) => (
              <line key={`h${i}`} x1={0} y1={i * GRID} x2={BASE_W} y2={i * GRID}
                stroke="#1a2332" strokeWidth={0.5} />
            ))}
            <line x1={0} y1={BASE_H / 2} x2={BASE_W} y2={BASE_H / 2} stroke="#334155" strokeWidth={1} />
            <line x1={BASE_W / 2} y1={0} x2={BASE_W / 2} y2={BASE_H} stroke="#334155" strokeWidth={1} />
            {entities.map(renderEntity)}
            {dimensions.map(d => {
              const [x1, y1] = sketchToSvg(d.p1[0], d.p1[1]);
              const [x2, y2] = sketchToSvg(d.p2[0], d.p2[1]);
              const mx = (x1 + x2) / 2;
              const my = (y1 + y2) / 2;
              return (
                <g key={`dim-${d.id}`}>
                  <line x1={x1} y1={y1} x2={x2} y2={y2} stroke="#22c55e" strokeWidth={1}
                    strokeDasharray="3 2" vectorEffect="non-scaling-stroke" />
                  <text x={mx} y={my - 4} fill="#22c55e" fontSize={10} textAnchor="middle"
                    className="font-mono">{d.length.toFixed(1)}</text>
                </g>
              );
            })}
            {draft && (tool === 'line' || tool === 'dimension') && (
              <line
                x1={sketchToSvg(draft.start[0], draft.start[1])[0]}
                y1={sketchToSvg(draft.start[0], draft.start[1])[1]}
                x2={sketchToSvg(draft.end[0], draft.end[1])[0]}
                y2={sketchToSvg(draft.end[0], draft.end[1])[1]}
                stroke="#00d4ff" strokeWidth={1} strokeDasharray="4 4" opacity={0.6}
                vectorEffect="non-scaling-stroke"
              />
            )}
            {draft && tool === 'polyline' && draft.points && (
              <>
                {draft.points.map((p, i) => {
                  if (i === 0) return null;
                  const a = draft.points![i - 1];
                  const [x1, y1] = sketchToSvg(a[0], a[1]);
                  const [x2, y2] = sketchToSvg(p[0], p[1]);
                  return <line key={`pp${i}`} x1={x1} y1={y1} x2={x2} y2={y2}
                    stroke="#00d4ff" strokeWidth={1} opacity={0.7} vectorEffect="non-scaling-stroke" />;
                })}
                <line
                  x1={sketchToSvg(draft.start[0], draft.start[1])[0]}
                  y1={sketchToSvg(draft.start[0], draft.start[1])[1]}
                  x2={sketchToSvg(draft.end[0], draft.end[1])[0]}
                  y2={sketchToSvg(draft.end[0], draft.end[1])[1]}
                  stroke="#00d4ff" strokeWidth={1} strokeDasharray="4 4" opacity={0.6}
                  vectorEffect="non-scaling-stroke"
                />
              </>
            )}
            {draft && tool === 'rectangle' && (
              <rect
                x={Math.min(sketchToSvg(draft.start[0], draft.start[1])[0], sketchToSvg(draft.end[0], draft.end[1])[0])}
                y={Math.min(sketchToSvg(draft.start[0], draft.start[1])[1], sketchToSvg(draft.end[0], draft.end[1])[1])}
                width={Math.abs(sketchToSvg(draft.end[0], draft.end[1])[0] - sketchToSvg(draft.start[0], draft.start[1])[0])}
                height={Math.abs(sketchToSvg(draft.end[0], draft.end[1])[1] - sketchToSvg(draft.start[0], draft.start[1])[1])}
                fill="none" stroke="#00d4ff" strokeWidth={1} strokeDasharray="4 4" opacity={0.6}
                vectorEffect="non-scaling-stroke"
              />
            )}
            {draft && tool === 'circle' && (() => {
              const [cx, cy] = sketchToSvg(draft.start[0], draft.start[1]);
              const r = dist(draft.start, draft.end) * SCALE;
              return <circle cx={cx} cy={cy} r={r} fill="none" stroke="#9d4edd" strokeWidth={1}
                strokeDasharray="4 4" opacity={0.6} vectorEffect="non-scaling-stroke" />;
            })()}
            {draft && tool === 'arc' && (() => {
              if ((draft.stage ?? 0) === 0) {
                const [cx, cy] = sketchToSvg(draft.start[0], draft.start[1]);
                const r = dist(draft.start, draft.end) * SCALE;
                return <circle cx={cx} cy={cy} r={r} fill="none" stroke="#f59e0b" strokeWidth={1}
                  strokeDasharray="4 4" opacity={0.6} vectorEffect="non-scaling-stroke" />;
              }
              const mid = draft.mid ?? draft.end;
              return (
                <path d={arcPath(draft.start, mid, draft.end)} fill="none" stroke="#f59e0b"
                  strokeWidth={1} strokeDasharray="4 4" opacity={0.6} vectorEffect="non-scaling-stroke" />
              );
            })()}
          </svg>
        </div>
      </div>
    </div>
  );
};
