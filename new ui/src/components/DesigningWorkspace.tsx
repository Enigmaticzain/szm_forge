import React, { useState } from 'react';
import {
  PenTool, Zap, Hexagon,
  Settings,
  Cpu, Activity
} from 'lucide-react';

import { BodyDesignLayout } from './BodyDesign/BodyDesignLayout';
import { useProject } from '../store/ProjectContext';

type DesignTab = 'body' | 'circuit' | 'kinematics' | 'materials';

export const DesigningWorkspace: React.FC = () => {
  const [activeTab, setActiveTab] = useState<DesignTab>('body');
  const { addMaterial, customMaterials } = useProject();

  const [matName, setMatName] = useState('Custom Polymer');
  const [matRestitution, setMatRestitution] = useState(0.75);
  const [matDensity, setMatDensity] = useState(1200);
  const [matYield, setMatYield] = useState(50);
  const [matColor, setMatColor] = useState('#ff00ff');

  const handleSynthesize = () => {
    addMaterial({
      name: matName,
      density: matDensity,
      yieldStrength: matYield,
      restitution: matRestitution,
      color: matColor,
      category: 'custom'
    });
  };

  return (
    <div className="h-full w-full flex flex-col bg-forge-black overflow-hidden relative">
      {/* Header & Sub-tabs */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-6 flex-shrink-0 relative z-10">
        <div className="flex items-center gap-2">
          <PenTool size={14} className="text-forge-accent" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-accent">DESIGNING WORKSPACE</div>
            <div className="text-[8px] font-mono text-forge-text-muted">SUB-SYSTEM LAYOUT & SCHEMATICS</div>
          </div>
        </div>
        
        <div className="w-px h-6 bg-forge-border mx-2" />

        {/* Tabs */}
        <div className="flex items-center gap-2">
          <button
            onClick={() => setActiveTab('body')}
            className={`flex items-center gap-1.5 px-4 py-1.5 rounded-md text-[10px] font-semibold tracking-wider transition-all duration-300
              ${activeTab === 'body'
                ? 'bg-forge-accent/15 text-forge-accent border border-forge-accent/40 shadow-[0_0_15px_rgba(0,212,255,0.15)]'
                : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
              }`}
          >
            <Hexagon size={12} />
            BODY DESIGN
          </button>
          
          <button
            onClick={() => setActiveTab('circuit')}
            className={`flex items-center gap-1.5 px-4 py-1.5 rounded-md text-[10px] font-semibold tracking-wider transition-all duration-300
              ${activeTab === 'circuit'
                ? 'bg-forge-purple/15 text-forge-purple border border-forge-purple/40 shadow-[0_0_15px_rgba(157,78,221,0.15)]'
                : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
              }`}
          >
            <Zap size={12} />
            CIRCUIT DESIGN
          </button>
          
          <button
            onClick={() => setActiveTab('kinematics')}
            className={`flex items-center gap-1.5 px-4 py-1.5 rounded-md text-[10px] font-semibold tracking-wider transition-all duration-300
              ${activeTab === 'kinematics'
                ? 'bg-forge-green/15 text-forge-green border border-forge-green/40 shadow-[0_0_15px_rgba(0,255,157,0.15)]'
                : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
              }`}
          >
            <Activity size={12} />
            KINEMATICS
          </button>

          <button
            onClick={() => setActiveTab('materials')}
            className={`flex items-center gap-1.5 px-4 py-1.5 rounded-md text-[10px] font-semibold tracking-wider transition-all duration-300
              ${activeTab === 'materials'
                ? 'bg-forge-yellow/15 text-forge-yellow border border-forge-yellow/40 shadow-[0_0_15px_rgba(255,204,0,0.15)]'
                : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 border border-transparent'
              }`}
          >
            <Hexagon size={12} />
            CHEMISTRY LAB
          </button>
        </div>
      </div>

      {/* Main Content Area */}
      <div className="flex-1 flex flex-col relative overflow-hidden">
        {activeTab === 'body' && (
          <BodyDesignLayout />
        )}

        
        {activeTab === 'circuit' && (
          <div className="flex flex-col items-center justify-center opacity-80 animate-in fade-in zoom-in duration-500">
            <Cpu size={64} className="text-forge-purple mb-6" strokeWidth={1} />
            <h2 className="text-xl font-bold text-forge-text tracking-widest mb-2">CIRCUIT DESIGN MODULE</h2>
            <p className="text-forge-text-muted text-sm font-mono max-w-md text-center">
              Awaiting schematic components. Use this workspace to route PCBs, assign pinouts, and configure power electronics.
            </p>
          </div>
        )}

        {activeTab === 'kinematics' && (
          <div className="flex flex-col items-center justify-center opacity-80 animate-in fade-in zoom-in duration-500 mt-20">
            <Settings size={64} className="text-forge-green mb-6 animate-spin-slow" strokeWidth={1} />
            <h2 className="text-xl font-bold text-forge-text tracking-widest mb-2">KINEMATICS MODULE</h2>
            <p className="text-forge-text-muted text-sm font-mono max-w-md text-center">
              Awaiting joint configuration components. Use this workspace to set up inverse kinematics, range of motion, and actuators.
            </p>
          </div>
        )}

        {activeTab === 'materials' && (
          <div className="flex flex-col items-center justify-center opacity-80 animate-in fade-in zoom-in duration-500 mt-20">
            <Hexagon size={64} className="text-forge-yellow mb-6" strokeWidth={1} />
            <h2 className="text-xl font-bold text-forge-text tracking-widest mb-2">CHEMISTRY LAB</h2>
            <p className="text-forge-text-muted text-sm font-mono max-w-md text-center mb-6">
              Create custom materials by mixing molecular properties. Set restitution, density, and mass to be used in the Real Physics engine.
            </p>
            
            <div className="bg-forge-dark/50 border border-forge-border p-6 rounded-lg w-full max-w-md">
              <div className="flex items-center justify-between mb-4">
                <span className="text-[10px] font-mono text-forge-text-muted">MATERIAL NAME</span>
                <input 
                  type="text" 
                  value={matName} 
                  onChange={e => setMatName(e.target.value)} 
                  className="bg-forge-black border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text" 
                />
              </div>
              <div className="flex items-center justify-between mb-4">
                <span className="text-[10px] font-mono text-forge-text-muted">RESTITUTION (BOUNCE): {matRestitution}</span>
                <input 
                  type="range" 
                  min="0" max="1" step="0.05" 
                  value={matRestitution} 
                  onChange={e => setMatRestitution(parseFloat(e.target.value))} 
                  className="w-32 accent-forge-yellow" 
                />
              </div>
              <div className="flex items-center justify-between mb-4">
                <span className="text-[10px] font-mono text-forge-text-muted">DENSITY (kg/m³): {matDensity}</span>
                <input 
                  type="range" 
                  min="100" max="10000" step="100" 
                  value={matDensity} 
                  onChange={e => setMatDensity(parseInt(e.target.value))} 
                  className="w-32 accent-forge-yellow" 
                />
              </div>
              <div className="flex items-center justify-between mb-4">
                <span className="text-[10px] font-mono text-forge-text-muted">YIELD STRENGTH (MPa): {matYield}</span>
                <input 
                  type="range" 
                  min="10" max="2000" step="10" 
                  value={matYield} 
                  onChange={e => setMatYield(parseInt(e.target.value))} 
                  className="w-32 accent-forge-yellow" 
                />
              </div>
              <div className="flex items-center justify-between mb-4">
                <span className="text-[10px] font-mono text-forge-text-muted">COLOR</span>
                <input 
                  type="color" 
                  value={matColor} 
                  onChange={e => setMatColor(e.target.value)} 
                  className="bg-transparent border-none cursor-pointer w-8 h-8 p-0" 
                />
              </div>
              <button 
                onClick={handleSynthesize}
                className="w-full mt-4 py-2 bg-forge-yellow/20 text-forge-yellow border border-forge-yellow/50 rounded font-mono text-[10px] tracking-wider hover:bg-forge-yellow/30 transition-colors"
              >
                SYNTHESIZE MATERIAL
              </button>
            </div>

            {/* List of synthesized materials */}
            <div className="w-full max-w-md mt-6 flex flex-col gap-2">
              {customMaterials.filter(m => m.category === 'custom').map(mat => (
                <div key={mat.id} className="bg-forge-surface/30 border border-forge-border p-3 rounded flex justify-between items-center">
                  <div className="flex items-center gap-3">
                    <div className="w-4 h-4 rounded-full" style={{ backgroundColor: mat.color }} />
                    <span className="text-[11px] font-bold text-forge-text">{mat.name}</span>
                  </div>
                  <div className="text-[9px] font-mono text-forge-text-muted text-right">
                    R: {mat.restitution} | D: {mat.density} | Y: {mat.yieldStrength}
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}
        
        {/* Background Grid Decoration */}
        <div className="absolute inset-0 z-[-1] opacity-5 pointer-events-none" 
             style={{ 
               backgroundImage: 'linear-gradient(rgba(255,255,255,0.2) 1px, transparent 1px), linear-gradient(90deg, rgba(255,255,255,0.2) 1px, transparent 1px)', 
               backgroundSize: '20px 20px' 
             }} 
        />
      </div>
    </div>
  );
};
