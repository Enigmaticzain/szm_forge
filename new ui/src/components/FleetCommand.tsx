import React, { useState, useEffect, useRef } from 'react';
import { machineFleet } from '../data/mockData';
import {
  Activity, Globe, Cpu, Shield,
  AlertTriangle, CheckCircle, Wrench, XCircle,
  Signal, Wifi, MapPin, Eye, Zap
} from 'lucide-react';

const statusColors: Record<string, string> = {
  online: '#00ff88',
  degraded: '#ffcc00',
  maintenance: '#ff8800',
  offline: '#555570',
  critical: '#ff3355',
};

export const FleetCommand: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const animRef = useRef(0);
  const timeRef = useRef(0);
  const [alerts] = useState([
    { id: 1, msg: 'Pump P-01 bearing temperature exceeding threshold — 95°C', severity: 'critical', time: '14:32' },
    { id: 2, msg: 'System Beta-01 efficiency dropped below 90%', severity: 'warning', time: '14:28' },
    { id: 3, msg: 'System Beta-02 maintenance cycle initiated', severity: 'info', time: '14:18' },
    { id: 4, msg: 'All CNC units operating within tolerance', severity: 'success', time: '14:10' },
    { id: 5, msg: 'Generator G-01 load balancing optimized by AI', severity: 'success', time: '14:05' },
  ]);

  useEffect(() => {
    const canvas = canvasRef.current;
    const container = containerRef.current;
    if (!canvas || !container) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const resize = () => {
      const rect = container.getBoundingClientRect();
      canvas.width = rect.width;
      canvas.height = rect.height;
    };
    resize();
    window.addEventListener('resize', resize);

    const draw = () => {
      timeRef.current += 0.01;
      const t = timeRef.current;
      const w = canvas.width;
      const h = canvas.height;

      ctx.fillStyle = '#0a0a0f';
      ctx.fillRect(0, 0, w, h);

      // Hex grid background
      const hexR = 30;
      const hexH = hexR * Math.sqrt(3);
      for (let row = -1; row < h / hexH + 1; row++) {
        for (let col = -1; col < w / (hexR * 3) + 1; col++) {
          const cx = col * hexR * 3 + (row % 2 === 0 ? 0 : hexR * 1.5);
          const cy = row * hexH * 0.5;
          ctx.beginPath();
          for (let i = 0; i < 6; i++) {
            const angle = (Math.PI / 3) * i + Math.PI / 6;
            const hx = cx + hexR * 0.9 * Math.cos(angle);
            const hy = cy + hexR * 0.9 * Math.sin(angle);
            if (i === 0) ctx.moveTo(hx, hy);
            else ctx.lineTo(hx, hy);
          }
          ctx.closePath();
          ctx.strokeStyle = 'rgba(30, 30, 42, 0.25)';
          ctx.lineWidth = 0.5;
          ctx.stroke();
        }
      }

      // Machine nodes
      const centerX = w * 0.5;
      const centerY = h * 0.5;
      const orbitR = Math.min(w, h) * 0.32;

      // Central command hub
      const hubPulse = Math.sin(t * 2) * 0.2 + 0.8;
      ctx.beginPath();
      ctx.arc(centerX, centerY, 30, 0, Math.PI * 2);
      ctx.fillStyle = `rgba(0, 212, 255, ${0.1 * hubPulse})`;
      ctx.fill();
      ctx.strokeStyle = `rgba(0, 212, 255, ${0.5 * hubPulse})`;
      ctx.lineWidth = 2;
      ctx.stroke();

      ctx.beginPath();
      ctx.arc(centerX, centerY, 50, 0, Math.PI * 2);
      ctx.strokeStyle = 'rgba(0, 212, 255, 0.1)';
      ctx.lineWidth = 1;
      ctx.setLineDash([4, 4]);
      ctx.stroke();
      ctx.setLineDash([]);

      ctx.fillStyle = '#00d4ff';
      ctx.font = 'bold 8px "JetBrains Mono"';
      ctx.textAlign = 'center';
      ctx.fillText('COMMAND', centerX, centerY - 4);
      ctx.fillText('HUB', centerX, centerY + 6);

      // Draw machines in orbit
      machineFleet.forEach((machine, i) => {
        const angle = (Math.PI * 2 * i) / machineFleet.length + t * 0.05;
        const mx = centerX + Math.cos(angle) * orbitR;
        const my = centerY + Math.sin(angle) * orbitR;
        const nodeR = 18;
        const color = statusColors[machine.status];

        // Connection line to hub
        ctx.beginPath();
        ctx.moveTo(centerX, centerY);
        ctx.lineTo(mx, my);
        ctx.strokeStyle = color + '20';
        ctx.lineWidth = 1;
        ctx.stroke();

        // Data pulse along connection
        const pulsePos = (t * 0.5 + i * 0.3) % 1;
        const px = centerX + (mx - centerX) * pulsePos;
        const py = centerY + (my - centerY) * pulsePos;
        ctx.beginPath();
        ctx.arc(px, py, 2, 0, Math.PI * 2);
        ctx.fillStyle = color + '80';
        ctx.fill();

        // Node glow
        const glowGrad = ctx.createRadialGradient(mx, my, 0, mx, my, nodeR * 2);
        glowGrad.addColorStop(0, color + '20');
        glowGrad.addColorStop(1, 'transparent');
        ctx.fillStyle = glowGrad;
        ctx.fillRect(mx - nodeR * 2, my - nodeR * 2, nodeR * 4, nodeR * 4);

        // Node circle
        ctx.beginPath();
        ctx.arc(mx, my, nodeR, 0, Math.PI * 2);
        ctx.fillStyle = '#111118';
        ctx.fill();
        ctx.strokeStyle = color;
        ctx.lineWidth = 1.5;
        ctx.stroke();

        // Health arc
        ctx.beginPath();
        ctx.arc(mx, my, nodeR + 4, -Math.PI / 2, -Math.PI / 2 + (Math.PI * 2 * machine.health / 100));
        ctx.strokeStyle = color + '60';
        ctx.lineWidth = 2;
        ctx.stroke();

        // Status dot
        ctx.beginPath();
        ctx.arc(mx + nodeR * 0.6, my - nodeR * 0.6, 3, 0, Math.PI * 2);
        ctx.fillStyle = color;
        ctx.fill();

        // Label
        ctx.fillStyle = '#e8e8f0';
        ctx.font = '7px "JetBrains Mono"';
        ctx.textAlign = 'center';
        ctx.fillText(machine.name.split(' ').pop() || '', mx, my + 2);

        // Health text below
        ctx.fillStyle = color;
        ctx.font = '8px "JetBrains Mono"';
        ctx.fillText(`${machine.health}%`, mx, my + nodeR + 14);
      });

      // Orbit ring
      ctx.beginPath();
      ctx.arc(centerX, centerY, orbitR, 0, Math.PI * 2);
      ctx.strokeStyle = 'rgba(30, 30, 42, 0.3)';
      ctx.lineWidth = 1;
      ctx.setLineDash([2, 6]);
      ctx.stroke();
      ctx.setLineDash([]);

      animRef.current = requestAnimationFrame(draw);
    };

    draw();
    return () => {
      cancelAnimationFrame(animRef.current);
      window.removeEventListener('resize', resize);
    };
  }, []);

  return (
    <div className="h-full flex flex-col bg-forge-black">
      {/* Header */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-6 flex-shrink-0">
        <div className="flex items-center gap-2">
          <Globe size={14} className="text-forge-accent" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-accent">FLEET COMMAND CENTER</div>
            <div className="text-[8px] font-mono text-forge-text-muted">AUTONOMOUS OPERATIONS • DISTRIBUTED INTELLIGENCE • REAL-TIME ORCHESTRATION</div>
          </div>
        </div>
        <div className="ml-auto flex items-center gap-4">
          <div className="flex items-center gap-1.5">
            <Signal size={10} className="text-forge-green" />
            <span className="text-[9px] font-mono text-forge-text-dim">ALL SYSTEMS LINKED</span>
          </div>
          <div className="flex items-center gap-1.5">
            <Shield size={10} className="text-forge-accent" />
            <span className="text-[9px] font-mono text-forge-text-dim">ENCRYPTED</span>
          </div>
        </div>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Fleet Visualization */}
        <div ref={containerRef} className="flex-1 relative">
          <canvas ref={canvasRef} className="absolute inset-0 w-full h-full" />

          {/* Overlay stats */}
          <div className="absolute top-3 left-3 z-10 space-y-2">
            <div className="px-3 py-2 rounded bg-forge-dark/80 border border-forge-border/50 backdrop-blur-sm">
              <div className="text-[8px] font-mono text-forge-text-muted">FLEET STATUS</div>
              <div className="flex items-center gap-3 mt-1">
                <span className="flex items-center gap-1 text-[9px] font-mono text-forge-green">
                  <CheckCircle size={8} /> 5 Online
                </span>
                <span className="flex items-center gap-1 text-[9px] font-mono text-forge-yellow">
                  <AlertTriangle size={8} /> 1 Degraded
                </span>
                <span className="flex items-center gap-1 text-[9px] font-mono text-forge-red">
                  <XCircle size={8} /> 1 Critical
                </span>
                <span className="flex items-center gap-1 text-[9px] font-mono text-forge-orange">
                  <Wrench size={8} /> 1 Maint
                </span>
              </div>
            </div>
          </div>

          <div className="absolute bottom-3 left-3 z-10">
            <div className="px-3 py-2 rounded bg-forge-dark/80 border border-forge-border/50 backdrop-blur-sm">
              <div className="flex items-center gap-2">
                <Wifi size={10} className="text-forge-green" />
                <span className="text-[8px] font-mono text-forge-text-dim">UPLINK: 847 Mbps</span>
                <span className="text-[8px] font-mono text-forge-text-muted">|</span>
                <Cpu size={10} className="text-forge-accent" />
                <span className="text-[8px] font-mono text-forge-text-dim">EDGE COMPUTE: ACTIVE</span>
              </div>
            </div>
          </div>
        </div>

        {/* Alerts Panel */}
        <div className="w-[300px] border-l border-forge-border bg-forge-panel overflow-y-auto">
          <div className="px-3 py-2.5 border-b border-forge-border flex items-center gap-1.5">
            <AlertTriangle size={11} className="text-forge-yellow" />
            <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">LIVE ALERTS</span>
          </div>
          <div className="p-2 space-y-1.5">
            {alerts.map(alert => (
              <div
                key={alert.id}
                className={`p-2.5 rounded border transition-all ${
                  alert.severity === 'critical' ? 'border-forge-red/20 bg-forge-red/5' :
                  alert.severity === 'warning' ? 'border-forge-yellow/20 bg-forge-yellow/5' :
                  alert.severity === 'success' ? 'border-forge-green/20 bg-forge-green/5' :
                  'border-forge-accent/20 bg-forge-accent/5'
                }`}
              >
                <div className="flex items-center gap-1.5 mb-1">
                  {alert.severity === 'critical' ? <XCircle size={9} className="text-forge-red" /> :
                   alert.severity === 'warning' ? <AlertTriangle size={9} className="text-forge-yellow" /> :
                   alert.severity === 'success' ? <CheckCircle size={9} className="text-forge-green" /> :
                   <Eye size={9} className="text-forge-accent" />}
                  <span className="text-[8px] font-mono text-forge-text-muted">{alert.time}</span>
                  <span className={`text-[7px] font-mono tracking-wider ml-auto ${
                    alert.severity === 'critical' ? 'text-forge-red' :
                    alert.severity === 'warning' ? 'text-forge-yellow' :
                    alert.severity === 'success' ? 'text-forge-green' :
                    'text-forge-accent'
                  }`}>{alert.severity.toUpperCase()}</span>
                </div>
                <p className="text-[9px] text-forge-text-dim leading-relaxed">{alert.msg}</p>
              </div>
            ))}
          </div>

          {/* Quick Actions */}
          <div className="px-3 py-3 border-t border-forge-border">
            <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">QUICK ACTIONS</span>
            <div className="space-y-1 mt-2">
              {[
                { label: 'Emergency Shutdown All', icon: <XCircle size={10} />, color: 'text-forge-red' },
                { label: 'Rebalance Fleet Load', icon: <Activity size={10} />, color: 'text-forge-accent' },
                { label: 'Run Fleet Diagnostics', icon: <Zap size={10} />, color: 'text-forge-purple' },
                { label: 'Export Fleet Report', icon: <MapPin size={10} />, color: 'text-forge-text-dim' },
              ].map((action, i) => (
                <button
                  key={i}
                  className={`w-full flex items-center gap-2 px-2 py-1.5 rounded text-[9px] font-mono ${action.color} hover:bg-forge-surface transition-all text-left`}
                >
                  {action.icon}
                  {action.label}
                </button>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
