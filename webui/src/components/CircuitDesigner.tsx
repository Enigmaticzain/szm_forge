import { useRef, useEffect, useState, useCallback } from 'react';

// Component types
type ComponentType = 'resistor' | 'capacitor' | 'inductor' | 'diode' | 'led' | 'npn_transistor' | 'pnp_transistor' | 'opamp' | 'vcc' | 'gnd' | 'wire' | 'node';

interface Component {
  id: string;
  type: ComponentType;
  x: number;
  y: number;
  rotation: number;
  value: string;
  label: string;
  selected: boolean;
  pins: { x: number; y: number; connected: boolean }[];
}

interface Wire {
  id: string;
  startX: number;
  startY: number;
  endX: number;
  endY: number;
  selected: boolean;
}

interface Tool {
  id: string;
  name: string;
  icon: string;
  shortcut: string;
}

const COMPONENTS: { type: ComponentType; name: string; icon: string; defaultValue: string }[] = [
  { type: 'resistor', name: 'Resistor', icon: '⏛', defaultValue: '1kΩ' },
  { type: 'capacitor', name: 'Capacitor', icon: '⊣⊢', defaultValue: '100nF' },
  { type: 'inductor', name: 'Inductor', icon: '⌇', defaultValue: '10mH' },
  { type: 'diode', name: 'Diode', icon: '◀|', defaultValue: '1N4148' },
  { type: 'led', name: 'LED', icon: '◐', defaultValue: '2V' },
  { type: 'npn_transistor', name: 'NPN', icon: '⌁', defaultValue: '2N2222' },
  { type: 'pnp_transistor', name: 'PNP', icon: '⌁', defaultValue: '2N2907' },
  { type: 'opamp', name: 'Op-Amp', icon: '△', defaultValue: 'LM741' },
  { type: 'vcc', name: 'VCC', icon: '⏊', defaultValue: '5V' },
  { type: 'gnd', name: 'Ground', icon: '⏚', defaultValue: '0V' },
];

const TOOLS: Tool[] = [
  { id: 'select', name: 'Select', icon: '↖', shortcut: 'V' },
  { id: 'wire', name: 'Wire', icon: '⤳', shortcut: 'W' },
  { id: 'delete', name: 'Delete', icon: '✕', shortcut: 'D' },
  { id: 'rotate', name: 'Rotate', icon: '↻', shortcut: 'R' },
];

const GRID_SIZE = 20;

export default function CircuitDesigner() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const [components, setComponents] = useState<Component[]>([]);
  const [wires, setWires] = useState<Wire[]>([]);
  const [selectedTool, setSelectedTool] = useState<string>('select');
  const [selectedComponent, setSelectedComponent] = useState<string | null>(null);
  const [dragging, setDragging] = useState<{ type: ComponentType; fromPalette: boolean } | null>(null);
  const [wireStart, setWireStart] = useState<{ x: number; y: number } | null>(null);
  const [mousePos, setMousePos] = useState<{ x: number; y: number }>({ x: 0, y: 0 });
  const [isSimulating, setIsSimulating] = useState(false);
  const [simulationTime, setSimulationTime] = useState(0);
  const [circuitErrors, setCircuitErrors] = useState<string[]>([]);
  const animRef = useRef<number>(0);
  const simRef = useRef<number>(0);

  // Snap to grid
  const snapToGrid = useCallback((value: number) => Math.round(value / GRID_SIZE) * GRID_SIZE, []);

  // Generate unique ID
  const generateId = () => Math.random().toString(36).substr(2, 9);

  // Get component pins based on type
  const getPins = (type: ComponentType): { x: number; y: number; connected: boolean }[] => {
    switch (type) {
      case 'resistor':
      case 'capacitor':
      case 'inductor':
      case 'diode':
      case 'led':
        return [
          { x: -30, y: 0, connected: false },
          { x: 30, y: 0, connected: false },
        ];
      case 'npn_transistor':
      case 'pnp_transistor':
        return [
          { x: -20, y: 0, connected: false },  // Base
          { x: 0, y: 20, connected: false },   // Collector
          { x: 0, y: -20, connected: false },   // Emitter
        ];
      case 'opamp':
        return [
          { x: -30, y: -15, connected: false }, // Non-inverting
          { x: -30, y: 15, connected: false },  // Inverting
          { x: 30, y: 0, connected: false },    // Output
          { x: 0, y: -25, connected: false },    // V+
          { x: 0, y: 25, connected: false },     // V-
        ];
      case 'vcc':
      case 'gnd':
        return [{ x: 0, y: 15, connected: false }];
      default:
        return [];
    }
  };

  // Create new component
  const createComponent = (type: ComponentType, x: number, y: number): Component => {
    const comp = COMPONENTS.find(c => c.type === type);
    return {
      id: generateId(),
      type,
      x: snapToGrid(x),
      y: snapToGrid(y),
      rotation: 0,
      value: comp?.defaultValue || '',
      label: `${type.toUpperCase()}_${generateId().slice(0, 3)}`,
      selected: false,
      pins: getPins(type),
    };
  };

  // Draw component shape on canvas
  const drawComponent = (ctx: CanvasRenderingContext2D, comp: Component, time: number) => {
    ctx.save();
    ctx.translate(comp.x, comp.y);
    ctx.rotate((comp.rotation * Math.PI) / 180);

    const isSelected = selectedComponent === comp.id;
    const strokeColor = isSelected ? '#00eeff' : '#00c2ff';
    const glowColor = isSelected ? 'rgba(0, 238, 255, 0.4)' : 'rgba(0, 194, 255, 0.2)';

    // Draw selection glow
    if (isSelected) {
      ctx.shadowColor = glowColor;
      ctx.shadowBlur = 15;
    }

    ctx.strokeStyle = strokeColor;
    ctx.fillStyle = 'rgba(0, 194, 255, 0.1)';
    ctx.lineWidth = 2;
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';

    // Component body
    switch (comp.type) {
      case 'resistor':
        ctx.beginPath();
        ctx.moveTo(-30, 0);
        ctx.lineTo(-20, 0);
        ctx.lineTo(-15, -10);
        ctx.lineTo(-5, 10);
        ctx.lineTo(5, -10);
        ctx.lineTo(15, 10);
        ctx.lineTo(20, 0);
        ctx.lineTo(30, 0);
        ctx.stroke();
        break;

      case 'capacitor':
        ctx.beginPath();
        ctx.moveTo(-30, 0);
        ctx.lineTo(-5, 0);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-5, -15);
        ctx.lineTo(-5, 15);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(5, -15);
        ctx.lineTo(5, 15);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(5, 0);
        ctx.lineTo(30, 0);
        ctx.stroke();
        break;

      case 'inductor':
        ctx.beginPath();
        ctx.moveTo(-30, 0);
        ctx.lineTo(-20, 0);
        for (let i = 0; i < 3; i++) {
          ctx.arc(-10 + i * 15, 0, 7.5, Math.PI, 0, false);
        }
        ctx.lineTo(30, 0);
        ctx.stroke();
        break;

      case 'diode':
        ctx.beginPath();
        ctx.moveTo(-30, 0);
        ctx.lineTo(-10, 0);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-10, -12);
        ctx.lineTo(-10, 12);
        ctx.lineTo(10, 0);
        ctx.closePath();
        ctx.fillStyle = 'rgba(255, 100, 0, 0.3)';
        ctx.fill();
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(10, -12);
        ctx.lineTo(10, 12);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(10, 0);
        ctx.lineTo(30, 0);
        ctx.stroke();
        break;

      case 'led':
        ctx.beginPath();
        ctx.moveTo(-30, 0);
        ctx.lineTo(-10, 0);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-10, -12);
        ctx.lineTo(-10, 12);
        ctx.lineTo(10, 0);
        ctx.closePath();
        ctx.fillStyle = isSimulating ? 'rgba(0, 255, 100, 0.5)' : 'rgba(255, 50, 50, 0.3)';
        ctx.fill();
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(10, -12);
        ctx.lineTo(10, 12);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(10, 0);
        ctx.lineTo(30, 0);
        ctx.stroke();
        // LED arrows
        ctx.beginPath();
        ctx.moveTo(3, -15);
        ctx.lineTo(8, -20);
        ctx.lineTo(5, -15);
        ctx.lineTo(10, -20);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(8, -12);
        ctx.lineTo(13, -17);
        ctx.moveTo(11, -12);
        ctx.lineTo(16, -17);
        ctx.stroke();
        break;

      case 'npn_transistor':
      case 'pnp_transistor':
        // Circle
        ctx.beginPath();
        ctx.arc(0, 0, 20, 0, Math.PI * 2);
        ctx.stroke();
        // Base line
        ctx.beginPath();
        ctx.moveTo(-20, 0);
        ctx.lineTo(-8, 0);
        ctx.stroke();
        // Vertical line
        ctx.beginPath();
        ctx.moveTo(-8, -12);
        ctx.lineTo(-8, 12);
        ctx.stroke();
        // Collector
        ctx.beginPath();
        ctx.moveTo(-8, -6);
        ctx.lineTo(5, -20);
        ctx.lineTo(5, -25);
        ctx.stroke();
        // Emitter
        ctx.beginPath();
        ctx.moveTo(-8, 6);
        ctx.lineTo(5, 20);
        ctx.lineTo(5, 25);
        ctx.stroke();
        // Arrow
        ctx.beginPath();
        ctx.moveTo(0, 12);
        ctx.lineTo(5, 20);
        ctx.lineTo(8, 12);
        ctx.stroke();
        // Fill base circle
        ctx.fillStyle = 'rgba(0, 194, 255, 0.1)';
        ctx.fill();
        break;

      case 'opamp':
        // Triangle
        ctx.beginPath();
        ctx.moveTo(-25, -20);
        ctx.lineTo(-25, 20);
        ctx.lineTo(25, 0);
        ctx.closePath();
        ctx.fillStyle = 'rgba(136, 85, 255, 0.1)';
        ctx.fill();
        ctx.stroke();
        // Labels
        ctx.font = '10px monospace';
        ctx.fillStyle = '#8855ff';
        ctx.textAlign = 'center';
        ctx.fillText('-', -18, -12);
        ctx.fillText('+', -18, 18);
        break;

      case 'vcc':
        ctx.beginPath();
        ctx.moveTo(0, 15);
        ctx.lineTo(0, -5);
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(0, -10, 10, Math.PI, 0, false);
        ctx.stroke();
        ctx.fillStyle = '#00ff88';
        ctx.font = 'bold 10px monospace';
        ctx.textAlign = 'center';
        ctx.fillText('VCC', 0, -25);
        break;

      case 'gnd':
        ctx.beginPath();
        ctx.moveTo(0, -15);
        ctx.lineTo(0, 0);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-12, 0);
        ctx.lineTo(12, 0);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-8, 5);
        ctx.lineTo(8, 5);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-4, 10);
        ctx.lineTo(4, 10);
        ctx.stroke();
        break;
    }

    // Draw pins
    comp.pins.forEach((pin, i) => {
      const pinX = pin.x;
      const pinY = pin.y;
      ctx.beginPath();
      ctx.arc(pinX, pinY, 3, 0, Math.PI * 2);
      ctx.fillStyle = pin.connected ? '#00ff88' : '#00eeff';
      ctx.fill();
      if (pin.connected) {
        ctx.beginPath();
        ctx.arc(pinX, pinY, 6, 0, Math.PI * 2);
        ctx.strokeStyle = 'rgba(0, 255, 136, 0.3)';
        ctx.lineWidth = 1;
        ctx.stroke();
      }
    });

    // Draw label
    ctx.font = '9px monospace';
    ctx.fillStyle = '#888';
    ctx.textAlign = 'center';
    ctx.fillText(comp.label, 0, 40);

    // Draw value
    ctx.font = '8px monospace';
    ctx.fillStyle = '#00c2ff';
    ctx.fillText(comp.value, 0, 50);

    ctx.restore();
  };

  // Main canvas rendering
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

    const render = (time: number) => {
      const w = canvas.width;
      const h = canvas.height;

      // Clear
      ctx.fillStyle = '#0a0c10';
      ctx.fillRect(0, 0, w, h);

      // Grid
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.5)';
      ctx.lineWidth = 0.5;
      for (let x = 0; x < w; x += GRID_SIZE) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = 0; y < h; y += GRID_SIZE) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }

      // Major grid lines
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.8)';
      ctx.lineWidth = 1;
      for (let x = 0; x < w; x += GRID_SIZE * 5) {
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, h);
        ctx.stroke();
      }
      for (let y = 0; y < h; y += GRID_SIZE * 5) {
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(w, y);
        ctx.stroke();
      }

      // Draw wires
      wires.forEach(wire => {
        const wireColor = wire.selected ? '#ff3366' : '#00eeff';
        ctx.strokeStyle = wireColor;
        ctx.lineWidth = wire.selected ? 3 : 2;
        ctx.shadowColor = wire.selected ? 'rgba(255, 51, 102, 0.5)' : 'rgba(0, 238, 255, 0.3)';
        ctx.shadowBlur = 8;
        ctx.beginPath();
        ctx.moveTo(wire.startX, wire.startY);
        ctx.lineTo(wire.endX, wire.endY);
        ctx.stroke();
        ctx.shadowBlur = 0;

        // Wire nodes
        ctx.beginPath();
        ctx.arc(wire.startX, wire.startY, 4, 0, Math.PI * 2);
        ctx.fillStyle = wireColor;
        ctx.fill();
        ctx.beginPath();
        ctx.arc(wire.endX, wire.endY, 4, 0, Math.PI * 2);
        ctx.fill();
      });

      // Draw in-progress wire
      if (wireStart) {
        ctx.strokeStyle = 'rgba(0, 238, 255, 0.6)';
        ctx.lineWidth = 2;
        ctx.setLineDash([5, 5]);
        ctx.beginPath();
        ctx.moveTo(wireStart.x, wireStart.y);
        ctx.lineTo(mousePos.x, mousePos.y);
        ctx.stroke();
        ctx.setLineDash([]);
      }

      // Draw components
      components.forEach(comp => {
        drawComponent(ctx, comp, time);
      });

      // Draw ghost component when dragging from palette
      if (dragging && dragging.fromPalette) {
        ctx.globalAlpha = 0.5;
        const ghost = createComponent(dragging.type, mousePos.x, mousePos.y);
        drawComponent(ctx, ghost, time);
        ctx.globalAlpha = 1;
      }

      animRef.current = requestAnimationFrame(render);
    };

    animRef.current = requestAnimationFrame(render);
    return () => cancelAnimationFrame(animRef.current);
  }, [components, wires, selectedComponent, wireStart, mousePos, dragging, isSimulating]);

  // Mouse handlers
  const handleMouseDown = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    if (selectedTool === 'wire') {
      if (!wireStart) {
        setWireStart({ x: snapToGrid(x), y: snapToGrid(y) });
      } else {
        setWires(prev => [
          ...prev,
          {
            id: generateId(),
            startX: wireStart.x,
            startY: wireStart.y,
            endX: snapToGrid(x),
            endY: snapToGrid(y),
            selected: false,
          },
        ]);
        setWireStart(null);
      }
    } else if (selectedTool === 'select') {
      // Check if clicking on a component
      const clickedComp = components.find(comp => {
        const dx = x - comp.x;
        const dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 30;
      });

      if (clickedComp) {
        setSelectedComponent(clickedComp.id);
        setComponents(prev =>
          prev.map(c => ({ ...c, selected: c.id === clickedComp.id }))
        );
      } else {
        setSelectedComponent(null);
        setComponents(prev => prev.map(c => ({ ...c, selected: false })));
      }
    } else if (selectedTool === 'delete') {
      // Delete component
      const clickedComp = components.find(comp => {
        const dx = x - comp.x;
        const dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 30;
      });
      if (clickedComp) {
        setComponents(prev => prev.filter(c => c.id !== clickedComp.id));
        setSelectedComponent(null);
      }
      // Delete wire
      const clickedWire = wires.find(wire => {
        const A = wire.startX, B = wire.startY;
        const C = wire.endX, D = wire.endY;
        const dx = C - A, dy = D - B;
        const len = Math.sqrt(dx * dx + dy * dy);
        if (len === 0) return false;
        const t = Math.max(0, Math.min(1, ((x - A) * dx + (y - B) * dy) / (len * len)));
        const nearX = A + t * dx, nearY = B + t * dy;
        const dist = Math.sqrt((x - nearX) ** 2 + (y - nearY) ** 2);
        return dist < 10;
      });
      if (clickedWire) {
        setWires(prev => prev.filter(w => w.id !== clickedWire.id));
      }
    } else if (selectedTool === 'rotate') {
      const clickedComp = components.find(comp => {
        const dx = x - comp.x;
        const dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 30;
      });
      if (clickedComp) {
        setComponents(prev =>
          prev.map(c =>
            c.id === clickedComp.id ? { ...c, rotation: (c.rotation + 90) % 360 } : c
          )
        );
      }
    }
  };

  const handleMouseMove = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    setMousePos({ x, y });

    if (dragging && dragging.fromPalette) {
      setMousePos({ x, y });
    } else if (selectedComponent && selectedTool === 'select') {
      // Drag selected component
      const isDragging = e.buttons === 1;
      if (isDragging) {
        setComponents(prev =>
          prev.map(c =>
            c.id === selectedComponent ? { ...c, x: snapToGrid(x), y: snapToGrid(y) } : c
          )
        );
      }
    }
  };

  const handleMouseUp = (e: React.MouseEvent<HTMLCanvasElement>) => {
    if (dragging && dragging.fromPalette) {
      const canvas = canvasRef.current;
      if (canvas) {
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        const newComp = createComponent(dragging.type, x, y);
        setComponents(prev => [...prev, newComp]);
      }
      setDragging(null);
    }
  };

  // Handle dropping from palette
  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    const type = e.dataTransfer.getData('componentType') as ComponentType;
    if (type) {
      const canvas = canvasRef.current;
      if (canvas) {
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        const newComp = createComponent(type, x, y);
        setComponents(prev => [...prev, newComp]);
      }
    }
  };

  const handleDragOver = (e: React.DragEvent) => {
    e.preventDefault();
  };

  // Update component value
  const updateComponentValue = (id: string, value: string) => {
    setComponents(prev =>
      prev.map(c => (c.id === id ? { ...c, value } : c))
    );
  };

  // Run simulation
  const runSimulation = () => {
    setCircuitErrors([]);
    // Basic circuit validation
    const vccCount = components.filter(c => c.type === 'vcc').length;
    const gndCount = components.filter(c => c.type === 'gnd').length;
    
    if (vccCount === 0) {
      setCircuitErrors(prev => [...prev, 'Warning: No VCC source connected']);
    }
    if (gndCount === 0) {
      setCircuitErrors(prev => [...prev, 'Warning: No Ground connected']);
    }

    setIsSimulating(true);
    let time = 0;
    const simulate = () => {
      time += 0.016;
      setSimulationTime(time);
      if (isSimulating) {
        simRef.current = requestAnimationFrame(simulate);
      }
    };
    simRef.current = requestAnimationFrame(simulate);
  };

  const stopSimulation = () => {
    setIsSimulating(false);
    cancelAnimationFrame(simRef.current);
  };

  // Clear all
  const clearAll = () => {
    setComponents([]);
    setWires([]);
    setSelectedComponent(null);
    setCircuitErrors([]);
    stopSimulation();
  };

  // Get selected component details
  const selectedCompData = components.find(c => c.id === selectedComponent);

  return (
    <div className="flex h-full">
      {/* Component Palette */}
      <div className="w-48 bg-forge-surface border-r border-forge-border p-2 overflow-y-auto">
        <div className="text-[9px] font-bold text-forge-electrical tracking-wider mb-2 px-1">COMPONENTS</div>
        <div className="grid grid-cols-2 gap-1">
          {COMPONENTS.map(comp => (
            <div
              key={comp.type}
              draggable
              onDragStart={(e) => {
                e.dataTransfer.setData('componentType', comp.type);
                setDragging({ type: comp.type, fromPalette: true });
              }}
              onDragEnd={() => setDragging(null)}
              className="glass-panel rounded p-2 cursor-grab hover:border-forge-electrical/50 transition-all text-center"
            >
              <div className="text-lg mb-0.5">{comp.icon}</div>
              <div className="text-[8px] text-forge-text-dim truncate">{comp.name}</div>
            </div>
          ))}
        </div>

        <div className="text-[9px] font-bold text-forge-text-muted tracking-wider mt-4 mb-2 px-1">TOOLS</div>
        <div className="space-y-1">
          {TOOLS.map(tool => (
            <button
              key={tool.id}
              onClick={() => setSelectedTool(tool.id)}
              className={`w-full px-2 py-1.5 rounded text-left flex items-center justify-between transition-all ${
                selectedTool === tool.id
                  ? 'bg-forge-electrical/20 text-forge-electrical border border-forge-electrical/30'
                  : 'bg-forge-panel/50 text-forge-text-dim hover:text-forge-text border border-transparent'
              }`}
            >
              <div className="flex items-center gap-2">
                <span>{tool.icon}</span>
                <span className="text-[9px] font-semibold">{tool.name}</span>
              </div>
              <span className="text-[8px] text-forge-text-muted">{tool.shortcut}</span>
            </button>
          ))}
        </div>

        <div className="mt-4 space-y-1">
          <button
            onClick={isSimulating ? stopSimulation : runSimulation}
            className={`w-full px-3 py-2 rounded text-[9px] font-bold tracking-wider transition-all ${
              isSimulating
                ? 'bg-forge-danger/20 text-forge-danger border border-forge-danger/30'
                : 'bg-forge-green/20 text-forge-green border border-forge-green/30 hover:bg-forge-green/30'
            }`}
          >
            {isSimulating ? '⏹ STOP' : '▶ SIMULATE'}
          </button>
          <button
            onClick={clearAll}
            className="w-full px-3 py-1.5 rounded text-[9px] font-semibold bg-forge-panel/50 text-forge-text-dim border border-transparent hover:text-forge-danger hover:border-forge-danger/30 transition-all"
          >
            🗑 CLEAR ALL
          </button>
        </div>
      </div>

      {/* Canvas Area */}
      <div className="flex-1 flex flex-col">
        <canvas
          ref={canvasRef}
          className="flex-1 cursor-crosshair"
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          onDrop={handleDrop}
          onDragOver={handleDragOver}
        />

        {/* Toolbar below canvas */}
        <div className="h-10 bg-forge-surface border-t border-forge-border flex items-center px-3 gap-4">
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">COMPONENTS:</span>
            <span className="text-forge-electrical">{components.length}</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">WIRES:</span>
            <span className="text-forge-electrical">{wires.length}</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          {isSimulating && (
            <>
              <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
                <span className="font-semibold">SIM TIME:</span>
                <span className="text-forge-green font-mono">{simulationTime.toFixed(2)}s</span>
              </div>
              <div className="w-px h-4 bg-forge-border" />
            </>
          )}
          {circuitErrors.length > 0 && (
            <div className="flex items-center gap-2 text-[9px] text-forge-warning">
              <span>⚠ {circuitErrors[0]}</span>
            </div>
          )}
          <div className="flex-1" />
          <div className="text-[8px] text-forge-text-muted">
            Drag components to canvas • Click to place wires • Right-click to rotate
          </div>
        </div>
      </div>

      {/* Properties Panel */}
      <div className="w-56 bg-forge-surface border-l border-forge-border p-3 overflow-y-auto">
        <div className="text-[9px] font-bold text-forge-electrical tracking-wider mb-3">PROPERTIES</div>
        
        {selectedCompData ? (
          <div className="space-y-3">
            <div className="glass-panel rounded-lg p-3 space-y-2">
              <div className="flex items-center justify-between">
                <span className="text-[8px] text-forge-text-muted">TYPE</span>
                <span className="text-[10px] text-forge-text font-semibold">
                  {selectedCompData.type.toUpperCase()}
                </span>
              </div>
              
              <div className="space-y-1.5">
                <label className="text-[8px] text-forge-text-muted block">LABEL</label>
                <input
                  type="text"
                  value={selectedCompData.label}
                  onChange={(e) => updateComponentValue(selectedCompData.id, e.target.value)}
                  className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-text focus:border-forge-electrical outline-none"
                />
              </div>

              <div className="space-y-1.5">
                <label className="text-[8px] text-forge-text-muted block">VALUE</label>
                <input
                  type="text"
                  value={selectedCompData.value}
                  onChange={(e) => {
                    const newComponents = components.map(c =>
                      c.id === selectedCompData.id ? { ...c, value: e.target.value } : c
                    );
                    setComponents(newComponents);
                  }}
                  className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none"
                />
              </div>

              <div className="space-y-1.5">
                <label className="text-[8px] text-forge-text-muted block">ROTATION</label>
                <div className="flex gap-1">
                  {[0, 90, 180, 270].map(rot => (
                    <button
                      key={rot}
                      onClick={() => {
                        setComponents(prev =>
                          prev.map(c =>
                            c.id === selectedCompData.id ? { ...c, rotation: rot } : c
                          )
                        );
                      }}
                      className={`flex-1 py-1 rounded text-[9px] font-semibold transition-all ${
                        selectedCompData.rotation === rot
                          ? 'bg-forge-electrical/30 text-forge-electrical border border-forge-electrical/50'
                          : 'bg-forge-panel/50 text-forge-text-dim border border-transparent hover:text-forge-text'
                      }`}
                    >
                      {rot}°
                    </button>
                  ))}
                </div>
              </div>
            </div>

            <div className="glass-panel rounded-lg p-3">
              <div className="text-[8px] text-forge-text-muted mb-2">PINS</div>
              <div className="space-y-1">
                {selectedCompData.pins.map((pin, i) => (
                  <div key={i} className="flex items-center justify-between text-[9px]">
                    <span className="text-forge-text-dim">Pin {i + 1}</span>
                    <div className={`w-2 h-2 rounded-full ${pin.connected ? 'bg-forge-green' : 'bg-forge-text-muted'}`} />
                  </div>
                ))}
              </div>
            </div>

            <button
              onClick={() => {
                setComponents(prev => prev.filter(c => c.id !== selectedCompData.id));
                setSelectedComponent(null);
              }}
              className="w-full py-2 rounded bg-forge-danger/20 text-forge-danger text-[9px] font-bold hover:bg-forge-danger/30 transition-all"
            >
              ✕ DELETE COMPONENT
            </button>
          </div>
        ) : (
          <div className="text-center py-8">
            <div className="text-3xl mb-2 opacity-30">⚡</div>
            <div className="text-[9px] text-forge-text-muted">Select a component to edit</div>
          </div>
        )}

        {/* Quick Reference */}
        <div className="mt-4 glass-panel rounded-lg p-3">
          <div className="text-[8px] font-bold text-forge-text-muted mb-2">QUICK REFERENCE</div>
          <div className="space-y-1 text-[8px] text-forge-text-dim">
            <div>• Drag components from palette</div>
            <div>• Wire tool (W) connects pins</div>
            <div>• Select tool (V) to move</div>
            <div>• Delete tool (D) removes</div>
            <div>• Rotate tool (R) spins</div>
          </div>
        </div>

        {/* Component Info */}
        <div className="mt-4 glass-panel rounded-lg p-3">
          <div className="text-[8px] font-bold text-forge-text-muted mb-2">CIRCUIT STATS</div>
          <div className="space-y-2">
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-dim">Resistors</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type === 'resistor').length}
              </span>
            </div>
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-dim">Capacitors</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type === 'capacitor').length}
              </span>
            </div>
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-dim">Transistors</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type.includes('transistor')).length}
              </span>
            </div>
            <div className="flex justify-between text-[9px]">
              <span className="text-forge-text-dim">ICs</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type === 'opamp').length}
              </span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
