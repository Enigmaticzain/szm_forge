import React from 'react';
import { useForgeStore } from '../store/ForgeStoreContext';
import { IsometricGearViewer } from './mechanical/IsometricGearViewer';

export const MechanicalWorkspace: React.FC = () => {
  const store = useForgeStore();

  return (
    <div className="w-full h-full flex flex-col bg-forge-dark/50 relative overflow-hidden">
      <div className="absolute top-2 right-40 z-20 flex gap-3">
        <button 
          onClick={() => store.setImportDialogOpen(true)}
          className="px-3 py-1 bg-forge-green/20 text-forge-green border border-forge-green/40 rounded shadow-sm backdrop-blur hover:bg-forge-green/30 text-[10px] font-bold font-mono transition-all"
        >
          DROP IN COMPONENT
        </button>
      </div>

      <div className="flex-1 w-full h-full min-h-0">
        <IsometricGearViewer />
      </div>
    </div>
  );
};
