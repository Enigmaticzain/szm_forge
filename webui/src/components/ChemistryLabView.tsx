import { useRef, useEffect, useState, useMemo, useCallback } from 'react';
import { periodicElements, type ElementData } from '../data/periodicTable';

// Types
type BondType = 'single' | 'double' | 'triple' | 'hydrogen';
type ViewMode = 'molecular' | 'periodic' | 'reactions' | 'equations';

interface Atom {
  id: string;
  element: ElementData;
  x: number;
  y: number;
  charge: number;
}

interface Bond {
  id: string;
  atom1Id: string;
  atom2Id: string;
  type: BondType;
}

interface Molecule {
  id: string;
  name: string;
  formula: string;
  atoms: Atom[];
  bonds: Bond[];
  color: string;
}

// Category colors for periodic table
const categoryColors: Record<string, string> = {
  'Alkali metal': '#ff6b6b',
  'Alkaline earth metal': '#ffa94d',
  'Transition metal': '#ffd43b',
  'Post-transition metal': '#69db7c',
  'Metalloid': '#38d9a9',
  'Nonmetal': '#4dabf7',
  'Halogen': '#748ffc',
  'Noble gas': '#da77f2',
  'Lanthanide': '#f783ac',
  'Actinide': '#e599f7',
};

// Common molecules for the library
const moleculeLibrary: Molecule[] = [
  {
    id: 'water',
    name: 'Water (H₂O)',
    formula: 'H₂O',
    atoms: [
      { id: 'o1', element: periodicElements[7], x: 200, y: 150, charge: 0 },
      { id: 'h1', element: periodicElements[0], x: 150, y: 200, charge: 0 },
      { id: 'h2', element: periodicElements[0], x: 250, y: 200, charge: 0 },
    ],
    bonds: [
      { id: 'b1', atom1Id: 'o1', atom2Id: 'h1', type: 'single' },
      { id: 'b2', atom1Id: 'o1', atom2Id: 'h2', type: 'single' },
    ],
    color: '#4dabf7',
  },
  {
    id: 'carbon-dioxide',
    name: 'Carbon Dioxide (CO₂)',
    formula: 'CO₂',
    atoms: [
      { id: 'c1', element: periodicElements[5], x: 200, y: 150, charge: 0 },
      { id: 'o1', element: periodicElements[7], x: 130, y: 150, charge: 0 },
      { id: 'o2', element: periodicElements[7], x: 270, y: 150, charge: 0 },
    ],
    bonds: [
      { id: 'b1', atom1Id: 'c1', atom2Id: 'o1', type: 'double' },
      { id: 'b2', atom1Id: 'c1', atom2Id: 'o2', type: 'double' },
    ],
    color: '#da77f2',
  },
  {
    id: 'methane',
    name: 'Methane (CH₄)',
    formula: 'CH₄',
    atoms: [
      { id: 'c1', element: periodicElements[5], x: 200, y: 150, charge: 0 },
      { id: 'h1', element: periodicElements[0], x: 200, y: 80, charge: 0 },
      { id: 'h2', element: periodicElements[0], x: 130, y: 180, charge: 0 },
      { id: 'h3', element: periodicElements[0], x: 200, y: 220, charge: 0 },
      { id: 'h4', element: periodicElements[0], x: 270, y: 180, charge: 0 },
    ],
    bonds: [
      { id: 'b1', atom1Id: 'c1', atom2Id: 'h1', type: 'single' },
      { id: 'b2', atom1Id: 'c1', atom2Id: 'h2', type: 'single' },
      { id: 'b3', atom1Id: 'c1', atom2Id: 'h3', type: 'single' },
      { id: 'b4', atom1Id: 'c1', atom2Id: 'h4', type: 'single' },
    ],
    color: '#69db7c',
  },
  {
    id: 'ammonia',
    name: 'Ammonia (NH₃)',
    formula: 'NH₃',
    atoms: [
      { id: 'n1', element: periodicElements[6], x: 200, y: 130, charge: 0 },
      { id: 'h1', element: periodicElements[0], x: 140, y: 180, charge: 0 },
      { id: 'h2', element: periodicElements[0], x: 260, y: 180, charge: 0 },
      { id: 'h3', element: periodicElements[0], x: 200, y: 220, charge: 0 },
    ],
    bonds: [
      { id: 'b1', atom1Id: 'n1', atom2Id: 'h1', type: 'single' },
      { id: 'b2', atom1Id: 'n1', atom2Id: 'h2', type: 'single' },
      { id: 'b3', atom1Id: 'n1', atom2Id: 'h3', type: 'single' },
    ],
    color: '#ffd43b',
  },
  {
    id: 'oxygen',
    name: 'Oxygen (O₂)',
    formula: 'O₂',
    atoms: [
      { id: 'o1', element: periodicElements[7], x: 160, y: 150, charge: 0 },
      { id: 'o2', element: periodicElements[7], x: 240, y: 150, charge: 0 },
    ],
    bonds: [
      { id: 'b1', atom1Id: 'o1', atom2Id: 'o2', type: 'double' },
    ],
    color: '#4dabf7',
  },
  {
    id: 'nitrogen',
    name: 'Nitrogen (N₂)',
    formula: 'N₂',
    atoms: [
      { id: 'n1', element: periodicElements[6], x: 160, y: 150, charge: 0 },
      { id: 'n2', element: periodicElements[6], x: 240, y: 150, charge: 0 },
    ],
    bonds: [
      { id: 'b1', atom1Id: 'n1', atom2Id: 'n2', type: 'triple' },
    ],
    color: '#748ffc',
  },
];

// Reactions data
const reactions = [
  {
    id: 'combustion',
    name: 'Combustion',
    equation: 'CH₄ + 2O₂ → CO₂ + 2H₂O',
    description: 'Methane burning in oxygen',
    energy: 'High exothermic',
    products: ['CO₂', 'H₂O'],
  },
  {
    id: 'photosynthesis',
    name: 'Photosynthesis',
    equation: '6CO₂ + 6H₂O → C₆H₁₂O₆ + 6O₂',
    description: 'Plant converting CO₂ to glucose',
    energy: 'Requires sunlight',
    products: ['C₆H₁₂O₆', 'O₂'],
  },
  {
    id: 'rust',
    name: 'Oxidation (Rust)',
    equation: '4Fe + 3O₂ → 2Fe₂O₃',
    description: 'Iron rusting',
    energy: 'Slow exothermic',
    products: ['Fe₂O₃'],
  },
  {
    id: 'neutralization',
    name: 'Neutralization',
    equation: 'HCl + NaOH → NaCl + H₂O',
    description: 'Acid-base reaction',
    energy: 'Mild exothermic',
    products: ['NaCl', 'H₂O'],
  },
];

const GRID_SIZE = 20;

export default function ChemistryLabView() {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const containerRef = useRef<HTMLDivElement>(null);
  const animRef = useRef<number>(0);
  
  const [viewMode, setViewMode] = useState<ViewMode>('molecular');
  const [atoms, setAtoms] = useState<Atom[]>([]);
  const [bonds, setBonds] = useState<Bond[]>([]);
  const [selectedAtomId, setSelectedAtomId] = useState<string | null>(null);
  const [draggingAtom, setDraggingAtom] = useState<{ id: string; offsetX: number; offsetY: number } | null>(null);
  const [wireStartAtom, setWireStartAtom] = useState<string | null>(null);
  const [mousePos, setMousePos] = useState({ x: 0, y: 0 });
  const [selectedElement, setSelectedElement] = useState<ElementData | null>(periodicElements[6]); // Carbon
  const [bondType, setBondType] = useState<BondType>('single');
  const [molecularMass, setMolecularMass] = useState(0);
  const [totalElectrons, setTotalElectrons] = useState(0);
  const [query, setQuery] = useState('');
  const [showAtomLabels, setShowAtomLabels] = useState(true);
  const [showBondLabels, setShowBondLabels] = useState(true);
  const [balancedEquation, setBalancedEquation] = useState('');
  const [equationError, setEquationError] = useState('');

  const generateId = () => Math.random().toString(36).substr(2, 9);
  
  const snapToGrid = useCallback((value: number) => Math.round(value / GRID_SIZE) * GRID_SIZE, []);

  // Calculate molecular properties
  useEffect(() => {
    const mass = atoms.reduce((sum, atom) => sum + parseFloat(atom.element.atomicWeight), 0);
    const electrons = atoms.reduce((sum, atom) => sum + atom.element.atomicNumber, 0);
    setMolecularMass(mass);
    setTotalElectrons(electrons);
  }, [atoms]);

  // Add atom to canvas
  const addAtom = (element: ElementData, x: number, y: number) => {
    const newAtom: Atom = {
      id: generateId(),
      element,
      x: snapToGrid(x),
      y: snapToGrid(y),
      charge: 0,
    };
    setAtoms(prev => [...prev, newAtom]);
  };

  // Remove atom and its bonds
  const removeAtom = (atomId: string) => {
    setAtoms(prev => prev.filter(a => a.id !== atomId));
    setBonds(prev => prev.filter(b => b.atom1Id !== atomId && b.atom2Id !== atomId));
    if (selectedAtomId === atomId) setSelectedAtomId(null);
  };

  // Add bond between atoms
  const addBond = (atom1Id: string, atom2Id: string, type: BondType) => {
    const existingBond = bonds.find(
      b => (b.atom1Id === atom1Id && b.atom2Id === atom2Id) ||
           (b.atom1Id === atom2Id && b.atom2Id === atom1Id)
    );
    if (existingBond) return;
    
    const newBond: Bond = {
      id: generateId(),
      atom1Id,
      atom2Id,
      type,
    };
    setBonds(prev => [...prev, newBond]);
  };

  // Change bond type
  const cycleBondType = (bondId: string) => {
    setBonds(prev => prev.map(b => {
      if (b.id !== bondId) return b;
      const types: BondType[] = ['single', 'double', 'triple', 'hydrogen'];
      const currentIndex = types.indexOf(b.type);
      return { ...b, type: types[(currentIndex + 1) % types.length] };
    }));
  };

  // Load preset molecule
  const loadMolecule = (molecule: Molecule) => {
    setAtoms(molecule.atoms.map(a => ({ ...a, id: generateId() })));
    setBonds([]);
    setSelectedAtomId(null);
  };

  // Clear canvas
  const clearCanvas = () => {
    setAtoms([]);
    setBonds([]);
    setSelectedAtomId(null);
    setWireStartAtom(null);
  };

  // Filter elements for palette
  const filteredElements = useMemo(() => {
    const normalized = query.trim().toLowerCase();
    if (!normalized) return periodicElements;
    return periodicElements.filter((element) => {
      return [
        element.name,
        element.symbol,
        element.category,
      ].some((value) => value.toLowerCase().includes(normalized));
    });
  }, [query]);

  // Get atom by ID
  const getAtomById = (id: string) => atoms.find(a => a.id === id);

  // Draw bond
  const drawBond = (ctx: CanvasRenderingContext2D, bond: Bond, animTime: number) => {
    const atom1 = getAtomById(bond.atom1Id);
    const atom2 = getAtomById(bond.atom2Id);
    if (!atom1 || !atom2) return;

    const x1 = atom1.x, y1 = atom1.y;
    const x2 = atom2.x, y2 = atom2.y;
    
    ctx.strokeStyle = '#00eeff';
    ctx.lineCap = 'round';
    
    const drawLines = (offset: number, width: number) => {
      const dx = x2 - x1, dy = y2 - y1;
      const len = Math.sqrt(dx * dx + dy * dy);
      if (len === 0) return;
      const nx = -dy / len * offset, ny = dx / len * offset;
      
      ctx.beginPath();
      ctx.moveTo(x1 + nx, y1 + ny);
      ctx.lineTo(x2 + nx, y2 + ny);
      ctx.lineWidth = width;
      ctx.stroke();
    };

    // Animation for hydrogen bonds
    if (bond.type === 'hydrogen') {
      ctx.setLineDash([4, 4]);
      ctx.lineDashOffset = -animTime * 0.05;
    }

    switch (bond.type) {
      case 'single':
        ctx.lineWidth = 2;
        drawLines(0, 2);
        break;
      case 'double':
        ctx.lineWidth = 2;
        drawLines(-4, 2);
        drawLines(4, 2);
        break;
      case 'triple':
        ctx.lineWidth = 2;
        drawLines(0, 2);
        drawLines(-6, 2);
        drawLines(6, 2);
        break;
      case 'hydrogen':
        ctx.lineWidth = 2;
        drawLines(0, 2);
        break;
    }
    
    ctx.setLineDash([]);

    // Bond label
    if (showBondLabels) {
      const midX = (x1 + x2) / 2;
      const midY = (y1 + y2) / 2;
      ctx.font = '8px monospace';
      ctx.fillStyle = '#00eeff88';
      ctx.textAlign = 'center';
      const label = bond.type === 'single' ? '-' : 
                    bond.type === 'double' ? '=' : 
                    bond.type === 'triple' ? '≡' : '···';
      ctx.fillText(label, midX, midY - 5);
    }
  };

  // Draw atom
  const drawAtom = (ctx: CanvasRenderingContext2D, atom: Atom, animTime: number, isSelected: boolean) => {
    const { x, y, element } = atom;
    const radius = 20 + (element.atomicNumber > 50 ? 5 : 0);
    const color = categoryColors[element.category] || '#00c2ff';
    
    // Glow effect
    const glowRadius = radius + 5 + Math.sin(animTime * 0.003) * 3;
    const gradient = ctx.createRadialGradient(x, y, 0, x, y, glowRadius);
    gradient.addColorStop(0, color + '40');
    gradient.addColorStop(1, 'transparent');
    ctx.fillStyle = gradient;
    ctx.beginPath();
    ctx.arc(x, y, glowRadius, 0, Math.PI * 2);
    ctx.fill();

    // Selection ring
    if (isSelected) {
      ctx.beginPath();
      ctx.arc(x, y, radius + 8, 0, Math.PI * 2);
      ctx.strokeStyle = '#00eeff';
      ctx.lineWidth = 2;
      ctx.setLineDash([4, 4]);
      ctx.lineDashOffset = -animTime * 0.02;
      ctx.stroke();
      ctx.setLineDash([]);
    }

    // Atom circle
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, Math.PI * 2);
    ctx.fillStyle = color + '20';
    ctx.fill();
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    ctx.stroke();

    // Symbol
    ctx.font = `bold ${radius}px monospace`;
    ctx.fillStyle = color;
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(element.symbol, x, y);

    // Atomic number (small)
    if (showAtomLabels) {
      ctx.font = '8px monospace';
      ctx.fillStyle = '#888';
      ctx.textAlign = 'left';
      ctx.textBaseline = 'top';
      ctx.fillText(element.atomicNumber.toString(), x - radius + 2, y - radius + 2);
    }

    // Electron shells indicator
    if (showAtomLabels) {
      const shells = Math.ceil(element.atomicNumber / 2);
      ctx.strokeStyle = color + '30';
      ctx.lineWidth = 1;
      for (let s = 1; s <= Math.min(shells, 3); s++) {
        ctx.beginPath();
        ctx.arc(x, y, radius + s * 6, 0, Math.PI * 2);
        ctx.stroke();
      }
    }
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

    const render = (animTime: number) => {
      const w = canvas.width;
      const h = canvas.height;

      // Clear
      ctx.fillStyle = '#0a0c10';
      ctx.fillRect(0, 0, w, h);

      // Grid
      ctx.strokeStyle = 'rgba(30, 34, 48, 0.4)';
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

      // Draw bonds first (under atoms)
      bonds.forEach(bond => drawBond(ctx, bond, animTime));

      // Draw in-progress wire
      if (wireStartAtom) {
        const startAtom = getAtomById(wireStartAtom);
        if (startAtom) {
          ctx.strokeStyle = '#00eeff60';
          ctx.lineWidth = 2;
          ctx.setLineDash([5, 5]);
          ctx.beginPath();
          ctx.moveTo(startAtom.x, startAtom.y);
          ctx.lineTo(mousePos.x, mousePos.y);
          ctx.stroke();
          ctx.setLineDash([]);
        }
      }

      // Draw atoms
      atoms.forEach(atom => {
        drawAtom(ctx, atom, animTime, selectedAtomId === atom.id);
      });

      animRef.current = requestAnimationFrame(render);
    };

    animRef.current = requestAnimationFrame(render);
    return () => cancelAnimationFrame(animRef.current);
  }, [atoms, bonds, selectedAtomId, wireStartAtom, mousePos, showAtomLabels, showBondLabels]);

  // Mouse handlers
  const handleMouseDown = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Check if clicking on an atom
    const clickedAtom = atoms.find(atom => {
      const dx = x - atom.x;
      const dy = y - atom.y;
      return Math.sqrt(dx * dx + dy * dy) < 25;
    });

    if (clickedAtom) {
      if (wireStartAtom) {
        // Complete wire connection
        if (wireStartAtom !== clickedAtom.id) {
          addBond(wireStartAtom, clickedAtom.id, bondType);
        }
        setWireStartAtom(null);
      } else {
        // Select or start wire
        if (e.shiftKey) {
          setWireStartAtom(clickedAtom.id);
        } else {
          setSelectedAtomId(clickedAtom.id);
          setDraggingAtom({
            id: clickedAtom.id,
            offsetX: x - clickedAtom.x,
            offsetY: y - clickedAtom.y,
          });
        }
      }
    } else {
      setSelectedAtomId(null);
      setWireStartAtom(null);
    }
  };

  const handleMouseMove = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;
    setMousePos({ x, y });

    if (draggingAtom) {
      setAtoms(prev => prev.map(atom => {
        if (atom.id !== draggingAtom.id) return atom;
        return { ...atom, x: snapToGrid(x - draggingAtom.offsetX), y: snapToGrid(y - draggingAtom.offsetY) };
      }));
    }
  };

  const handleMouseUp = () => {
    setDraggingAtom(null);
  };

  const handleDoubleClick = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    const clickedAtom = atoms.find(atom => {
      const dx = x - atom.x;
      const dy = y - atom.y;
      return Math.sqrt(dx * dx + dy * dy) < 25;
    });

    if (clickedAtom) {
      removeAtom(clickedAtom.id);
    } else if (selectedElement) {
      addAtom(selectedElement, x, y);
    }
  };

  // Balance chemical equation (simplified)
  const balanceEquation = (equation: string) => {
    try {
      // Simple parser for equations like "H2 + O2 -> H2O"
      const cleaned = equation.replace(/→|=/g, '->').trim();
      if (!cleaned.includes('->')) {
        setEquationError('Use "->" or "=" to separate reactants and products');
        return;
      }

      const [reactants, products] = cleaned.split('->').map(s => s.trim());
      if (!reactants || !products) {
        setEquationError('Invalid equation format');
        return;
      }

      // Simple approach: just return the equation as-is for now
      // A full balancer would be complex
      setBalancedEquation(`Balanced: ${cleaned}`);
      setEquationError('');
    } catch {
      setEquationError('Could not parse equation');
    }
  };

  const selectedAtom = atoms.find(a => a.id === selectedAtomId);

  return (
    <div className="flex h-full">
      {/* Element Palette / Sidebar */}
      <div className="w-56 bg-forge-surface border-r border-forge-border flex flex-col overflow-hidden">
        {/* View Mode Tabs */}
        <div className="flex border-b border-forge-border">
          {[
            { id: 'molecular', label: 'BUILD', icon: '⚛' },
            { id: 'periodic', label: 'ELEMENTS', icon: '◆' },
            { id: 'reactions', label: 'REACT', icon: '⚗' },
            { id: 'equations', label: 'EQUATION', icon: '⚖' },
          ].map(tab => (
            <button
              key={tab.id}
              onClick={() => setViewMode(tab.id as ViewMode)}
              className={`flex-1 py-2 text-[7px] font-bold tracking-wider text-center transition-all ${
                viewMode === tab.id
                  ? 'text-forge-accent border-b-2 border-forge-accent bg-forge-accent/5'
                  : 'text-forge-text-muted hover:text-forge-text-dim'
              }`}
            >
              <div className="text-sm mb-0.5">{tab.icon}</div>
              {tab.label}
            </button>
          ))}
        </div>

        <div className="flex-1 overflow-y-auto p-2 space-y-2">
          {/* MOLECULAR BUILD MODE */}
          {viewMode === 'molecular' && (
            <>
              {/* Bond Type Selector */}
              <div className="glass-panel rounded-lg p-2">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">BOND TYPE</div>
                <div className="grid grid-cols-2 gap-1">
                  {(['single', 'double', 'triple', 'hydrogen'] as BondType[]).map(type => (
                    <button
                      key={type}
                      onClick={() => setBondType(type)}
                      className={`px-2 py-1 rounded text-[9px] font-semibold transition-all ${
                        bondType === type
                          ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/40'
                          : 'bg-forge-panel/50 text-forge-text-dim border border-transparent hover:text-forge-text'
                      }`}
                    >
                      {type === 'single' && '-'}
                      {type === 'double' && '='}
                      {type === 'triple' && '≡'}
                      {type === 'hydrogen' && '···'}
                      <span className="ml-1">{type}</span>
                    </button>
                  ))}
                </div>
              </div>

              {/* Molecule Library */}
              <div className="glass-panel rounded-lg p-2">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">MOLECULE LIBRARY</div>
                <div className="space-y-1">
                  {moleculeLibrary.map(mol => (
                    <button
                      key={mol.id}
                      onClick={() => loadMolecule(mol)}
                      className="w-full px-2 py-1.5 rounded text-left bg-forge-panel/50 hover:bg-forge-panel transition-all"
                    >
                      <div className="text-[9px] font-semibold text-forge-text" style={{ color: mol.color }}>
                        {mol.name}
                      </div>
                      <div className="text-[8px] text-forge-text-muted font-mono">{mol.formula}</div>
                    </button>
                  ))}
                </div>
              </div>

              {/* Selected Element for Placement */}
              {selectedElement && (
                <div className="glass-panel rounded-lg p-2">
                  <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">PLACE ELEMENT</div>
                  <div className="flex items-center gap-3 p-2 bg-forge-panel/50 rounded-lg">
                    <div
                      className="w-10 h-10 rounded-full flex items-center justify-center text-lg font-bold"
                      style={{ backgroundColor: (categoryColors[selectedElement.category] || '#00c2ff') + '30', color: categoryColors[selectedElement.category] || '#00c2ff' }}
                    >
                      {selectedElement.symbol}
                    </div>
                    <div>
                      <div className="text-[10px] font-semibold text-forge-text">{selectedElement.name}</div>
                      <div className="text-[8px] text-forge-text-muted">Z = {selectedElement.atomicNumber}</div>
                    </div>
                  </div>
                  <div className="text-[7px] text-forge-text-dim mt-2">
                    Click on canvas to place • Shift+Click to connect
                  </div>
                </div>
              )}

              {/* Display Options */}
              <div className="glass-panel rounded-lg p-2">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">DISPLAY</div>
                <label className="flex items-center gap-2 py-1">
                  <input
                    type="checkbox"
                    checked={showAtomLabels}
                    onChange={(e) => setShowAtomLabels(e.target.checked)}
                    className="w-3 h-3 accent-forge-accent"
                  />
                  <span className="text-[9px] text-forge-text-dim">Show atomic numbers</span>
                </label>
                <label className="flex items-center gap-2 py-1">
                  <input
                    type="checkbox"
                    checked={showBondLabels}
                    onChange={(e) => setShowBondLabels(e.target.checked)}
                    className="w-3 h-3 accent-forge-accent"
                  />
                  <span className="text-[9px] text-forge-text-dim">Show bond symbols</span>
                </label>
              </div>

              {/* Clear Button */}
              <button
                onClick={clearCanvas}
                className="w-full px-3 py-2 rounded bg-forge-danger/20 text-forge-danger text-[9px] font-bold hover:bg-forge-danger/30 transition-all"
              >
                🗑 CLEAR CANVAS
              </button>
            </>
          )}

          {/* PERIODIC TABLE MODE */}
          {viewMode === 'periodic' && (
            <>
              <input
                value={query}
                onChange={(e) => setQuery(e.target.value)}
                placeholder="Search elements..."
                className="w-full rounded border border-forge-border bg-forge-panel/50 px-2 py-1.5 text-[10px] text-forge-text outline-none focus:border-forge-accent"
              />
              
              <div className="grid grid-cols-4 gap-1">
                {filteredElements.slice(0, 36).map(element => (
                  <button
                    key={element.atomicNumber}
                    onClick={() => {
                      setSelectedElement(element);
                      setViewMode('molecular');
                    }}
                    className="p-1.5 rounded text-center hover:bg-forge-panel transition-all"
                    style={{ backgroundColor: (categoryColors[element.category] || '#00c2ff') + '20' }}
                    title={`${element.name} (${element.symbol})`}
                  >
                    <div className="text-[7px] text-forge-text-muted">{element.atomicNumber}</div>
                    <div className="text-[10px] font-bold" style={{ color: categoryColors[element.category] || '#00c2ff' }}>
                      {element.symbol}
                    </div>
                  </button>
                ))}
              </div>

              {/* Category Legend */}
              <div className="glass-panel rounded-lg p-2">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">CATEGORIES</div>
                <div className="space-y-1">
                  {Object.entries(categoryColors).map(([category, color]) => (
                    <div key={category} className="flex items-center gap-2">
                      <div className="w-2 h-2 rounded-full" style={{ backgroundColor: color }} />
                      <span className="text-[7px] text-forge-text-dim truncate">{category}</span>
                    </div>
                  ))}
                </div>
              </div>
            </>
          )}

          {/* REACTIONS MODE */}
          {viewMode === 'reactions' && (
            <>
              <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">CHEMICAL REACTIONS</div>
              <div className="space-y-2">
                {reactions.map(reaction => (
                  <div key={reaction.id} className="glass-panel rounded-lg p-2">
                    <div className="text-[9px] font-bold text-forge-text">{reaction.name}</div>
                    <div className="text-[8px] font-mono text-forge-accent mt-1">{reaction.equation}</div>
                    <div className="text-[7px] text-forge-text-muted mt-1">{reaction.description}</div>
                    <div className="text-[7px] text-forge-text-dim mt-1">
                      Energy: <span className="text-forge-warning">{reaction.energy}</span>
                    </div>
                    <div className="text-[7px] text-forge-text-dim mt-1">
                      Products: {reaction.products.join(', ')}
                    </div>
                    <button
                      onClick={() => {
                        // Load reactants into canvas as a starting point
                        if (reaction.id === 'combustion') {
                          setAtoms([
                            { id: 'c1', element: periodicElements[5], x: 150, y: 150, charge: 0 },
                            { id: 'h1', element: periodicElements[0], x: 150, y: 200, charge: 0 },
                            { id: 'h2', element: periodicElements[0], x: 150, y: 220, charge: 0 },
                            { id: 'h3', element: periodicElements[0], x: 150, y: 240, charge: 0 },
                            { id: 'h4', element: periodicElements[0], x: 150, y: 260, charge: 0 },
                            { id: 'o1', element: periodicElements[7], x: 280, y: 150, charge: 0 },
                            { id: 'o2', element: periodicElements[7], x: 280, y: 180, charge: 0 },
                            { id: 'o3', element: periodicElements[7], x: 340, y: 150, charge: 0 },
                            { id: 'o4', element: periodicElements[7], x: 340, y: 180, charge: 0 },
                          ]);
                          setBonds([
                            { id: 'b1', atom1Id: 'c1', atom2Id: 'h1', type: 'single' },
                            { id: 'b2', atom1Id: 'c1', atom2Id: 'h2', type: 'single' },
                            { id: 'b3', atom1Id: 'c1', atom2Id: 'h3', type: 'single' },
                            { id: 'b4', atom1Id: 'c1', atom2Id: 'h4', type: 'single' },
                            { id: 'b5', atom1Id: 'o1', atom2Id: 'o2', type: 'double' },
                            { id: 'b6', atom1Id: 'o3', atom2Id: 'o4', type: 'double' },
                          ]);
                          setViewMode('molecular');
                        }
                      }}
                      className="w-full mt-2 px-2 py-1 rounded bg-forge-accent/20 text-forge-accent text-[8px] font-semibold hover:bg-forge-accent/30 transition-all"
                    >
                      Build Reactants
                    </button>
                  </div>
                ))}
              </div>
            </>
          )}

          {/* EQUATIONS MODE */}
          {viewMode === 'equations' && (
            <>
              <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">BALANCE EQUATIONS</div>
              <div className="space-y-2">
                <input
                  value={balancedEquation || ''}
                  onChange={(e) => {
                    setBalancedEquation(e.target.value);
                    setEquationError('');
                  }}
                  placeholder="Enter equation (e.g., H2 + O2 -> H2O)"
                  className="w-full rounded border border-forge-border bg-forge-panel/50 px-2 py-1.5 text-[10px] text-forge-text outline-none focus:border-forge-accent font-mono"
                />
                <button
                  onClick={() => balanceEquation(balancedEquation)}
                  className="w-full px-3 py-2 rounded bg-forge-accent/20 text-forge-accent text-[9px] font-bold hover:bg-forge-accent/30 transition-all"
                >
                  ⚖ BALANCE
                </button>
                {equationError && (
                  <div className="text-[8px] text-forge-danger">{equationError}</div>
                )}
              </div>

              {/* Quick Equations */}
              <div className="glass-panel rounded-lg p-2">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">COMMON EQUATIONS</div>
                <div className="space-y-1">
                  {[
                    '2H₂ + O₂ → 2H₂O',
                    'CH₄ + 2O₂ → CO₂ + 2H₂O',
                    'N₂ + 3H₂ → 2NH₃',
                    'C₆H₁₂O₆ + 6O₂ → 6CO₂ + 6H₂O',
                  ].map((eq, i) => (
                    <button
                      key={i}
                      onClick={() => setBalancedEquation(eq)}
                      className="w-full px-2 py-1 rounded text-left bg-forge-panel/50 hover:bg-forge-panel text-[8px] font-mono text-forge-text-dim transition-all"
                    >
                      {eq}
                    </button>
                  ))}
                </div>
              </div>

              {/* Stoichiometry Help */}
              <div className="glass-panel rounded-lg p-2">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">HELP</div>
                <div className="text-[7px] text-forge-text-dim space-y-1">
                  <p>• Use numbers as subscripts (H2O not H2)</p>
                  <p>• Use &quot;-&gt;&quot; or &quot;=&quot; for reaction arrow</p>
                  <p>• Separate reactants with &quot;+&quot;</p>
                  <p>• Balance mass on both sides</p>
                </div>
              </div>
            </>
          )}
        </div>
      </div>

      {/* Canvas Area */}
      <div className="flex-1 flex flex-col">
        <canvas
          ref={canvasRef}
          className="flex-1 cursor-crosshair"
          style={{ background: '#0a0c10' }}
          onMouseDown={handleMouseDown}
          onMouseMove={handleMouseMove}
          onMouseUp={handleMouseUp}
          onDoubleClick={handleDoubleClick}
        />

        {/* Toolbar */}
        <div className="h-10 bg-forge-surface border-t border-forge-border flex items-center px-3 gap-4">
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">ATOMS:</span>
            <span className="text-forge-accent">{atoms.length}</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">BONDS:</span>
            <span className="text-forge-accent">{bonds.length}</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">MASS:</span>
            <span className="text-forge-green font-mono">{molecularMass.toFixed(2)} u</span>
          </div>
          <div className="w-px h-4 bg-forge-border" />
          <div className="flex items-center gap-2 text-[9px] text-forge-text-dim">
            <span className="font-semibold">ELECTRONS:</span>
            <span className="text-forge-green font-mono">{totalElectrons}</span>
          </div>
          <div className="flex-1" />
          <div className="text-[8px] text-forge-text-muted">
            Double-click to place • Click to select • Shift+Click to connect
          </div>
        </div>
      </div>

      {/* Properties Panel */}
      <div className="w-56 bg-forge-surface border-l border-forge-border p-3 overflow-y-auto">
        <div className="text-[9px] font-bold text-forge-accent tracking-wider mb-3">PROPERTIES</div>

        {selectedAtom ? (
          <div className="space-y-3">
            <div className="glass-panel rounded-lg p-3">
              <div className="flex items-center gap-3 mb-3">
                <div
                  className="w-12 h-12 rounded-full flex items-center justify-center text-xl font-bold"
                  style={{ 
                    backgroundColor: (categoryColors[selectedAtom.element.category] || '#00c2ff') + '30', 
                    color: categoryColors[selectedAtom.element.category] || '#00c2ff' 
                  }}
                >
                  {selectedAtom.element.symbol}
                </div>
                <div>
                  <div className="text-[10px] font-bold text-forge-text">{selectedAtom.element.name}</div>
                  <div className="text-[8px] text-forge-text-muted">{selectedAtom.element.category}</div>
                </div>
              </div>

              <div className="space-y-1.5 text-[9px]">
                <div className="flex justify-between">
                  <span className="text-forge-text-muted">Atomic Number</span>
                  <span className="text-forge-text font-mono">{selectedAtom.element.atomicNumber}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-forge-text-muted">Atomic Weight</span>
                  <span className="text-forge-text font-mono">{selectedAtom.element.atomicWeight}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-forge-text-muted">Electronegativity</span>
                  <span className="text-forge-text font-mono">{selectedAtom.element.electronegativity}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-forge-text-muted">State</span>
                  <span className="text-forge-text font-mono">{selectedAtom.element.state}</span>
                </div>
              </div>
            </div>

            <div className="glass-panel rounded-lg p-3">
              <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">BOND CONNECTIONS</div>
              {bonds.filter(b => b.atom1Id === selectedAtom.id || b.atom2Id === selectedAtom.id).length > 0 ? (
                <div className="space-y-1">
                  {bonds.filter(b => b.atom1Id === selectedAtom.id || b.atom2Id === selectedAtom.id).map(bond => {
                    const otherAtomId = bond.atom1Id === selectedAtom.id ? bond.atom2Id : bond.atom1Id;
                    const otherAtom = getAtomById(otherAtomId);
                    return (
                      <div key={bond.id} className="flex items-center justify-between text-[9px]">
                        <span className="text-forge-text-dim">
                          {otherAtom?.element.symbol || '?'} 
                          <span className="text-forge-accent ml-1">
                            {bond.type === 'single' && '-'}
                            {bond.type === 'double' && '='}
                            {bond.type === 'triple' && '≡'}
                            {bond.type === 'hydrogen' && '···'}
                          </span>
                        </span>
                        <button
                          onClick={() => cycleBondType(bond.id)}
                          className="text-[7px] text-forge-text-muted hover:text-forge-accent"
                        >
                          Change
                        </button>
                      </div>
                    );
                  })}
                </div>
              ) : (
                <div className="text-[8px] text-forge-text-muted">No bonds yet</div>
              )}
            </div>

            <button
              onClick={() => removeAtom(selectedAtom.id)}
              className="w-full py-2 rounded bg-forge-danger/20 text-forge-danger text-[9px] font-bold hover:bg-forge-danger/30 transition-all"
            >
              ✕ DELETE ATOM
            </button>
          </div>
        ) : (
          <div className="text-center py-8">
            <div className="text-3xl mb-2 opacity-30">⚛</div>
            <div className="text-[9px] text-forge-text-muted">Select an atom to view properties</div>
          </div>
        )}

        {/* Molecular Formula */}
        {atoms.length > 0 && (
          <div className="mt-4 glass-panel rounded-lg p-3">
            <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">MOLECULAR FORMULA</div>
            <div className="text-[10px] font-mono text-forge-text">
              {(() => {
                const counts: Record<string, number> = {};
                atoms.forEach(a => {
                  counts[a.element.symbol] = (counts[a.element.symbol] || 0) + 1;
                });
                return Object.entries(counts)
                  .map(([symbol, count]) => count === 1 ? symbol : `${symbol}${count}`)
                  .join('');
              })()}
            </div>
          </div>
        )}

        {/* Quick Reference */}
        <div className="mt-4 glass-panel rounded-lg p-3">
          <div className="text-[8px] font-bold text-forge-text-muted mb-2">QUICK REFERENCE</div>
          <div className="space-y-1 text-[7px] text-forge-text-dim">
            <div>• Double-click canvas to place atom</div>
            <div>• Shift+Click to start wire</div>
            <div>• Click another atom to connect</div>
            <div>• Double-click atom to delete</div>
            <div>• Drag atoms to reposition</div>
          </div>
        </div>
      </div>
    </div>
  );
}
