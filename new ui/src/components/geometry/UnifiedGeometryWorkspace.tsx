import React, { useState } from 'react';
import {
  PenTool, Box, Layers, Wand2, Puzzle, Download, Upload,
  Ruler, Grid3x3, Cylinder, Combine, Sparkles,
} from 'lucide-react';
import { BodyDesignLayout } from '../BodyDesign/BodyDesignLayout';
import { GeometryWorkspace } from '../GeometryWorkspace';
import { TextToCADPanel } from '../TextToCADPanel';
import { SketchEditor } from './SketchEditor';

type GeometryMode =
  | 'sketch'    // AutoCAD-like 2D parametric
  | 'solid'     // FreeCAD/OCCT parametric parts
  | 'mesh'      // Blender-like mesh modeling
  | 'ai'        // Natural language CAD
  | 'assembly'; // Joints & fastening

const MODES: Array<{
  id: GeometryMode;
  label: string;
  subtitle: string;
  icon: React.ReactNode;
  engines: string[];
}> = [
  {
    id: 'sketch',
    label: 'SKETCH',
    subtitle: '2D constraints · dimensions · extrude',
    icon: <PenTool size={12} />,
    engines: ['OCCT', 'Constraint Solver'],
  },
  {
    id: 'solid',
    label: 'SOLID',
    subtitle: 'Parametric brackets · shafts · gears',
    icon: <Box size={12} />,
    engines: ['OpenCASCADE', 'build123d'],
  },
  {
    id: 'mesh',
    label: 'MESH',
    subtitle: 'Sculpt · boolean · bend · deform',
    icon: <Layers size={12} />,
    engines: ['Three.js', 'CSG'],
  },
  {
    id: 'ai',
    label: 'AI CAD',
    subtitle: 'Text → STEP · FreeCAD headless',
    icon: <Wand2 size={12} />,
    engines: ['LLM', 'FreeCAD'],
  },
  {
    id: 'assembly',
    label: 'ASSEMBLY',
    subtitle: 'Joints · welds · fasteners',
    icon: <Puzzle size={12} />,
    engines: ['Mate markers', 'Physics'],
  },
];

const FEATURE_MATRIX = [
  { category: 'Precision CAD (AutoCAD / SolidWorks)', features: [
    '2D sketch with constraints', 'Parametric extrude/revolve', 'DXF/STEP import',
    'Dimension-driven design', 'Feature history (parametric solids)',
  ]},
  { category: 'Mesh & Sculpt (Blender)', features: [
    'Primitives & transforms', 'Boolean union/cut/intersect', 'Brush deform',
    'Bend & snip operations', 'Undo/redo stack', 'Magnetic snap',
  ]},
  { category: 'Engineering (FreeCAD / Fusion)', features: [
    'Bracket, shaft, flange, gear', 'DFM analysis', 'Manufacturability checks',
    'G-code export path', 'Material assignment',
  ]},
  { category: 'AI-Assisted', features: [
    'Natural language → geometry', 'STEP download', 'Live 3D preview',
  ]},
];

export const UnifiedGeometryWorkspace: React.FC = () => {
  const [mode, setMode] = useState<GeometryMode>('mesh');
  const [showFeatures, setShowFeatures] = useState(false);

  return (
    <div className="h-full w-full flex flex-col bg-forge-black overflow-hidden">
      {/* Unified header */}
      <div className="h-[52px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-4 flex-shrink-0 z-20">
        <div className="flex items-center gap-2">
          <Grid3x3 size={14} className="text-forge-accent" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-accent">
              UNIFIED GEOMETRY STUDIO
            </div>
            <div className="text-[7px] font-mono text-forge-text-muted">
              AutoCAD · Blender · FreeCAD · Fusion — best-of-all modes
            </div>
          </div>
        </div>

        <div className="w-px h-8 bg-forge-border" />

        {/* Mode tabs */}
        <div className="flex items-center gap-1.5 flex-1 overflow-x-auto">
          {MODES.map(m => (
            <button
              key={m.id}
              onClick={() => setMode(m.id)}
              title={m.subtitle}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded-md text-[9px] font-semibold tracking-wider whitespace-nowrap transition-all
                ${mode === m.id
                  ? 'bg-forge-accent/15 text-forge-accent border border-forge-accent/40 shadow-[0_0_12px_rgba(0,212,255,0.12)]'
                  : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
                }`}
            >
              {m.icon}
              {m.label}
            </button>
          ))}
        </div>

        <div className="flex items-center gap-2 flex-shrink-0">
          <button
            onClick={() => setShowFeatures(v => !v)}
            className={`px-2 py-1 rounded text-[8px] font-mono border transition-colors
              ${showFeatures
                ? 'bg-forge-purple/20 text-forge-purple border-forge-purple/40'
                : 'text-forge-text-muted border-forge-border/50 hover:text-forge-text'}`}
          >
            <Sparkles size={10} className="inline mr-1" />
            FEATURES
          </button>
          <button className="p-1.5 rounded border border-forge-border/50 text-forge-text-muted hover:text-forge-accent" title="Import STEP/STL/DXF">
            <Upload size={12} />
          </button>
          <button className="p-1.5 rounded border border-forge-border/50 text-forge-text-muted hover:text-forge-accent" title="Export STEP/STL/USD">
            <Download size={12} />
          </button>
        </div>
      </div>

      {/* Engine badges for active mode */}
      <div className="h-7 border-b border-forge-border/40 bg-forge-dark/40 px-4 flex items-center gap-3 flex-shrink-0">
        {MODES.find(m => m.id === mode)?.engines.map(eng => (
          <span key={eng} className="text-[7px] font-mono px-1.5 py-0.5 rounded bg-forge-surface/50 text-forge-text-muted border border-forge-border/30">
            {eng}
          </span>
        ))}
        <span className="text-[7px] font-mono text-forge-text-muted ml-auto">
          {MODES.find(m => m.id === mode)?.subtitle}
        </span>
      </div>

      {/* Feature matrix overlay */}
      {showFeatures && (
        <div className="border-b border-forge-border bg-forge-panel/95 px-4 py-3 flex-shrink-0 max-h-[140px] overflow-y-auto">
          <div className="grid grid-cols-2 lg:grid-cols-4 gap-3">
            {FEATURE_MATRIX.map(block => (
              <div key={block.category} className="flex flex-col gap-1">
                <div className="text-[8px] font-bold text-forge-accent tracking-wider">{block.category}</div>
                {block.features.map(f => (
                  <div key={f} className="text-[7px] font-mono text-forge-text-muted flex items-center gap-1">
                    <Combine size={8} className="text-forge-green flex-shrink-0" />
                    {f}
                  </div>
                ))}
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Mode content */}
      <div className="flex-1 min-h-0 overflow-hidden">
        {mode === 'sketch' && <SketchEditor />}

        {mode === 'solid' && <GeometryWorkspace />}

        {mode === 'mesh' && <BodyDesignLayout />}

        {mode === 'ai' && <TextToCADPanel />}

        {mode === 'assembly' && (
          <div className="h-full flex flex-col">
            <div className="h-8 border-b border-forge-border/40 bg-forge-dark/60 px-4 flex items-center gap-2 text-[8px] font-mono text-forge-text-muted">
              <Puzzle size={10} className="text-forge-green" />
              Assembly mode — use MESH tab → Assembly & Fastening for joint placement
            </div>
            <BodyDesignLayout />
          </div>
        )}
      </div>

      {/* Quick tool reference bar */}
      <div className="h-6 border-t border-forge-border/40 bg-forge-dark/80 px-4 flex items-center gap-4 text-[7px] font-mono text-forge-text-muted flex-shrink-0">
        <span className="flex items-center gap-1"><Ruler size={8} /> Sketch: click-click lines · rect · circle</span>
        <span className="flex items-center gap-1"><Cylinder size={8} /> Solid: parametric OCCT parts</span>
        <span className="flex items-center gap-1"><Layers size={8} /> Mesh: CSG subtract · bend · deform</span>
        <span className="flex items-center gap-1"><Wand2 size={8} /> AI: describe part in English</span>
      </div>
    </div>
  );
};
