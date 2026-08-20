import { useRef, useEffect, useState } from 'react';

interface Viewport3DProps {
  mode: 'geometry' | 'stress' | 'thermal' | 'electrical' | 'twin' | 'manufacturing';
}

interface Particle {
  x: number;
  y: number;
  z: number;
  vx: number;
  vy: number;
  size: number;
  life: number;
  maxLife: number;
  color: string;
}

export default function Viewport3D({ mode }: Viewport3DProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animRef = useRef<number>(0);
  const timeRef = useRef(0);
  const [hoveredNode, setHoveredNode] = useState<{ x: number; y: number; data: Record<string, string> } | null>(null);
  const particlesRef = useRef<Particle[]>([]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    let canvasWidth = 0;
    let canvasHeight = 0;
    let dpr = window.devicePixelRatio || 1;

    const resize = () => {
      const rect = canvas.parentElement?.getBoundingClientRect();
      if (!rect) return;
      dpr = window.devicePixelRatio || 1;
      canvasWidth = Math.max(1, Math.floor(rect.width));
      canvasHeight = Math.max(1, Math.floor(rect.height));
      canvas.width = canvasWidth * dpr;
      canvas.height = canvasHeight * dpr;
      canvas.style.width = `${canvasWidth}px`;
      canvas.style.height = `${canvasHeight}px`;
      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    };
    resize();

    const parent = canvas.parentElement;
    const resizeObserver = new ResizeObserver(() => resize());
    if (parent) resizeObserver.observe(parent);

    // Initialize particles
    particlesRef.current = Array.from({ length: 50 }, () => ({
      x: Math.random() * canvasWidth,
      y: Math.random() * canvasHeight,
      z: Math.random(),
      vx: (Math.random() - 0.5) * 0.5,
      vy: (Math.random() - 0.5) * 0.3,
      size: Math.random() * 2 + 0.5,
      life: Math.random() * 200,
      maxLife: 200 + Math.random() * 100,
      color: mode === 'stress' ? '#ff3344' : mode === 'thermal' ? '#ff6600' : mode === 'electrical' ? '#00eeff' : '#00c2ff',
    }));

    const w = () => canvasWidth;
    const h = () => canvasHeight;

    const drawGrid = (t: number) => {
      const gridSize = 40;
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.4)';
      ctx.lineWidth = 0.5;
      
      for (let x = 0; x <= w(); x += gridSize) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h());
        ctx.stroke();
      }
      for (let y = 0; y <= h(); y += gridSize) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w(), y);
        ctx.stroke();
      }

      // Animated scan line
      const scanY = (t * 0.02) % h();
      const grad = ctx.createLinearGradient(0, scanY - 20, 0, scanY + 20);
      grad.addColorStop(0, 'transparent');
      grad.addColorStop(0.5, 'rgba(0, 194, 255, 0.06)');
      grad.addColorStop(1, 'transparent');
      ctx.fillStyle = grad;
      ctx.fillRect(0, scanY - 20, w(), 40);
    };

    const drawEngineering3DShape = (t: number) => {
      const cx = w() / 2;
      const cy = h() / 2;
      const scale = Math.min(w(), h()) * 0.28;
      const rot = t * 0.0005;

      // Draw a complex engineering assembly - turbine/gear shape
      ctx.save();
      ctx.translate(cx, cy);
      
      // Outer ring
      const outerR = scale;
      const innerR = scale * 0.7;
      const teeth = 24;

      for (let ring = 0; ring < 3; ring++) {
        const ringScale = 1 - ring * 0.25;
        const ringRot = rot * (ring % 2 === 0 ? 1 : -0.7);
        
        ctx.save();
        ctx.rotate(ringRot);
        
        const currentOuter = outerR * ringScale;
        const currentInner = innerR * ringScale;
        
        // Draw gear teeth
        ctx.beginPath();
        for (let i = 0; i < teeth; i++) {
          const angle = (i / teeth) * Math.PI * 2;
          const nextAngle = ((i + 0.5) / teeth) * Math.PI * 2;
          const midAngle = ((i + 0.25) / teeth) * Math.PI * 2;
          const midAngle2 = ((i + 0.75) / teeth) * Math.PI * 2;
          
          if (i === 0) {
            ctx.moveTo(Math.cos(angle) * currentOuter, Math.sin(angle) * currentOuter);
          }
          ctx.lineTo(Math.cos(midAngle) * currentOuter, Math.sin(midAngle) * currentOuter);
          ctx.lineTo(Math.cos(midAngle) * (currentOuter + 6 * ringScale), Math.sin(midAngle) * (currentOuter + 6 * ringScale));
          ctx.lineTo(Math.cos(midAngle2) * (currentOuter + 6 * ringScale), Math.sin(midAngle2) * (currentOuter + 6 * ringScale));
          ctx.lineTo(Math.cos(midAngle2) * currentOuter, Math.sin(midAngle2) * currentOuter);
          ctx.lineTo(Math.cos(nextAngle) * currentOuter, Math.sin(nextAngle) * currentOuter);
        }
        ctx.closePath();

        const getColor = () => {
          switch (mode) {
            case 'stress': return ring === 0 ? 'rgba(255, 51, 68, 0.15)' : ring === 1 ? 'rgba(255, 170, 0, 0.1)' : 'rgba(0, 204, 102, 0.08)';
            case 'thermal': return ring === 0 ? 'rgba(255, 102, 0, 0.15)' : ring === 1 ? 'rgba(255, 170, 0, 0.1)' : 'rgba(0, 68, 255, 0.08)';
            case 'electrical': return `rgba(0, 238, 255, ${0.12 - ring * 0.03})`;
            default: return `rgba(0, 194, 255, ${0.1 - ring * 0.025})`;
          }
        };

        ctx.fillStyle = getColor();
        ctx.fill();

        const getStroke = () => {
          switch (mode) {
            case 'stress': return ring === 0 ? 'rgba(255, 51, 68, 0.6)' : ring === 1 ? 'rgba(255, 170, 0, 0.4)' : 'rgba(0, 204, 102, 0.3)';
            case 'thermal': return ring === 0 ? 'rgba(255, 102, 0, 0.6)' : ring === 1 ? 'rgba(255, 170, 0, 0.4)' : 'rgba(0, 68, 255, 0.3)';
            case 'electrical': return `rgba(0, 238, 255, ${0.5 - ring * 0.12})`;
            default: return `rgba(0, 194, 255, ${0.5 - ring * 0.12})`;
          }
        };

        ctx.strokeStyle = getStroke();
        ctx.lineWidth = 1;
        ctx.stroke();

        // Inner circle
        ctx.beginPath();
        ctx.arc(0, 0, currentInner, 0, Math.PI * 2);
        ctx.strokeStyle = getStroke();
        ctx.lineWidth = 0.5;
        ctx.stroke();

        // Spokes
        const spokeCount = 8;
        for (let s = 0; s < spokeCount; s++) {
          const sAngle = (s / spokeCount) * Math.PI * 2;
          ctx.beginPath();
          ctx.moveTo(Math.cos(sAngle) * currentInner * 0.3, Math.sin(sAngle) * currentInner * 0.3);
          ctx.lineTo(Math.cos(sAngle) * currentInner, Math.sin(sAngle) * currentInner);
          ctx.strokeStyle = getStroke();
          ctx.lineWidth = ring === 0 ? 1.5 : 0.8;
          ctx.stroke();
        }

        // Center hub
        ctx.beginPath();
        ctx.arc(0, 0, currentInner * 0.3, 0, Math.PI * 2);
        ctx.strokeStyle = getStroke();
        ctx.lineWidth = 1;
        ctx.stroke();

        ctx.restore();
      }

      // Draw stress/thermal hotspots
      if (mode === 'stress' || mode === 'thermal') {
        const hotspots = [
          { angle: rot * 2, dist: scale * 0.5, intensity: 0.5 + Math.sin(t * 0.003) * 0.3 },
          { angle: rot * 2 + Math.PI * 0.7, dist: scale * 0.65, intensity: 0.4 + Math.sin(t * 0.004 + 1) * 0.2 },
          { angle: rot * 2 + Math.PI * 1.3, dist: scale * 0.45, intensity: 0.6 + Math.sin(t * 0.002 + 2) * 0.25 },
          { angle: rot * 2 + Math.PI * 1.8, dist: scale * 0.8, intensity: 0.3 + Math.sin(t * 0.005 + 3) * 0.15 },
        ];

        hotspots.forEach(hs => {
          const hx = Math.cos(hs.angle) * hs.dist;
          const hy = Math.sin(hs.angle) * hs.dist;
          const grad = ctx.createRadialGradient(hx, hy, 0, hx, hy, 35);
          
          if (mode === 'stress') {
            grad.addColorStop(0, `rgba(255, 51, 68, ${hs.intensity * 0.6})`);
            grad.addColorStop(0.5, `rgba(255, 170, 0, ${hs.intensity * 0.3})`);
            grad.addColorStop(1, 'transparent');
          } else {
            grad.addColorStop(0, `rgba(255, 0, 51, ${hs.intensity * 0.5})`);
            grad.addColorStop(0.4, `rgba(255, 102, 0, ${hs.intensity * 0.3})`);
            grad.addColorStop(1, 'transparent');
          }
          ctx.fillStyle = grad;
          ctx.fillRect(hx - 40, hy - 40, 80, 80);
        });
      }

      // Electrical flow lines
      if (mode === 'electrical') {
        for (let i = 0; i < 12; i++) {
          const startAngle = (i / 12) * Math.PI * 2 + rot;
          const endAngle = startAngle + Math.PI * 0.5;
          const startDist = scale * 0.3;
          const endDist = scale * 0.9;
          
          ctx.beginPath();
          const segments = 20;
          for (let s = 0; s <= segments; s++) {
            const progress = s / segments;
            const angle = startAngle + (endAngle - startAngle) * progress;
            const dist = startDist + (endDist - startDist) * progress;
            const px = Math.cos(angle) * dist;
            const py = Math.sin(angle) * dist;
            
            if (s === 0) ctx.moveTo(px, py);
            else ctx.lineTo(px, py);
          }
          
          const dashOffset = (t * 0.05) % 20;
          ctx.setLineDash([4, 8]);
          ctx.lineDashOffset = -dashOffset;
          ctx.strokeStyle = `rgba(0, 238, 255, ${0.3 + Math.sin(t * 0.005 + i) * 0.15})`;
          ctx.lineWidth = 1.5;
          ctx.stroke();
          ctx.setLineDash([]);
        }
      }

      ctx.restore();
    };

    const drawParticles = (_t: number) => {
      particlesRef.current.forEach(p => {
        p.x += p.vx;
        p.y += p.vy;
        p.life += 1;
        
        if (p.life > p.maxLife || p.x < 0 || p.x > w() || p.y < 0 || p.y > h()) {
          p.x = Math.random() * w();
          p.y = Math.random() * h();
          p.life = 0;
        }

        const alpha = Math.sin((p.life / p.maxLife) * Math.PI) * 0.5;
        ctx.beginPath();
        ctx.arc(p.x, p.y, p.size * (1 - p.life / p.maxLife * 0.5), 0, Math.PI * 2);
        ctx.fillStyle = p.color.replace(')', `, ${alpha})`).replace('rgb', 'rgba');
        
        // Parse hex color to rgba
        const hex = p.color;
        const r = parseInt(hex.slice(1, 3), 16);
        const g = parseInt(hex.slice(3, 5), 16);
        const b = parseInt(hex.slice(5, 7), 16);
        ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${alpha})`;
        ctx.fill();
      });
    };

    const drawMeasurements = (_t: number) => {
      const cx = w() / 2;
      const cy = h() / 2;
      const sz = Math.min(w(), h()) * 0.28;
      
      // Dimension lines
      ctx.setLineDash([3, 3]);
      ctx.strokeStyle = 'rgba(0, 194, 255, 0.25)';
      ctx.lineWidth = 0.5;
      
      // Horizontal dim
      ctx.beginPath();
      ctx.moveTo(cx - sz, cy + sz + 30);
      ctx.lineTo(cx + sz, cy + sz + 30);
      ctx.stroke();
      
      ctx.beginPath();
      ctx.moveTo(cx - sz, cy + sz + 20);
      ctx.lineTo(cx - sz, cy + sz + 40);
      ctx.stroke();
      ctx.beginPath();
      ctx.moveTo(cx + sz, cy + sz + 20);
      ctx.lineTo(cx + sz, cy + sz + 40);
      ctx.stroke();
      
      ctx.setLineDash([]);
      ctx.font = '9px monospace';
      ctx.fillStyle = 'rgba(0, 194, 255, 0.6)';
      ctx.textAlign = 'center';
      ctx.fillText('∅ 450.00 mm', cx, cy + sz + 45);

      // Vertical dim
      ctx.setLineDash([3, 3]);
      ctx.beginPath();
      ctx.moveTo(cx + sz + 30, cy - sz);
      ctx.lineTo(cx + sz + 30, cy + sz);
      ctx.stroke();
      ctx.setLineDash([]);
      
      ctx.save();
      ctx.translate(cx + sz + 45, cy);
      ctx.rotate(-Math.PI / 2);
      ctx.fillText('L: 320.00 mm', 0, 0);
      ctx.restore();

      // Coordinate axes indicator
      const axisLen = 40;
      const axisX = 60;
      const axisY = h() - 60;

      // X axis
      ctx.beginPath();
      ctx.moveTo(axisX, axisY);
      ctx.lineTo(axisX + axisLen, axisY);
      ctx.strokeStyle = 'rgba(255, 51, 68, 0.7)';
      ctx.lineWidth = 1.5;
      ctx.setLineDash([]);
      ctx.stroke();
      ctx.fillStyle = 'rgba(255, 51, 68, 0.8)';
      ctx.font = '9px monospace';
      ctx.fillText('X', axisX + axisLen + 5, axisY + 3);

      // Y axis
      ctx.beginPath();
      ctx.moveTo(axisX, axisY);
      ctx.lineTo(axisX, axisY - axisLen);
      ctx.strokeStyle = 'rgba(0, 204, 102, 0.7)';
      ctx.stroke();
      ctx.fillStyle = 'rgba(0, 204, 102, 0.8)';
      ctx.fillText('Y', axisX - 3, axisY - axisLen - 5);

      // Z axis  
      ctx.beginPath();
      ctx.moveTo(axisX, axisY);
      ctx.lineTo(axisX - axisLen * 0.5, axisY + axisLen * 0.3);
      ctx.strokeStyle = 'rgba(0, 136, 255, 0.7)';
      ctx.stroke();
      ctx.fillStyle = 'rgba(0, 136, 255, 0.8)';
      ctx.fillText('Z', axisX - axisLen * 0.5 - 10, axisY + axisLen * 0.3 + 5);
    };

    const drawViewportInfo = () => {
      ctx.font = '9px monospace';
      ctx.fillStyle = 'rgba(122, 130, 153, 0.6)';
      ctx.textAlign = 'left';
      
      const info = [
        `MODE: ${mode.toUpperCase()}`,
        `MESH: 3.89M ELEMENTS`,
        `SOLVER: IMPLICIT NONLINEAR`,
        `FRAME: ${Math.floor(timeRef.current / 16.67)}`,
      ];
      
      info.forEach((line, i) => {
        ctx.fillText(line, 15, 25 + i * 14);
      });

      // Top right info
      ctx.textAlign = 'right';
      const rightInfo = [
        `PERSPECTIVE VIEW`,
        `SCALE: 1:1`,
        `UNITS: mm / MPa / °C`,
      ];
      rightInfo.forEach((line, i) => {
        ctx.fillText(line, w() - 15, 25 + i * 14);
      });
    };

    const render = (timestamp: number) => {
      timeRef.current = timestamp;
      const cw = canvasWidth;
      const ch = canvasHeight;

      ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
      
      // Background
      ctx.fillStyle = '#0a0c10';
      ctx.fillRect(0, 0, cw, ch);

      // Vignette
      const vignette = ctx.createRadialGradient(cw / 2, ch / 2, cw * 0.2, cw / 2, ch / 2, cw * 0.7);
      vignette.addColorStop(0, 'transparent');
      vignette.addColorStop(1, 'rgba(0, 0, 0, 0.4)');
      ctx.fillStyle = vignette;
      ctx.fillRect(0, 0, cw, ch);

      drawGrid(timestamp);
      drawEngineering3DShape(timestamp);
      drawParticles(timestamp);
      drawMeasurements(timestamp);
      drawViewportInfo();

      animRef.current = requestAnimationFrame(render);
    };

    animRef.current = requestAnimationFrame(render);

    return () => {
      cancelAnimationFrame(animRef.current);
      resizeObserver.disconnect();
    };
  }, [mode]);

  const handleMouseMove = (e: React.MouseEvent) => {
    const rect = canvasRef.current?.getBoundingClientRect();
    if (!rect) return;
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    const cx = rect.width / 2;
    const cy = rect.height / 2;
    const dist = Math.sqrt((x - cx) ** 2 + (y - cy) ** 2);

    if (dist < Math.min(rect.width, rect.height) * 0.3) {
      const stress = (450 - dist * 1.5 + Math.random() * 20).toFixed(1);
      const temp = (342 + (Math.random() - 0.5) * 40).toFixed(1);
      setHoveredNode({
        x: e.clientX - rect.left,
        y: e.clientY - rect.top,
        data: {
          'Von Mises': `${stress} MPa`,
          'Temperature': `${temp} °C`,
          'Displacement': `${(Math.random() * 2.5).toFixed(3)} mm`,
          'Safety Factor': `${(1 + Math.random() * 2).toFixed(2)}`,
        }
      });
    } else {
      setHoveredNode(null);
    }
  };

  return (
    <div className="relative w-full h-full overflow-hidden bg-forge-bg">
      <canvas 
        ref={canvasRef}
        className="w-full h-full cursor-crosshair"
        onMouseMove={handleMouseMove}
        onMouseLeave={() => setHoveredNode(null)}
      />
      
      {/* Hover tooltip */}
      {hoveredNode && (
        <div
          className="absolute pointer-events-none z-30 glass-panel rounded-lg p-2.5 min-w-[160px] glow-accent"
          style={{ left: hoveredNode.x + 15, top: hoveredNode.y - 10 }}
        >
          <div className="text-[8px] text-forge-accent tracking-wider mb-1.5 font-bold">NODE INSPECTION</div>
          {Object.entries(hoveredNode.data).map(([key, value]) => (
            <div key={key} className="flex justify-between gap-4 text-[9px]">
              <span className="text-forge-text-muted">{key}</span>
              <span className="text-forge-text font-mono">{value}</span>
            </div>
          ))}
        </div>
      )}

      {/* Corner markers */}
      {[
        'top-0 left-0 border-t-2 border-l-2',
        'top-0 right-0 border-t-2 border-r-2',
        'bottom-0 left-0 border-b-2 border-l-2',
        'bottom-0 right-0 border-b-2 border-r-2',
      ].map((pos, i) => (
        <div key={i} className={`absolute ${pos} w-4 h-4 border-forge-accent/30 pointer-events-none`} />
      ))}

      {/* Center crosshair */}
      <div className="absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2 pointer-events-none">
        <div className="w-6 h-px bg-forge-accent/20" />
        <div className="w-px h-6 bg-forge-accent/20 absolute top-1/2 left-1/2 -translate-x-1/2 -translate-y-1/2" />
      </div>
    </div>
  );
}
