import React, { useState } from 'react';
import { Layers, Droplet, Filter, Box } from 'lucide-react';
import { useProject } from '../../store/ProjectContext';
import { ViewportThree } from '../ViewportThree';

const materialCategories = [
  { id: 'steel', name: 'Steels & Alloys' },
  { id: 'alu', name: 'Aluminum Alloys' },
  { id: 'composite', name: 'Composites' },
  { id: 'plastic', name: 'Polymers & Plastics' },
  { id: 'custom', name: 'Custom Lab' }
];

export const MaterialSelectionView: React.FC = () => {
  const [activeCat, setActiveCat] = useState('alu');
  const [selectedPartId, setSelectedPartId] = useState<string | null>(null);
  const { bodyParts, setBodyParts, customMaterials } = useProject();

  const handleAssignMaterial = (matId: string) => {
    if (!selectedPartId) return;
    setBodyParts(prev => prev.map(p => p.id === selectedPartId ? { ...p, materialId: matId } : p));
  };

  return (
    <div className="h-full w-full flex bg-forge-black relative">
      {/* Material Library Sidebar */}
      <div className="w-[280px] border-r border-forge-border bg-forge-dark/80 flex flex-col flex-shrink-0 z-10">
        <div className="p-3 border-b border-forge-border">
          <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-3 px-1">MATERIAL LIBRARY</h3>
          
          <div className="flex gap-1 overflow-x-auto pb-1 hide-scrollbar">
            {materialCategories.map(cat => (
              <button
                key={cat.id}
                onClick={() => setActiveCat(cat.id)}
                className={`flex-shrink-0 px-2 py-1 rounded text-[9px] font-mono transition-colors ${
                  activeCat === cat.id 
                    ? 'bg-forge-accent text-forge-black font-bold' 
                    : 'bg-forge-surface/50 text-forge-text-muted hover:text-forge-text'
                }`}
              >
                {cat.name}
              </button>
            ))}
          </div>
        </div>

        <div className="flex-1 overflow-y-auto p-3 space-y-2">
          <div className="flex items-center gap-2 mb-2 text-forge-text-muted px-1">
            <Filter size={10} />
            <span className="text-[8px] font-mono uppercase">Select part in BOM, click material</span>
          </div>

          {customMaterials.filter(m => m.category === activeCat).map(mat => (
            <div 
              key={mat.id} 
              onClick={() => handleAssignMaterial(mat.id)}
              className="p-2 border border-forge-border bg-forge-surface/30 rounded cursor-pointer hover:border-forge-accent/50 transition-colors group"
            >
              <div className="flex justify-between items-center mb-1.5">
                <span className="text-[11px] font-bold text-forge-text-dim group-hover:text-forge-text">{mat.name}</span>
                <Droplet size={12} className="opacity-50 group-hover:opacity-100" style={{ color: mat.color }} />
              </div>
              <div className="flex gap-3">
                <div>
                  <div className="text-[7px] font-mono text-forge-text-muted uppercase">Density</div>
                  <div className="text-[9px] font-mono text-forge-text">{mat.density} kg/m³</div>
                </div>
                <div>
                  <div className="text-[7px] font-mono text-forge-text-muted uppercase">Yield Strength</div>
                  <div className="text-[9px] font-mono text-forge-text">{mat.yieldStrength} MPa</div>
                </div>
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Main Viewport */}
      <div className="flex-1 flex flex-col relative overflow-hidden">
        {/* Viewport */}
        <div className="absolute inset-0 z-0">
          <ViewportThree bodyParts={bodyParts} renderMode="solid" />
        </div>

        {/* Material BOM Overlay */}
        <div className="absolute bottom-4 right-4 bg-forge-panel border border-forge-border rounded shadow-lg w-[260px] z-20 flex flex-col max-h-[40%]">
          <div className="p-3 border-b border-forge-border flex items-center gap-2">
            <Layers size={12} className="text-forge-purple" />
            <span className="text-[10px] font-bold tracking-widest text-forge-text-dim">MATERIAL BOM</span>
          </div>
          <div className="p-3 overflow-y-auto space-y-2 flex-1">
            {bodyParts.map(part => {
              const assignedMat = customMaterials.find(m => m.id === part.materialId);
              return (
                <div 
                  key={part.id} 
                  onClick={() => setSelectedPartId(part.id)}
                  className={`flex justify-between items-center text-[9px] font-mono cursor-pointer p-1.5 rounded transition-all ${
                    selectedPartId === part.id 
                      ? 'bg-forge-accent/20 border border-forge-accent text-forge-accent' 
                      : 'hover:bg-forge-surface/50 border border-transparent text-forge-text'
                  }`}
                >
                  <span>{part.name}</span>
                  <span className={assignedMat ? "text-forge-purple" : "text-forge-text-muted italic"}>
                    {assignedMat ? assignedMat.name : 'Unassigned'}
                  </span>
                </div>
              );
            })}
            {bodyParts.length === 0 && (
              <div className="text-[9px] font-mono text-forge-text-muted italic">No parts created yet.</div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
