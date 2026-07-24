import React, { useState } from 'react';
import { ComponentDesignView } from './ComponentDesignView';
import { AssemblyView } from './AssemblyView';
import { MaterialSelectionView } from './MaterialSelectionView';
import { ChainSprocketLab } from './ChainSprocketLab';
import { Box, Puzzle, Layers, FlaskConical } from 'lucide-react';
import { useProject } from '../../store/ProjectContext';

export interface Joint {
  id: string;
  partAId: string;
  partBId: string;
  type: string; // 'bolt' | 'glue' | 'weld_tig' | 'weld_mig' | 'hinge' | 'motor'
  position: { x: number; y: number; z: number };
  size: number;
}

type BodyTab = 'component' | 'assembly' | 'materials' | 'experiment';

export const BodyDesignLayout: React.FC = () => {
  const [activeTab, setActiveTab] = useState<BodyTab>('component');
  const { bodyParts: parts, setBodyParts: setParts, bodyJoints: joints, setBodyJoints: setJoints } = useProject();

  return (
    <div className="h-full w-full flex flex-col bg-forge-black relative">
      {/* Body Design Sub-Navigation */}
      <div className="h-[40px] bg-forge-dark/60 border-b border-forge-border/40 flex items-center px-4 gap-2 flex-shrink-0 relative z-20 shadow-md">
        <button
          onClick={() => setActiveTab('component')}
          className={`flex items-center gap-1.5 px-3 py-1 rounded text-[9px] font-semibold tracking-wider transition-all duration-200
            ${activeTab === 'component'
              ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/40'
              : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface border border-transparent'
            }`}
        >
          <Box size={10} />
          COMPONENT DESIGN
        </button>
        
        <div className="w-px h-4 bg-forge-border mx-1" />

        <button
          onClick={() => setActiveTab('assembly')}
          className={`flex items-center gap-1.5 px-3 py-1 rounded text-[9px] font-semibold tracking-wider transition-all duration-200
            ${activeTab === 'assembly'
              ? 'bg-forge-green/20 text-forge-green border border-forge-green/40'
              : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface border border-transparent'
            }`}
        >
          <Puzzle size={10} />
          ASSEMBLY & FASTENING
        </button>

        <div className="w-px h-4 bg-forge-border mx-1" />

        <button
          onClick={() => setActiveTab('materials')}
          className={`flex items-center gap-1.5 px-3 py-1 rounded text-[9px] font-semibold tracking-wider transition-all duration-200
            ${activeTab === 'materials'
              ? 'bg-forge-purple/20 text-forge-purple border border-forge-purple/40'
              : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface border border-transparent'
            }`}
        >
          <Layers size={10} />
          FULL BODY & MATERIALS
        </button>

        <div className="w-px h-4 bg-forge-border mx-1" />

        <button
          onClick={() => setActiveTab('experiment')}
          className={`flex items-center gap-1.5 px-3 py-1 rounded text-[9px] font-semibold tracking-wider transition-all duration-200
            ${activeTab === 'experiment'
              ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/40'
              : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface border border-transparent'
            }`}
        >
          <FlaskConical size={10} />
          EXPERIMENTS LAB
        </button>
      </div>

      {/* Render active sub-view */}
      <div className="flex-1 overflow-hidden">
        {activeTab === 'component' && <ComponentDesignView parts={parts} setParts={setParts} />}
        {activeTab === 'assembly' && <AssemblyView parts={parts} joints={joints} setJoints={setJoints} />}
        {activeTab === 'materials' && <MaterialSelectionView />}
        {activeTab === 'experiment' && <ChainSprocketLab />}
      </div>
    </div>
  );
};
