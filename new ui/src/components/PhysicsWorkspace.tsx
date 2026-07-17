import React, { useState } from 'react';
import { InteractivePhysicsViewport } from './InteractivePhysicsViewport';
import { Activity, Play, Pause, RotateCcw, Settings, Zap } from 'lucide-react';

export const PhysicsWorkspace: React.FC = () => {
  const [gravity, setGravity] = useState(-9.81);
  const [isPlaying, setIsPlaying] = useState(false);
  const [resetTrigger, setResetTrigger] = useState(0);

  return (
    <div className="h-full w-full flex flex-col bg-forge-black overflow-hidden relative">
      {/* Header */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-6 flex-shrink-0 relative z-10">
        <div className="flex items-center gap-2">
          <Zap size={14} className="text-forge-yellow" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-yellow">REAL PHYSICS</div>
            <div className="text-[8px] font-mono text-forge-text-muted">REAL-TIME KINEMATICS & COLLISION</div>
          </div>
        </div>
        
        <div className="w-px h-6 bg-forge-border mx-2" />

        {/* Physics Controls */}
        <div className="flex items-center gap-4">
          <button
            onClick={() => setIsPlaying(!isPlaying)}
            className={`flex items-center gap-1.5 px-4 py-1.5 rounded-md text-[10px] font-semibold tracking-wider transition-all duration-300
              ${isPlaying
                ? 'bg-forge-red/15 text-forge-red border border-forge-red/40'
                : 'bg-forge-green/15 text-forge-green border border-forge-green/40'
              }`}
          >
            {isPlaying ? <Pause size={12} /> : <Play size={12} />}
            {isPlaying ? 'PAUSE PHYSICS' : 'RUN PHYSICS'}
          </button>

          <button
            onClick={() => setResetTrigger(prev => prev + 1)}
            className="flex items-center gap-1.5 px-3 py-1.5 rounded-md text-[10px] font-semibold text-forge-text-muted hover:text-forge-text hover:bg-forge-surface/80 transition-all"
          >
            <RotateCcw size={12} />
            RESET
          </button>

          <div className="flex items-center gap-2 ml-4">
            <span className="text-[9px] font-mono text-forge-text-muted">GRAVITY</span>
            <input 
              type="range" 
              min="-20" 
              max="0" 
              step="0.1" 
              value={gravity} 
              onChange={(e) => setGravity(parseFloat(e.target.value))}
              className="w-24 accent-forge-yellow"
            />
            <span className="text-[9px] font-mono text-forge-yellow w-8 text-right">{gravity}</span>
          </div>
        </div>
      </div>

      {/* Main Viewport */}
      <div className="flex-1 relative">
        <InteractivePhysicsViewport 
          isPlaying={isPlaying} 
          gravity={gravity} 
          resetTrigger={resetTrigger} 
        />
        
        {/* Overlay HUD */}
        <div className="absolute top-4 right-4 bg-forge-dark/80 backdrop-blur border border-forge-border p-3 rounded pointer-events-none">
          <div className="flex items-center gap-2 mb-2">
            <Activity size={12} className="text-forge-yellow animate-pulse" />
            <span className="text-[9px] font-mono font-bold text-forge-text">PHYSICS ENGINE LIVE</span>
          </div>
          <div className="text-[8px] font-mono text-forge-text-muted">
            RUNGE-KUTTA 4 INTEGRATOR<br/>
            CUSTOM MATERIAL RESTITUTION
          </div>
        </div>
      </div>
    </div>
  );
};
