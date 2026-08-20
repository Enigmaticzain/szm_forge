import React, { useRef, useEffect, useState, useCallback } from 'react';
import { ViewportRenderMode } from '../types';
import {
  Move, RotateCw, Maximize, ZoomIn, ZoomOut,
  Crosshair, Ruler, Scissors, Eye, Camera,
  Grid3x3, Axis3d
} from 'lucide-react';
import { useTooltips } from '../store/TooltipContext';
import { tooltips } from '../data/tooltips';
import type { FurnitureType } from '../workshop/furnitureEngine';
import type { StructuralMember } from '../workshop/structuralAssembly';
import { membersToViewportBoxes } from '../workshop/structuralAssembly';
import {
  boxesForChair,
  boxesForLadder,
  boxesForTable,
  drawIsoBox,
  drawLoadArrow,
  drawStressLegend,
  projectIso,
} from '../utils/viewportFurniture3d';
import { useForgeStore } from '../store/ForgeStoreContext';

interface Props {
  renderMode: ViewportRenderMode;
  furnitureType?: FurnitureType;
  assemblyMembers?: StructuralMember[];
  partStress?: Record<string, number>;
  loadKg?: number;
}

const renderModeColors: Record<ViewportRenderMode, { primary: string; secondary: string; label: string }> = {
  shaded: { primary: '#3366ff', secondary: '#00d4ff', label: 'SHADED' },
  wireframe: { primary: '#00d4ff', secondary: '#0099bb', label: 'WIREFRAME' },
  xray: { primary: '#8844ff', secondary: '#3366ff', label: 'X-RAY' },
  thermal: { primary: '#ff3355', secondary: '#ff8800', label: 'THERMAL' },
  stress: { primary: '#ff8800', secondary: '#ffcc00', label: 'STRESS FEA' },
  pressure: { primary: '#00ccaa', secondary: '#00ff88', label: 'PRESSURE' },
  electrical: { primary: '#ffcc00', secondary: '#ff8800', label: 'ELECTRICAL' },
};

const viewportTools: { label: string; tip: string; icon: React.ReactNode }[] = [
  { label: 'Move', tip: 'Move tool — translate selected instance', icon: <Move size={12} /> },
  { label: 'Rotate', tip: 'Rotate tool — adjust instance orientation', icon: <RotateCw size={12} /> },
  { label: 'Scale', tip: 'Scale tool — resize instance non-uniformly', icon: <Maximize size={12} /> },
  { label: 'Zoom In', tip: tooltips.viewport.zoom, icon: <ZoomIn size={12} /> },
  { label: 'Zoom Out', tip: tooltips.viewport.zoom, icon: <ZoomOut size={12} /> },
  { label: 'Center', tip: 'Frame selection in viewport', icon: <Crosshair size={12} /> },
  { label: 'Measure', tip: tooltips.viewport.measure, icon: <Ruler size={12} /> },
  { label: 'Section', tip: tooltips.viewport.section, icon: <Scissors size={12} /> },
  { label: 'Visibility', tip: 'Toggle instance visibility', icon: <Eye size={12} /> },
  { label: 'Screenshot', tip: 'Capture viewport image', icon: <Camera size={12} /> },
];

export const Viewport3D: React.FC<Props> = ({
  renderMode,
  furnitureType,
  assemblyMembers,
  partStress = {},
  loadKg = 0,
}) => {
  const { t } = useTooltips();
  const { viewportZoom, setViewportZoom, showViewportGrid, toggleViewportGrid, showToast } = useForgeStore();
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animFrameRef = useRef<number>(0);
  const [canvasSize, setCanvasSize] = useState({ width: 800, height: 600 });
  const timeRef = useRef(0);
  const containerRef = useRef<HTMLDivElement>(null);
  const [orbitYaw, setOrbitYaw] = useState(0.55);
  const dragRef = useRef<{ active: boolean; lastX: number }>({ active: false, lastX: 0 });

  const colors = renderModeColors[renderMode];

  const resizeCanvas = useCallback(() => {
    if (containerRef.current) {
      const rect = containerRef.current.getBoundingClientRect();
      setCanvasSize({ width: rect.width, height: rect.height });
    }
  }, []);

  useEffect(() => {
    resizeCanvas();
    window.addEventListener('resize', resizeCanvas);
    return () => window.removeEventListener('resize', resizeCanvas);
  }, [resizeCanvas]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const w = canvasSize.width;
    const h = canvasSize.height;
    canvas.width = w;
    canvas.height = h;

    const draw = () => {
      timeRef.current += 0.008;
      const t = timeRef.current;

      // Clear
      ctx.fillStyle = '#0a0a0f';
      ctx.fillRect(0, 0, w, h);

      // Engineering grid
      if (showViewportGrid) {
      ctx.strokeStyle = 'rgba(30, 30, 42, 0.4)';
      ctx.lineWidth = 0.5;
      const gridSize = 40;
      for (let x = 0; x < w; x += gridSize) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = 0; y < h; y += gridSize) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }
      }

      // Center axes
      const cx = w / 2;
      const cy = h / 2;
      const scaleMul = viewportZoom;

      // Perspective grid floor
      if (showViewportGrid) {
      ctx.save();
      ctx.globalAlpha = 0.15;
      const vanishY = cy - 50;
      for (let i = -10; i <= 10; i++) {
        const x1 = cx + i * 50;
        ctx.strokeStyle = i === 0 ? colors.primary : '#1e1e2a';
        ctx.lineWidth = i === 0 ? 1.5 : 0.5;
        ctx.beginPath();
        ctx.moveTo(x1, vanishY);
        ctx.lineTo(cx + i * 120, h);
        ctx.stroke();
      }
      for (let i = 0; i < 12; i++) {
        const y = vanishY + (h - vanishY) * (i / 12);
        const spread = (y - vanishY) / (h - vanishY);
        ctx.strokeStyle = '#1e1e2a';
        ctx.lineWidth = 0.5;
        ctx.beginPath();
        ctx.moveTo(cx - spread * 1200, y);
        ctx.lineTo(cx + spread * 1200, y);
        ctx.stroke();
      }
      ctx.restore();
      }

      const showAssembly = assemblyMembers && assemblyMembers.length > 0;
      if (showAssembly || furnitureType) {
        const useStressColors =
          renderMode === 'stress' ||
          renderMode === 'shaded' ||
          Object.keys(partStress).length > 0;
        const scale = Math.min(w, h) * 0.22 * scaleMul;
        const originX = cx;
        const originY = cy + scale * 0.15;
        const yaw = orbitYaw + Math.sin(t * 0.15) * 0.02;

        const boxes = showAssembly
          ? membersToViewportBoxes(assemblyMembers!, partStress)
          : furnitureType === 'table'
            ? boxesForTable(partStress)
            : furnitureType === 'chair'
              ? boxesForChair(partStress)
              : boxesForLadder(partStress);

        // Ground shadow
        ctx.fillStyle = 'rgba(0,0,0,0.35)';
        ctx.beginPath();
        ctx.ellipse(cx, originY + scale * 0.55, scale * 1.1, scale * 0.35, 0, 0, Math.PI * 2);
        ctx.fill();

        // Draw legs/rails first, top last (painter's order)
        const sorted = [...boxes].sort((a, b) => a.cy - b.cy);
        for (const box of sorted) {
          drawIsoBox(ctx, box, originX, originY, scale, yaw, useStressColors);
        }

        if (loadKg > 0) {
          const maxY = Math.max(...boxes.map(b => b.cy + b.h));
          const top = projectIso(0, maxY + 0.05, 0, originX, originY, scale, yaw);
          drawLoadArrow(ctx, top.x, top.y, loadKg, t);
        }

        drawStressLegend(ctx, w - 52, 48);

        ctx.fillStyle = '#ffffff90';
        ctx.font = '10px monospace';
        ctx.textAlign = 'left';
        const title = showAssembly
          ? `ASSEMBLY — ${assemblyMembers!.length} MEMBERS`
          : furnitureType === 'table'
            ? 'TABLE — ISOMETRIC FEA'
            : furnitureType === 'chair'
              ? 'CHAIR — ISOMETRIC FEA'
              : 'LADDER — ISOMETRIC FEA';
        ctx.fillText(title, 14, h - 36);
        ctx.fillStyle = '#8888aa';
        ctx.fillText('Drag viewport to orbit · each part colored by stress %', 14, h - 22);
        ctx.textAlign = 'right';
        ctx.fillText(`${loadKg} kg load`, w - 14, h - 22);

        animFrameRef.current = requestAnimationFrame(draw);
        return;
      }

      // System geometry - central hub
      const hubR = Math.min(w, h) * 0.08;
      const bladeCount = 12;
      const rotation = t * 0.3;

      // Glow effect behind system
      const glowGrad = ctx.createRadialGradient(cx, cy, 0, cx, cy, hubR * 5);
      glowGrad.addColorStop(0, `${colors.primary}15`);
      glowGrad.addColorStop(0.5, `${colors.secondary}08`);
      glowGrad.addColorStop(1, 'transparent');
      ctx.fillStyle = glowGrad;
      ctx.fillRect(0, 0, w, h);

      // Draw blades
      for (let i = 0; i < bladeCount; i++) {
        const angle = (Math.PI * 2 * i) / bladeCount + rotation;
        const bladeLen = hubR * 3.5;
        const bx = cx + Math.cos(angle) * hubR * 1.2;
        const by = cy + Math.sin(angle) * hubR * 1.2;
        const ex = cx + Math.cos(angle) * (hubR + bladeLen);
        const ey = cy + Math.sin(angle) * (hubR + bladeLen);

        // Blade body
        const perpAngle = angle + Math.PI / 2;
        const bladeWidth = hubR * 0.35;

        ctx.beginPath();
        ctx.moveTo(bx + Math.cos(perpAngle) * bladeWidth, by + Math.sin(perpAngle) * bladeWidth);
        ctx.lineTo(ex + Math.cos(perpAngle) * bladeWidth * 0.15, ey + Math.sin(perpAngle) * bladeWidth * 0.15);
        ctx.lineTo(ex - Math.cos(perpAngle) * bladeWidth * 0.15, ey - Math.sin(perpAngle) * bladeWidth * 0.15);
        ctx.lineTo(bx - Math.cos(perpAngle) * bladeWidth, by - Math.sin(perpAngle) * bladeWidth);
        ctx.closePath();

        if (renderMode === 'wireframe' || renderMode === 'xray') {
          ctx.strokeStyle = colors.primary + '80';
          ctx.lineWidth = 1;
          ctx.stroke();
          if (renderMode === 'xray') {
            ctx.fillStyle = colors.primary + '10';
            ctx.fill();
          }
        } else if (renderMode === 'thermal') {
          const gradient = ctx.createLinearGradient(bx, by, ex, ey);
          gradient.addColorStop(0, '#ff335580');
          gradient.addColorStop(0.5, '#ff880060');
          gradient.addColorStop(1, '#ffcc0040');
          ctx.fillStyle = gradient;
          ctx.fill();
          ctx.strokeStyle = '#ff335560';
          ctx.lineWidth = 0.5;
          ctx.stroke();
        } else if (renderMode === 'stress') {
          const stressVal = Math.sin(angle * 3 + t) * 0.5 + 0.5;
          const gradient = ctx.createLinearGradient(bx, by, ex, ey);
          gradient.addColorStop(0, `rgba(255, ${Math.floor(200 * (1 - stressVal))}, 0, 0.6)`);
          gradient.addColorStop(1, `rgba(255, 204, 0, 0.3)`);
          ctx.fillStyle = gradient;
          ctx.fill();
          ctx.strokeStyle = '#ff880040';
          ctx.lineWidth = 0.5;
          ctx.stroke();
        } else if (renderMode === 'electrical') {
          ctx.fillStyle = '#ffcc0015';
          ctx.fill();
          ctx.strokeStyle = '#ffcc0060';
          ctx.lineWidth = 1;
          ctx.stroke();
          // Electric arcs
          if (Math.random() > 0.95) {
            ctx.strokeStyle = '#ffcc00';
            ctx.lineWidth = 1.5;
            ctx.beginPath();
            const mid = 0.3 + Math.random() * 0.4;
            const mx = bx + (ex - bx) * mid + (Math.random() - 0.5) * 10;
            const my = by + (ey - by) * mid + (Math.random() - 0.5) * 10;
            ctx.moveTo(bx, by);
            ctx.lineTo(mx, my);
            ctx.lineTo(ex, ey);
            ctx.stroke();
          }
        } else {
          // Shaded mode
          const gradient = ctx.createLinearGradient(bx, by, ex, ey);
          gradient.addColorStop(0, '#2a3050');
          gradient.addColorStop(0.5, '#1e2540');
          gradient.addColorStop(1, '#151a30');
          ctx.fillStyle = gradient;
          ctx.fill();
          ctx.strokeStyle = colors.primary + '40';
          ctx.lineWidth = 1;
          ctx.stroke();

          // Edge highlight
          ctx.strokeStyle = colors.secondary + '20';
          ctx.lineWidth = 0.5;
          ctx.beginPath();
          ctx.moveTo(bx + Math.cos(perpAngle) * bladeWidth, by + Math.sin(perpAngle) * bladeWidth);
          ctx.lineTo(ex + Math.cos(perpAngle) * bladeWidth * 0.15, ey + Math.sin(perpAngle) * bladeWidth * 0.15);
          ctx.stroke();
        }
      }

      // Hub ring
      ctx.beginPath();
      ctx.arc(cx, cy, hubR, 0, Math.PI * 2);
      if (renderMode === 'wireframe') {
        ctx.strokeStyle = colors.primary;
        ctx.lineWidth = 1.5;
        ctx.stroke();
      } else {
        const hubGrad = ctx.createRadialGradient(cx - hubR * 0.3, cy - hubR * 0.3, 0, cx, cy, hubR);
        hubGrad.addColorStop(0, '#2a3050');
        hubGrad.addColorStop(1, '#151a30');
        ctx.fillStyle = hubGrad;
        ctx.fill();
        ctx.strokeStyle = colors.primary + '60';
        ctx.lineWidth = 1.5;
        ctx.stroke();
      }

      // Inner hub detail
      ctx.beginPath();
      ctx.arc(cx, cy, hubR * 0.5, 0, Math.PI * 2);
      ctx.strokeStyle = colors.primary + '40';
      ctx.lineWidth = 1;
      ctx.stroke();

      // Center dot
      ctx.beginPath();
      ctx.arc(cx, cy, 3, 0, Math.PI * 2);
      ctx.fillStyle = colors.primary;
      ctx.fill();

      // Outer housing ring
      ctx.beginPath();
      ctx.arc(cx, cy, hubR * 4.2, 0, Math.PI * 2);
      ctx.strokeStyle = colors.primary + '15';
      ctx.lineWidth = 2;
      ctx.setLineDash([8, 4]);
      ctx.stroke();
      ctx.setLineDash([]);

      // Measurement annotation
      const measAngle = Math.PI * 0.25;
      const measStart = hubR * 1.2;
      const measEnd = hubR * 3.5;
      ctx.strokeStyle = '#ffcc0060';
      ctx.lineWidth = 0.5;
      ctx.setLineDash([3, 3]);
      ctx.beginPath();
      ctx.moveTo(cx + Math.cos(measAngle) * measStart, cy + Math.sin(measAngle) * measStart);
      ctx.lineTo(cx + Math.cos(measAngle) * measEnd, cy + Math.sin(measAngle) * measEnd);
      ctx.stroke();
      ctx.setLineDash([]);

      // Dimension label
      const labelX = cx + Math.cos(measAngle) * (measStart + measEnd) / 2;
      const labelY = cy + Math.sin(measAngle) * (measStart + measEnd) / 2;
      ctx.fillStyle = '#ffcc00';
      ctx.font = '9px "JetBrains Mono"';
      ctx.textAlign = 'center';
      ctx.fillText('342.5mm', labelX + 15, labelY - 8);

      // Stress hotspot indicators
      if (renderMode === 'stress' || renderMode === 'thermal') {
        for (let i = 0; i < 3; i++) {
          const spotAngle = (Math.PI * 2 * i) / 3 + rotation;
          const spotR = hubR * 1.3;
          const sx = cx + Math.cos(spotAngle) * spotR;
          const sy = cy + Math.sin(spotAngle) * spotR;

          ctx.beginPath();
          ctx.arc(sx, sy, 6 + Math.sin(t * 3) * 2, 0, Math.PI * 2);
          ctx.strokeStyle = renderMode === 'thermal' ? '#ff335580' : '#ff880080';
          ctx.lineWidth = 1.5;
          ctx.stroke();

          ctx.beginPath();
          ctx.arc(sx, sy, 2, 0, Math.PI * 2);
          ctx.fillStyle = renderMode === 'thermal' ? '#ff3355' : '#ff8800';
          ctx.fill();
        }
      }

      // Coordinate axes indicator (bottom-left)
      const axLen = 35;
      const axX = 50;
      const axY = h - 50;

      // X axis
      ctx.strokeStyle = '#ff3355';
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.moveTo(axX, axY);
      ctx.lineTo(axX + axLen, axY);
      ctx.stroke();
      ctx.fillStyle = '#ff3355';
      ctx.font = '8px "JetBrains Mono"';
      ctx.textAlign = 'center';
      ctx.fillText('X', axX + axLen + 8, axY + 3);

      // Y axis
      ctx.strokeStyle = '#00ff88';
      ctx.beginPath();
      ctx.moveTo(axX, axY);
      ctx.lineTo(axX, axY - axLen);
      ctx.stroke();
      ctx.fillStyle = '#00ff88';
      ctx.fillText('Y', axX, axY - axLen - 5);

      // Z axis
      ctx.strokeStyle = '#3366ff';
      ctx.beginPath();
      ctx.moveTo(axX, axY);
      ctx.lineTo(axX + axLen * 0.5, axY + axLen * 0.3);
      ctx.stroke();
      ctx.fillStyle = '#3366ff';
      ctx.fillText('Z', axX + axLen * 0.5 + 8, axY + axLen * 0.3 + 3);

      // Viewport info
      ctx.fillStyle = '#555570';
      ctx.font = '9px "JetBrains Mono"';
      ctx.textAlign = 'left';
      ctx.fillText(`MODE: ${renderModeColors[renderMode].label}`, 14, 20);
      ctx.fillText(`NODES: 2,847,293`, 14, 32);
      ctx.fillText(`ELEMENTS: 8,412,847`, 14, 44);

      // FPS counter
      ctx.textAlign = 'right';
      ctx.fillStyle = '#00ff88';
      ctx.fillText('60 FPS', w - 14, 20);
      ctx.fillStyle = '#555570';
      ctx.fillText('PERSPECTIVE', w - 14, 32);

      // Scan line effect
      const scanY = (t * 80) % h;
      ctx.strokeStyle = `${colors.primary}08`;
      ctx.lineWidth = 1;
      ctx.beginPath();
      ctx.moveTo(0, scanY);
      ctx.lineTo(w, scanY);
      ctx.stroke();

      // Particle system for simulation modes
      if (renderMode === 'thermal' || renderMode === 'stress' || renderMode === 'electrical') {
        for (let i = 0; i < 20; i++) {
          const px = cx + Math.cos(t * 0.5 + i * 0.7) * (hubR * 2 + Math.sin(t + i) * hubR);
          const py = cy + Math.sin(t * 0.5 + i * 0.7) * (hubR * 2 + Math.cos(t + i) * hubR);
          const alpha = 0.3 + Math.sin(t * 2 + i) * 0.2;
          ctx.beginPath();
          ctx.arc(px, py, 1.5, 0, Math.PI * 2);
          ctx.fillStyle = `${colors.primary}${Math.floor(alpha * 255).toString(16).padStart(2, '0')}`;
          ctx.fill();
        }
      }

      animFrameRef.current = requestAnimationFrame(draw);
    };

    draw();
    return () => cancelAnimationFrame(animFrameRef.current);
  }, [canvasSize, renderMode, colors, furnitureType, assemblyMembers, partStress, loadKg, orbitYaw, viewportZoom, showViewportGrid]);

  const onPointerDown = (e: React.PointerEvent) => {
    if (!furnitureType && !assemblyMembers?.length) return;
    dragRef.current = { active: true, lastX: e.clientX };
    (e.target as HTMLElement).setPointerCapture(e.pointerId);
  };
  const onPointerMove = (e: React.PointerEvent) => {
    if (!dragRef.current.active) return;
    const dx = e.clientX - dragRef.current.lastX;
    dragRef.current.lastX = e.clientX;
    setOrbitYaw(y => y + dx * 0.008);
  };
  const onPointerUp = (e: React.PointerEvent) => {
    dragRef.current.active = false;
    (e.target as HTMLElement).releasePointerCapture(e.pointerId);
  };

  return (
    <div ref={containerRef} className="relative w-full h-full bg-forge-black overflow-hidden">
      <canvas
        ref={canvasRef}
        className="absolute inset-0 w-full h-full touch-none"
        style={{ cursor: furnitureType || assemblyMembers?.length ? 'grab' : 'default' }}
        onPointerDown={onPointerDown}
        onPointerMove={onPointerMove}
        onPointerUp={onPointerUp}
        onPointerLeave={onPointerUp}
      />

      {/* Viewport toolbar overlay */}
      <div className="absolute top-2 right-2 flex flex-col gap-1 z-10">
        {viewportTools.map((tool, i) => (
          <button
            key={i}
            type="button"
            title={t(tool.tip)}
            aria-label={t(tool.tip)}
            onClick={() => {
              if (tool.label === 'Zoom In') setViewportZoom(z => Math.min(2.5, z + 0.15));
              else if (tool.label === 'Zoom Out') setViewportZoom(z => Math.max(0.4, z - 0.15));
              else if (tool.label === 'Center') setViewportZoom(1);
              else showToast(`${tool.label} — use orbit drag on model`);
            }}
            className="w-9 h-9 flex items-center justify-center glass-pill text-forge-text-muted hover:text-forge-accent hover:border-forge-accent/30 transition-all hover:scale-105"
          >
            {tool.icon}
          </button>
        ))}
      </div>

      {/* View cube (top-left) */}
      <div className="absolute top-4 left-4 z-10">
        <div className="w-16 h-16 glass-panel rounded-xl flex items-center justify-center shadow-lg">
          <div className="relative w-10 h-10">
            <Axis3d size={32} className="text-forge-accent/40" />
            <div className="absolute top-0 right-0 text-[6px] font-mono text-forge-green">Y</div>
            <div className="absolute bottom-0 right-0 text-[6px] font-mono text-forge-red">X</div>
            <div className="absolute bottom-0 left-0 text-[6px] font-mono text-forge-blue">Z</div>
          </div>
        </div>
      </div>

      {/* Grid toggle */}
      <div className="absolute bottom-4 right-4 z-10 flex items-center gap-2">
        <button
          type="button"
          title={t(tooltips.viewport.grid)}
          onClick={toggleViewportGrid}
          className={`px-4 py-2 glass-pill text-[10px] font-mono transition-all flex items-center gap-1 hover:scale-105 ${
            showViewportGrid ? 'text-forge-accent border-forge-accent/40' : 'text-forge-text-muted hover:text-forge-text'
          }`}
        >
          <Grid3x3 size={12} />
          GRID
        </button>
        <button className="px-4 py-2 glass-pill text-[10px] font-mono text-forge-text-muted hover:text-forge-text transition-all hover:scale-105">
          SNAP
        </button>
        <button className="px-4 py-2 glass-pill text-[10px] font-mono text-forge-text-muted hover:text-forge-text transition-all hover:scale-105">
          ORTHO
        </button>
      </div>

      {/* Render mode badge */}
      <div className="absolute bottom-2 left-2 z-10">
        <div
          className="px-2.5 py-1 rounded text-[9px] font-mono font-bold tracking-widest backdrop-blur-sm border"
          style={{
            color: colors.primary,
            backgroundColor: colors.primary + '15',
            borderColor: colors.primary + '30',
          }}
        >
          {colors.label}
        </div>
      </div>
    </div>
  );
};
