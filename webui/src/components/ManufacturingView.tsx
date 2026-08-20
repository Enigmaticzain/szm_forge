import { useState, useEffect, useRef } from 'react';

interface ToolpathPoint {
  x: number;
  y: number;
  z: number;
}

interface ToolpathResult {
  status: string;
  engineUsed?: string;
  engine_used?: string;
  component: string;
  operation: string;
  gcode: string;
  previewPoints: ToolpathPoint[];
  metrics: {
    lineCount: number;
    cutLengthMm: number;
    passes: number;
    boundsMm: { length: number; width: number; depth: number };
    toolDiameterMm: number;
    stepoverMm: number;
    stepdownMm: number;
    feedRateMmMin: number;
    spindleSpeedRpm: number;
    estimatedCycleTimeMin: number;
  };
  warnings: string[];
}

export default function ManufacturingView() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animRef = useRef(0);
  const [activeProcess, setActiveProcess] = useState('machining');
  const [processStep, setProcessStep] = useState(0);
  const [cncOperation, setCncOperation] = useState<'pocket' | 'profile' | 'facing'>('pocket');
  const [toolpathResult, setToolpathResult] = useState<ToolpathResult | null>(null);
  const [toolpathError, setToolpathError] = useState<string | null>(null);
  const [isGeneratingToolpath, setIsGeneratingToolpath] = useState(false);

  const processes = [
    { id: 'machining', label: 'CNC MACHINING', icon: '⚙', steps: ['Roughing', 'Semi-Finish', 'Finishing', 'Deburring'] },
    { id: 'assembly', label: 'ASSEMBLY', icon: '⬡', steps: ['Sub-Assembly A', 'Sub-Assembly B', 'Integration', 'QC Check'] },
    { id: 'inspection', label: 'INSPECTION', icon: '◎', steps: ['CMM Scan', 'Surface Check', 'Tolerance Verify', 'Sign-Off'] },
    { id: 'robotics', label: 'ROBOTICS', icon: '⚡', steps: ['Path Plan', 'Execute', 'Verify', 'Report'] },
  ];

  useEffect(() => {
    const interval = setInterval(() => {
      setProcessStep(prev => (prev + 1) % 4);
    }, 3000);
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const resize = () => {
      const rect = canvas.parentElement?.getBoundingClientRect();
      if (rect) {
        canvas.width = rect.width * 2;
        canvas.height = rect.height * 2;
        canvas.style.width = `${rect.width}px`;
        canvas.style.height = `${rect.height}px`;
        ctx.scale(2, 2);
      }
    };
    resize();

    const render = (t: number) => {
      const cw = canvas.width / 2;
      const ch = canvas.height / 2;
      ctx.setTransform(2, 0, 0, 2, 0, 0);

      ctx.fillStyle = '#0a0c10';
      ctx.fillRect(0, 0, cw, ch);

      // Grid
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.25)';
      ctx.lineWidth = 0.5;
      for (let x = 0; x < cw; x += 25) {
        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, ch); ctx.stroke();
      }
      for (let y = 0; y < ch; y += 25) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(cw, y); ctx.stroke();
      }

      const cx = cw / 2;
      const cy = ch / 2;

      if (activeProcess === 'machining') {
        // Draw CNC toolpath visualization
        const toolRot = t * 0.002;
        
        // Workpiece outline
        ctx.beginPath();
        ctx.roundRect(cx - 80, cy - 50, 160, 100, 5);
        ctx.strokeStyle = 'rgba(0, 194, 255, 0.3)';
        ctx.lineWidth = 1;
        ctx.stroke();
        ctx.fillStyle = 'rgba(0, 194, 255, 0.03)';
        ctx.fill();

        // Toolpath lines
        const pathOffset = (t * 0.01) % 200;
        if (toolpathResult?.previewPoints?.length) {
          const bounds = toolpathResult.metrics.boundsMm;
          ctx.beginPath();
          toolpathResult.previewPoints.forEach((point, index) => {
            const px = cx - 80 + (point.x / Math.max(bounds.length, 1)) * 160;
            const py = cy - 50 + (point.y / Math.max(bounds.width, 1)) * 100;
            if (index === 0) ctx.moveTo(px, py);
            else ctx.lineTo(px, py);
          });
          ctx.strokeStyle = 'rgba(0, 255, 136, 0.58)';
          ctx.lineWidth = 1.2;
          ctx.setLineDash([]);
          ctx.stroke();
        } else {
          for (let i = 0; i < 20; i++) {
            const y = cy - 45 + i * 5;
            const xStart = cx - 75;
            const xEnd = cx + 75;
            
            ctx.beginPath();
            ctx.moveTo(xStart, y);
            ctx.lineTo(xEnd, y);
            ctx.strokeStyle = `rgba(0, 255, 136, ${0.15 + (i === Math.floor(pathOffset / 10) % 20 ? 0.4 : 0)})`;
            ctx.lineWidth = i === Math.floor(pathOffset / 10) % 20 ? 1.5 : 0.3;
            ctx.setLineDash(i === Math.floor(pathOffset / 10) % 20 ? [] : [2, 4]);
            ctx.stroke();
            ctx.setLineDash([]);
          }
        }

        // Tool head
        const toolY = cy - 45 + (Math.floor(pathOffset / 10) % 20) * 5;
        const toolX = cx - 75 + (pathOffset % 150);
        
        ctx.beginPath();
        ctx.arc(toolX, toolY, 5, 0, Math.PI * 2);
        ctx.fillStyle = '#00ff88';
        ctx.fill();
        
        // Tool rotation indicator
        for (let a = 0; a < 4; a++) {
          const angle = toolRot + (a * Math.PI / 2);
          ctx.beginPath();
          ctx.moveTo(toolX, toolY);
          ctx.lineTo(toolX + Math.cos(angle) * 8, toolY + Math.sin(angle) * 8);
          ctx.strokeStyle = 'rgba(0, 255, 136, 0.5)';
          ctx.lineWidth = 1;
          ctx.stroke();
        }

        // Chip evacuation particles
        for (let p = 0; p < 5; p++) {
          const px = toolX + Math.cos(toolRot + p * 1.2) * (10 + p * 3);
          const py = toolY + Math.sin(toolRot + p * 0.8) * (8 + p * 2);
          ctx.beginPath();
          ctx.arc(px, py, 1, 0, Math.PI * 2);
          ctx.fillStyle = `rgba(255, 170, 0, ${0.5 - p * 0.1})`;
          ctx.fill();
        }

        // Dimensions
        ctx.font = '7px monospace';
        ctx.fillStyle = 'rgba(0, 194, 255, 0.5)';
        ctx.textAlign = 'center';
        ctx.fillText('160.00 mm', cx, cy + 65);
        ctx.save();
        ctx.translate(cx - 90, cy);
        ctx.rotate(-Math.PI / 2);
        ctx.fillText('100.00 mm', 0, 0);
        ctx.restore();

      } else if (activeProcess === 'robotics') {
        // Robotic arm visualization
        const armRot = Math.sin(t * 0.001) * 0.5;
        const armLen1 = 80;
        const armLen2 = 60;
        const armLen3 = 40;

        const baseX = cx;
        const baseY = cy + 40;

        // Base
        ctx.beginPath();
        ctx.arc(baseX, baseY, 15, 0, Math.PI * 2);
        ctx.strokeStyle = 'rgba(0, 194, 255, 0.5)';
        ctx.lineWidth = 2;
        ctx.stroke();
        ctx.fillStyle = 'rgba(0, 194, 255, 0.05)';
        ctx.fill();

        // Arm segments
        const j1x = baseX + Math.cos(-Math.PI / 2 + armRot * 0.5) * armLen1;
        const j1y = baseY + Math.sin(-Math.PI / 2 + armRot * 0.5) * armLen1;

        const j2x = j1x + Math.cos(-Math.PI / 3 + armRot) * armLen2;
        const j2y = j1y + Math.sin(-Math.PI / 3 + armRot) * armLen2;

        const j3x = j2x + Math.cos(-Math.PI / 4 + armRot * 1.5) * armLen3;
        const j3y = j2y + Math.sin(-Math.PI / 4 + armRot * 1.5) * armLen3;

        // Draw arms
        const drawArm = (x1: number, y1: number, x2: number, y2: number, width: number) => {
          ctx.beginPath();
          ctx.moveTo(x1, y1);
          ctx.lineTo(x2, y2);
          ctx.strokeStyle = 'rgba(0, 194, 255, 0.6)';
          ctx.lineWidth = width;
          ctx.stroke();

          // Joint
          ctx.beginPath();
          ctx.arc(x2, y2, 4, 0, Math.PI * 2);
          ctx.strokeStyle = 'rgba(0, 238, 255, 0.7)';
          ctx.lineWidth = 1.5;
          ctx.stroke();
        };

        drawArm(baseX, baseY, j1x, j1y, 4);
        drawArm(j1x, j1y, j2x, j2y, 3);
        drawArm(j2x, j2y, j3x, j3y, 2);

        // End effector
        ctx.beginPath();
        ctx.arc(j3x, j3y, 6, 0, Math.PI * 2);
        ctx.fillStyle = 'rgba(0, 255, 136, 0.3)';
        ctx.fill();
        ctx.strokeStyle = '#00ff88';
        ctx.lineWidth = 1;
        ctx.stroke();

        // Range circles
        ctx.beginPath();
        ctx.arc(baseX, baseY, armLen1 + armLen2 + armLen3, 0, Math.PI * 2);
        ctx.strokeStyle = 'rgba(0, 194, 255, 0.08)';
        ctx.lineWidth = 0.5;
        ctx.setLineDash([3, 5]);
        ctx.stroke();
        ctx.setLineDash([]);

        // Path preview
        ctx.beginPath();
        for (let i = 0; i <= 50; i++) {
          const pt = i / 50;
          const px = baseX + Math.cos(pt * Math.PI * 2) * 50 + Math.sin(pt * 4) * 20;
          const py = cy - 20 + Math.sin(pt * Math.PI * 2) * 30;
          if (i === 0) ctx.moveTo(px, py);
          else ctx.lineTo(px, py);
        }
        ctx.strokeStyle = 'rgba(136, 85, 255, 0.3)';
        ctx.lineWidth = 0.5;
        ctx.setLineDash([2, 3]);
        ctx.stroke();
        ctx.setLineDash([]);

      } else {
        // Assembly / Inspection view - exploded parts
        const parts = [
          { x: cx - 60, y: cy - 60, w: 40, h: 25, label: 'Housing', color: '#00c2ff' },
          { x: cx + 20, y: cy - 50, w: 35, h: 20, label: 'Bearing', color: '#00ff88' },
          { x: cx - 30, y: cy, w: 60, h: 30, label: 'Shaft', color: '#ffaa00' },
          { x: cx + 40, y: cy + 10, w: 30, h: 25, label: 'Seal', color: '#8855ff' },
          { x: cx - 50, y: cy + 45, w: 45, h: 20, label: 'Flange', color: '#ff6600' },
          { x: cx + 20, y: cy + 50, w: 35, h: 18, label: 'Fasteners', color: '#00eeff' },
        ];

        const explodeOffset = Math.sin(t * 0.0005) * 15;

        parts.forEach((part, i) => {
          const offsetX = (part.x - cx) * 0.1 * (1 + explodeOffset * 0.02);
          const offsetY = (part.y - cy) * 0.1 * (1 + explodeOffset * 0.02);

          ctx.beginPath();
          ctx.roundRect(part.x + offsetX, part.y + offsetY, part.w, part.h, 3);
          ctx.fillStyle = part.color + '10';
          ctx.fill();
          ctx.strokeStyle = part.color + '60';
          ctx.lineWidth = 1;
          ctx.stroke();

          // Connection lines to center
          ctx.beginPath();
          ctx.moveTo(part.x + offsetX + part.w / 2, part.y + offsetY + part.h / 2);
          ctx.lineTo(cx, cy);
          ctx.strokeStyle = part.color + '15';
          ctx.lineWidth = 0.5;
          ctx.setLineDash([2, 4]);
          ctx.stroke();
          ctx.setLineDash([]);

          ctx.font = '7px monospace';
          ctx.fillStyle = part.color + 'aa';
          ctx.textAlign = 'center';
          ctx.fillText(part.label, part.x + offsetX + part.w / 2, part.y + offsetY + part.h + 10);

          // Step highlight
          if (i === processStep) {
            ctx.beginPath();
            ctx.roundRect(part.x + offsetX - 2, part.y + offsetY - 2, part.w + 4, part.h + 4, 4);
            ctx.strokeStyle = '#00ff88';
            ctx.lineWidth = 1.5;
            ctx.stroke();
          }
        });
      }

      animRef.current = requestAnimationFrame(render);
    };

    animRef.current = requestAnimationFrame(render);
    return () => cancelAnimationFrame(animRef.current);
  }, [activeProcess, processStep, toolpathResult]);

  const currentProcess = processes.find(p => p.id === activeProcess)!;

  const generateToolpath = async () => {
    setIsGeneratingToolpath(true);
    setToolpathError(null);
    try {
      const response = await fetch('/api/manufacturing/toolpath', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          componentName: 'Actuated Bracket Demo Block',
          operation: cncOperation,
          bounds: { length: 0.16, width: 0.10, depth: 0.012 },
          toolDiameterMm: 10,
          stepdownMm: 2,
          stepoverMm: 4,
          feedRateMmMin: 1200,
          plungeRateMmMin: 300,
          spindleSpeedRpm: 8000,
        }),
      });

      if (!response.ok) {
        throw new Error(`Toolpath request failed (${response.status})`);
      }

      const payload = await response.json();
      setToolpathResult(payload);
      setActiveProcess('machining');
    } catch (error) {
      setToolpathError(error instanceof Error ? error.message : 'Toolpath generation failed');
    } finally {
      setIsGeneratingToolpath(false);
    }
  };

  return (
    <div className="flex h-full">
      <div className="flex-1 relative">
        <canvas ref={canvasRef} className="w-full h-full" />
        
        {/* Process selector */}
        <div className="absolute top-3 left-3 flex gap-1">
          {processes.map(p => (
            <button
              key={p.id}
              onClick={() => setActiveProcess(p.id)}
              className={`px-2.5 py-1.5 text-[9px] font-bold tracking-wider rounded-lg transition-all flex items-center gap-1.5 ${
                activeProcess === p.id
                  ? 'glass-panel text-forge-accent border border-forge-accent/30 glow-accent'
                  : 'bg-forge-panel/50 text-forge-text-muted hover:text-forge-text-dim border border-transparent'
              }`}
            >
              <span>{p.icon}</span>
              {p.label}
            </button>
          ))}
        </div>

        {/* Process steps */}
        <div className="absolute bottom-3 left-3 right-3 glass-panel rounded-lg p-3">
          <div className="flex items-center gap-3">
            <span className="text-[8px] text-forge-text-muted tracking-wider flex-shrink-0">PROCESS STEPS</span>
            <div className="flex-1 flex items-center gap-1">
              {currentProcess.steps.map((step, i) => (
                <div key={i} className="flex items-center gap-1 flex-1">
                  <div className={`flex items-center gap-1.5 flex-1 p-1.5 rounded text-[8px] font-medium transition-all ${
                    i < processStep ? 'bg-forge-green/10 text-forge-green border border-forge-green/20' :
                    i === processStep ? 'bg-forge-accent/10 text-forge-accent border border-forge-accent/30 animate-pulse-glow' :
                    'bg-forge-panel/30 text-forge-text-muted border border-forge-border/30'
                  }`}>
                    <span className="text-[7px]">
                      {i < processStep ? '✓' : i === processStep ? '▶' : '○'}
                    </span>
                    {step}
                  </div>
                  {i < currentProcess.steps.length - 1 && (
                    <div className={`w-4 h-px ${i < processStep ? 'bg-forge-green/50' : 'bg-forge-border'}`} />
                  )}
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>

      {/* Manufacturing metrics */}
      <div className="w-56 border-l border-forge-border bg-forge-surface overflow-y-auto p-2 space-y-2">
        <div className="text-[9px] font-bold text-forge-text tracking-wider px-1 mb-1">MFG METRICS</div>

        <div className="glass-panel rounded-lg p-2.5">
          <div className="flex items-center justify-between mb-2">
            <div className="text-[8px] font-bold text-forge-accent tracking-wider">CNC TOOLPATH</div>
            <span className="text-[7px] text-forge-text-muted">PLAN 22</span>
          </div>

          <label className="block text-[7px] text-forge-text-muted tracking-wider mb-1">OPERATION</label>
          <select
            value={cncOperation}
            onChange={(event) => setCncOperation(event.target.value as 'pocket' | 'profile' | 'facing')}
            className="w-full mb-2 bg-forge-panel border border-forge-border rounded px-2 py-1 text-[9px] text-forge-text font-mono"
          >
            <option value="pocket">Pocket roughing</option>
            <option value="profile">Profile contour</option>
            <option value="facing">Facing raster</option>
          </select>

          <button
            type="button"
            onClick={generateToolpath}
            disabled={isGeneratingToolpath}
            className="w-full rounded bg-forge-accent/15 border border-forge-accent/30 text-forge-accent hover:bg-forge-accent/25 disabled:opacity-50 px-2 py-1.5 text-[9px] font-bold tracking-wider"
          >
            {isGeneratingToolpath ? 'GENERATING…' : 'GENERATE G-CODE'}
          </button>

          {toolpathError && (
            <div className="mt-2 text-[8px] text-forge-danger leading-relaxed">{toolpathError}</div>
          )}

          {toolpathResult && (
            <div className="mt-2 space-y-1.5">
              <div className="flex justify-between text-[8px]">
                <span className="text-forge-text-muted">Engine</span>
                <span className="text-forge-text font-mono text-right">{toolpathResult.engineUsed || toolpathResult.engine_used}</span>
              </div>
              <div className="flex justify-between text-[8px]">
                <span className="text-forge-text-muted">Passes</span>
                <span className="text-forge-green font-mono">{toolpathResult.metrics.passes}</span>
              </div>
              <div className="flex justify-between text-[8px]">
                <span className="text-forge-text-muted">Cut Length</span>
                <span className="text-forge-text font-mono">{toolpathResult.metrics.cutLengthMm.toFixed(1)} mm</span>
              </div>
              <div className="flex justify-between text-[8px]">
                <span className="text-forge-text-muted">Lines</span>
                <span className="text-forge-text font-mono">{toolpathResult.metrics.lineCount}</span>
              </div>
              <pre className="max-h-24 overflow-auto rounded bg-black/30 border border-forge-border/50 p-1.5 text-[7px] text-forge-green/80 font-mono">
                {toolpathResult.gcode.split('\n').slice(0, 10).join('\n')}
              </pre>
              {toolpathResult.warnings.length > 0 && (
                <div className="text-[8px] text-forge-warning">
                  {toolpathResult.warnings[0]}
                </div>
              )}
            </div>
          )}
        </div>
        
        {[
          { label: 'Cycle Time', value: '47.3 min', target: '50.0 min', status: 'good' },
          { label: 'Tool Wear', value: '23%', target: '< 40%', status: 'good' },
          { label: 'Surface Finish', value: 'Ra 0.8μm', target: 'Ra 1.6μm', status: 'good' },
          { label: 'Tolerance', value: '±0.01mm', target: '±0.02mm', status: 'good' },
          { label: 'Material Removal', value: '34.2%', target: '< 40%', status: 'warning' },
          { label: 'Spindle Load', value: '78%', target: '< 85%', status: 'warning' },
          { label: 'Coolant Temp', value: '28.4°C', target: '< 30°C', status: 'good' },
          { label: 'Scrap Rate', value: '0.3%', target: '< 1.0%', status: 'good' },
        ].map(m => (
          <div key={m.label} className="glass-panel rounded-lg p-2">
            <div className="flex justify-between items-center mb-1">
              <span className="text-[8px] text-forge-text-muted">{m.label}</span>
              <span className={`text-[7px] font-bold tracking-wider ${
                m.status === 'good' ? 'text-forge-green' : 'text-forge-warning'
              }`}>
                {m.status === 'good' ? '✓ PASS' : '⚠ CHECK'}
              </span>
            </div>
            <div className="flex justify-between items-baseline">
              <span className="text-sm font-bold text-forge-text font-mono">{m.value}</span>
              <span className="text-[8px] text-forge-text-muted">target: {m.target}</span>
            </div>
          </div>
        ))}

        <div className="glass-panel rounded-lg p-2.5 mt-2">
          <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">PRODUCTION STATUS</div>
          <div className="space-y-1.5">
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-muted">Batch</span>
              <span className="text-forge-text font-mono">GT-7800-B42</span>
            </div>
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-muted">Parts Complete</span>
              <span className="text-forge-green font-mono">147 / 200</span>
            </div>
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-muted">ETA</span>
              <span className="text-forge-text font-mono">06:24:00</span>
            </div>
            <div className="h-1 bg-forge-panel rounded-full overflow-hidden mt-1">
              <div className="h-full rounded-full bg-forge-green w-[73%]" />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
