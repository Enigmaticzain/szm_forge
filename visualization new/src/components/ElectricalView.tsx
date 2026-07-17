import { useRef, useEffect, useState } from 'react';

export default function ElectricalView() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animRef = useRef(0);
  const [activeLayer, setActiveLayer] = useState('current');

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

      // Dark grid
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.2)';
      ctx.lineWidth = 0.5;
      for (let x = 0; x < cw; x += 30) {
        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, ch); ctx.stroke();
      }
      for (let y = 0; y < ch; y += 30) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(cw, y); ctx.stroke();
      }

      const cx = cw / 2;
      const cy = ch / 2;

      // Circuit nodes
      const nodes = [
        { x: cx - 150, y: cy - 60, label: 'PSU', type: 'source' },
        { x: cx - 80, y: cy - 80, label: 'INV-A', type: 'inverter' },
        { x: cx + 10, y: cy - 90, label: 'INV-B', type: 'inverter' },
        { x: cx + 100, y: cy - 70, label: 'MOTOR', type: 'load' },
        { x: cx - 120, y: cy + 20, label: 'CTRL', type: 'controller' },
        { x: cx - 30, y: cy + 40, label: 'SENS', type: 'sensor' },
        { x: cx + 60, y: cy + 30, label: 'DRIV', type: 'driver' },
        { x: cx + 150, y: cy, label: 'GND', type: 'ground' },
        { x: cx - 60, y: cy - 20, label: 'BUS-A', type: 'bus' },
        { x: cx + 40, y: cy - 30, label: 'BUS-B', type: 'bus' },
      ];

      // Connection pairs
      const connections: [number, number][] = [
        [0, 1], [1, 2], [2, 3], [0, 4], [4, 5], [5, 6], [6, 3],
        [1, 8], [8, 9], [9, 2], [3, 7], [6, 7],
      ];

      // Draw connections with animated current flow
      connections.forEach(([from, to]) => {
        const n1 = nodes[from];
        const n2 = nodes[to];
        
        // Base line
        ctx.beginPath();
        ctx.moveTo(n1.x, n1.y);
        ctx.lineTo(n2.x, n2.y);
        ctx.strokeStyle = 'rgba(0, 238, 255, 0.15)';
        ctx.lineWidth = 1;
        ctx.stroke();

        // Animated flow
        const dx = n2.x - n1.x;
        const dy = n2.y - n1.y;
        const dist = Math.sqrt(dx * dx + dy * dy);
        const numDots = Math.floor(dist / 15);
        
        for (let d = 0; d < numDots; d++) {
          const progress = ((d / numDots) + (t * 0.001)) % 1;
          const px = n1.x + dx * progress;
          const py = n1.y + dy * progress;
          const alpha = Math.sin(progress * Math.PI) * 0.8;
          
          ctx.beginPath();
          ctx.arc(px, py, 2, 0, Math.PI * 2);
          ctx.fillStyle = `rgba(0, 238, 255, ${alpha})`;
          ctx.fill();
        }

        // Glow effect on line
        if (activeLayer === 'current') {
          const glowGrad = ctx.createLinearGradient(n1.x, n1.y, n2.x, n2.y);
          const pulseAlpha = 0.05 + Math.sin(t * 0.003 + from) * 0.03;
          glowGrad.addColorStop(0, `rgba(0, 238, 255, ${pulseAlpha})`);
          glowGrad.addColorStop(0.5, `rgba(0, 238, 255, ${pulseAlpha * 2})`);
          glowGrad.addColorStop(1, `rgba(0, 238, 255, ${pulseAlpha})`);
          ctx.beginPath();
          ctx.moveTo(n1.x, n1.y);
          ctx.lineTo(n2.x, n2.y);
          ctx.strokeStyle = glowGrad;
          ctx.lineWidth = 6;
          ctx.stroke();
        }
      });

      // Draw nodes
      nodes.forEach((node, i) => {
        const nodeColors: Record<string, string> = {
          source: '#00ff88',
          inverter: '#00c2ff',
          load: '#ff6600',
          controller: '#8855ff',
          sensor: '#ffaa00',
          driver: '#00eeff',
          ground: '#666',
          bus: '#00c2ff',
        };

        const color = nodeColors[node.type] || '#00c2ff';
        const pulseSize = 12 + Math.sin(t * 0.003 + i * 0.5) * 2;

        // Outer glow
        const glow = ctx.createRadialGradient(node.x, node.y, 0, node.x, node.y, pulseSize * 2);
        glow.addColorStop(0, color + '20');
        glow.addColorStop(1, 'transparent');
        ctx.fillStyle = glow;
        ctx.fillRect(node.x - pulseSize * 2, node.y - pulseSize * 2, pulseSize * 4, pulseSize * 4);

        // Node circle
        ctx.beginPath();
        ctx.arc(node.x, node.y, pulseSize, 0, Math.PI * 2);
        ctx.fillStyle = color + '15';
        ctx.fill();
        ctx.strokeStyle = color + '80';
        ctx.lineWidth = 1.5;
        ctx.stroke();

        // Inner dot
        ctx.beginPath();
        ctx.arc(node.x, node.y, 4, 0, Math.PI * 2);
        ctx.fillStyle = color;
        ctx.fill();

        // Label
        ctx.font = '8px monospace';
        ctx.fillStyle = color + 'cc';
        ctx.textAlign = 'center';
        ctx.fillText(node.label, node.x, node.y + pulseSize + 12);

        // Value
        const values: Record<string, string> = {
          PSU: '480V AC',
          'INV-A': '320V DC',
          'INV-B': '280V DC',
          MOTOR: '32.1A',
          CTRL: '5V Logic',
          SENS: '4-20mA',
          DRIV: '24V DC',
          GND: '0V REF',
          'BUS-A': '48V DC',
          'BUS-B': '48V DC',
        };
        ctx.font = '7px monospace';
        ctx.fillStyle = color + '88';
        ctx.fillText(values[node.label] || '', node.x, node.y + pulseSize + 20);
      });

      // EMF visualization rings (when in EM mode)
      if (activeLayer === 'electromagnetic') {
        const motorNode = nodes[3];
        for (let ring = 0; ring < 5; ring++) {
          const radius = 30 + ring * 20 + Math.sin(t * 0.002) * 5;
          ctx.beginPath();
          ctx.arc(motorNode.x, motorNode.y, radius, 0, Math.PI * 2);
          ctx.strokeStyle = `rgba(136, 85, 255, ${0.2 - ring * 0.035})`;
          ctx.lineWidth = 1;
          ctx.setLineDash([4, 8]);
          ctx.lineDashOffset = t * 0.02 * (ring % 2 === 0 ? 1 : -1);
          ctx.stroke();
          ctx.setLineDash([]);
        }
      }

      // Voltage gradient overlay
      if (activeLayer === 'voltage') {
        const voltGrad = ctx.createLinearGradient(cx - 170, 0, cx + 170, 0);
        voltGrad.addColorStop(0, 'rgba(0, 255, 136, 0.08)');
        voltGrad.addColorStop(0.3, 'rgba(0, 194, 255, 0.06)');
        voltGrad.addColorStop(0.7, 'rgba(255, 170, 0, 0.06)');
        voltGrad.addColorStop(1, 'rgba(255, 51, 68, 0.04)');
        ctx.fillStyle = voltGrad;
        ctx.fillRect(0, 0, cw, ch);
      }

      animRef.current = requestAnimationFrame(render);
    };

    animRef.current = requestAnimationFrame(render);
    return () => cancelAnimationFrame(animRef.current);
  }, [activeLayer]);

  return (
    <div className="flex h-full">
      <div className="flex-1 relative">
        <canvas ref={canvasRef} className="w-full h-full" />
        
        {/* Layer selector */}
        <div className="absolute top-3 left-3 flex gap-1">
          {[
            { id: 'current', label: 'CURRENT FLOW' },
            { id: 'voltage', label: 'VOLTAGE MAP' },
            { id: 'electromagnetic', label: 'EM FIELD' },
            { id: 'signal', label: 'SIGNAL PATH' },
          ].map(l => (
            <button
              key={l.id}
              onClick={() => setActiveLayer(l.id)}
              className={`px-2.5 py-1.5 text-[8px] font-bold tracking-wider rounded-lg transition-all ${
                activeLayer === l.id
                  ? 'glass-panel text-forge-electrical border border-forge-electrical/30'
                  : 'bg-forge-panel/50 text-forge-text-muted hover:text-forge-text-dim border border-transparent'
              }`}
            >
              {l.label}
            </button>
          ))}
        </div>
      </div>

      {/* Electrical diagnostics panel */}
      <div className="w-56 border-l border-forge-border bg-forge-surface overflow-y-auto p-2 space-y-2">
        <div className="text-[9px] font-bold text-forge-text tracking-wider px-1">ELECTRICAL DIAGNOSTICS</div>
        
        {[
          { label: 'Input Voltage', value: '480V AC', status: 'normal' },
          { label: 'Bus Voltage', value: '672V DC', status: 'normal' },
          { label: 'Phase A Current', value: '32.1A', status: 'normal' },
          { label: 'Phase B Current', value: '31.8A', status: 'normal' },
          { label: 'Phase C Current', value: '33.4A', status: 'warning' },
          { label: 'Power Factor', value: '0.94', status: 'normal' },
          { label: 'THD', value: '4.2%', status: 'normal' },
          { label: 'Frequency', value: '60.01 Hz', status: 'normal' },
          { label: 'Active Power', value: '45.2 kW', status: 'normal' },
          { label: 'Reactive Power', value: '12.8 kVAR', status: 'warning' },
          { label: 'Efficiency', value: '96.3%', status: 'normal' },
          { label: 'Temperature Rise', value: '42°C', status: 'normal' },
        ].map(d => (
          <div key={d.label} className="glass-panel rounded-lg p-2 flex items-center justify-between">
            <div>
              <div className="text-[8px] text-forge-text-muted">{d.label}</div>
              <div className="text-[11px] font-bold text-forge-text font-mono">{d.value}</div>
            </div>
            <div className={`w-1.5 h-1.5 rounded-full ${
              d.status === 'normal' ? 'bg-forge-green' : 'bg-forge-warning'
            }`} />
          </div>
        ))}

        {/* Power distribution chart */}
        <div className="glass-panel rounded-lg p-2.5">
          <div className="text-[8px] font-bold text-forge-electrical tracking-wider mb-2">POWER DISTRIBUTION</div>
          <div className="space-y-1.5">
            {[
              { name: 'Motor Drive', pct: 72, color: '#00eeff' },
              { name: 'Control Systems', pct: 8, color: '#8855ff' },
              { name: 'Cooling', pct: 12, color: '#00ff88' },
              { name: 'Auxiliaries', pct: 5, color: '#ffaa00' },
              { name: 'Losses', pct: 3, color: '#ff3344' },
            ].map(item => (
              <div key={item.name} className="space-y-0.5">
                <div className="flex justify-between text-[8px]">
                  <span className="text-forge-text-dim">{item.name}</span>
                  <span className="text-forge-text font-mono">{item.pct}%</span>
                </div>
                <div className="h-1 bg-forge-panel rounded-full overflow-hidden">
                  <div className="h-full rounded-full" style={{ width: `${item.pct}%`, backgroundColor: item.color }} />
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}
