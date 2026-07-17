import React, { useState, useRef, useEffect } from 'react';
import {
  BarChart3, Download,
  Maximize2, Play, Pause, ChevronDown
} from 'lucide-react';

type ResultType = 'stress' | 'displacement' | 'strain' | 'temperature' | 'factor-of-safety';

interface Props {
  visible: boolean;
}

const resultTypes: { id: ResultType; label: string; unit: string; min: number; max: number; critical: number }[] = [
  { id: 'stress', label: 'Von Mises Stress', unit: 'MPa', min: 0, max: 847.3, critical: 880 },
  { id: 'displacement', label: 'Total Displacement', unit: 'mm', min: 0, max: 0.342, critical: 0.5 },
  { id: 'strain', label: 'Equivalent Strain', unit: 'mm/mm', min: 0, max: 0.0074, critical: 0.01 },
  { id: 'temperature', label: 'Temperature', unit: '°C', min: 25, max: 1247, critical: 1200 },
  { id: 'factor-of-safety', label: 'Safety Factor', unit: '', min: 1.04, max: 15.2, critical: 2.0 },
];

const colorScales: Record<ResultType, string[]> = {
  'stress': ['#0033ff', '#00ffff', '#00ff00', '#ffff00', '#ff8800', '#ff0000'],
  'displacement': ['#0033ff', '#0088ff', '#00ffff', '#00ff88', '#88ff00', '#ffff00'],
  'strain': ['#000066', '#0033cc', '#0066ff', '#00ccff', '#66ffcc', '#ccff66'],
  'temperature': ['#0000ff', '#0088ff', '#00ff88', '#88ff00', '#ffff00', '#ff8800', '#ff0000'],
  'factor-of-safety': ['#ff0000', '#ff8800', '#ffff00', '#88ff00', '#00ff00', '#00ff88'],
};

export const ResultsViewer: React.FC<Props> = ({ visible }) => {
  const [activeResult, setActiveResult] = useState<ResultType>('stress');
  const [showLegend, setShowLegend] = useState(true);
  const [animating, setAnimating] = useState(false);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animRef = useRef(0);
  const timeRef = useRef(0);

  const result = resultTypes.find(r => r.id === activeResult)!;
  const scale = colorScales[activeResult];

  // Draw contour visualization
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || !visible) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    canvas.width = 400;
    canvas.height = 200;

    const draw = () => {
      timeRef.current += animating ? 0.02 : 0;
      const t = timeRef.current;
      const w = canvas.width;
      const h = canvas.height;

      ctx.fillStyle = '#0a0a0f';
      ctx.fillRect(0, 0, w, h);

      // Create gradient visualization (simplified contour representation)
      for (let x = 0; x < w; x += 4) {
        for (let y = 0; y < h; y += 4) {
          const nx = (x / w - 0.5) * 2;
          const ny = (y / h - 0.5) * 2;
          const dist = Math.sqrt(nx * nx + ny * ny);
          
          // Create a stress-like pattern
          let value = 0;
          if (activeResult === 'stress' || activeResult === 'strain') {
            value = Math.sin(nx * 3 + t) * Math.cos(ny * 2 + t * 0.5) * 0.3 + 
                    Math.exp(-dist * 1.5) * 0.7 +
                    Math.sin(dist * 5 - t * 2) * 0.1;
          } else if (activeResult === 'temperature') {
            value = 1 - dist * 0.7 + Math.sin(nx * 2 + t) * 0.1;
          } else if (activeResult === 'displacement') {
            value = Math.exp(-dist * 2) + Math.sin(dist * 4 - t) * 0.15;
          } else {
            value = 1 - Math.exp(-dist * 1.5) * 0.6;
          }
          
          value = Math.max(0, Math.min(1, value));
          const colorIdx = Math.floor(value * (scale.length - 1));
          ctx.fillStyle = scale[colorIdx];
          ctx.globalAlpha = 0.8;
          ctx.fillRect(x, y, 4, 4);
        }
      }
      ctx.globalAlpha = 1;

      // Draw mesh overlay
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.05)';
      ctx.lineWidth = 0.5;
      for (let x = 0; x < w; x += 20) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = 0; y < h; y += 20) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }

      // Critical region indicator
      ctx.strokeStyle = '#ff3355';
      ctx.lineWidth = 2;
      ctx.setLineDash([4, 4]);
      ctx.beginPath();
      ctx.arc(w * 0.3, h * 0.4, 25, 0, Math.PI * 2);
      ctx.stroke();
      ctx.setLineDash([]);

      ctx.fillStyle = '#ff3355';
      ctx.font = '8px "JetBrains Mono"';
      ctx.textAlign = 'center';
      ctx.fillText('MAX', w * 0.3, h * 0.4 + 35);

      if (animating) {
        animRef.current = requestAnimationFrame(draw);
      }
    };

    draw();
    return () => cancelAnimationFrame(animRef.current);
  }, [visible, activeResult, animating, scale]);

  if (!visible) return null;

  return (
    <div className="h-full flex flex-col bg-forge-panel border-l border-forge-border">
      {/* Header */}
      <div className="px-3 py-2.5 border-b border-forge-border flex items-center gap-1.5">
        <BarChart3 size={12} className="text-forge-accent" />
        <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">RESULTS VIEWER</span>
      </div>

      {/* Result Type Selector */}
      <div className="px-2 py-2 border-b border-forge-border/50">
        <div className="relative">
          <button className="w-full flex items-center justify-between px-2.5 py-1.5 rounded bg-forge-surface border border-forge-border/50 text-[10px] font-medium text-forge-text hover:border-forge-accent/30 transition-all">
            <span>{result.label}</span>
            <ChevronDown size={10} className="text-forge-text-muted" />
          </button>
        </div>
        <div className="flex flex-wrap gap-1 mt-2">
          {resultTypes.map(r => (
            <button
              key={r.id}
              onClick={() => setActiveResult(r.id)}
              className={`px-2 py-0.5 rounded text-[8px] font-mono transition-all ${
                activeResult === r.id
                  ? 'bg-forge-accent/15 text-forge-accent border border-forge-accent/30'
                  : 'text-forge-text-muted hover:text-forge-text-dim border border-transparent'
              }`}
            >
              {r.label.split(' ')[0]}
            </button>
          ))}
        </div>
      </div>

      {/* Contour Preview */}
      <div className="px-2 py-2 border-b border-forge-border/50">
        <div className="flex items-center justify-between mb-2">
          <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">CONTOUR PREVIEW</span>
          <div className="flex items-center gap-1">
            <button 
              onClick={() => setAnimating(!animating)}
              className={`p-1 rounded ${animating ? 'bg-forge-accent/20 text-forge-accent' : 'text-forge-text-muted hover:text-forge-text-dim'}`}
            >
              {animating ? <Pause size={10} /> : <Play size={10} />}
            </button>
            <button className="p-1 rounded text-forge-text-muted hover:text-forge-text-dim">
              <Maximize2 size={10} />
            </button>
          </div>
        </div>
        <canvas ref={canvasRef} className="w-full h-[140px] rounded border border-forge-border/30" />
      </div>

      {/* Color Legend */}
      {showLegend && (
        <div className="px-2 py-2 border-b border-forge-border/50">
          <div className="flex items-center justify-between mb-2">
            <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">LEGEND</span>
            <button 
              onClick={() => setShowLegend(false)}
              className="text-[8px] text-forge-text-muted hover:text-forge-text-dim"
            >
              Hide
            </button>
          </div>
          <div className="flex items-center gap-2">
            <div 
              className="flex-1 h-3 rounded"
              style={{ background: `linear-gradient(to right, ${scale.join(', ')})` }}
            />
          </div>
          <div className="flex justify-between mt-1">
            <span className="text-[8px] font-mono text-forge-text-muted">{result.min} {result.unit}</span>
            <span className="text-[8px] font-mono text-forge-red">{result.max} {result.unit}</span>
          </div>
          {result.critical && (
            <div className="mt-1 text-[8px] font-mono text-forge-yellow">
              Critical: {result.critical} {result.unit}
            </div>
          )}
        </div>
      )}

      {/* Statistics */}
      <div className="px-2 py-2 flex-1 overflow-y-auto">
        <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">STATISTICS</span>
        <div className="mt-2 space-y-1.5">
          {[
            { label: 'Minimum', value: `${result.min} ${result.unit}`, color: 'text-forge-blue' },
            { label: 'Maximum', value: `${result.max} ${result.unit}`, color: 'text-forge-red' },
            { label: 'Average', value: `${((result.min + result.max) / 2).toFixed(2)} ${result.unit}`, color: 'text-forge-text' },
            { label: 'Std Dev', value: `${((result.max - result.min) * 0.2).toFixed(3)} ${result.unit}`, color: 'text-forge-text-dim' },
          ].map((stat, i) => (
            <div key={i} className="flex items-center justify-between px-2 py-1 rounded bg-forge-surface/30">
              <span className="text-[9px] text-forge-text-dim">{stat.label}</span>
              <span className={`text-[9px] font-mono ${stat.color}`}>{stat.value}</span>
            </div>
          ))}
        </div>

        {/* Export Options */}
        <div className="mt-4">
          <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">EXPORT</span>
          <div className="mt-2 space-y-1">
            <button className="w-full flex items-center gap-2 px-2 py-1.5 rounded text-[9px] font-mono text-forge-text-dim hover:bg-forge-surface transition-all">
              <Download size={10} /> Export Image (PNG)
            </button>
            <button className="w-full flex items-center gap-2 px-2 py-1.5 rounded text-[9px] font-mono text-forge-text-dim hover:bg-forge-surface transition-all">
              <Download size={10} /> Export Data (CSV)
            </button>
            <button className="w-full flex items-center gap-2 px-2 py-1.5 rounded text-[9px] font-mono text-forge-text-dim hover:bg-forge-surface transition-all">
              <Download size={10} /> Generate Report
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};
