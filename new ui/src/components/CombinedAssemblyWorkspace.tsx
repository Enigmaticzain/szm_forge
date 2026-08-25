import React from 'react';
import { useForgeStore } from '../store/ForgeStoreContext';
import { Combine } from 'lucide-react';

export const CombinedAssemblyWorkspace: React.FC = () => {
  const store = useForgeStore();

  return (
    <div className="w-full h-full flex flex-col bg-forge-dark/50 relative overflow-hidden">
      <div className="absolute top-4 right-4 z-10 flex gap-3">
        <button 
          onClick={() => store.setImportDialogOpen(true)}
          className="px-4 py-2 bg-forge-green/20 text-forge-green border border-forge-green/40 rounded shadow-lg backdrop-blur hover:bg-forge-green/30 text-xs font-bold font-mono transition-all"
        >
          IMPORT SUB-ASSEMBLY
        </button>
      </div>

      <div className="flex-1 w-full h-full flex items-center justify-center">
        <div className="flex flex-col items-center opacity-40">
          <Combine size={48} className="text-forge-accent mb-4" />
          <span className="text-forge-text font-mono text-sm tracking-wider">COMBINED ASSEMBLY WORKSPACE</span>
        </div>
      </div>
    </div>
  );
};
