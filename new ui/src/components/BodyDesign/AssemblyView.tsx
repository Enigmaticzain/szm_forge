import React, { useState } from 'react';
import { 
  MousePointer2, Move, Crosshair, 
  Link, Combine, Wrench, Shield, CheckCircle2, Box, Cylinder,
  Plus, Settings, RotateCw, Play, Square
} from 'lucide-react';
import { SceneObject } from './InteractivePartEditor';
import { InteractiveAssemblyEditor } from './InteractiveAssemblyEditor';
import { Joint } from './BodyDesignLayout';

const fasteningMethods = [
  { id: 'bolt', label: 'Bolt / Rivet', icon: <Wrench size={12} /> },
  { id: 'glue', label: 'Adhesive / Glue', icon: <Link size={12} /> },
  { id: 'weld_tig', label: 'TIG Weld', icon: <Combine size={12} /> },
  { id: 'weld_mig', label: 'MIG Weld', icon: <Combine size={12} /> },
  { id: 'hinge', label: 'Hinge Joint', icon: <Settings size={12} /> },
  { id: 'motor', label: 'Rotary Motor', icon: <RotateCw size={12} /> },
];

interface Props {
  parts: SceneObject[];
  joints: Joint[];
  setJoints: React.Dispatch<React.SetStateAction<Joint[]>>;
}

export const AssemblyView: React.FC<Props> = ({ parts, joints, setJoints }) => {
  const [activeFastener, setActiveFastener] = useState('bolt');
  const [proposedJointSize, setProposedJointSize] = useState(10);
  const [selectedForJoint, setSelectedForJoint] = useState<string[]>([]);
  // Store the proposed joint position from the 3D view
  const [proposedPosition, setProposedPosition] = useState<{x:number, y:number, z:number} | null>(null);
  const [isSimulating, setIsSimulating] = useState(false);

  const handleCreateJoint = () => {
    if (selectedForJoint.length !== 2 || !proposedPosition) return;

    const newJoint: Joint = {
      id: `joint_${Date.now()}`,
      partAId: selectedForJoint[0],
      partBId: selectedForJoint[1],
      type: activeFastener,
      size: proposedJointSize,
      position: proposedPosition
    };

    setJoints(prev => [...prev, newJoint]);
    setSelectedForJoint([]);
    setProposedPosition(null);
  };

  return (
    <div className="h-full w-full flex bg-forge-black relative">
      {/* Assembly Sidebar */}
      <div className="w-[240px] border-r border-forge-border bg-forge-dark/80 flex flex-col flex-shrink-0 z-10 p-3">
        <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-4 px-2">ASSEMBLY & FASTENING</h3>

        <div className="px-2 mb-4">
          <button 
            onClick={() => setIsSimulating(!isSimulating)}
            className={`w-full flex items-center justify-center gap-2 py-2 rounded border transition-colors ${
              isSimulating 
                ? 'bg-forge-accent/20 text-forge-accent border-forge-accent/40' 
                : 'bg-forge-surface text-forge-text-muted hover:bg-forge-surface/50 hover:text-forge-text border-forge-border'
            }`}
          >
            {isSimulating ? <Square size={12} fill="currentColor" /> : <Play size={12} fill="currentColor" />}
            <span className="text-[10px] font-bold tracking-wider">{isSimulating ? 'STOP SIMULATION' : 'PLAY SIMULATION'}</span>
          </button>
        </div>

        <div className="space-y-4 mb-6">
          {/* Tools */}
          <div>
            <div className="text-[8px] font-mono text-forge-text-muted px-2 mb-2">TRANSFORM</div>
            <div className="flex bg-forge-surface/30 rounded p-1 gap-1">
              <button className="flex-1 py-1.5 rounded bg-forge-surface flex justify-center text-forge-accent border border-forge-border">
                <MousePointer2 size={12} />
              </button>
              <button className="flex-1 py-1.5 rounded hover:bg-forge-surface flex justify-center text-forge-text-muted">
                <Move size={12} />
              </button>
              <button className="flex-1 py-1.5 rounded hover:bg-forge-surface flex justify-center text-forge-text-muted">
                <Crosshair size={12} />
              </button>
            </div>
          </div>

          {/* Fastening */}
          <div>
            <div className="text-[8px] font-mono text-forge-text-muted px-2 mb-2">ATTACHMENT METHODS</div>
            <div className="space-y-1">
              {fasteningMethods.map((method) => (
                <button 
                  key={method.id}
                  onClick={() => setActiveFastener(method.id)}
                  className={`w-full flex items-center gap-2 px-3 py-2 rounded text-[10px] font-mono transition-colors ${
                    activeFastener === method.id 
                      ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/40' 
                      : 'hover:bg-forge-surface/50 text-forge-text-dim border border-transparent'
                  }`}
                >
                  {method.icon}
                  {method.label}
                  {activeFastener === method.id && <CheckCircle2 size={10} className="ml-auto text-forge-accent" />}
                </button>
              ))}
            </div>
          </div>
          
          {/* Fastener Properties */}
          {selectedForJoint.length === 2 && (
            <div className="bg-forge-surface/20 border border-forge-border p-2 rounded">
              <div className="text-[8px] font-mono text-forge-text-muted mb-2">PROPOSED JOINT</div>
              
              <div className="flex items-center justify-between mb-3">
                <span className="text-[9px] font-mono text-forge-text-muted">SIZE (MM)</span>
                <input 
                  type="number" 
                  min="1"
                  max="50"
                  value={proposedJointSize} 
                  onChange={(e) => setProposedJointSize(Math.max(1, parseInt(e.target.value) || 10))}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right"
                />
              </div>

              <button
                onClick={handleCreateJoint}
                className="w-full flex items-center justify-center gap-2 py-1.5 rounded bg-forge-accent/20 text-forge-accent hover:bg-forge-accent/30 border border-forge-accent/40 transition-colors"
              >
                <Plus size={12} />
                <span className="text-[9px] font-mono font-bold">APPLY FASTENER</span>
              </button>
            </div>
          )}
          
          {/* Part Library */}
          <div className="flex-1 overflow-hidden flex flex-col">
            <div className="text-[8px] font-mono text-forge-text-muted px-2 mb-2 mt-4">DESIGNED PARTS</div>
            <div className="space-y-2 overflow-y-auto pr-2 pb-4 max-h-[150px]">
              {parts.length === 0 ? (
                <div className="text-[9px] text-forge-text-muted italic px-2">No parts designed yet.</div>
              ) : (
                parts.map(part => (
                  <div key={part.id} className="px-3 py-2 border border-forge-border/60 bg-forge-surface/30 rounded flex items-center gap-2 cursor-pointer hover:border-forge-accent/50 transition-colors">
                    {part.type === 'box' || part.type === 'plate' ? <Box size={12} className="text-forge-text-muted" /> : <Cylinder size={12} className="text-forge-text-muted" />}
                    <span className="text-[10px] font-mono text-forge-text-dim">{part.name}</span>
                  </div>
                ))
              )}
            </div>
            
            <div className="text-[8px] font-mono text-forge-text-muted px-2 mb-2 mt-4">APPLIED JOINTS</div>
            <div className="space-y-2 overflow-y-auto pr-2 pb-4 flex-1">
              {joints.length === 0 ? (
                <div className="text-[9px] text-forge-text-muted italic px-2">No joints created.</div>
              ) : (
                joints.map(joint => {
                  const method = fasteningMethods.find(m => m.id === joint.type);
                  return (
                    <div key={joint.id} className="px-3 py-2 border border-forge-border/40 bg-forge-surface/10 rounded flex flex-col gap-1">
                      <div className="flex items-center gap-2">
                        {method?.icon}
                        <span className="text-[10px] font-mono text-forge-text">{method?.label}</span>
                        <span className="text-[8px] font-mono text-forge-text-muted ml-auto">sz: {joint.size}mm</span>
                      </div>
                    </div>
                  );
                })
              )}
            </div>
          </div>
        </div>
      </div>

      {/* Main Assembly Viewport */}
      <div className="flex-1 flex flex-col relative overflow-hidden">
        
        {/* Interactive 3D Editor */}
        <div className="absolute inset-0 z-10">
          <InteractiveAssemblyEditor 
            parts={parts} 
            activeFastener={activeFastener}
            joints={joints}
            selectedForJoint={selectedForJoint}
            setSelectedForJoint={setSelectedForJoint}
            proposedJointSize={proposedJointSize}
            onProposedPositionChange={setProposedPosition}
            isSimulating={isSimulating}
          />
        </div>

        {/* Viewport Grid Background */}
        <div className="absolute inset-0 z-0 opacity-10 pointer-events-none"
             style={{ 
               backgroundImage: 'radial-gradient(circle at center, rgba(255,255,255,0.2) 1px, transparent 1px)', 
               backgroundSize: '20px 20px',
               backgroundPosition: 'center center'
             }} 
        />
        
        {/* Connection status */}
        <div className="absolute top-4 right-4 bg-forge-panel border border-forge-border rounded p-3 z-20 shadow-lg min-w-[200px]">
          <div className="flex items-center gap-2 mb-2">
            <Shield size={12} className="text-forge-green" />
            <span className="text-[10px] font-bold tracking-widest text-forge-text-dim">ASSEMBLY STATUS</span>
          </div>
          <div className="flex justify-between items-center mb-1">
            <span className="text-[9px] font-mono text-forge-text-muted">Total Parts</span>
            <span className="text-[10px] font-mono font-bold text-forge-text">{parts.length}</span>
          </div>
          <div className="flex justify-between items-center mb-1">
            <span className="text-[9px] font-mono text-forge-text-muted">Total Joints</span>
            <span className="text-[10px] font-mono font-bold text-forge-text">{joints.length}</span>
          </div>
        </div>

        {/* Floating status bar */}
        <div className="absolute bottom-4 left-4 h-[32px] bg-forge-panel border border-forge-border rounded flex items-center px-4 justify-between z-20 shadow-lg">
          <div className="flex items-center gap-4">
            <span className="text-[9px] font-mono text-forge-text-muted">
              {selectedForJoint.length === 2 ? 'DRAG JOINT MARKER TO POSITION. THEN CLICK APPLY.' : 'SELECT 2 PARTS TO CREATE JOINT'}
            </span>
            <span className="text-[9px] font-mono text-forge-accent">
              METHOD: {fasteningMethods.find(m => m.id === activeFastener)?.label.toUpperCase()}
            </span>
          </div>
        </div>

      </div>
    </div>
  );
};
