import { useState, useEffect, useRef } from 'react';
import { SensorData } from '../hooks/useSimulationData';

interface DigitalTwinViewProps {
  sensors: SensorData[];
  simulationTime: number;
}

interface SystemStatus {
  name: string;
  health: number;
  status: 'operational' | 'degraded' | 'maintenance' | 'critical';
  rul: number; // Remaining useful life in hours
  lastMaintenance: string;
}

const systems: SystemStatus[] = [
  { name: 'Primary Drive', health: 94, status: 'operational', rul: 12400, lastMaintenance: '2024-11-15' },
  { name: 'Bearing Assembly A', health: 78, status: 'degraded', rul: 4200, lastMaintenance: '2024-10-22' },
  { name: 'Cooling System', health: 91, status: 'operational', rul: 8900, lastMaintenance: '2024-11-01' },
  { name: 'Hydraulic Unit', health: 96, status: 'operational', rul: 15600, lastMaintenance: '2024-11-18' },
  { name: 'Gearbox Assembly', health: 85, status: 'operational', rul: 6700, lastMaintenance: '2024-10-30' },
  { name: 'Control Electronics', health: 99, status: 'operational', rul: 22000, lastMaintenance: '2024-11-20' },
  { name: 'Seal System B', health: 62, status: 'maintenance', rul: 1200, lastMaintenance: '2024-09-15' },
  { name: 'Exhaust Manifold', health: 71, status: 'degraded', rul: 3100, lastMaintenance: '2024-10-05' },
];

export default function DigitalTwinView({ sensors, simulationTime }: DigitalTwinViewProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animRef = useRef(0);
  const [selectedSystem, setSelectedSystem] = useState<string>('Primary Drive');
  const [systemStatuses, setSystemStatuses] = useState(systems);

  useEffect(() => {
    setSystemStatuses(prev => prev.map(s => ({
      ...s,
      health: Math.max(10, Math.min(100, s.health + (Math.random() - 0.52) * 0.3)),
    })));
  }, [simulationTime]);

  // Digital Twin 3D visualization canvas
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

      // Background
      ctx.fillStyle = '#0a0c10';
      ctx.fillRect(0, 0, cw, ch);

      // Grid
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.3)';
      ctx.lineWidth = 0.5;
      for (let x = 0; x < cw; x += 30) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, ch);
        ctx.stroke();
      }
      for (let y = 0; y < ch; y += 30) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(cw, y);
        ctx.stroke();
      }

      const cx = cw / 2;
      const cy = ch / 2;
      const rot = t * 0.0003;

      // Draw machine assembly - isometric view
      const drawBlock = (x: number, y: number, w: number, h: number, d: number, _color: string, health: number) => {
        const iso = (bx: number, by: number, bz: number) => ({
          x: cx + (bx - by) * 0.7,
          y: cy + (bx + by) * 0.35 - bz,
        });

        const rx = x * Math.cos(rot) - y * Math.sin(rot);
        const ry = x * Math.sin(rot) + y * Math.cos(rot);

        const p1 = iso(rx, ry, 0);
        const p2 = iso(rx + w, ry, 0);
        const p3 = iso(rx + w, ry + d, 0);
        iso(rx, ry + d, 0); // p4 - back-left bottom
        const p5 = iso(rx, ry, h);
        const p6 = iso(rx + w, ry, h);
        const p7 = iso(rx + w, ry + d, h);
        const p8 = iso(rx, ry + d, h);

        const healthColor = health > 80 ? 'rgba(0, 255, 136, 0.08)' :
                           health > 60 ? 'rgba(255, 170, 0, 0.08)' :
                           'rgba(255, 51, 68, 0.1)';

        const strokeColor = health > 80 ? 'rgba(0, 194, 255, 0.4)' :
                           health > 60 ? 'rgba(255, 170, 0, 0.5)' :
                           'rgba(255, 51, 68, 0.6)';

        // Top face
        ctx.beginPath();
        ctx.moveTo(p5.x, p5.y);
        ctx.lineTo(p6.x, p6.y);
        ctx.lineTo(p7.x, p7.y);
        ctx.lineTo(p8.x, p8.y);
        ctx.closePath();
        ctx.fillStyle = healthColor;
        ctx.fill();
        ctx.strokeStyle = strokeColor;
        ctx.lineWidth = 1;
        ctx.stroke();

        // Front face
        ctx.beginPath();
        ctx.moveTo(p5.x, p5.y);
        ctx.lineTo(p6.x, p6.y);
        ctx.lineTo(p2.x, p2.y);
        ctx.lineTo(p1.x, p1.y);
        ctx.closePath();
        ctx.fillStyle = healthColor;
        ctx.fill();
        ctx.strokeStyle = strokeColor;
        ctx.lineWidth = 0.5;
        ctx.stroke();

        // Side face
        ctx.beginPath();
        ctx.moveTo(p6.x, p6.y);
        ctx.lineTo(p7.x, p7.y);
        ctx.lineTo(p3.x, p3.y);
        ctx.lineTo(p2.x, p2.y);
        ctx.closePath();
        ctx.fillStyle = healthColor;
        ctx.fill();
        ctx.strokeStyle = strokeColor;
        ctx.stroke();

        // Health glow
        if (health < 70) {
          const glowAlpha = 0.1 + Math.sin(t * 0.003) * 0.05;
          const glow = ctx.createRadialGradient(
            (p5.x + p7.x) / 2, (p5.y + p7.y) / 2, 0,
            (p5.x + p7.x) / 2, (p5.y + p7.y) / 2, 40
          );
          glow.addColorStop(0, `rgba(255, 51, 68, ${glowAlpha})`);
          glow.addColorStop(1, 'transparent');
          ctx.fillStyle = glow;
          ctx.fillRect((p5.x + p7.x) / 2 - 40, (p5.y + p7.y) / 2 - 40, 80, 80);
        }
      };

      // Draw machine components
      drawBlock(-80, -40, 160, 30, 80, '#00c2ff', systemStatuses[0].health);
      drawBlock(-60, -30, 40, 25, 50, '#00ff88', systemStatuses[1].health);
      drawBlock(20, -30, 40, 25, 50, '#00ff88', systemStatuses[4].health);
      drawBlock(-40, -50, 80, 15, 30, '#ffaa00', systemStatuses[2].health);
      drawBlock(-90, -20, 20, 20, 40, '#8855ff', systemStatuses[5].health);
      drawBlock(70, -20, 20, 20, 40, '#8855ff', systemStatuses[3].health);
      drawBlock(-50, 30, 100, 10, 20, '#ff6600', systemStatuses[6].health);
      drawBlock(-30, 45, 60, 8, 15, '#ff3344', systemStatuses[7].health);

      // Connection lines (data flow)
      const drawFlow = (x1: number, y1: number, x2: number, y2: number) => {
        ctx.beginPath();
        ctx.moveTo(cx + x1, cy + y1);
        ctx.lineTo(cx + x2, cy + y2);
        ctx.setLineDash([3, 5]);
        ctx.lineDashOffset = -(t * 0.02);
        ctx.strokeStyle = 'rgba(0, 194, 255, 0.2)';
        ctx.lineWidth = 0.5;
        ctx.stroke();
        ctx.setLineDash([]);
      };

      drawFlow(-60, -10, -40, 20);
      drawFlow(60, -10, 40, 20);
      drawFlow(0, -30, 0, 10);

      // Sensor indicators
      const sensorPositions = [
        { x: cx - 40, y: cy - 50, sensor: sensors[0] },
        { x: cx + 50, y: cy - 30, sensor: sensors[1] },
        { x: cx - 60, y: cy + 10, sensor: sensors[2] },
        { x: cx + 30, y: cy + 40, sensor: sensors[3] },
      ];

      sensorPositions.forEach(sp => {
        const color = sp.sensor.status === 'critical' ? '#ff3344' :
                     sp.sensor.status === 'warning' ? '#ffaa00' : '#00ff88';
        
        ctx.beginPath();
        ctx.arc(sp.x, sp.y, 4, 0, Math.PI * 2);
        ctx.fillStyle = color;
        ctx.fill();

        // Pulse ring
        const pulseSize = 4 + Math.sin(t * 0.004) * 3;
        ctx.beginPath();
        ctx.arc(sp.x, sp.y, pulseSize, 0, Math.PI * 2);
        ctx.strokeStyle = color.replace(')', ', 0.3)').replace('#', 'rgba(').replace(/([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})/, (_, r, g, b) => `${parseInt(r, 16)}, ${parseInt(g, 16)}, ${parseInt(b, 16)}`);
        ctx.strokeStyle = `${color}44`;
        ctx.lineWidth = 1;
        ctx.stroke();

        // Value label
        ctx.font = '8px monospace';
        ctx.fillStyle = 'rgba(228, 232, 240, 0.7)';
        ctx.textAlign = 'center';
        ctx.fillText(`${sp.sensor.value.toFixed(1)} ${sp.sensor.unit}`, sp.x, sp.y - 10);
      });

      animRef.current = requestAnimationFrame(render);
    };

    animRef.current = requestAnimationFrame(render);
    return () => cancelAnimationFrame(animRef.current);
  }, [sensors, systemStatuses]);

  // Selected system info is used in the render below

  return (
    <div className="flex h-full">
      {/* Twin Canvas */}
      <div className="flex-1 relative">
        <canvas ref={canvasRef} className="w-full h-full" />
        
        {/* Overlay info */}
        <div className="absolute top-3 left-3 glass-panel rounded-lg p-2.5 glow-accent">
          <div className="text-[8px] text-forge-accent font-bold tracking-wider mb-1">DIGITAL TWIN — LIVE</div>
          <div className="text-[9px] text-forge-text-dim">Structural Assembly GT-7800</div>
          <div className="text-[8px] text-forge-green mt-1 flex items-center gap-1">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse-glow" />
            SYNCHRONIZED
          </div>
        </div>

        {/* Overall health ring */}
        <div className="absolute top-3 right-3 glass-panel rounded-lg p-3 text-center">
          <div className="text-[7px] text-forge-text-muted tracking-wider mb-1">OVERALL HEALTH</div>
          <div className="relative w-14 h-14 mx-auto">
            <svg viewBox="0 0 40 40" className="w-full h-full -rotate-90">
              <circle cx="20" cy="20" r="16" fill="none" stroke="rgba(30,34,48,0.5)" strokeWidth="3" />
              <circle cx="20" cy="20" r="16" fill="none" stroke="#00ff88" strokeWidth="3"
                strokeDasharray={`${84.5 * 100.53 / 100} 100.53`}
                strokeLinecap="round"
              />
            </svg>
            <div className="absolute inset-0 flex items-center justify-center">
              <span className="text-sm font-bold text-forge-green font-mono">84.5</span>
            </div>
          </div>
          <div className="text-[7px] text-forge-green mt-1">GOOD</div>
        </div>
      </div>

      {/* System Status Panel */}
      <div className="w-64 border-l border-forge-border bg-forge-surface overflow-y-auto p-2 space-y-1.5">
        <div className="text-[9px] font-bold text-forge-text tracking-wider px-1 mb-2">SUBSYSTEM STATUS</div>
        
        {systemStatuses.map(sys => {
          const isSelected = sys.name === selectedSystem;
          const statusColor = sys.status === 'operational' ? '#00ff88' :
                             sys.status === 'degraded' ? '#ffaa00' :
                             sys.status === 'maintenance' ? '#ff6600' : '#ff3344';
          
          return (
            <div
              key={sys.name}
              onClick={() => setSelectedSystem(sys.name)}
              className={`rounded-lg p-2.5 cursor-pointer transition-all ${
                isSelected ? 'glass-panel border border-forge-accent/30' : 'hover:bg-forge-panel/50'
              }`}
            >
              <div className="flex items-center justify-between mb-1.5">
                <div className="flex items-center gap-1.5">
                  <div className="w-1.5 h-1.5 rounded-full" style={{ backgroundColor: statusColor }} />
                  <span className="text-[9px] font-medium text-forge-text">{sys.name}</span>
                </div>
                <span className="text-[7px] font-bold tracking-wider" style={{ color: statusColor }}>
                  {sys.status.toUpperCase()}
                </span>
              </div>

              {/* Health bar */}
              <div className="flex items-center gap-2">
                <div className="flex-1 h-1 bg-forge-panel rounded-full overflow-hidden">
                  <div
                    className="h-full rounded-full transition-all duration-500"
                    style={{
                      width: `${sys.health}%`,
                      backgroundColor: statusColor,
                    }}
                  />
                </div>
                <span className="text-[8px] font-mono text-forge-text-dim w-8 text-right">
                  {sys.health.toFixed(0)}%
                </span>
              </div>

              {isSelected && (
                <div className="mt-2 pt-2 border-t border-forge-border/50 space-y-1 animate-fade-in-up">
                  <div className="flex justify-between text-[8px]">
                    <span className="text-forge-text-muted">RUL</span>
                    <span className="text-forge-text font-mono">{sys.rul.toLocaleString()} hrs</span>
                  </div>
                  <div className="flex justify-between text-[8px]">
                    <span className="text-forge-text-muted">Last Service</span>
                    <span className="text-forge-text font-mono">{sys.lastMaintenance}</span>
                  </div>
                  <div className="flex justify-between text-[8px]">
                    <span className="text-forge-text-muted">Next Service</span>
                    <span className={`font-mono ${sys.rul < 2000 ? 'text-forge-danger' : 'text-forge-text'}`}>
                      {sys.rul < 2000 ? 'OVERDUE' : `${Math.floor(sys.rul / 24)} days`}
                    </span>
                  </div>
                </div>
              )}
            </div>
          );
        })}

        {/* Lifecycle chart */}
        <div className="glass-panel rounded-lg p-2.5 mt-3">
          <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">LIFECYCLE DEGRADATION</div>
          <div className="h-20">
            <svg viewBox="0 0 200 80" className="w-full h-full">
              <defs>
                <linearGradient id="lifecycleGrad" x1="0" y1="0" x2="0" y2="1">
                  <stop offset="0%" stopColor="#00ff88" stopOpacity="0.3" />
                  <stop offset="100%" stopColor="#00ff88" stopOpacity="0" />
                </linearGradient>
              </defs>
              {/* Threshold line */}
              <line x1="0" y1="60" x2="200" y2="60" stroke="rgba(255,51,68,0.3)" strokeWidth="0.5" strokeDasharray="3,3" />
              <text x="195" y="58" fill="rgba(255,51,68,0.5)" fontSize="5" textAnchor="end">CRITICAL</text>
              
              {/* Degradation curve */}
              <path
                d="M0,10 Q50,12 80,18 Q120,28 150,42 Q175,55 200,72"
                fill="none"
                stroke="#00ff88"
                strokeWidth="1.5"
              />
              <path
                d="M0,10 Q50,12 80,18 Q120,28 150,42 Q175,55 200,72 L200,80 L0,80 Z"
                fill="url(#lifecycleGrad)"
              />
              
              {/* Current position */}
              <circle cx="120" cy="28" r="3" fill="#00c2ff" stroke="#00c2ff" strokeWidth="1" opacity="0.8" />
              <text x="125" y="25" fill="#00c2ff" fontSize="5">NOW</text>
            </svg>
          </div>
        </div>
      </div>
    </div>
  );
}
