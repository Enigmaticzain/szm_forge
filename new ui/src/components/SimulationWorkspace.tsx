import React, { useState, useEffect, useRef } from 'react';
import { ViewportRenderMode } from '../types';
import { Viewport3D } from './Viewport3D';
import { solverMetrics } from '../data/mockData';
import {
  Cpu, Play, Settings,
  Grid3x3, Layers, CheckCircle,
  Activity, Gauge, Clock, Brain
} from 'lucide-react';
import { useForgeStore } from '../store/ForgeStoreContext';

interface Props {
  renderMode: ViewportRenderMode;
  viewport?: React.ReactNode;
}

const simTypes = [
  { id: 'static', label: 'Static Structural', active: true },
  { id: 'thermal', label: 'Thermal Steady-State', active: true },
  { id: 'modal', label: 'Modal Analysis', active: false },
  { id: 'fatigue', label: 'Fatigue Life', active: false },
  { id: 'cfd', label: 'CFD Flow', active: false },
  { id: 'coupled', label: 'Coupled Multi-Physics', active: false },
];

export const SimulationWorkspace: React.FC<Props> = ({ renderMode, viewport }) => {
  const [activeSim, setActiveSim] = useState('static');
  const [convergenceData, setConvergenceData] = useState<number[]>([]);
  const [optimizing, setOptimizing] = useState(false);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const { showToast } = useForgeStore();

  useEffect(() => {
    if (optimizing) return;
    const initial = Array.from({ length: 30 }, (_, i) => Math.exp(-i * 0.1) * 100);
    setConvergenceData(initial);
    const interval = setInterval(() => {
      setConvergenceData(prev => {
        const next = [...prev];
        const last = next[next.length - 1];
        next.push(Math.max(0.001, last * (0.92 + Math.random() * 0.06)));
        if (next.length > 50) next.shift();
        return next;
      });
    }, 500);
    return () => clearInterval(interval);
  }, [optimizing]);

  const handleOptimize = async () => {
    setOptimizing(true);
    showToast('Initializing AI Shape Optimizer...');
    setConvergenceData([]);
    
    try {
      const res = await fetch('http://localhost:8000/api/simulation/optimize', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ iterations: 40 })
      });
      const data = await res.json();
      
      if (data.history) {
        let i = 0;
        const interval = setInterval(() => {
          if (i < data.history.length) {
            setConvergenceData(prev => {
              const next = [...prev, data.history[i].stress_mpa];
              if (next.length > 50) next.shift();
              return next;
            });
            i++;
          } else {
            clearInterval(interval);
            showToast(`Optimization complete! Final stress: ${data.final_stress} MPa`);
            setTimeout(() => setOptimizing(false), 5000);
          }
        }, 100);
      } else {
        setOptimizing(false);
      }
    } catch (err) {
      showToast('Optimization failed to connect.');
      setOptimizing(false);
    }
  };

  // Draw convergence chart
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || convergenceData.length < 2) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const w = canvas.width;
    const h = canvas.height;
    ctx.clearRect(0, 0, w, h);

    const max = Math.max(...convergenceData);
    const min = Math.min(...convergenceData);
    const range = max - min || 1;

    // Grid
    ctx.strokeStyle = 'rgba(30, 30, 42, 0.5)';
    ctx.lineWidth = 0.5;
    for (let i = 0; i < 5; i++) {
      const y = (i / 4) * h;
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(w, y);
      ctx.stroke();
    }

    // Line
    ctx.beginPath();
    convergenceData.forEach((v, i) => {
      const x = (i / (convergenceData.length - 1)) * w;
      const y = h - ((v - min) / range) * h * 0.9 - h * 0.05;
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 1.5;
    ctx.stroke();

    // Fill
    const lastX = w;
    const lastY = h - ((convergenceData[convergenceData.length - 1] - min) / range) * h * 0.9 - h * 0.05;
    ctx.lineTo(lastX, h);
    ctx.lineTo(0, h);
    ctx.closePath();
    const grad = ctx.createLinearGradient(0, 0, 0, h);
    grad.addColorStop(0, 'rgba(0, 212, 255, 0.15)');
    grad.addColorStop(1, 'rgba(0, 212, 255, 0)');
    ctx.fillStyle = grad;
    ctx.fill();

    // Current value dot
    ctx.beginPath();
    ctx.arc(lastX, lastY, 3, 0, Math.PI * 2);
    ctx.fillStyle = '#00d4ff';
    ctx.fill();

  }, [convergenceData]);

  return (
    <div className="h-full flex flex-col bg-forge-black">
      {/* Sim Header */}
      <div className="h-[48px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-4 flex-shrink-0">
        <Cpu size={14} className="text-forge-cyan" />
        <span className="text-[10px] font-bold tracking-widest text-forge-cyan">SIMULATION ENGINE</span>

        <div className="flex items-center gap-1 ml-4">
          {simTypes.map(sim => (
            <button
              key={sim.id}
              type="button"
              onClick={() => {
                if (!sim.active) {
                  showToast(`${sim.label} — enable in next release`);
                  return;
                }
                setActiveSim(sim.id);
              }}
              className={`px-2.5 py-1 rounded text-[8px] font-mono tracking-wider transition-all ${
                activeSim === sim.id
                  ? 'bg-forge-cyan/10 text-forge-cyan border border-forge-cyan/30'
                  : 'text-forge-text-muted hover:text-forge-text-dim hover:bg-forge-surface border border-transparent'
              }`}
            >
              {sim.label}
            </button>
          ))}
        </div>

        <div className="ml-auto flex items-center gap-2">
          <button 
            onClick={handleOptimize}
            disabled={optimizing}
            className="flex items-center gap-1 px-3 py-1 rounded text-[9px] font-mono bg-forge-purple/10 text-forge-purple border border-forge-purple/20 hover:bg-forge-purple/20 disabled:opacity-50 transition-colors"
          >
            <Brain size={10} /> {optimizing ? 'OPTIMIZING...' : 'AI OPTIMIZE'}
          </button>
          <button className="flex items-center gap-1 px-3 py-1 rounded text-[9px] font-mono bg-forge-green/10 text-forge-green border border-forge-green/20 hover:bg-forge-green/20">
            <Play size={10} /> SOLVE
          </button>
          <button className="p-1.5 rounded text-forge-text-muted hover:bg-forge-surface">
            <Settings size={12} />
          </button>
        </div>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Sim Setup Panel */}
        <div className="w-[260px] border-r border-forge-border bg-forge-panel overflow-y-auto">
          <div className="p-3 space-y-3">
            {/* Boundary Conditions */}
            <div>
              <div className="flex items-center gap-1.5 mb-2">
                <Layers size={10} className="text-forge-cyan" />
                <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">BOUNDARY CONDITIONS</span>
              </div>
              <div className="space-y-1">
                {[
                  { label: 'Fixed Support', target: 'Hub base face', icon: '🔒' },
                  { label: 'Centrifugal Load', target: '3200 RPM', icon: '🔄' },
                  { label: 'Pressure', target: '142 kPa (blade surfaces)', icon: '💨' },
                  { label: 'Temperature', target: '1247°C (inlet)', icon: '🌡️' },
                  { label: 'Cyclic Load', target: '±50 kN (10⁶ cycles)', icon: '📊' },
                ].map((bc, i) => (
                  <div key={i} className="flex items-center gap-2 p-2 rounded border border-forge-border/50 bg-forge-surface/20 hover:bg-forge-surface/40 transition-all cursor-pointer">
                    <span className="text-sm">{bc.icon}</span>
                    <div className="flex-1 min-w-0">
                      <div className="text-[9px] font-medium text-forge-text">{bc.label}</div>
                      <div className="text-[8px] font-mono text-forge-text-muted truncate">{bc.target}</div>
                    </div>
                  </div>
                ))}
              </div>
            </div>

            {/* Mesh Settings */}
            <div>
              <div className="flex items-center gap-1.5 mb-2">
                <Grid3x3 size={10} className="text-forge-cyan" />
                <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">MESH CONFIGURATION</span>
              </div>
              <div className="space-y-1.5">
                {[
                  { label: 'Element Type', value: 'Hex-dominant' },
                  { label: 'Global Size', value: '2.0 mm' },
                  { label: 'Min Size', value: '0.5 mm' },
                  { label: 'Growth Rate', value: '1.2' },
                  { label: 'Refinement', value: 'Contact regions' },
                  { label: 'Quality Target', value: '>0.7 Jacobian' },
                ].map((setting, i) => (
                  <div key={i} className="flex items-center justify-between px-2 py-1.5 rounded bg-forge-surface/20">
                    <span className="text-[8px] text-forge-text-dim">{setting.label}</span>
                    <span className="text-[9px] font-mono text-forge-accent">{setting.value}</span>
                  </div>
                ))}
              </div>
            </div>

            {/* Solver Settings */}
            <div>
              <div className="flex items-center gap-1.5 mb-2">
                <Cpu size={10} className="text-forge-cyan" />
                <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">SOLVER CONFIG</span>
              </div>
              <div className="space-y-1.5">
                {[
                  { label: 'Method', value: 'Iterative (PCG)' },
                  { label: 'Convergence', value: '1e-6' },
                  { label: 'Max Iterations', value: '1000' },
                  { label: 'Substeps', value: 'Auto (5-50)' },
                  { label: 'Nonlinearity', value: 'Large deflection ON' },
                  { label: 'Contact', value: 'Augmented Lagrange' },
                ].map((setting, i) => (
                  <div key={i} className="flex items-center justify-between px-2 py-1.5 rounded bg-forge-surface/20">
                    <span className="text-[8px] text-forge-text-dim">{setting.label}</span>
                    <span className="text-[9px] font-mono text-forge-cyan">{setting.value}</span>
                  </div>
                ))}
              </div>
            </div>
          </div>
        </div>

        {/* 3D Viewport */}
        <div className="flex-1 flex flex-col">
          <div className="flex-1 min-h-0">
            {viewport ?? <Viewport3D renderMode={renderMode} />}
          </div>

          {/* Convergence Monitor */}
          <div className="h-[120px] border-t border-forge-border bg-forge-dark px-4 py-2 flex gap-4">
            <div className="flex-1">
              <div className="flex items-center gap-1.5 mb-1">
                <Activity size={10} className="text-forge-accent" />
                <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">CONVERGENCE MONITOR</span>
                <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse ml-2" />
              </div>
              <canvas ref={canvasRef} width={500} height={70} className="w-full h-[70px]" />
            </div>
            <div className="w-[180px] grid grid-cols-2 gap-2">
              {[
                { label: 'ITERATION', value: `${solverMetrics.iterations}`, icon: <Clock size={9} /> },
                { label: 'RESIDUAL', value: `${solverMetrics.residual.toExponential(1)}`, icon: <Gauge size={9} /> },
                { label: 'WALL TIME', value: solverMetrics.wallTime, icon: <Clock size={9} /> },
                { label: 'STATUS', value: 'CONVERGED', icon: <CheckCircle size={9} className="text-forge-green" /> },
              ].map((stat, i) => (
                <div key={i} className="flex flex-col justify-center">
                  <span className="text-[7px] font-mono text-forge-text-muted flex items-center gap-1">{stat.icon} {stat.label}</span>
                  <span className="text-[11px] font-bold font-mono text-forge-text">{stat.value}</span>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
