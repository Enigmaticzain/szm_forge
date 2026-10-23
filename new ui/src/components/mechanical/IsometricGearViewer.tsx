/**
 * IsometricGearViewer — Interactive compound gear-train visualisation.
 *
 * An SVG-based isometric assembly viewer that renders inside the
 * MechanicalWorkspace. Based on the Anatomy skill's rendering patterns:
 * - Isometric projection (45° azimuth, 30° elevation)
 * - Four-tone shading with accent highlight
 * - True 3D volume on gears and shafts via cylindrical side facets
 * - Multi-tier compound mounting eliminating co-planar gear collisions
 * - Animated gear rotation driven by kinematics with external mesh direction reversal
 * - Half-pitch meshing phase alignment so teeth mesh into gaps
 * - Click-to-select with property readout
 * - Explode / collapse assembly view with axis alignment guides
 * - Keyboard & pointer interaction with input-guarding
 * - Accessible ARIA labels and prefers-reduced-motion support
 *
 * The gear train is the compound 5-gear, 3-shaft, 6:1 reduction from
 * the reference demo, adapted to SZM Forge's UI conventions.
 */
import React, { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import {
  DEFAULT_GEAR_TRAIN,
  gearAngle,
  gearPositions,
  shaftAngles,
  shaftRPMs,
  type GearDef,
  type GearTrainModel,
} from './gearTrainModel';
import {
  createCamera,
  cylinderFacets,
  depthOf,
  gearPath,
  isoCirclePath,
  PALETTE,
  project,
  shaftPaths,
} from './isoEngine';
import { Play, Pause, RotateCcw, Maximize2, Minimize2, Info } from 'lucide-react';

// ── Constants ──────────────────────────────────────────────────

const SVG_W = 800;
const SVG_H = 500;
const INPUT_RPM = 120;
const GEAR_THICKNESS = 11;      // model units
const SHAFT_RADIUS = 6;
const BED_HEIGHT = 10;
const TIER_HEIGHT = 18;         // vertical spacing between gear tiers

/** Calculate vertical base Z for a gear based on its tier and explode state */
export function getGearZ(gear: GearDef, exploded: boolean): number {
  const tier = gear.tier ?? (gear.shaft === 0 ? 0 : gear.shaft === 1 && gear.radius === 32 ? 1 : gear.shaft === 2 && gear.radius === 32 ? 2 : gear.shaft === 2 ? 1 : 0);
  const baseZ = BED_HEIGHT + 6 + tier * TIER_HEIGHT;
  return exploded ? baseZ + tier * 32 : baseZ;
}

// ── Component ──────────────────────────────────────────────────

interface Props {
  model?: GearTrainModel;
}

export const IsometricGearViewer: React.FC<Props> = ({ model = DEFAULT_GEAR_TRAIN }) => {
  // Respect prefers-reduced-motion initially and on changes
  const [playing, setPlaying] = useState<boolean>(() => {
    if (typeof window !== 'undefined' && window.matchMedia) {
      return !window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    }
    return true;
  });

  const [speed, setSpeed] = useState(1);
  const [exploded, setExploded] = useState(false);
  const [selectedGear, setSelectedGear] = useState<number | null>(null);
  const [inputAngle, setInputAngle] = useState(0);
  const rafRef = useRef<number>(0);
  const lastTimeRef = useRef<number>(0);
  const svgRef = useRef<SVGSVGElement>(null);
  const prefersReduced = useRef(false);

  useEffect(() => {
    if (typeof window === 'undefined' || !window.matchMedia) return;
    const mq = window.matchMedia('(prefers-reduced-motion: reduce)');
    prefersReduced.current = mq.matches;
    if (mq.matches) setPlaying(false);

    const handler = (e: MediaQueryListEvent) => {
      prefersReduced.current = e.matches;
      if (e.matches) setPlaying(false);
    };
    mq.addEventListener('change', handler);
    return () => mq.removeEventListener('change', handler);
  }, []);

  // Camera (Anatomy dimetric / isometric convention)
  const cam = useMemo(() => createCamera({
    width: SVG_W,
    height: SVG_H,
    scale: 1.4,
    azimuth: 45,
    elevation: 30,
  }), []);

  // Gear positions (2D layout, then projected)
  const positions = useMemo(() => gearPositions(model), [model]);

  // Center the layout
  const centerOffsetX = useMemo(() => {
    const xs = positions.map(p => p.x);
    return -(Math.min(...xs) + Math.max(...xs)) / 2;
  }, [positions]);

  // Shaft angles & RPMs
  const angles = useMemo(() => shaftAngles(model, inputAngle), [model, inputAngle]);
  const rpms = useMemo(() => shaftRPMs(model, INPUT_RPM), [model]);

  // Animation loop
  useEffect(() => {
    if (!playing || prefersReduced.current) return;
    const tick = (now: number) => {
      if (lastTimeRef.current === 0) lastTimeRef.current = now;
      const dt = (now - lastTimeRef.current) / 1000;
      lastTimeRef.current = now;
      const omega = (INPUT_RPM / 60) * 2 * Math.PI * speed;
      setInputAngle(prev => prev + omega * dt);
      rafRef.current = requestAnimationFrame(tick);
    };
    rafRef.current = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(rafRef.current);
  }, [playing, speed]);

  // Keyboard navigation (with form input guard)
  useEffect(() => {
    const handler = (e: KeyboardEvent) => {
      const target = e.target as HTMLElement | null;
      if (target && (target.tagName === 'INPUT' || target.tagName === 'TEXTAREA' || target.isContentEditable)) {
        return;
      }

      if (e.key === ' ') { e.preventDefault(); setPlaying(p => !p); }
      if (e.key === 'ArrowRight') setSpeed(s => Math.min(s + 0.5, 5));
      if (e.key === 'ArrowLeft') setSpeed(s => Math.max(s - 0.5, 0.1));
      if (e.key === 'r' || e.key === 'R') { setInputAngle(0); lastTimeRef.current = 0; }
      if (e.key === 'e' || e.key === 'E') setExploded(v => !v);
      if (e.key === 'Escape') setSelectedGear(null);
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, []);

  // Shaft info and heights for realistic mechanical mounting
  const uniqueShaftData = useMemo(() => {
    const shaftMap = new Map<number, { shaft: number; x: number; y: number; maxZ: number }>();
    model.gears.forEach((gear, idx) => {
      const pos = positions[idx];
      const gearTop = getGearZ(gear, exploded) + GEAR_THICKNESS;
      const existing = shaftMap.get(gear.shaft);
      if (!existing) {
        shaftMap.set(gear.shaft, {
          shaft: gear.shaft,
          x: pos.x + centerOffsetX,
          y: pos.y,
          maxZ: gearTop + 6,
        });
      } else {
        if (gearTop + 6 > existing.maxZ) {
          existing.maxZ = gearTop + 6;
        }
      }
    });
    return Array.from(shaftMap.values()).sort((a, b) => {
      return depthOf(b.x, b.y, b.maxZ / 2, cam) - depthOf(a.x, a.y, a.maxZ / 2, cam);
    });
  }, [model, positions, centerOffsetX, cam, exploded]);

  // Sort gears by true 3D depth for painter's order
  const sortedIndices = useMemo(() => {
    return model.gears
      .map((g, i) => {
        const zBase = getGearZ(g, exploded);
        return {
          i,
          depth: depthOf(positions[i].x + centerOffsetX, positions[i].y, zBase + GEAR_THICKNESS / 2, cam),
        };
      })
      .sort((a, b) => b.depth - a.depth) // far first
      .map(d => d.i);
  }, [model, positions, centerOffsetX, cam, exploded]);

  const handleGearClick = useCallback((idx: number) => {
    setSelectedGear(prev => prev === idx ? null : idx);
  }, []);

  const selectedInfo = selectedGear !== null ? model.gears[selectedGear] : null;
  const selectedRPM = selectedGear !== null ? rpms[model.gears[selectedGear].shaft] : null;

  return (
    <div className="w-full h-full flex flex-col bg-forge-dark/50 relative overflow-hidden">
      {/* Top bar */}
      <div className="flex items-center justify-between px-4 py-2 border-b border-forge-border/30 bg-forge-dark/80 z-10">
        <div className="flex items-center gap-3">
          <span className="text-[10px] font-bold tracking-widest text-forge-accent font-mono">
            COMPOUND GEAR TRAIN · {model.reduction}:1 REDUCTION
          </span>
          <span className="text-[9px] text-forge-text-muted font-mono">
            {model.gears.length} gears · {uniqueShaftData.length} shafts · {model.directionNote}
          </span>
        </div>
        <div className="flex items-center gap-2">
          <button
            onClick={() => setExploded(v => !v)}
            className={`p-1.5 rounded border transition-colors ${
              exploded
                ? 'bg-forge-accent/20 text-forge-accent border-forge-accent/40'
                : 'text-forge-text-muted border-forge-border hover:text-forge-text'
            }`}
            title={exploded ? 'Collapse assembly (E)' : 'Explode assembly (E)'}
            aria-pressed={exploded}
          >
            {exploded ? <Minimize2 size={14} /> : <Maximize2 size={14} />}
          </button>
          <button
            onClick={() => { setInputAngle(0); lastTimeRef.current = 0; }}
            className="p-1.5 rounded border border-forge-border text-forge-text-muted hover:text-forge-text transition-colors"
            title="Reset rotation (R)"
          >
            <RotateCcw size={14} />
          </button>
          <button
            onClick={() => setPlaying(p => !p)}
            className={`p-1.5 rounded border transition-colors ${
              playing
                ? 'bg-forge-green/20 text-forge-green border-forge-green/40'
                : 'text-forge-text-muted border-forge-border hover:text-forge-text'
            }`}
            title={playing ? 'Pause (Space)' : 'Play (Space)'}
            aria-pressed={playing}
          >
            {playing ? <Pause size={14} /> : <Play size={14} />}
          </button>
        </div>
      </div>

      {/* SVG viewport */}
      <div className="flex-1 relative">
        <svg
          ref={svgRef}
          viewBox={`0 0 ${SVG_W} ${SVG_H}`}
          className="w-full h-full"
          style={{ background: '#141416' }}
          role="img"
          aria-label="Interactive compound gear train — isometric assembly view"
        >
          <defs>
            <radialGradient id="bed-halo" cx="50%" cy="50%" r="50%">
              <stop offset="0%" stopColor="#000000" stopOpacity="0.5" />
              <stop offset="60%" stopColor="#000000" stopOpacity="0.2" />
              <stop offset="100%" stopColor="#000000" stopOpacity="0" />
            </radialGradient>
            <filter id="glow">
              <feGaussianBlur stdDeviation="2" result="blur" />
              <feMerge>
                <feMergeNode in="blur" />
                <feMergeNode in="SourceGraphic" />
              </feMerge>
            </filter>
          </defs>

          {/* Machine bed (base plate with feet, ticks & machined bearing mounts) */}
          <MachineBase cam={cam} positions={positions} centerOffsetX={centerOffsetX} />

          {/* Exploded alignment centerlines along shaft axes */}
          {exploded && uniqueShaftData.map((shaft, sIdx) => {
            const [b0x, b0y] = project(shaft.x, 0, BED_HEIGHT, cam);
            const [b1x, b1y] = project(shaft.x, 0, shaft.maxZ, cam);
            return (
              <line
                key={`shaft-axis-${sIdx}`}
                x1={b0x.toFixed(1)}
                y1={b0y.toFixed(1)}
                x2={b1x.toFixed(1)}
                y2={b1y.toFixed(1)}
                stroke={PALETTE.accent}
                strokeWidth="1"
                strokeDasharray="4 3"
                opacity="0.45"
              />
            );
          })}

          {/* Shaft cylinders */}
          {uniqueShaftData.map((shaft, sIdx) => {
            const isShaftSelected = selectedGear !== null && model.gears[selectedGear].shaft === shaft.shaft;
            return (
              <ShaftElement
                key={`shaft-${sIdx}`}
                cx={shaft.x}
                cy={shaft.y}
                z0={BED_HEIGHT}
                z1={shaft.maxZ}
                cam={cam}
                highlighted={isShaftSelected}
              />
            );
          })}

          {/* Gears in true painter's order */}
          {sortedIndices.map(idx => {
            const gear = model.gears[idx];
            const pos = positions[idx];
            const gx = pos.x + centerOffsetX;
            const gy = pos.y;
            const angle = gearAngle(model, idx, angles);
            const zBase = getGearZ(gear, exploded);
            const isSelected = selectedGear === idx;

            return (
              <g
                key={idx}
                style={{ cursor: 'pointer' }}
                onClick={() => handleGearClick(idx)}
                role="button"
                aria-label={`Gear ${gear.label}: ${gear.teeth} teeth, radius ${gear.radius}`}
                tabIndex={0}
                onKeyDown={e => {
                  if (e.key === 'Enter' || e.key === ' ') {
                    e.preventDefault();
                    handleGearClick(idx);
                  }
                }}
              >
                {/* 3D Gear disc with four-tone shaded cylindrical side facets */}
                <GearDisc
                  gear={gear}
                  gx={gx}
                  gy={gy}
                  angle={angle}
                  zBase={zBase}
                  cam={cam}
                  selected={isSelected}
                />
              </g>
            );
          })}

          {/* Selection highlight ring */}
          {selectedGear !== null && (() => {
            const selGear = model.gears[selectedGear];
            const selZ = getGearZ(selGear, exploded) + GEAR_THICKNESS / 2;
            const [scx, scy] = project(positions[selectedGear].x + centerOffsetX, 0, selZ, cam);
            return (
              <circle
                cx={scx}
                cy={scy}
                r={selGear.radius * cam.scale + 10}
                fill="none"
                stroke={PALETTE.accent}
                strokeWidth="1.5"
                strokeDasharray="4 3"
                opacity="0.7"
                filter="url(#glow)"
              />
            );
          })()}

          {/* Card: fig label & live readout */}
          <CardOverlay
            cam={cam}
            playing={playing}
            speed={speed}
            rpms={rpms}
            reduction={model.reduction}
          />
        </svg>

        {/* Property panel overlay */}
        {selectedInfo && (
          <div className="absolute top-4 right-4 w-56 bg-forge-dark/95 border border-forge-border/50 rounded-lg shadow-xl p-3 z-20 backdrop-blur-sm">
            <div className="flex items-center justify-between mb-2">
              <span className="text-[10px] font-bold tracking-widest text-forge-accent font-mono">
                {selectedInfo.label}
              </span>
              <button
                onClick={() => setSelectedGear(null)}
                className="text-forge-text-muted hover:text-forge-red text-xs"
                aria-label="Close properties"
              >
                ✕
              </button>
            </div>
            <div className="space-y-1.5 text-[10px] font-mono">
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Teeth</span>
                <span className="text-forge-text">{selectedInfo.teeth}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Pitch Radius</span>
                <span className="text-forge-text">{selectedInfo.radius} mm</span>
              </div>
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Shaft</span>
                <span className="text-forge-text">#{selectedInfo.shaft + 1}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Tier</span>
                <span className="text-forge-text">Tier #{((selectedInfo.tier ?? 0) + 1)}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Speed</span>
                <span className="text-forge-green">{selectedRPM?.toFixed(1)} RPM</span>
              </div>
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Module</span>
                <span className="text-forge-text">
                  {(2 * selectedInfo.radius / selectedInfo.teeth).toFixed(2)}
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-forge-text-muted">Circ. Pitch</span>
                <span className="text-forge-text">
                  {(Math.PI * 2 * selectedInfo.radius / selectedInfo.teeth).toFixed(2)} mm
                </span>
              </div>
              {selectedInfo.highlight && (
                <div className="mt-1.5 pt-1.5 border-t border-forge-border/30">
                  <span className="text-forge-accent text-[9px]">★ Compound junction gear</span>
                </div>
              )}
            </div>
          </div>
        )}
      </div>

      {/* Bottom readout bar */}
      <div className="flex items-center justify-between px-4 py-1.5 border-t border-forge-border/30 bg-forge-dark/80 z-10">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-1.5">
            <Info size={10} className="text-forge-text-muted" />
            <span className="text-[9px] font-mono text-forge-text-muted">
              Click gear to inspect · Space to pause · ←→ speed · E explode · R reset
            </span>
          </div>
        </div>
        <div className="flex items-center gap-4">
          <span className="text-[9px] font-mono text-forge-text-muted">
            SPEED: {speed.toFixed(1)}×
          </span>
          <span className="text-[9px] font-mono text-forge-green">
            INPUT: {(INPUT_RPM * speed).toFixed(0)} RPM
          </span>
          <span className="text-[9px] font-mono text-forge-accent">
            OUTPUT: {(rpms[rpms.length - 1] * speed).toFixed(1)} RPM
          </span>
        </div>
      </div>
    </div>
  );
};

// ── Sub-components ─────────────────────────────────────────────

/** Machine bed / base plate with Anatomy craft recipe */
const MachineBase: React.FC<{
  cam: ReturnType<typeof createCamera>;
  positions: Array<{ x: number; y: number }>;
  centerOffsetX: number;
}> = React.memo(({ cam, positions, centerOffsetX }) => {
  const xs = positions.map(p => p.x + centerOffsetX);
  const minX = Math.min(...xs) - 45;
  const maxX = Math.max(...xs) + 45;
  const bedD = 80;

  // Bed top face (at BED_HEIGHT)
  const topCorners = [
    [minX, -bedD / 2, BED_HEIGHT],
    [maxX, -bedD / 2, BED_HEIGHT],
    [maxX, bedD / 2, BED_HEIGHT],
    [minX, bedD / 2, BED_HEIGHT],
  ] as const;

  const topPath = topCorners
    .map(([x, y, z], i) => {
      const [sx, sy] = project(x, y, z, cam);
      return `${i === 0 ? 'M' : 'L'} ${sx.toFixed(1)},${sy.toFixed(1)}`;
    })
    .join(' ') + ' Z';

  // Front face
  const frontCorners = [
    [minX, bedD / 2, BED_HEIGHT],
    [maxX, bedD / 2, BED_HEIGHT],
    [maxX, bedD / 2, 0],
    [minX, bedD / 2, 0],
  ] as const;

  const frontPath = frontCorners
    .map(([x, y, z], i) => {
      const [sx, sy] = project(x, y, z, cam);
      return `${i === 0 ? 'M' : 'L'} ${sx.toFixed(1)},${sy.toFixed(1)}`;
    })
    .join(' ') + ' Z';

  // Right face
  const rightCorners = [
    [maxX, -bedD / 2, BED_HEIGHT],
    [maxX, bedD / 2, BED_HEIGHT],
    [maxX, bedD / 2, 0],
    [maxX, -bedD / 2, 0],
  ] as const;

  const rightPath = rightCorners
    .map(([x, y, z], i) => {
      const [sx, sy] = project(x, y, z, cam);
      return `${i === 0 ? 'M' : 'L'} ${sx.toFixed(1)},${sy.toFixed(1)}`;
    })
    .join(' ') + ' Z';

  // Inset groove on top
  const grooveInset = 5;
  const grooveCorners = [
    [minX + grooveInset, -bedD / 2 + grooveInset, BED_HEIGHT + 0.1],
    [maxX - grooveInset, -bedD / 2 + grooveInset, BED_HEIGHT + 0.1],
    [maxX - grooveInset, bedD / 2 - grooveInset, BED_HEIGHT + 0.1],
    [minX + grooveInset, bedD / 2 - grooveInset, BED_HEIGHT + 0.1],
  ] as const;

  const groovePath = grooveCorners
    .map(([x, y, z], i) => {
      const [sx, sy] = project(x, y, z, cam);
      return `${i === 0 ? 'M' : 'L'} ${sx.toFixed(1)},${sy.toFixed(1)}`;
    })
    .join(' ') + ' Z';

  // Drop shadow halo underneath plate
  const [haloLeft, haloTop] = project(minX - 10, bedD / 2 + 10, -2, cam);
  const [haloRight, haloBottom] = project(maxX + 10, -bedD / 2 - 10, -2, cam);

  // Calibration tick marks along the front top edge
  const ticks: JSX.Element[] = [];
  const tickStep = 15;
  for (let x = minX + 15; x <= maxX - 15; x += tickStep) {
    const isMajor = Math.round((x - minX) / tickStep) % 2 === 0;
    const tickLen = isMajor ? 4 : 2.5;
    const [t0x, t0y] = project(x, bedD / 2, BED_HEIGHT + 0.2, cam);
    const [t1x, t1y] = project(x, bedD / 2 - tickLen, BED_HEIGHT + 0.2, cam);
    ticks.push(
      <line
        key={`tick-${x}`}
        x1={t0x.toFixed(1)}
        y1={t0y.toFixed(1)}
        x2={t1x.toFixed(1)}
        y2={t1y.toFixed(1)}
        stroke={isMajor ? '#9ca3af' : '#6b7280'}
        strokeWidth={isMajor ? '0.8' : '0.5'}
      />
    );
  }

  // Unique shaft X coordinates
  const uniqueShaftXs = Array.from(new Set(positions.map(p => p.x + centerOffsetX)));

  return (
    <g aria-label="Machine bed">
      {/* Soft ground shadow underneath */}
      <ellipse
        cx={(haloLeft + haloRight) / 2}
        cy={(haloTop + haloBottom) / 2 + 15}
        rx={Math.abs(haloRight - haloLeft) * 0.55}
        ry={25}
        fill="url(#bed-halo)"
      />

      {/* Base plate faces with four-tone tones */}
      <path d={rightPath} fill="#23262b" stroke="#181a1d" strokeWidth="0.6" />
      <path d={frontPath} fill="#2f333a" stroke="#1c1e22" strokeWidth="0.6" />
      <path d={topPath} fill="#424750" stroke="#25282e" strokeWidth="0.6" />

      {/* Inset perimeter groove */}
      <path d={groovePath} fill="none" stroke="#2a2e35" strokeWidth="0.6" />

      {/* Front edge calibration ticks */}
      {ticks}

      {/* Machined concentric bearing mounts under each shaft */}
      {uniqueShaftXs.map((sxVal, i) => {
        const [sx, sy] = project(sxVal, 0, BED_HEIGHT + 0.2, cam);
        const outerR = (SHAFT_RADIUS + 7) * cam.scale;
        const innerR = (SHAFT_RADIUS + 3) * cam.scale;
        return (
          <g key={`bearing-mount-${i}`} aria-label={`Shaft #${i + 1} bearing mount`}>
            {/* Outer flange ring */}
            <circle cx={sx} cy={sy} r={outerR} fill="#353940" stroke="#1f2227" strokeWidth="0.6" />
            {/* Inner bushing ring */}
            <circle cx={sx} cy={sy} r={innerR} fill="#4f545d" stroke="#2b2f36" strokeWidth="0.5" />
            {/* Retention ring */}
            <circle cx={sx} cy={sy} r={SHAFT_RADIUS * cam.scale + 1} fill="#1a1c20" stroke="#333" strokeWidth="0.5" />
            {/* 4 fastener dots */}
            {[0, 90, 180, 270].map(deg => {
              const rad = (deg * Math.PI) / 180;
              const fx = sx + (outerR - 2.5) * Math.cos(rad);
              const fy = sy + (outerR - 2.5) * Math.sin(rad) * 0.6;
              return <circle key={`bolt-${deg}`} cx={fx} cy={fy} r={0.8} fill="#9ca3af" />;
            })}
          </g>
        );
      })}
    </g>
  );
});
MachineBase.displayName = 'MachineBase';

/** Individual gear disc with 3D cylindrical side facets & four-tone shading */
const GearDisc: React.FC<{
  gear: GearDef;
  gx: number;
  gy: number;
  angle: number;
  zBase: number;
  cam: ReturnType<typeof createCamera>;
  selected: boolean;
}> = React.memo(({ gear, gx, gy, angle, zBase, cam, selected }) => {
  const topZ = zBase + GEAR_THICKNESS;

  // Four-tone side facets along the cylindrical thickness
  const facets = useMemo(() => {
    return cylinderFacets(gx, gy, zBase, topZ, gear.radius, cam, 20);
  }, [gx, gy, zBase, topZ, gear.radius, cam]);

  // Tooth outline on top face
  const toothPath = gearPath(gear.teeth, gear.radius, angle, cam, gx, gy, topZ);
  // Tooth outline on bottom face for depth rim
  const bottomRimPath = isoCirclePath(gx, gy, zBase, gear.radius, cam, 32);
  // Bevel inset on top
  const bevelPath = isoCirclePath(gx, gy, topZ, gear.radius * 0.92, cam, 32);
  // Hub ring on top
  const hubPath = isoCirclePath(gx, gy, topZ, gear.radius * 0.32, cam, 24);
  // Bore hole
  const borePath = isoCirclePath(gx, gy, topZ + 0.5, SHAFT_RADIUS + 1, cam, 16);

  // Palette selection based on state
  const sidePalette = gear.highlight
    ? PALETTE.accentShade
    : selected
    ? PALETTE.selectedShade
    : PALETTE.steelShade;

  const topColor = gear.highlight
    ? PALETTE.accent
    : selected
    ? '#7aa0d0'
    : PALETTE.steel;

  const strokeColor = gear.highlight
    ? PALETTE.accentDim
    : selected
    ? '#3b82f6'
    : '#2d3035';

  const [lx, ly] = project(gx, gy, topZ + 2, cam);

  return (
    <g aria-label={`Gear ${gear.label}: ${gear.teeth} teeth`}>
      {/* Bottom rim shadow */}
      <path d={bottomRimPath} fill="#1a1c20" stroke={strokeColor} strokeWidth="0.5" />

      {/* 3D Cylindrical side facets with four-tone shading */}
      {facets.map((facet, fIdx) => (
        <path
          key={`facet-${fIdx}`}
          d={facet.path}
          fill={sidePalette[facet.tone]}
          stroke={sidePalette[facet.tone]}
          strokeWidth="0.3"
        />
      ))}

      {/* Top tooth profile */}
      <path d={toothPath} fill={topColor} stroke={strokeColor} strokeWidth="0.6" />

      {/* Bevel ring on face */}
      <path d={bevelPath} fill="none" stroke={gear.highlight ? PALETTE.highlight : '#9ca3af'} strokeWidth="0.4" opacity="0.4" />

      {/* Machined hub */}
      <path d={hubPath} fill={gear.highlight ? '#d97706' : '#64686e'} stroke={strokeColor} strokeWidth="0.5" />

      {/* Center bore */}
      <path d={borePath} fill="#141416" stroke="#222" strokeWidth="0.4" />

      {/* Label engraving */}
      <text
        x={lx}
        y={ly - 4}
        textAnchor="middle"
        fill={gear.highlight ? PALETTE.highlight : PALETTE.text}
        fontSize="10"
        fontFamily="monospace"
        fontWeight="bold"
        style={{ pointerEvents: 'none' }}
      >
        {gear.label}
      </text>
      {/* Teeth count */}
      <text
        x={lx}
        y={ly + 7}
        textAnchor="middle"
        fill={gear.highlight ? '#fde68a' : PALETTE.textDim}
        fontSize="8"
        fontFamily="monospace"
        style={{ pointerEvents: 'none' }}
      >
        {gear.teeth}T
      </text>
    </g>
  );
});
GearDisc.displayName = 'GearDisc';

/** Shaft cylinder using true 3D cylinder silhouette and shaded facets */
const ShaftElement: React.FC<{
  cx: number;
  cy: number;
  z0: number;
  z1: number;
  cam: ReturnType<typeof createCamera>;
  highlighted: boolean;
}> = React.memo(({ cx, cy, z0, z1, cam, highlighted }) => {
  const { outline, topCap } = useMemo(
    () => shaftPaths(cx, cy, z0, z1, SHAFT_RADIUS, cam, 24),
    [cx, cy, z0, z1, cam]
  );
  const facets = useMemo(
    () => cylinderFacets(cx, cy, z0, z1, SHAFT_RADIUS, cam, 12),
    [cx, cy, z0, z1, cam]
  );

  const shades = highlighted ? PALETTE.selectedShade : PALETTE.shaftShade;

  return (
    <g aria-label="Shaft assembly">
      {/* Outer silhouette outline */}
      <path d={outline} fill={shades[2]} stroke="#222" strokeWidth="0.5" />
      {/* Shaded side facets */}
      {facets.map((facet, idx) => (
        <path
          key={`shaft-f-${idx}`}
          d={facet.path}
          fill={shades[facet.tone]}
          stroke={shades[facet.tone]}
          strokeWidth="0.2"
        />
      ))}
      {/* Top collar cap */}
      <path d={topCap} fill={shades[0]} stroke="#333" strokeWidth="0.5" />
    </g>
  );
});
ShaftElement.displayName = 'ShaftElement';

/** Fig label & live readout overlay */
const CardOverlay: React.FC<{
  cam: ReturnType<typeof createCamera>;
  playing: boolean;
  speed: number;
  rpms: number[];
  reduction: number;
}> = React.memo(({ playing, speed, rpms, reduction }) => {
  return (
    <g>
      {/* Fig label — top left */}
      <text x={16} y={24} fill={PALETTE.textDim} fontSize="10" fontFamily="monospace" fontWeight="bold">
        Fig 1
      </text>
      <text x={16} y={36} fill={PALETTE.text} fontSize="11" fontFamily="monospace">
        Compound Gear Train
      </text>

      {/* Live readout — bottom right */}
      <text x={SVG_W - 16} y={SVG_H - 28} textAnchor="end" fill={PALETTE.textDim} fontSize="9" fontFamily="monospace">
        {playing ? `${(rpms[0] * speed).toFixed(0)} → ${(rpms[rpms.length - 1] * speed).toFixed(1)} RPM` : 'PAUSED'}
      </text>
      <text x={SVG_W - 16} y={SVG_H - 16} textAnchor="end" fill={PALETTE.accent} fontSize="10" fontFamily="monospace" fontWeight="bold">
        {reduction}:1 reduction
      </text>
    </g>
  );
});
CardOverlay.displayName = 'CardOverlay';

export default IsometricGearViewer;
