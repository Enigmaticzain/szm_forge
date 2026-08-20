import React, { useState } from 'react';
import * as THREE from 'three';
import { Box, Circle, Cylinder, Maximize, Minus, Move, Scissors, Square, Activity, Copy, Magnet, Undo2, Redo2, Type } from 'lucide-react';
import { InteractivePartEditor } from './InteractivePartEditor';
import type { DeformSelectionMode, SceneObject, ShapeType } from './InteractivePartEditor';

interface Props {
  parts: SceneObject[];
  setParts: React.Dispatch<React.SetStateAction<SceneObject[]>>;
}

export const ComponentDesignView: React.FC<Props> = ({ parts: objects, setParts: setObjects }) => {
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [transformMode, setTransformMode] = useState<'translate' | 'scale'>('scale');
  const [deformSelectionMode, setDeformSelectionMode] = useState<DeformSelectionMode>('object');
  const [booleanToolMode, setBooleanToolMode] = useState(false);
  const [isSnappingEnabled, setIsSnappingEnabled] = useState(false);
  const [deformBrushSize, setDeformBrushSize] = useState(30);

  // Undo/Redo State
  const [history, setHistory] = useState<SceneObject[][]>([objects]);
  const [historyIndex, setHistoryIndex] = useState(0);

  const handleSetObjects = (action: React.SetStateAction<SceneObject[]>) => {
    setObjects(prev => {
      const next = typeof action === 'function' ? (action as any)(prev) : action;
      
      // Prevent saving if no actual change (optional optimization)
      if (JSON.stringify(prev) === JSON.stringify(next)) return prev;

      const newHistory = history.slice(0, historyIndex + 1);
      newHistory.push(next);
      
      // Limit history size to 50
      if (newHistory.length > 50) {
        newHistory.shift();
      }
      
      setHistory(newHistory);
      setHistoryIndex(newHistory.length - 1);
      
      return next;
    });
  };

  const handleUndo = () => {
    if (historyIndex > 0) {
      const newIndex = historyIndex - 1;
      setHistoryIndex(newIndex);
      setObjects(history[newIndex]);
    }
  };

  const handleRedo = () => {
    if (historyIndex < history.length - 1) {
      const newIndex = historyIndex + 1;
      setHistoryIndex(newIndex);
      setObjects(history[newIndex]);
    }
  };

  const handleAddObject = (type: ShapeType) => {
    const id = `obj_${Date.now()}`;
    const name = `${type.charAt(0).toUpperCase() + type.slice(1)} ${objects.length + 1}`;
    
    let initialSize = { x: 100, y: 100, z: 100 };
    if (type === 'plate') {
      initialSize = { x: 200, y: 10, z: 200 };
    } else if (type === 'cylinder') {
      initialSize = { x: 50, y: 150, z: 50 };
    }

    const newObj: SceneObject = {
      id,
      name,
      type,
      position: { x: 0, y: 0, z: 0 },
      size: initialSize,
      bendAngle: 0
    };

    handleSetObjects(prev => [...prev, newObj]);
    setSelectedId(id);
    setBooleanToolMode(false);
    setDeformSelectionMode('object');
  };

  const handleObjectChange = (id: string, newSize: {x:number, y:number, z:number}, newPos: {x:number, y:number, z:number}) => {
    handleSetObjects(prev => prev.map(obj => 
      obj.id === id ? { ...obj, size: newSize, position: newPos } : obj
    ));
  };

  const handleGeometryChange = (id: string, geometry: THREE.BufferGeometry, newSize: {x:number, y:number, z:number}) => {
    handleSetObjects(prev => prev.map(obj => 
      obj.id === id ? { ...obj, customGeometry: geometry, bendAngle: 0, size: newSize } : obj
    ));
  };

  const handleSizeInput = (axis: 'x' | 'y' | 'z', value: string) => {
    if (!selectedId) return;
    const num = parseFloat(value);
    if (!isNaN(num) && num > 0) {
      handleSetObjects(prev => prev.map(obj => 
        obj.id === selectedId ? { ...obj, size: { ...obj.size, [axis]: num } } : obj
      ));
    }
  };

  const handlePositionInput = (axis: 'x' | 'y' | 'z', value: string) => {
    if (!selectedId) return;
    const num = parseFloat(value);
    if (!isNaN(num)) {
      handleSetObjects(prev => prev.map(obj => 
        obj.id === selectedId ? { ...obj, position: { ...obj.position, [axis]: num } } : obj
      ));
    }
  };

  const handleBendInput = (value: string) => {
    if (!selectedId) return;
    const num = parseFloat(value);
    if (!isNaN(num)) {
      // Convert degrees to radians for internal storage, limit to something reasonable
      const radians = Math.max(0, Math.min(Math.PI * 2, (num * Math.PI) / 180));
      handleSetObjects(prev => prev.map(obj => 
        obj.id === selectedId ? { ...obj, bendAngle: radians } : obj
      ));
    }
  };

  const handleBendAxisInput = (axis: 'x' | 'y' | 'z') => {
    if (!selectedId) return;
    handleSetObjects(prev => prev.map(obj => 
      obj.id === selectedId ? { ...obj, bendAxis: axis } : obj
    ));
  };

  const handlePerformCut = (targetId: string, toolId: string) => {
    if (typeof (window as any)._performCSG === 'function') {
      const resultGeo = (window as any)._performCSG(targetId, toolId);
      if (resultGeo) {
        handleSetObjects(prev => {
          const filtered = prev.filter(o => o.id !== toolId);
          return filtered.map(o => o.id === targetId ? { ...o, customGeometry: resultGeo } : o);
        });
      }
    }
    setBooleanToolMode(false);
  };

  const handleApplyBend = () => {
    if (!selectedId) return;
    if (typeof (window as any)._bakeBend === 'function') {
      const bakedGeo = (window as any)._bakeBend(selectedId);
      if (bakedGeo) {
        handleSetObjects(prev => prev.map(obj => 
          obj.id === selectedId ? { ...obj, customGeometry: bakedGeo, bendAngle: 0 } : obj
        ));
      }
    }
  };

  const handleDuplicate = () => {
    if (!selectedId) return;
    const selectedObj = objects.find(o => o.id === selectedId);
    if (!selectedObj) return;

    const id = `obj_${Date.now()}`;
    // Simple naming logic
    const name = selectedObj.name.includes('(Copy)') 
      ? selectedObj.name 
      : `${selectedObj.name} (Copy)`;
    
    const newObj: SceneObject = {
      ...selectedObj,
      id,
      name,
      position: { 
        x: selectedObj.position.x + 20, 
        y: selectedObj.position.y + 20, 
        z: selectedObj.position.z + 20 
      },
      customGeometry: selectedObj.customGeometry ? selectedObj.customGeometry.clone() : undefined
    };

    handleSetObjects(prev => [...prev, newObj]);
    setSelectedId(id);
    setBooleanToolMode(false);
    setDeformSelectionMode('object');
  };

  const handleTransformMode = (mode: 'translate' | 'scale') => {
    setTransformMode(mode);
    setDeformSelectionMode('object');
  };

  const handleDeformSelectionMode = (mode: Exclude<DeformSelectionMode, 'object'>) => {
    setBooleanToolMode(false);
    setDeformSelectionMode(prev => prev === mode ? 'object' : mode);
  };

  const selectedObject = objects.find(o => o.id === selectedId);
  const bendDegrees = selectedObject?.bendAngle ? (selectedObject.bendAngle * 180) / Math.PI : 0;
  const bendAxis = selectedObject?.bendAxis || 'x';

  return (
    <div className="h-full w-full flex bg-forge-black relative">
      {/* CAD Toolbar sidebar */}
      <div className="w-[220px] border-r border-forge-border bg-forge-dark/80 flex flex-col flex-shrink-0 z-10">
        <div className="p-3 border-b border-forge-border">
          <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-3 px-1">ADD PRIMITIVES</h3>
          
          <div className="grid grid-cols-2 gap-2 mb-4">
            <button 
              onClick={() => handleAddObject('box')}
              className="flex flex-col items-center justify-center gap-1 p-2 rounded bg-forge-surface/30 hover:bg-forge-surface text-forge-text-muted hover:text-forge-text transition-colors border border-forge-border/50 hover:border-forge-accent/50"
            >
              <Box size={14} className="text-forge-accent" />
              <span className="text-[9px] font-mono">Block</span>
            </button>
            <button 
              onClick={() => handleAddObject('plate')}
              className="flex flex-col items-center justify-center gap-1 p-2 rounded bg-forge-surface/30 hover:bg-forge-surface text-forge-text-muted hover:text-forge-text transition-colors border border-forge-border/50 hover:border-forge-accent/50"
            >
              <Box size={14} className="text-forge-purple" style={{ transform: 'scaleY(0.5)' }} />
              <span className="text-[9px] font-mono">Plate</span>
            </button>
            <button 
              onClick={() => handleAddObject('cylinder')}
              className="flex flex-col items-center justify-center gap-1 p-2 rounded bg-forge-surface/30 hover:bg-forge-surface text-forge-text-muted hover:text-forge-text transition-colors border border-forge-border/50 hover:border-forge-accent/50"
            >
              <Cylinder size={14} className="text-forge-green" />
              <span className="text-[9px] font-mono">Cylinder</span>
            </button>
            <button 
              className="flex flex-col items-center justify-center gap-1 p-2 rounded bg-forge-surface/10 text-forge-text-muted/50 cursor-not-allowed border border-forge-border/20"
            >
              <Type size={14} />
              <span className="text-[9px] font-mono">Custom</span>
            </button>
          </div>

          <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-3 px-1">OPERATIONS</h3>
          <button 
            disabled={!selectedId || objects.length < 2}
            onClick={() => setBooleanToolMode(!booleanToolMode)}
            className={`w-full flex items-center justify-center gap-2 p-2 rounded transition-colors border 
              ${booleanToolMode 
                ? 'bg-forge-red/20 text-forge-red border-forge-red/50' 
                : !selectedId || objects.length < 2
                  ? 'bg-forge-surface/10 text-forge-text-muted/50 border-transparent cursor-not-allowed'
                  : 'bg-forge-surface/30 hover:bg-forge-surface text-forge-text-muted hover:text-forge-text border-forge-border/50 hover:border-forge-red/50'
              }`}
          >
            <Scissors size={14} />
            <span className="text-[9px] font-mono font-bold">BOOLEAN CUT (CSG)</span>
          </button>
          {booleanToolMode && (
             <div className="text-[8px] font-mono text-forge-red mt-2 px-1 leading-tight mb-2">
               Select the shape you want to subtract from {selectedObject?.name}. It will be deleted after the cut.
             </div>
          )}
          
          <button 
            disabled={!selectedId}
            onClick={handleDuplicate}
            className={`w-full flex items-center justify-center gap-2 p-2 rounded transition-colors border mt-2
              ${!selectedId 
                  ? 'bg-forge-surface/10 text-forge-text-muted/50 border-transparent cursor-not-allowed'
                  : 'bg-forge-surface/30 hover:bg-forge-surface text-forge-text-muted hover:text-forge-text border-forge-border/50 hover:border-forge-accent/50'
              }`}
          >
            <Copy size={14} />
            <span className="text-[9px] font-mono font-bold">DUPLICATE SHAPE</span>
          </button>
        </div>

        {/* Scene List */}
        <div className="flex-1 overflow-y-auto p-3">
          <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-3 px-1">SCENE OBJECTS</h3>
          {objects.length === 0 ? (
            <div className="text-[9px] text-forge-text-muted italic px-2">No objects in scene. Add a primitive to begin.</div>
          ) : (
            <div className="space-y-1">
              {objects.map(obj => (
                <button
                  key={obj.id}
                  onClick={() => setSelectedId(obj.id)}
                  className={`w-full flex items-center gap-2 px-2 py-1.5 rounded text-[10px] font-mono transition-colors text-left
                    ${selectedId === obj.id 
                      ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/40' 
                      : 'hover:bg-forge-surface/50 text-forge-text-dim border border-transparent'
                    }`}
                >
                  {obj.type === 'box' || obj.type === 'plate' ? <Box size={10} /> : <Cylinder size={10} />}
                  <span className="truncate">{obj.name}</span>
                  {obj.customGeometry && <span title="Modified by CSG" className="ml-auto flex"><Activity size={10} className="text-forge-red" /></span>}
                </button>
              ))}
            </div>
          )}
        </div>
      </div>

      {/* Main viewport */}
      <div className="flex-1 flex flex-col relative overflow-hidden">
        
        {/* Transform Toolbar */}
        {!booleanToolMode && (
          <div className="absolute top-4 left-4 flex gap-1 bg-forge-panel border border-forge-border p-1 rounded z-20 shadow-lg">
            <button 
              onClick={() => handleTransformMode('translate')}
              title="Translate (Move)"
              className={`flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors ${transformMode === 'translate' ? 'bg-forge-accent/20 text-forge-accent' : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface'}`}
            >
              <Move size={14} />
              <span className="text-[9px] font-bold tracking-wider">MOVE</span>
            </button>
            <button 
              onClick={() => handleTransformMode('scale')}
              title="Scale"
              className={`flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors ${transformMode === 'scale' ? 'bg-forge-accent/20 text-forge-accent' : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface'}`}
            >
              <Maximize size={14} />
              <span className="text-[9px] font-bold tracking-wider">SCALE</span>
            </button>
            <div className="w-px h-6 bg-forge-border mx-1 self-center" />
            <button 
              onClick={() => handleDeformSelectionMode('point')}
              title="Drag shape points"
              className={`flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors ${deformSelectionMode === 'point' ? 'bg-forge-yellow/20 text-forge-yellow' : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface'}`}
            >
              <Circle size={14} />
              <span className="text-[9px] font-bold tracking-wider">POINT</span>
            </button>
            <button 
              onClick={() => handleDeformSelectionMode('line')}
              title="Drag shape lines"
              className={`flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors ${deformSelectionMode === 'line' ? 'bg-forge-green/20 text-forge-green' : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface'}`}
            >
              <Minus size={14} />
              <span className="text-[9px] font-bold tracking-wider">LINE</span>
            </button>
            <button 
              onClick={() => handleDeformSelectionMode('plane')}
              title="Drag shape planes"
              className={`flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors ${deformSelectionMode === 'plane' ? 'bg-forge-purple/20 text-forge-purple' : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface'}`}
            >
              <Square size={14} />
              <span className="text-[9px] font-bold tracking-wider">PLANE</span>
            </button>
            <div className="w-px h-6 bg-forge-border mx-1 self-center" />
            <button 
              onClick={() => setIsSnappingEnabled(!isSnappingEnabled)}
              title="Magnetic Snap"
              className={`flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors ${isSnappingEnabled ? 'bg-forge-yellow/20 text-forge-yellow' : 'text-forge-text-muted hover:text-forge-text hover:bg-forge-surface'}`}
            >
              <Magnet size={14} />
              <span className="text-[9px] font-bold tracking-wider">SNAP</span>
            </button>
            <div className="w-px h-6 bg-forge-border mx-1 self-center" />
            <button 
              onClick={handleUndo}
              disabled={historyIndex <= 0}
              title="Undo"
              className="flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors text-forge-text-muted hover:text-forge-text hover:bg-forge-surface disabled:opacity-30 disabled:cursor-not-allowed"
            >
              <Undo2 size={14} />
            </button>
            <button 
              onClick={handleRedo}
              disabled={historyIndex >= history.length - 1}
              title="Redo"
              className="flex items-center gap-1.5 px-2 py-1.5 rounded transition-colors text-forge-text-muted hover:text-forge-text hover:bg-forge-surface disabled:opacity-30 disabled:cursor-not-allowed"
            >
              <Redo2 size={14} />
            </button>
          </div>
        )}

        {/* Interactive 3D Editor */}
        <div className="absolute inset-0 z-10">
          <InteractivePartEditor 
            objects={objects} 
            selectedId={selectedId}
            transformMode={transformMode}
            deformSelectionMode={deformSelectionMode}
            booleanToolMode={booleanToolMode}
            isSnappingEnabled={isSnappingEnabled}
            deformBrushSize={deformBrushSize}
            onObjectChange={handleObjectChange}
            onGeometryChange={handleGeometryChange}
            onSelect={setSelectedId}
            onPerformCut={handlePerformCut}
          />
        </div>
        
        {/* Viewport Grid Background */}
        <div className="absolute inset-0 z-0 opacity-10 pointer-events-none"
             style={{ 
               backgroundImage: 'linear-gradient(rgba(255,255,255,0.2) 1px, transparent 1px), linear-gradient(90deg, rgba(255,255,255,0.2) 1px, transparent 1px)', 
               backgroundSize: '40px 40px',
               backgroundPosition: 'center center'
             }} 
        />

        {/* Floating properties panel */}
        {selectedObject && !booleanToolMode && (
          <div className="absolute top-4 right-4 bg-forge-panel border border-forge-border rounded p-3 z-20 shadow-lg w-[200px]">
            <h4 className="text-[10px] font-bold tracking-widest text-forge-accent mb-1 uppercase truncate">{selectedObject.name}</h4>
            
            <div className="text-[8px] font-mono text-forge-text-muted mb-2 mt-3 border-b border-forge-border pb-1">DIMENSIONS (mm)</div>
            <div className="space-y-1.5">
              <div className="flex items-center justify-between">
                <span className="text-[9px] font-mono text-forge-text-muted">{selectedObject.type === 'cylinder' ? 'DIA (X)' : 'WIDTH (X)'}</span>
                <input 
                  type="number" 
                  value={selectedObject.size.x.toFixed(1)} 
                  onChange={(e) => handleSizeInput('x', e.target.value)}
                  disabled={!!selectedObject.customGeometry}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right disabled:opacity-50"
                />
              </div>
              <div className="flex items-center justify-between">
                <span className="text-[9px] font-mono text-forge-text-muted">HEIGHT (Y)</span>
                <input 
                  type="number" 
                  value={selectedObject.size.y.toFixed(1)} 
                  onChange={(e) => handleSizeInput('y', e.target.value)}
                  disabled={!!selectedObject.customGeometry}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right disabled:opacity-50"
                />
              </div>
              <div className="flex items-center justify-between">
                <span className="text-[9px] font-mono text-forge-text-muted">{selectedObject.type === 'cylinder' ? 'DIA (Z)' : 'DEPTH (Z)'}</span>
                <input 
                  type="number" 
                  value={selectedObject.size.z.toFixed(1)} 
                  onChange={(e) => handleSizeInput('z', e.target.value)}
                  disabled={!!selectedObject.customGeometry}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right disabled:opacity-50"
                />
              </div>
            </div>

            {(selectedObject.type === 'plate' || selectedObject.type === 'box') && (
              <>
                <div className="text-[8px] font-mono text-forge-text-muted mb-2 mt-4 border-b border-forge-border pb-1">DEFORMATION</div>
                <div className="space-y-1.5">
                  <div className="flex items-center justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">BEND AXIS</span>
                    <div className="flex gap-1">
                      <button 
                        onClick={() => handleBendAxisInput('x')}
                        className={`w-5 h-5 flex items-center justify-center rounded text-[10px] font-mono font-bold transition-colors ${bendAxis === 'x' ? 'bg-forge-accent text-forge-black' : 'bg-forge-dark border border-forge-border text-forge-text-muted hover:text-forge-text'}`}
                      >
                        X
                      </button>
                      <button 
                        onClick={() => handleBendAxisInput('y')}
                        className={`w-5 h-5 flex items-center justify-center rounded text-[10px] font-mono font-bold transition-colors ${bendAxis === 'y' ? 'bg-forge-accent text-forge-black' : 'bg-forge-dark border border-forge-border text-forge-text-muted hover:text-forge-text'}`}
                      >
                        Y
                      </button>
                      <button 
                        onClick={() => handleBendAxisInput('z')}
                        className={`w-5 h-5 flex items-center justify-center rounded text-[10px] font-mono font-bold transition-colors ${bendAxis === 'z' ? 'bg-forge-accent text-forge-black' : 'bg-forge-dark border border-forge-border text-forge-text-muted hover:text-forge-text'}`}
                      >
                        Z
                      </button>
                    </div>
                  </div>
                  <div className="flex items-center justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">BEND (DEG)</span>
                    <input 
                      type="number" 
                      min="0"
                      max="360"
                      value={bendDegrees.toFixed(1)} 
                      onChange={(e) => handleBendInput(e.target.value)}
                      className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right"
                    />
                  </div>
                  {bendDegrees > 0 && (
                    <button
                      onClick={handleApplyBend}
                      className="w-full mt-2 py-1 px-2 text-[9px] font-mono rounded bg-forge-accent/20 text-forge-accent border border-forge-accent/50 hover:bg-forge-accent/40 transition-colors"
                    >
                      APPLY BEND (BAKE)
                    </button>
                  )}
                </div>
              </>
            )}


            <div className="text-[8px] font-mono text-forge-text-muted mb-2 mt-4 border-b border-forge-border pb-1">POSITION (X,Y,Z)</div>
            <div className="space-y-1.5">
              <div className="flex items-center justify-between">
                <span className="text-[9px] font-mono text-forge-text-muted">POS X</span>
                <input 
                  type="number" 
                  value={selectedObject.position.x.toFixed(1)} 
                  onChange={(e) => handlePositionInput('x', e.target.value)}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right"
                />
              </div>
              <div className="flex items-center justify-between">
                <span className="text-[9px] font-mono text-forge-text-muted">POS Y</span>
                <input 
                  type="number" 
                  value={selectedObject.position.y.toFixed(1)} 
                  onChange={(e) => handlePositionInput('y', e.target.value)}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right"
                />
              </div>
              <div className="flex items-center justify-between">
                <span className="text-[9px] font-mono text-forge-text-muted">POS Z</span>
                <input 
                  type="number" 
                  value={selectedObject.position.z.toFixed(1)} 
                  onChange={(e) => handlePositionInput('z', e.target.value)}
                  className="w-16 bg-forge-dark border border-forge-border rounded px-1.5 py-0.5 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent text-right"
                />
              </div>
            </div>
          </div>
        )}

        {/* Floating properties panel for Deform Brush Size */}
        {deformSelectionMode !== 'object' && (
          <div className="absolute top-4 right-4 bg-forge-panel border border-forge-border rounded p-3 z-20 shadow-lg w-[200px]">
            <h4 className="text-[10px] font-bold tracking-widest text-forge-accent mb-1 uppercase truncate">DEFORM BRUSH</h4>
            <div className="text-[8px] font-mono text-forge-text-muted mb-2 mt-3 border-b border-forge-border pb-1">SOFT SELECTION RADIUS</div>
            <div className="space-y-1.5">
              <div className="flex flex-col gap-2">
                <div className="flex items-center justify-between">
                  <span className="text-[9px] font-mono text-forge-text-muted">SIZE (mm)</span>
                  <span className="text-[10px] font-mono text-forge-accent">{deformBrushSize}</span>
                </div>
                <input 
                  type="range" 
                  min="5" 
                  max="200" 
                  value={deformBrushSize} 
                  onChange={(e) => setDeformBrushSize(parseInt(e.target.value, 10))}
                  className="w-full h-1 bg-forge-dark rounded appearance-none cursor-pointer [&::-webkit-slider-thumb]:appearance-none [&::-webkit-slider-thumb]:w-3 [&::-webkit-slider-thumb]:h-3 [&::-webkit-slider-thumb]:rounded-full [&::-webkit-slider-thumb]:bg-forge-accent"
                />
              </div>
            </div>
          </div>
        )}

        {/* Floating status bar */}
        <div className="absolute bottom-4 left-4 h-[32px] bg-forge-panel border border-forge-border rounded flex items-center px-4 justify-between z-20 shadow-lg">
          <div className="flex items-center gap-4">
            <span className="text-[9px] font-mono text-forge-text-muted">
              {booleanToolMode ? 'CSG MODE: SELECT TOOL SHAPE' : deformSelectionMode === 'object' ? 'SELECT: CLICK' : `DRAG ${deformSelectionMode.toUpperCase()} HANDLE`}
            </span>
            <span className={`text-[9px] font-mono ${booleanToolMode ? 'text-forge-red' : 'text-forge-accent'}`}>
              MODE: {booleanToolMode ? 'BOOLEAN CUT' : deformSelectionMode === 'object' ? transformMode.toUpperCase() : deformSelectionMode.toUpperCase()}
            </span>
          </div>
        </div>
      </div>
    </div>
  );
};
