import { useRef, useEffect, useState, useCallback } from 'react';
import { useProject } from '../store/ProjectContext';

// Component types
type ComponentType = 
  // Electrical
  | 'resistor' | 'capacitor' | 'inductor' | 'diode' | 'led' 
  | 'npn_transistor' | 'pnp_transistor' | 'opamp' | 'vcc' | 'gnd'
  // Motors
  | 'dc_motor' | 'stepper_motor' | 'servo_motor'
  // Motor Parts
  | 'stator' | 'rotor' | 'coil' | 'magnet' | 'bearing'
  // Mechanical
  | 'sprocket' | 'gear' | 'pulley' | 'shaft'
  // Chain connection
  | 'chain';

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
  // Motor specific
  rpm?: number;
  torque?: number;
  direction?: 'cw' | 'ccw';
  // Sprocket specific
  teeth?: number;
  diameter?: number;
  // Motor internals
  material?: string;
  turns?: number;
  magneticStrength?: number;
  lubricant?: string;
  // Chain connection
  chainStartId?: string;
  chainEndId?: string;
}

interface Wire {
  id: string;
  startX: number;
  startY: number;
  endX: number;
  endY: number;
  selected: boolean;
}

interface ChainConnection {
  id: string;
  startSprocketId: string;
  endSprocketId: string;
  startAngle: number;
  endAngle: number;
}

interface Tool {
  id: string;
  name: string;
  icon: string;
  shortcut: string;
  category: string;
}

type ComponentCategory = 'electrical' | 'motors' | 'motor_parts' | 'mechanical';

const ELECTRICAL_COMPONENTS: { type: ComponentType; name: string; icon: string; defaultValue: string }[] = [
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

const MOTOR_COMPONENTS: { type: ComponentType; name: string; icon: string; defaultValue: string }[] = [
  { type: 'dc_motor', name: 'DC Motor', icon: '⚙', defaultValue: '12V 3000RPM' },
  { type: 'stepper_motor', name: 'Stepper', icon: '◑', defaultValue: 'NEMA17' },
  { type: 'servo_motor', name: 'Servo', icon: '◎', defaultValue: 'MG996R' },
];

const MECHANICAL_COMPONENTS: { type: ComponentType; name: string; icon: string; defaultValue: string }[] = [
  { type: 'sprocket', name: 'Sprocket', icon: '⭘', defaultValue: '20T' },
  { type: 'gear', name: 'Gear', icon: '⚪', defaultValue: '40T' },
  { type: 'pulley', name: 'Pulley', icon: '◯', defaultValue: '50mm' },
  { type: 'shaft', name: 'Shaft', icon: '|', defaultValue: '8mm' },
];

const MOTOR_PARTS: { type: ComponentType; name: string; icon: string; defaultValue: string }[] = [
  { type: 'stator', name: 'Stator Body', icon: 'Ⓞ', defaultValue: 'Silicon Steel' },
  { type: 'rotor', name: 'Rotor Body', icon: '◎', defaultValue: 'Steel' },
  { type: 'coil', name: 'Copper Coil', icon: '➿', defaultValue: '100 Turns' },
  { type: 'magnet', name: 'Magnet', icon: '🧲', defaultValue: 'Neodymium' },
  { type: 'bearing', name: 'Bearing & Oil', icon: '⊚', defaultValue: 'Lubricated' },
];

const TOOLS: Tool[] = [
  { id: 'select', name: 'Select', icon: '↖', shortcut: 'V', category: 'all' },
  { id: 'wire', name: 'Wire', icon: '⤳', shortcut: 'W', category: 'electrical' },
  { id: 'chain', name: 'Chain', icon: '🔗', shortcut: 'C', category: 'mechanical' },
  { id: 'delete', name: 'Delete', icon: '✕', shortcut: 'D', category: 'all' },
  { id: 'rotate', name: 'Rotate', icon: '↻', shortcut: 'R', category: 'all' },
];

const GRID_SIZE = 20;

export default function CircuitDesigner() {
  const { customMaterials, saveMotorProject } = useProject();
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const animRef = useRef<number>(0);
  const simRef = useRef<number>(0);
  
  const [components, setComponents] = useState<Component[]>([]);
  const [wires, setWires] = useState<Wire[]>([]);
  const [chainConnections, setChainConnections] = useState<ChainConnection[]>([]);
  const [selectedTool, setSelectedTool] = useState<string>('select');
  const [selectedCategory, setSelectedCategory] = useState<ComponentCategory>('electrical');
  const [selectedComponent, setSelectedComponent] = useState<string | null>(null);
  const [dragging, setDragging] = useState<{ type: ComponentType; fromPalette: boolean } | null>(null);
  const [wireStart, setWireStart] = useState<{ x: number; y: number } | null>(null);
  const [chainStart, setChainStart] = useState<string | null>(null);
  const [mousePos, setMousePos] = useState<{ x: number; y: number }>({ x: 0, y: 0 });
  const [isSimulating, setIsSimulating] = useState(false);
  const [simulationTime, setSimulationTime] = useState(0);
  const [circuitErrors, setCircuitErrors] = useState<string[]>([]);
  const [motorRPMs, setMotorRPMs] = useState<Record<string, number>>({});

  const snapToGrid = useCallback((value: number) => Math.round(value / GRID_SIZE) * GRID_SIZE, []);
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
          { x: -30, y: -15, connected: false },
          { x: -30, y: 15, connected: false },
          { x: 30, y: 0, connected: false },
          { x: 0, y: -25, connected: false },
          { x: 0, y: 25, connected: false },
        ];
      case 'vcc':
      case 'gnd':
        return [{ x: 0, y: 15, connected: false }];
      case 'dc_motor':
      case 'stepper_motor':
      case 'servo_motor':
        return [
          { x: -35, y: 0, connected: false },  // Negative/Motor-
          { x: 35, y: 0, connected: false },   // Positive/Motor+
        ];
      case 'coil':
        return [
          { x: -15, y: 0, connected: false },
          { x: 15, y: 0, connected: false },
        ];
      default:
        return [];
    }
  };

  const createComponent = (type: ComponentType, x: number, y: number): Component => {
    const allComps = [...ELECTRICAL_COMPONENTS, ...MOTOR_COMPONENTS, ...MOTOR_PARTS, ...MECHANICAL_COMPONENTS];
    const comp = allComps.find(c => c.type === type);
    
    const isMotor = type === 'dc_motor' || type === 'stepper_motor' || type === 'servo_motor';
    const isSprocket = type === 'sprocket' || type === 'gear' || type === 'pulley';
    
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
      rpm: isMotor ? 1500 : undefined,
      torque: isMotor ? 0.5 : undefined,
      direction: 'cw',
      teeth: isSprocket ? 20 : undefined,
      diameter: type === 'pulley' ? 50 : undefined,
      material: type === 'stator' ? 'Silicon Steel' : type === 'rotor' ? 'Steel' : type === 'magnet' ? 'Neodymium' : type === 'coil' ? 'Copper' : undefined,
      turns: type === 'coil' ? 100 : undefined,
      magneticStrength: type === 'magnet' ? 1.2 : undefined,
      lubricant: type === 'bearing' ? 'Synthetic Oil' : undefined,
    };
  };

  // Calculate chain path between two sprockets
  const getChainPath = (start: Component, end: Component, time: number) => {
    const teeth = (start.teeth || 20) + (end.teeth || 20);
    const rpm1 = motorRPMs[start.id] || 0;
    const rpm2 = motorRPMs[end.id] || 0;
    const ratio = rpm2 / (rpm1 || 1);
    
    const x1 = start.x, y1 = start.y;
    const x2 = end.x, y2 = end.y;
    const dist = Math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2);
    
    const dx = x2 - x1, dy = y2 - y1;
    const angle = Math.atan2(dy, dx);
    
    // Chain path with tensioner offset
    const offset = 20;
    const midX = (x1 + x2) / 2 + Math.cos(angle + Math.PI / 2) * offset;
    const midY = (y1 + y2) / 2 + Math.sin(angle + Math.PI / 2) * offset;
    
    return {
      points: [
        { x: x1, y: y1 },
        { x: midX, y: midY },
        { x: x2, y: y2 },
      ],
      animatedOffset: time * (rpm1 / 60) * 2 * Math.PI * 0.1,
      ratio,
    };
  };

  // Draw sprocket with teeth
  const drawSprocket = (ctx: CanvasRenderingContext2D, comp: Component, time: number, isSelected: boolean) => {
    const { x, y, rotation, teeth = 20 } = comp;
    const radius = 30 + teeth * 0.5;
    const isPowered = motorRPMs[comp.id] && motorRPMs[comp.id] !== 0;
    
    ctx.save();
    ctx.translate(x, y);
    ctx.rotate((rotation * Math.PI) / 180);
    
    // Outer glow for powered sprockets
    if (isPowered && isSimulating) {
      const glowGrad = ctx.createRadialGradient(0, 0, radius - 5, 0, 0, radius + 15);
      glowGrad.addColorStop(0, 'rgba(0, 255, 136, 0.3)');
      glowGrad.addColorStop(1, 'transparent');
      ctx.fillStyle = glowGrad;
      ctx.beginPath();
      ctx.arc(0, 0, radius + 15, 0, Math.PI * 2);
      ctx.fill();
    }

    // Selection glow
    if (isSelected) {
      ctx.shadowColor = 'rgba(0, 238, 255, 0.6)';
      ctx.shadowBlur = 20;
    }

    // Draw teeth
    ctx.fillStyle = '#00c2ff20';
    ctx.strokeStyle = isPowered && isSimulating ? '#00ff88' : '#00c2ff';
    ctx.lineWidth = 2;
    
    for (let i = 0; i < teeth; i++) {
      const angle = (i / teeth) * Math.PI * 2 + (isSimulating ? time * 0.01 * (motorRPMs[comp.id] || 0) / 60 : 0);
      const innerR = radius - 8;
      const outerR = radius + 5;
      
      const a1 = angle - 0.15;
      const a2 = angle + 0.15;
      
      ctx.beginPath();
      ctx.moveTo(Math.cos(a1) * innerR, Math.sin(a1) * innerR);
      ctx.lineTo(Math.cos(a1) * outerR, Math.sin(a1) * outerR);
      ctx.lineTo(Math.cos(a2) * outerR, Math.sin(a2) * outerR);
      ctx.lineTo(Math.cos(a2) * innerR, Math.sin(a2) * innerR);
      ctx.closePath();
      ctx.fill();
      ctx.stroke();
    }

    // Center hub
    ctx.beginPath();
    ctx.arc(0, 0, radius - 12, 0, Math.PI * 2);
    ctx.fill();
    ctx.stroke();

    // Center hole
    ctx.beginPath();
    ctx.arc(0, 0, 8, 0, Math.PI * 2);
    ctx.fillStyle = '#0a0c10';
    ctx.fill();
    ctx.strokeStyle = '#00c2ff';
    ctx.stroke();

    // Teeth count label
    ctx.font = '9px monospace';
    ctx.fillStyle = '#00c2ff';
    ctx.textAlign = 'center';
    ctx.fillText(`${teeth}T`, 0, radius + 18);

    ctx.restore();
  };

  // Draw DC Motor
  const drawMotor = (ctx: CanvasRenderingContext2D, comp: Component, time: number, isSelected: boolean) => {
    const { x, y, rotation, type, rpm = 0 } = comp;
    const isPowered = rpm !== 0 && isSimulating;
    
    ctx.save();
    ctx.translate(x, y);
    ctx.rotate((rotation * Math.PI) / 180);

    if (isSelected) {
      ctx.shadowColor = 'rgba(0, 238, 255, 0.6)';
      ctx.shadowBlur = 20;
    }

    ctx.strokeStyle = isPowered ? '#00ff88' : '#00c2ff';
    ctx.fillStyle = '#00c2ff15';
    ctx.lineWidth = 2;

    // Motor body
    ctx.beginPath();
    ctx.roundRect(-40, -25, 70, 50, 5);
    ctx.fill();
    ctx.stroke();

    // Motor shaft
    ctx.beginPath();
    ctx.moveTo(30, 0);
    ctx.lineTo(50, 0);
    ctx.stroke();

    // Shaft rotation indicator
    if (isPowered) {
      const rotAngle = time * 0.01 * rpm / 60 * 360;
      ctx.beginPath();
      ctx.arc(50, 0, 8, 0, Math.PI * 2);
      ctx.stroke();
      
      // Arrow showing rotation
      ctx.beginPath();
      ctx.moveTo(50 + 6 * Math.cos(rotAngle), 6 * Math.sin(rotAngle));
      ctx.lineTo(50 + 10 * Math.cos(rotAngle), 10 * Math.sin(rotAngle));
      ctx.stroke();
    }

    // Connection terminals
    ctx.fillStyle = '#00ff88';
    ctx.fillRect(-42, -8, 6, 6);
    ctx.fillRect(-42, 2, 6, 6);

    // Motor type icon
    ctx.font = '10px monospace';
    ctx.fillStyle = isPowered ? '#00ff88' : '#00c2ff';
    ctx.textAlign = 'center';
    ctx.fillText(type === 'dc_motor' ? 'M' : type === 'stepper_motor' ? 'ST' : 'SV', -10, 5);

    // Power indicator
    if (isPowered) {
      ctx.fillStyle = '#00ff88';
      ctx.beginPath();
      ctx.arc(-35, -15, 3, 0, Math.PI * 2);
      ctx.fill();
    }

    ctx.restore();
  };

  // Draw gear
  const drawGear = (ctx: CanvasRenderingContext2D, comp: Component, time: number, isSelected: boolean) => {
    const { x, y, rotation, teeth = 40 } = comp;
    const radius = 25 + teeth * 0.4;
    const isPowered = motorRPMs[comp.id] && motorRPMs[comp.id] !== 0;
    
    ctx.save();
    ctx.translate(x, y);
    ctx.rotate((rotation * Math.PI) / 180);

    if (isSelected) {
      ctx.shadowColor = 'rgba(0, 238, 255, 0.6)';
      ctx.shadowBlur = 20;
    }

    ctx.strokeStyle = isPowered && isSimulating ? '#00ff88' : '#ffa500';
    ctx.fillStyle = '#ffa50015';
    ctx.lineWidth = 2;

    // Draw gear teeth
    for (let i = 0; i < teeth; i++) {
      const angle = (i / teeth) * Math.PI * 2 + (isSimulating ? time * 0.01 * (motorRPMs[comp.id] || 0) / 60 : 0);
      const innerR = radius - 6;
      const outerR = radius;
      
      const a1 = angle - Math.PI / teeth * 0.8;
      const a2 = angle + Math.PI / teeth * 0.8;
      
      ctx.beginPath();
      ctx.moveTo(Math.cos(a1) * innerR, Math.sin(a1) * innerR);
      ctx.lineTo(Math.cos(a1) * outerR, Math.sin(a1) * outerR);
      ctx.lineTo(Math.cos(a2) * outerR, Math.sin(a2) * outerR);
      ctx.lineTo(Math.cos(a2) * innerR, Math.sin(a2) * innerR);
      ctx.closePath();
      ctx.fill();
      ctx.stroke();
    }

    // Center
    ctx.beginPath();
    ctx.arc(0, 0, radius - 10, 0, Math.PI * 2);
    ctx.fill();
    ctx.stroke();

    ctx.beginPath();
    ctx.arc(0, 0, 5, 0, Math.PI * 2);
    ctx.fillStyle = '#0a0c10';
    ctx.fill();
    ctx.stroke();

    ctx.restore();
  };

  // Draw pulley
  const drawPulley = (ctx: CanvasRenderingContext2D, comp: Component, time: number, isSelected: boolean) => {
    const { x, y, rotation, diameter = 50 } = comp;
    const radius = diameter * 0.4;
    const isPowered = motorRPMs[comp.id] && motorRPMs[comp.id] !== 0;
    
    ctx.save();
    ctx.translate(x, y);
    ctx.rotate((rotation * Math.PI) / 180);

    if (isSelected) {
      ctx.shadowColor = 'rgba(0, 238, 255, 0.6)';
      ctx.shadowBlur = 20;
    }

    ctx.strokeStyle = isPowered && isSimulating ? '#00ff88' : '#ff6600';
    ctx.fillStyle = '#ff660015';
    ctx.lineWidth = 2;

    // Grooved pulley
    ctx.beginPath();
    ctx.ellipse(0, 0, radius, radius * 0.6, 0, 0, Math.PI * 2);
    ctx.stroke();

    // Inner groove
    ctx.beginPath();
    ctx.ellipse(0, 0, radius - 5, (radius - 5) * 0.6, 0, 0, Math.PI * 2);
    ctx.stroke();

    // Belt wrap indicator
    if (isPowered && isSimulating) {
      const beltAngle = time * 0.01 * (motorRPMs[comp.id] || 0) / 60 * Math.PI;
      ctx.beginPath();
      ctx.arc(0, 0, radius + 3, beltAngle, beltAngle + Math.PI * 1.5);
      ctx.strokeStyle = '#ff660060';
      ctx.stroke();
    }

    // Center
    ctx.beginPath();
    ctx.arc(0, 0, 5, 0, Math.PI * 2);
    ctx.fillStyle = '#0a0c10';
    ctx.fill();
    ctx.strokeStyle = '#ff6600';
    ctx.stroke();

    ctx.restore();
  };

  // Draw chain between sprockets
  const drawChain = (ctx: CanvasRenderingContext2D, chain: ChainConnection, time: number) => {
    const startComp = components.find(c => c.id === chain.startSprocketId);
    const endComp = components.find(c => c.id === chain.endSprocketId);
    if (!startComp || !endComp) return;

    const path = getChainPath(startComp, endComp, time);
    
    ctx.save();
    ctx.strokeStyle = '#888888';
    ctx.lineWidth = 3;
    ctx.setLineDash([]);

    // Draw chain path
    ctx.beginPath();
    ctx.moveTo(path.points[0].x, path.points[0].y);
    for (let i = 1; i < path.points.length; i++) {
      ctx.lineTo(path.points[i].x, path.points[i].y);
    }
    ctx.stroke();

    // Animated chain links
    const totalDist = Math.sqrt(
      (path.points[path.points.length - 1].x - path.points[0].x) ** 2 +
      (path.points[path.points.length - 1].y - path.points[0].y) ** 2
    );
    const numLinks = Math.floor(totalDist / 10);
    
    for (let i = 0; i < numLinks; i++) {
      const t = ((i / numLinks) + (path.animatedOffset % 1)) % 1;
      const linkX = path.points[0].x + (path.points[2].x - path.points[0].x) * t;
      const linkY = path.points[0].y + (path.points[2].y - path.points[0].y) * t;
      
      ctx.beginPath();
      ctx.arc(linkX, linkY, 3, 0, Math.PI * 2);
      ctx.fillStyle = i % 2 === 0 ? '#666' : '#888';
      ctx.fill();
    }

    ctx.restore();
  };

  // Draw component
  const drawComponent = (ctx: CanvasRenderingContext2D, comp: Component, time: number) => {
    const isSelected = selectedComponent === comp.id;
    const isMechanical = ['sprocket', 'gear', 'pulley'].includes(comp.type);
    const isMotor = ['dc_motor', 'stepper_motor', 'servo_motor'].includes(comp.type);
    const isMotorPart = ['stator', 'rotor', 'coil', 'magnet', 'bearing'].includes(comp.type);

    if (isMechanical) {
      if (comp.type === 'sprocket') {
        drawSprocket(ctx, comp, time, isSelected);
      } else if (comp.type === 'gear') {
        drawGear(ctx, comp, time, isSelected);
      } else if (comp.type === 'pulley') {
        drawPulley(ctx, comp, time, isSelected);
      }
    } else if (isMotor) {
      drawMotor(ctx, comp, time, isSelected);
    } else if (isMotorPart) {
      drawMotorPart(ctx, comp, time, isSelected);
    } else {
      drawElectricalComponent(ctx, comp, time, isSelected);
    }
  };

  const drawMotorPart = (ctx: CanvasRenderingContext2D, comp: Component, time: number, isSelected: boolean) => {
    ctx.save();
    ctx.translate(comp.x, comp.y);
    ctx.rotate((comp.rotation * Math.PI) / 180);

    if (isSelected) {
      ctx.shadowColor = 'rgba(0, 238, 255, 0.6)';
      ctx.shadowBlur = 20;
    }

    ctx.lineWidth = 2;

    switch (comp.type) {
      case 'stator':
        ctx.strokeStyle = '#8899aa';
        ctx.fillStyle = '#8899aa15';
        ctx.beginPath();
        ctx.arc(0, 0, 60, 0, Math.PI * 2);
        ctx.arc(0, 0, 45, 0, Math.PI * 2, true);
        ctx.fill();
        ctx.stroke();
        // Slots
        for(let i=0; i<12; i++) {
          ctx.beginPath();
          const a = (i/12)*Math.PI*2;
          ctx.moveTo(Math.cos(a)*45, Math.sin(a)*45);
          ctx.lineTo(Math.cos(a)*55, Math.sin(a)*55);
          ctx.stroke();
        }
        break;
      case 'rotor':
        // If simulating and part of a built motor, spin it!
        const r_rpm = motorRPMs[comp.id] || 0;
        if (isSimulating && r_rpm !== 0) {
          ctx.rotate(time * 0.01 * r_rpm / 60 * Math.PI * 2);
        }
        ctx.strokeStyle = '#aaaaaa';
        ctx.fillStyle = '#aaaaaa30';
        ctx.beginPath();
        ctx.arc(0, 0, 40, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(0, 0, 10, 0, Math.PI * 2);
        ctx.fillStyle = '#111';
        ctx.fill();
        ctx.stroke();
        break;
      case 'coil':
        ctx.strokeStyle = '#ff8800'; // Copper
        ctx.fillStyle = '#ff880020';
        ctx.beginPath();
        ctx.roundRect(-20, -10, 40, 20, 5);
        ctx.fill();
        ctx.stroke();
        // Windings
        for(let i=-15; i<=15; i+=5) {
          ctx.beginPath();
          ctx.moveTo(i, -10);
          ctx.lineTo(i, 10);
          ctx.stroke();
        }
        break;
      case 'magnet':
        ctx.strokeStyle = '#ff3333';
        ctx.fillStyle = '#ff333340';
        ctx.fillRect(-15, -10, 15, 20);
        ctx.strokeRect(-15, -10, 15, 20);
        ctx.strokeStyle = '#3333ff';
        ctx.fillStyle = '#3333ff40';
        ctx.fillRect(0, -10, 15, 20);
        ctx.strokeRect(0, -10, 15, 20);
        ctx.fillStyle = '#fff';
        ctx.font = '10px monospace';
        ctx.fillText('N', -7, 4);
        ctx.fillText('S', 8, 4);
        break;
      case 'bearing':
        ctx.strokeStyle = '#gold';
        ctx.fillStyle = '#ffff0020';
        ctx.beginPath();
        ctx.arc(0, 0, 15, 0, Math.PI * 2);
        ctx.arc(0, 0, 8, 0, Math.PI * 2, true);
        ctx.fill();
        ctx.stroke();
        // Balls
        ctx.fillStyle = '#ddd';
        for(let i=0; i<8; i++) {
          const a = (i/8)*Math.PI*2;
          ctx.beginPath();
          ctx.arc(Math.cos(a)*11.5, Math.sin(a)*11.5, 2.5, 0, Math.PI*2);
          ctx.fill();
        }
        break;
    }

    // Draw pins
    comp.pins.forEach(pin => {
      ctx.beginPath();
      ctx.arc(pin.x, pin.y, 3, 0, Math.PI * 2);
      ctx.fillStyle = pin.connected ? '#00ff88' : '#00eeff';
      ctx.fill();
    });

    // Label
    ctx.font = '8px monospace';
    ctx.fillStyle = '#00c2ff';
    ctx.textAlign = 'center';
    ctx.fillText(comp.label, 0, 25);

    ctx.restore();
  };

  // Draw electrical component (simplified)
  const drawElectricalComponent = (ctx: CanvasRenderingContext2D, comp: Component, time: number, isSelected: boolean) => {
    ctx.save();
    ctx.translate(comp.x, comp.y);
    ctx.rotate((comp.rotation * Math.PI) / 180);

    if (isSelected) {
      ctx.shadowColor = 'rgba(0, 238, 255, 0.4)';
      ctx.shadowBlur = 15;
    }

    ctx.strokeStyle = isSelected ? '#00eeff' : '#00c2ff';
    ctx.fillStyle = '#00c2ff10';
    ctx.lineWidth = 2;
    ctx.lineCap = 'round';

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
        ctx.moveTo(-30, 0); ctx.lineTo(-5, 0); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(-5, -15); ctx.lineTo(-5, 15); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(5, -15); ctx.lineTo(5, 15); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(5, 0); ctx.lineTo(30, 0); ctx.stroke();
        break;
      case 'vcc':
        ctx.beginPath();
        ctx.moveTo(0, 15); ctx.lineTo(0, -5); ctx.stroke();
        ctx.beginPath();
        ctx.arc(0, -10, 10, Math.PI, 0, false); ctx.stroke();
        ctx.fillStyle = '#00ff88';
        ctx.font = 'bold 10px monospace';
        ctx.textAlign = 'center';
        ctx.fillText('VCC', 0, -25);
        break;
      case 'gnd':
        ctx.beginPath();
        ctx.moveTo(0, -15); ctx.lineTo(0, 0); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(-12, 0); ctx.lineTo(12, 0); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(-8, 5); ctx.lineTo(8, 5); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(-4, 10); ctx.lineTo(4, 10); ctx.stroke();
        break;
      case 'led':
        ctx.beginPath();
        ctx.moveTo(-30, 0); ctx.lineTo(-10, 0); ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(-10, -12); ctx.lineTo(-10, 12); ctx.lineTo(10, 0); ctx.closePath();
        ctx.fillStyle = isSimulating ? 'rgba(0, 255, 100, 0.5)' : 'rgba(255, 50, 50, 0.3)';
        ctx.fill(); ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(10, -12); ctx.lineTo(10, 12); ctx.stroke();
        ctx.beginPath(); ctx.moveTo(10, 0); ctx.lineTo(30, 0); ctx.stroke();
        break;
      default:
        ctx.beginPath();
        ctx.roundRect(-25, -15, 50, 30, 3);
        ctx.fill();
        ctx.stroke();
    }

    // Draw pins
    comp.pins.forEach(pin => {
      ctx.beginPath();
      ctx.arc(pin.x, pin.y, 3, 0, Math.PI * 2);
      ctx.fillStyle = pin.connected ? '#00ff88' : '#00eeff';
      ctx.fill();
    });

    // Label
    ctx.font = '9px monospace';
    ctx.fillStyle = '#888';
    ctx.textAlign = 'center';
    ctx.fillText(comp.label, 0, 40);

    ctx.font = '8px monospace';
    ctx.fillStyle = '#00c2ff';
    ctx.fillText(comp.value, 0, 52);

    ctx.restore();
  };

  // Canvas rendering
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
      const w = canvas.width, h = canvas.height;

      ctx.fillStyle = '#0a0c10';
      ctx.fillRect(0, 0, w, h);

      // Grid
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.5)';
      ctx.lineWidth = 0.5;
      for (let x = 0; x < w; x += GRID_SIZE) {
        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke();
      }
      for (let y = 0; y < h; y += GRID_SIZE) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
      }

      // Draw chains
      chainConnections.forEach(chain => drawChain(ctx, chain, time));

      // Draw wires
      wires.forEach(wire => {
        ctx.strokeStyle = wire.selected ? '#ff3366' : '#00eeff';
        ctx.lineWidth = wire.selected ? 3 : 2;
        ctx.shadowColor = wire.selected ? 'rgba(255, 51, 102, 0.5)' : 'rgba(0, 238, 255, 0.3)';
        ctx.shadowBlur = 8;
        ctx.beginPath();
        ctx.moveTo(wire.startX, wire.startY);
        ctx.lineTo(wire.endX, wire.endY);
        ctx.stroke();
        ctx.shadowBlur = 0;

        ctx.beginPath();
        ctx.arc(wire.startX, wire.startY, 4, 0, Math.PI * 2);
        ctx.fillStyle = wire.selected ? '#ff3366' : '#00eeff';
        ctx.fill();
        ctx.beginPath();
        ctx.arc(wire.endX, wire.endY, 4, 0, Math.PI * 2);
        ctx.fill();
      });

      // In-progress wire
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

      // In-progress chain
      if (chainStart) {
        const startComp = components.find(c => c.id === chainStart);
        if (startComp && ['sprocket', 'gear', 'pulley'].includes(startComp.type)) {
          ctx.strokeStyle = 'rgba(136, 136, 136, 0.6)';
          ctx.lineWidth = 3;
          ctx.setLineDash([5, 5]);
          ctx.beginPath();
          ctx.arc(mousePos.x, mousePos.y, 40, 0, Math.PI * 2);
          ctx.stroke();
          ctx.beginPath();
          ctx.moveTo(startComp.x, startComp.y);
          ctx.lineTo(mousePos.x, mousePos.y);
          ctx.stroke();
          ctx.setLineDash([]);
        }
      }

      // Draw components
      components.forEach(comp => drawComponent(ctx, comp, time));

      // Ghost component
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
  }, [components, wires, chainConnections, selectedComponent, wireStart, chainStart, mousePos, dragging, isSimulating, motorRPMs]);

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
        setWires(prev => [...prev, {
          id: generateId(), startX: wireStart.x, startY: wireStart.y,
          endX: snapToGrid(x), endY: snapToGrid(y), selected: false,
        }]);
        setWireStart(null);
      }
    } else if (selectedTool === 'chain') {
      const clickedSprocket = components.find(comp => {
        if (!['sprocket', 'gear', 'pulley', 'dc_motor', 'stepper_motor', 'servo_motor'].includes(comp.type)) return false;
        const dx = x - comp.x, dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 40;
      });

      if (clickedSprocket) {
        if (!chainStart) {
          setChainStart(clickedSprocket.id);
        } else if (chainStart !== clickedSprocket.id) {
          setChainConnections(prev => [...prev, {
            id: generateId(),
            startSprocketId: chainStart,
            endSprocketId: clickedSprocket.id,
            startAngle: 0,
            endAngle: 0,
          }]);
          setChainStart(null);
        }
      } else {
        setChainStart(null);
      }
    } else if (selectedTool === 'select') {
      const clickedComp = components.find(comp => {
        const dx = x - comp.x, dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 35;
      });

      if (clickedComp) {
        setSelectedComponent(clickedComp.id);
        setComponents(prev => prev.map(c => ({ ...c, selected: c.id === clickedComp.id })));
      } else {
        setSelectedComponent(null);
        setComponents(prev => prev.map(c => ({ ...c, selected: false })));
      }
    } else if (selectedTool === 'delete') {
      const clickedComp = components.find(comp => {
        const dx = x - comp.x, dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 35;
      });
      if (clickedComp) {
        setComponents(prev => prev.filter(c => c.id !== clickedComp.id));
        setChainConnections(prev => prev.filter(ch => 
          ch.startSprocketId !== clickedComp.id && ch.endSprocketId !== clickedComp.id
        ));
        setSelectedComponent(null);
      }
      const clickedWire = wires.find(wire => {
        const A = wire.startX, B = wire.startY, C = wire.endX, D = wire.endY;
        const dx = C - A, dy = D - B, len = Math.sqrt(dx * dx + dy * dy);
        if (len === 0) return false;
        const t = Math.max(0, Math.min(1, ((x - A) * dx + (y - B) * dy) / (len * len)));
        return Math.sqrt((x - (A + t * dx)) ** 2 + (y - (B + t * dy)) ** 2) < 10;
      });
      if (clickedWire) setWires(prev => prev.filter(w => w.id !== clickedWire.id));
      
      const clickedChain = chainConnections.find(ch => {
        const startC = components.find(c => c.id === ch.startSprocketId);
        const endC = components.find(c => c.id === ch.endSprocketId);
        if (!startC || !endC) return false;
        const dist = Math.sqrt((endC.x - startC.x) ** 2 + (endC.y - startC.y) ** 2);
        const t = ((x - startC.x) * (endC.x - startC.x) + (y - startC.y) * (endC.y - startC.y)) / (dist * dist);
        const nearX = startC.x + t * (endC.x - startC.x);
        const nearY = startC.y + t * (endC.y - startC.y);
        return Math.sqrt((x - nearX) ** 2 + (y - nearY) ** 2) < 15;
      });
      if (clickedChain) setChainConnections(prev => prev.filter(c => c.id !== clickedChain.id));
    } else if (selectedTool === 'rotate') {
      const clickedComp = components.find(comp => {
        const dx = x - comp.x, dy = y - comp.y;
        return Math.sqrt(dx * dx + dy * dy) < 35;
      });
      if (clickedComp) {
        setComponents(prev => prev.map(c =>
          c.id === clickedComp.id ? { ...c, rotation: (c.rotation + 45) % 360 } : c
        ));
      }
    }
  };

  const handleMouseMove = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    setMousePos({ x: e.clientX - rect.left, y: e.clientY - rect.top });

    if (dragging && dragging.fromPalette) return;
    
    if (selectedComponent && selectedTool === 'select' && e.buttons === 1) {
      setComponents(prev => prev.map(c =>
        c.id === selectedComponent ? { ...c, x: snapToGrid(e.clientX - rect.left), y: snapToGrid(e.clientY - rect.top) } : c
      ));
    }
  };

  const handleMouseUp = (e: React.MouseEvent<HTMLCanvasElement>) => {
    if (dragging && dragging.fromPalette) {
      const canvas = canvasRef.current;
      if (canvas) {
        const rect = canvas.getBoundingClientRect();
        const newComp = createComponent(dragging.type, e.clientX - rect.left, e.clientY - rect.top);
        setComponents(prev => [...prev, newComp]);
      }
      setDragging(null);
    }
  };

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    const type = e.dataTransfer.getData('componentType') as ComponentType;
    if (type) {
      const canvas = canvasRef.current;
      if (canvas) {
        const rect = canvas.getBoundingClientRect();
        const newComp = createComponent(type, e.clientX - rect.left, e.clientY - rect.top);
        setComponents(prev => [...prev, newComp]);
      }
    }
  };

  // Run simulation
  const runSimulation = () => {
    setCircuitErrors([]);
    
    // Check for motors connected to power
    const motors = components.filter(c => 
      ['dc_motor', 'stepper_motor', 'servo_motor'].includes(c.type)
    );
    const vccCount = components.filter(c => c.type === 'vcc').length;
    const gndCount = components.filter(c => c.type === 'gnd').length;
    
    if (vccCount === 0) setCircuitErrors(prev => [...prev, 'Warning: No VCC source']);
    if (gndCount === 0) setCircuitErrors(prev => [...prev, 'Warning: No Ground connected']);
    
    // Set motor RPMs based on connected power
    const newRPMs: Record<string, number> = {};
    motors.forEach((motor, i) => {
      newRPMs[motor.id] = vccCount > 0 ? (motor.rpm || 1500) * (0.8 + Math.random() * 0.4) : 0;
      
      // Also power any sprockets that are placed directly on the motor (overlapping)
      components.forEach(comp => {
        if (['sprocket', 'gear', 'pulley'].includes(comp.type)) {
          if (Math.abs(comp.x - motor.x) < 20 && Math.abs(comp.y - motor.y) < 20) {
            newRPMs[comp.id] = newRPMs[motor.id];
          }
        }
      });
    });

    // Custom Built Motors Logic
    const rotors = components.filter(c => c.type === 'rotor');
    const stators = components.filter(c => c.type === 'stator');
    const coils = components.filter(c => c.type === 'coil');
    const magnets = components.filter(c => c.type === 'magnet');
    const bearings = components.filter(c => c.type === 'bearing');

    rotors.forEach(rotor => {
      // Find enclosing stator
      const stator = stators.find(s => Math.abs(s.x - rotor.x) < 30 && Math.abs(s.y - rotor.y) < 30);
      if (stator && vccCount > 0) {
        // Find attached coils to stator
        const attachedCoils = coils.filter(c => Math.abs(c.x - stator.x) < 80 && Math.abs(c.y - stator.y) < 80);
        // Find attached magnets to rotor
        const attachedMagnets = magnets.filter(m => Math.abs(m.x - rotor.x) < 50 && Math.abs(m.y - rotor.y) < 50);
        // Find attached bearing
        const attachedBearing = bearings.find(b => Math.abs(b.x - rotor.x) < 40 && Math.abs(b.y - rotor.y) < 40);

        if (attachedCoils.length > 0 && attachedMagnets.length > 0) {
          const totalTurns = attachedCoils.reduce((sum, c) => sum + (c.turns || 100), 0);
          const totalStrength = attachedMagnets.reduce((sum, m) => sum + (m.magneticStrength || 1.2), 0);
          
          let rpm = (totalTurns * totalStrength * 0.5);
          if (attachedBearing && attachedBearing.lubricant) {
            rpm *= 1.2; // Lubricant reduces friction
          }
          
          newRPMs[rotor.id] = rpm;
          
          // Power any overlapping sprockets
          components.forEach(comp => {
            if (['sprocket', 'gear', 'pulley'].includes(comp.type)) {
              if (Math.abs(comp.x - rotor.x) < 20 && Math.abs(comp.y - rotor.y) < 20) {
                newRPMs[comp.id] = rpm;
              }
            }
          });
          
          // Save the motor assembly to the global project state
          saveMotorProject([stator, rotor, ...attachedCoils, ...attachedMagnets, ...(attachedBearing ? [attachedBearing] : [])], rpm);
        }
      }
    });

    // Propagate RPM through chains
    let changed = true;
    while (changed) {
      changed = false;
      chainConnections.forEach(chain => {
        const startComp = components.find(c => c.id === chain.startSprocketId);
        const endComp = components.find(c => c.id === chain.endSprocketId);
        if (startComp && endComp) {
          const r1 = newRPMs[startComp.id];
          const r2 = newRPMs[endComp.id];
          
          if (r1 !== undefined && r2 === undefined) {
             const t1 = startComp.teeth || startComp.diameter || 20;
             const t2 = endComp.teeth || endComp.diameter || 20;
             newRPMs[endComp.id] = r1 * (t1 / t2);
             changed = true;
          } else if (r2 !== undefined && r1 === undefined) {
             const t1 = startComp.teeth || startComp.diameter || 20;
             const t2 = endComp.teeth || endComp.diameter || 20;
             newRPMs[startComp.id] = r2 * (t2 / t1);
             changed = true;
          }
        }
      });
    }
    
    setMotorRPMs(newRPMs);
    setIsSimulating(true);
    
    let time = 0;
    const simulate = () => {
      time += 0.016;
      setSimulationTime(time);
      if (isSimulating) simRef.current = requestAnimationFrame(simulate);
    };
    simRef.current = requestAnimationFrame(simulate);
  };

  const stopSimulation = () => {
    setIsSimulating(false);
    setMotorRPMs({});
    cancelAnimationFrame(simRef.current);
  };

  const clearAll = () => {
    setComponents([]);
    setWires([]);
    setChainConnections([]);
    setSelectedComponent(null);
    setCircuitErrors([]);
    stopSimulation();
  };

  const selectedCompData = components.find(c => c.id === selectedComponent);
  const allComponents = [...ELECTRICAL_COMPONENTS, ...MOTOR_COMPONENTS, ...MOTOR_PARTS, ...MECHANICAL_COMPONENTS];
  const filteredComponents = selectedCategory === 'electrical' ? ELECTRICAL_COMPONENTS 
    : selectedCategory === 'motors' ? MOTOR_COMPONENTS 
    : selectedCategory === 'motor_parts' ? MOTOR_PARTS
    : MECHANICAL_COMPONENTS;
  const filteredTools = TOOLS.filter(t => 
    t.category === 'all' || t.category === selectedCategory
  );

  return (
    <div className="flex h-full">
      {/* Component Palette */}
      <div className="w-48 bg-forge-surface border-r border-forge-border flex flex-col overflow-hidden">
        {/* Category Tabs */}
        <div className="flex border-b border-forge-border">
          {[
            { id: 'electrical', label: '⚡ ELEC', color: '#00eeff' },
            { id: 'motors', label: '⚙ MOTO', color: '#00ff88' },
            { id: 'motor_parts', label: '🔩 PARTS', color: '#ff3366' },
            { id: 'mechanical', label: '⚙ MECH', color: '#ffa500' },
          ].map(cat => (
            <button
              key={cat.id}
              onClick={() => setSelectedCategory(cat.id as ComponentCategory)}
              className={`flex-1 py-2 text-[7px] font-bold tracking-wider text-center transition-all ${
                selectedCategory === cat.id
                  ? 'text-white border-b-2'
                  : 'text-forge-text-muted hover:text-forge-text'
              }`}
              style={{ 
                borderBottomColor: selectedCategory === cat.id ? cat.color : 'transparent',
                backgroundColor: selectedCategory === cat.id ? `${cat.color}15` : 'transparent',
              }}
            >
              {cat.label}
            </button>
          ))}
        </div>

        <div className="flex-1 overflow-y-auto p-2 space-y-2">
          {/* Tools */}
          <div className="text-[8px] font-bold text-forge-text-muted tracking-wider mb-1 px-1">TOOLS</div>
          <div className="grid grid-cols-5 gap-1">
            {filteredTools.map(tool => (
              <button
                key={tool.id}
                onClick={() => setSelectedTool(tool.id)}
                className={`p-1.5 rounded text-center transition-all ${
                  selectedTool === tool.id
                    ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/40'
                    : 'bg-forge-panel/50 text-forge-text-dim hover:text-forge-text border border-transparent'
                }`}
                title={`${tool.name} (${tool.shortcut})`}
              >
                <div className="text-sm">{tool.icon}</div>
              </button>
            ))}
          </div>

          {/* Components */}
          <div className="text-[8px] font-bold text-forge-text-muted tracking-wider mb-1 px-1 mt-3">
            {selectedCategory.toUpperCase()} COMPONENTS
          </div>
          <div className="grid grid-cols-2 gap-1">
            {filteredComponents.map(comp => (
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
                <div className="text-[7px] text-forge-text-dim truncate">{comp.name}</div>
              </div>
            ))}
          </div>

          {/* Simulation Controls */}
          <div className="mt-4 space-y-1">
            <button
              onClick={isSimulating ? stopSimulation : runSimulation}
              className={`w-full px-3 py-2 rounded text-[9px] font-bold tracking-wider transition-all ${
                isSimulating
                  ? 'bg-forge-danger/20 text-forge-danger border border-forge-danger/30'
                  : 'bg-forge-green/20 text-forge-green border border-forge-green/30 hover:bg-forge-green/30'
              }`}
            >
              {isSimulating ? '⏹ STOP' : '▶ START'}
            </button>
            <button
              onClick={clearAll}
              className="w-full px-3 py-1.5 rounded text-[9px] font-semibold bg-forge-panel/50 text-forge-text-dim border border-transparent hover:text-forge-danger hover:border-forge-danger/30 transition-all"
            >
              🗑 CLEAR
            </button>
          </div>

          {/* Motor Presets */}
          {(selectedCategory === 'motors' || selectedCategory === 'motor_parts') && (
            <div className="mt-4 glass-panel rounded-lg p-2">
              <div className="text-[8px] font-bold text-forge-text-muted tracking-wider mb-2">MOTOR PRESETS</div>
              <div className="space-y-1">
                {[
                  { name: 'Conveyor', rpm: 100, teeth: 20, ratio: '1:2' },
                  { name: 'Hoist', rpm: 500, teeth: 10, ratio: '2:1' },
                  { name: 'Wheel Drive', rpm: 200, teeth: 30, ratio: '1:3' },
                ].map(preset => (
                  <button
                    key={preset.name}
                    onClick={() => {
                      const sprocket = createComponent('sprocket', 200, 150);
                      sprocket.teeth = preset.teeth;
                      const motor = createComponent('dc_motor', 200, 150);
                      motor.rpm = preset.rpm;
                      const vcc = createComponent('vcc', 200, 150);
                      const gnd = createComponent('gnd', 200, 150);
                      setComponents(prev => [...prev, sprocket, motor, vcc, gnd]);
                    }}
                    className="w-full px-2 py-1 rounded text-left bg-forge-panel/50 hover:bg-forge-panel text-[8px] text-forge-text-dim transition-all"
                  >
                    <div className="font-semibold">{preset.name}</div>
                    <div className="text-forge-text-muted">{preset.rpm}RPM | {preset.ratio}</div>
                  </button>
                ))}
                
                <button
                  onClick={() => {
                    const cx = 250, cy = 150;
                    const stator = createComponent('stator', cx, cy);
                    const rotor = createComponent('rotor', cx, cy);
                    const coil1 = createComponent('coil', cx, cy - 30);
                    const coil2 = createComponent('coil', cx, cy + 30);
                    coil1.rotation = 90; coil2.rotation = 90;
                    const magnet1 = createComponent('magnet', cx, cy - 15);
                    const magnet2 = createComponent('magnet', cx, cy + 15);
                    const bearing = createComponent('bearing', cx, cy);
                    const vcc = createComponent('vcc', cx - 100, cy);
                    const gnd = createComponent('gnd', cx - 100, cy + 30);
                    setComponents(prev => [...prev, stator, rotor, coil1, coil2, magnet1, magnet2, bearing, vcc, gnd]);
                  }}
                  className="w-full px-2 py-1 rounded text-left bg-forge-accent/20 hover:bg-forge-accent/40 text-[8px] text-forge-text-dim transition-all border border-forge-accent/30 mt-2"
                >
                  <div className="font-semibold text-forge-accent">Custom Motor Assembly</div>
                  <div className="text-forge-text-muted">Stator, Rotor, Coils & Magnets</div>
                </button>
              </div>
            </div>
          )}
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
          onDragOver={(e) => e.preventDefault()}
        />

        {/* Status Bar */}
        <div className="h-10 bg-forge-surface border-t border-forge-border flex items-center px-3 gap-4">
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">COMPONENTS:</span>
            <span className="text-forge-electrical">{components.length}</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">CHAINS:</span>
            <span className="text-forge-text-muted">{chainConnections.length}</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">MOTORS:</span>
            <span className="text-forge-green">{components.filter(c => ['dc_motor', 'stepper_motor', 'servo_motor'].includes(c.type)).length}</span>
          </div>
          {isSimulating && (
            <>
              <div className="w-px h-4 bg-forge-border" />
              <div className="flex items-center gap-2 text-[9px] text-green-400">
                <span className="font-semibold">● LIVE</span>
                <span className="font-mono">{simulationTime.toFixed(1)}s</span>
              </div>
            </>
          )}
          {circuitErrors.map((err, i) => (
            <span key={i} className="text-[9px] text-forge-warning">⚠ {err}</span>
          ))}
          <div className="flex-1" />
          <div className="text-[8px] text-forge-text-muted">
            {selectedTool === 'chain' ? 'Click sprockets to connect with chain' : 
             selectedTool === 'wire' ? 'Click to draw wire connections' :
             'Drag components • Chain tool links sprockets'}
          </div>
        </div>
      </div>

      {/* Properties Panel */}
      <div className="w-56 bg-forge-surface border-l border-forge-border p-3 overflow-y-auto">
        <div className="text-[9px] font-bold text-forge-electrical tracking-wider mb-3">PROPERTIES</div>

        {selectedCompData ? (
          <div className="space-y-3">
            <div className="glass-panel rounded-lg p-3">
              <div className="flex items-center justify-between mb-3">
                <span className="text-[10px] font-bold text-forge-text">{selectedCompData.type.toUpperCase()}</span>
                <span className="text-[8px] text-forge-text-muted">{selectedCompData.type}</span>
              </div>

              <div className="space-y-2 text-[9px]">
                <div>
                  <label className="text-[8px] text-forge-text-muted block mb-1">LABEL</label>
                  <input
                    type="text"
                    value={selectedCompData.label}
                    onChange={(e) => setComponents(prev => prev.map(c => 
                      c.id === selectedCompData.id ? { ...c, label: e.target.value } : c
                    ))}
                    className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-text focus:border-forge-electrical outline-none"
                  />
                </div>

                <div>
                  <label className="text-[8px] text-forge-text-muted block mb-1">VALUE</label>
                  <input
                    type="text"
                    value={selectedCompData.value}
                    onChange={(e) => setComponents(prev => prev.map(c => 
                      c.id === selectedCompData.id ? { ...c, value: e.target.value } : c
                    ))}
                    className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none"
                  />
                </div>

                {/* Motor-specific properties */}
                {['dc_motor', 'stepper_motor', 'servo_motor'].includes(selectedCompData.type) && (
                  <>
                    <div>
                      <label className="text-[8px] text-forge-text-muted block mb-1">RPM</label>
                      <input
                        type="number"
                        value={selectedCompData.rpm || 0}
                        onChange={(e) => setComponents(prev => prev.map(c => 
                          c.id === selectedCompData.id ? { ...c, rpm: parseInt(e.target.value) || 0 } : c
                        ))}
                        className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-green font-mono focus:border-forge-electrical outline-none"
                      />
                    </div>
                    <div>
                      <label className="text-[8px] text-forge-text-muted block mb-1">TORQUE (Nm)</label>
                      <input
                        type="number"
                        step="0.1"
                        value={selectedCompData.torque || 0}
                        onChange={(e) => setComponents(prev => prev.map(c => 
                          c.id === selectedCompData.id ? { ...c, torque: parseFloat(e.target.value) || 0 } : c
                        ))}
                        className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-green font-mono focus:border-forge-electrical outline-none"
                      />
                    </div>
                    <div>
                      <label className="text-[8px] text-forge-text-muted block mb-1">DIRECTION</label>
                      <div className="flex gap-1">
                        {['cw', 'ccw'].map(dir => (
                          <button
                            key={dir}
                            onClick={() => setComponents(prev => prev.map(c => 
                              c.id === selectedCompData.id ? { ...c, direction: dir as 'cw' | 'ccw' } : c
                            ))}
                            className={`flex-1 py-1 rounded text-[8px] font-semibold transition-all ${
                              selectedCompData.direction === dir
                                ? 'bg-forge-green/30 text-forge-green border border-forge-green/50'
                                : 'bg-forge-panel/50 text-forge-text-dim border border-transparent'
                            }`}
                          >
                            {dir === 'cw' ? '↻ CW' : '↺ CCW'}
                          </button>
                        ))}
                      </div>
                    </div>
                  </>
                )}

                {/* Motor Part Properties */}
                {['stator', 'rotor', 'coil', 'magnet', 'bearing'].includes(selectedCompData.type) && (
                  <>
                    {['stator', 'rotor', 'coil', 'magnet'].includes(selectedCompData.type) && (
                      <div>
                        <label className="text-[8px] text-forge-text-muted block mb-1">MATERIAL (Synthesized)</label>
                        <select
                          value={selectedCompData.material || ''}
                          onChange={(e) => setComponents(prev => prev.map(c => 
                            c.id === selectedCompData.id ? { ...c, material: e.target.value } : c
                          ))}
                          className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none appearance-none"
                        >
                          <option value="">Select Material...</option>
                          <option value="Copper">Standard Copper</option>
                          <option value="Neodymium">Standard Neodymium</option>
                          <option value="Silicon Steel">Standard Silicon Steel</option>
                          {customMaterials.map(mat => (
                            <option key={mat.id || mat.name} value={mat.name}>
                              {mat.name} (R:{mat.restitution} D:{mat.density})
                            </option>
                          ))}
                        </select>
                      </div>
                    )}
                    {selectedCompData.type === 'coil' && (
                      <div>
                        <label className="text-[8px] text-forge-text-muted block mb-1">TURNS</label>
                        <input
                          type="number"
                          value={selectedCompData.turns || 0}
                          onChange={(e) => setComponents(prev => prev.map(c => 
                            c.id === selectedCompData.id ? { ...c, turns: parseInt(e.target.value) || 0 } : c
                          ))}
                          className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none"
                        />
                      </div>
                    )}
                    {selectedCompData.type === 'magnet' && (
                      <div>
                        <label className="text-[8px] text-forge-text-muted block mb-1">STRENGTH (T)</label>
                        <input
                          type="number"
                          step="0.1"
                          value={selectedCompData.magneticStrength || 0}
                          onChange={(e) => setComponents(prev => prev.map(c => 
                            c.id === selectedCompData.id ? { ...c, magneticStrength: parseFloat(e.target.value) || 0 } : c
                          ))}
                          className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none"
                        />
                      </div>
                    )}
                    {selectedCompData.type === 'bearing' && (
                      <div>
                        <label className="text-[8px] text-forge-text-muted block mb-1">LUBRICANT (Synthesized)</label>
                        <select
                          value={selectedCompData.lubricant || ''}
                          onChange={(e) => setComponents(prev => prev.map(c => 
                            c.id === selectedCompData.id ? { ...c, lubricant: e.target.value } : c
                          ))}
                          className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none appearance-none"
                        >
                          <option value="">Select Lubricant...</option>
                          <option value="Synthetic Oil">Standard Synthetic Oil</option>
                          <option value="Lithium Grease">Standard Lithium Grease</option>
                          {customMaterials.map(mat => (
                            <option key={mat.id || mat.name} value={mat.name}>
                              {mat.name} (Synthesized)
                            </option>
                          ))}
                        </select>
                      </div>
                    )}
                  </>
                )}

                {/* Sprocket-specific properties */}
                {['sprocket', 'gear', 'pulley'].includes(selectedCompData.type) && (
                  <>
                    <div>
                      <label className="text-[8px] text-forge-text-muted block mb-1">
                        {selectedCompData.type === 'pulley' ? 'DIAMETER (mm)' : 'TEETH'}
                      </label>
                      <input
                        type="number"
                        value={selectedCompData.type === 'pulley' ? (selectedCompData.diameter || 50) : (selectedCompData.teeth || 20)}
                        onChange={(e) => setComponents(prev => prev.map(c => 
                          c.id === selectedCompData.id 
                            ? selectedCompData.type === 'pulley' 
                              ? { ...c, diameter: parseInt(e.target.value) || 50 }
                              : { ...c, teeth: parseInt(e.target.value) || 20 }
                            : c
                        ))}
                        className="w-full bg-forge-panel/50 border border-forge-border rounded px-2 py-1 text-[10px] text-forge-electrical font-mono focus:border-forge-electrical outline-none"
                      />
                    </div>
                    {motorRPMs[selectedCompData.id] !== undefined && (
                      <div className="flex justify-between text-[9px]">
                        <span className="text-forge-text-muted">Current RPM</span>
                        <span className="text-forge-green font-mono">{motorRPMs[selectedCompData.id]?.toFixed(0) || 0}</span>
                      </div>
                    )}
                  </>
                )}

                {/* Rotation */}
                <div>
                  <label className="text-[8px] text-forge-text-muted block mb-1">ROTATION</label>
                  <div className="flex gap-1">
                    {[0, 45, 90, 135, 180, 225, 270, 315].map(rot => (
                      <button
                        key={rot}
                        onClick={() => setComponents(prev => prev.map(c => 
                          c.id === selectedCompData.id ? { ...c, rotation: rot } : c
                        ))}
                        className={`flex-1 py-1 rounded text-[7px] font-semibold transition-all ${
                          selectedCompData.rotation === rot
                            ? 'bg-forge-accent/30 text-forge-accent border border-forge-accent/50'
                            : 'bg-forge-panel/50 text-forge-text-dim border border-transparent'
                        }`}
                      >
                        {rot}°
                      </button>
                    ))}
                  </div>
                </div>
              </div>
            </div>

            {/* Chain connections info */}
            {['sprocket', 'gear', 'pulley'].includes(selectedCompData.type) && (
              <div className="glass-panel rounded-lg p-3">
                <div className="text-[8px] font-bold text-forge-text-muted mb-2">CHAIN CONNECTIONS</div>
                {chainConnections.filter(ch => 
                  ch.startSprocketId === selectedCompData.id || ch.endSprocketId === selectedCompData.id
                ).length > 0 ? (
                  <div className="space-y-1">
                    {chainConnections.filter(ch => 
                      ch.startSprocketId === selectedCompData.id || ch.endSprocketId === selectedCompData.id
                    ).map(ch => {
                      const otherId = ch.startSprocketId === selectedCompData.id ? ch.endSprocketId : ch.startSprocketId;
                      const other = components.find(c => c.id === otherId);
                      return (
                        <div key={ch.id} className="flex items-center justify-between text-[8px] text-forge-text-dim">
                          <span>→ {other?.label || 'Unknown'}</span>
                          <button
                            onClick={() => setChainConnections(prev => prev.filter(c => c.id !== ch.id))}
                            className="text-forge-danger hover:text-forge-danger/70"
                          >
                            ✕
                          </button>
                        </div>
                      );
                    })}
                  </div>
                ) : (
                  <div className="text-[8px] text-forge-text-muted">Use Chain tool to connect</div>
                )}
              </div>
            )}

            <button
              onClick={() => {
                setComponents(prev => prev.filter(c => c.id !== selectedCompData.id));
                setChainConnections(prev => prev.filter(ch => 
                  ch.startSprocketId !== selectedCompData.id && ch.endSprocketId !== selectedCompData.id
                ));
                setSelectedComponent(null);
              }}
              className="w-full py-2 rounded bg-forge-danger/20 text-forge-danger text-[9px] font-bold hover:bg-forge-danger/30 transition-all"
            >
              ✕ DELETE COMPONENT
            </button>
          </div>
        ) : (
          <div className="text-center py-8">
            <div className="text-3xl mb-2 opacity-30">⚙</div>
            <div className="text-[9px] text-forge-text-muted">Select a component to edit</div>
          </div>
        )}

        {/* Mechanical Assembly Info */}
        <div className="mt-4 glass-panel rounded-lg p-3">
          <div className="text-[8px] font-bold text-forge-text-muted mb-2">MECHANICAL ASSEMBLY</div>
          <div className="space-y-2 text-[9px]">
            <div className="flex justify-between">
              <span className="text-forge-text-dim">Sprockets</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type === 'sprocket').length}
              </span>
            </div>
            <div className="flex justify-between">
              <span className="text-forge-text-dim">Gears</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type === 'gear').length}
              </span>
            </div>
            <div className="flex justify-between">
              <span className="text-forge-text-dim">Pulleys</span>
              <span className="text-forge-electrical font-mono">
                {components.filter(c => c.type === 'pulley').length}
              </span>
            </div>
            <div className="flex justify-between">
              <span className="text-forge-text-dim">Active Motors</span>
              <span className="text-forge-green font-mono">
                {Object.values(motorRPMs).filter(rpm => rpm > 0).length}
              </span>
            </div>
          </div>
        </div>

        {/* Quick Reference */}
        <div className="mt-4 glass-panel rounded-lg p-3">
          <div className="text-[8px] font-bold text-forge-text-muted mb-2">QUICK REFERENCE</div>
          <div className="space-y-1 text-[7px] text-forge-text-dim">
            <div>• Drag motors/sprockets from palette</div>
            <div>• Chain tool (C) links sprockets</div>
            <div>• Wire tool (W) for electrical</div>
            <div>• Motors power linked sprockets</div>
            <div>• Gear ratio affects chain speed</div>
          </div>
        </div>
      </div>
    </div>
  );
}
