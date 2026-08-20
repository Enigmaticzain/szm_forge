import React, { useState } from 'react';
import { GitMerge, Play, Cpu, Zap, Box } from 'lucide-react';

interface NodePin {
  id: string;
  name: string;
}

interface NodeData {
  id: string;
  type: string;
  title: string;
  x: number;
  y: number;
  inputs: NodePin[];
  outputs: NodePin[];
  icon: React.ReactNode;
}

interface EdgeData {
  id: string;
  sourceNodeId: string;
  sourcePinId: string;
  targetNodeId: string;
  targetPinId: string;
}

const initialNodes: NodeData[] = [
  {
    id: 'node-1',
    type: 'trigger',
    title: 'Simulation Start Event',
    x: 100,
    y: 150,
    inputs: [],
    outputs: [{ id: 'out-1', name: 'On Trigger' }],
    icon: <Play size={14} className="text-forge-green" />
  },
  {
    id: 'node-2',
    type: 'action',
    title: 'Export USD Scene',
    x: 400,
    y: 100,
    inputs: [{ id: 'in-1', name: 'Exec' }],
    outputs: [{ id: 'out-1', name: 'On Complete' }],
    icon: <Box size={14} className="text-forge-accent" />
  },
  {
    id: 'node-3',
    type: 'ai-decision',
    title: 'AI Logic Gate',
    x: 700,
    y: 200,
    inputs: [{ id: 'in-1', name: 'Exec' }],
    outputs: [{ id: 'out-true', name: 'Success' }, { id: 'out-false', name: 'Failure' }],
    icon: <Cpu size={14} className="text-forge-purple" />
  },
  {
    id: 'node-4',
    type: 'action',
    title: 'OptiX Raytrace',
    x: 1000,
    y: 150,
    inputs: [{ id: 'in-1', name: 'Exec' }],
    outputs: [],
    icon: <Zap size={14} className="text-forge-yellow" />
  }
];

const initialEdges: EdgeData[] = [
  { id: 'edge-1', sourceNodeId: 'node-1', sourcePinId: 'out-1', targetNodeId: 'node-2', targetPinId: 'in-1' },
  { id: 'edge-2', sourceNodeId: 'node-2', sourcePinId: 'out-1', targetNodeId: 'node-3', targetPinId: 'in-1' },
  { id: 'edge-3', sourceNodeId: 'node-3', sourcePinId: 'out-true', targetNodeId: 'node-4', targetPinId: 'in-1' }
];

export const NodeProgrammingWorkspace: React.FC = () => {
  const [nodes, setNodes] = useState<NodeData[]>(initialNodes);
  const [edges] = useState<EdgeData[]>(initialEdges);
  const [pan, setPan] = useState({ x: 0, y: 0 });
  const [isDraggingCanvas, setIsDraggingCanvas] = useState(false);
  const [draggedNode, setDraggedNode] = useState<string | null>(null);

  const handlePointerDown = (e: React.PointerEvent) => {
    if ((e.target as HTMLElement).id === 'node-canvas-bg') {
      setIsDraggingCanvas(true);
      e.currentTarget.setPointerCapture(e.pointerId);
    }
  };

  const handlePointerMove = (e: React.PointerEvent) => {
    if (isDraggingCanvas) {
      setPan(prev => ({ x: prev.x + e.movementX, y: prev.y + e.movementY }));
    } else if (draggedNode) {
      setNodes(prev => prev.map(n => 
        n.id === draggedNode ? { ...n, x: n.x + e.movementX, y: n.y + e.movementY } : n
      ));
    }
  };

  const handlePointerUp = (e: React.PointerEvent) => {
    setIsDraggingCanvas(false);
    setDraggedNode(null);
    e.currentTarget.releasePointerCapture(e.pointerId);
  };

  const getNodePinCoords = (nodeId: string, pinId: string, isOutput: boolean) => {
    const node = nodes.find(n => n.id === nodeId);
    if (!node) return { x: 0, y: 0 };
    
    const pinIndex = isOutput 
      ? node.outputs.findIndex(p => p.id === pinId)
      : node.inputs.findIndex(p => p.id === pinId);
    
    const nodeWidth = 200;
    const headerHeight = 36;
    const pinRowHeight = 24;
    
    const x = isOutput ? node.x + nodeWidth : node.x;
    const y = node.y + headerHeight + (pinIndex * pinRowHeight) + (pinRowHeight / 2) + 8;
    return { x, y };
  };

  return (
    <div 
      className="w-full h-full relative overflow-hidden bg-forge-black select-none"
      onPointerDown={handlePointerDown}
      onPointerMove={handlePointerMove}
      onPointerUp={handlePointerUp}
      onPointerCancel={handlePointerUp}
    >
      <div 
        id="node-canvas-bg" 
        className="absolute inset-0 grid-bg cursor-grab active:cursor-grabbing"
        style={{ backgroundPosition: `${pan.x}px ${pan.y}px` }}
      />
      
      {/* SVG Edge Layer */}
      <svg className="absolute inset-0 w-full h-full pointer-events-none" style={{ transform: `translate(${pan.x}px, ${pan.y}px)` }}>
        {edges.map(edge => {
          const src = getNodePinCoords(edge.sourceNodeId, edge.sourcePinId, true);
          const tgt = getNodePinCoords(edge.targetNodeId, edge.targetPinId, false);
          const dx = Math.max(Math.abs(tgt.x - src.x) / 2, 50);
          const path = `M ${src.x} ${src.y} C ${src.x + dx} ${src.y}, ${tgt.x - dx} ${tgt.y}, ${tgt.x} ${tgt.y}`;
          return (
            <path
              key={edge.id}
              d={path}
              fill="none"
              stroke="#00d4ff"
              strokeWidth={2}
              strokeOpacity={0.6}
              className="transition-all duration-75"
            />
          );
        })}
      </svg>
      
      {/* Node Layer */}
      <div className="absolute inset-0" style={{ transform: `translate(${pan.x}px, ${pan.y}px)` }}>
        {nodes.map(node => (
          <div
            key={node.id}
            className="absolute glass-panel gradient-border rounded flex flex-col w-[200px] shadow-lg animate-fade-in"
            style={{ left: node.x, top: node.y }}
          >
            {/* Header */}
            <div 
              className="flex items-center gap-2 p-2 border-b border-white/10 bg-white/5 cursor-pointer"
              onPointerDown={(e) => { e.stopPropagation(); setDraggedNode(node.id); }}
            >
              {node.icon}
              <span className="text-[11px] font-bold tracking-wide text-white">{node.title}</span>
            </div>
            {/* Body */}
            <div className="p-2 flex flex-col gap-1 bg-black/20 relative min-h-[40px]">
              <div className="flex justify-between w-full h-full relative">
                {/* Inputs */}
                <div className="flex flex-col gap-2 relative -left-3">
                  {node.inputs.map(pin => (
                    <div key={pin.id} className="flex items-center gap-1.5 h-4">
                      <div className="w-2.5 h-2.5 rounded-full bg-forge-border border border-forge-text-muted hover:border-forge-accent hover:bg-forge-accent/20 cursor-crosshair transition-colors" />
                      <span className="text-[9px] font-mono text-forge-text-dim">{pin.name}</span>
                    </div>
                  ))}
                </div>
                {/* Outputs */}
                <div className="flex flex-col gap-2 relative -right-3 text-right">
                  {node.outputs.map(pin => (
                    <div key={pin.id} className="flex items-center justify-end gap-1.5 h-4">
                      <span className="text-[9px] font-mono text-forge-text-dim">{pin.name}</span>
                      <div className="w-2.5 h-2.5 rounded-full bg-forge-border border border-forge-accent glow-accent cursor-crosshair transition-colors" />
                    </div>
                  ))}
                </div>
              </div>
            </div>
          </div>
        ))}
      </div>
      
      {/* HUD overlay */}
      <div className="absolute top-4 left-4 glass-panel rounded border border-forge-border p-3 pointer-events-none animate-fade-in">
        <div className="flex items-center gap-2 mb-2">
          <GitMerge size={16} className="text-forge-accent glow-accent" />
          <h2 className="text-sm font-bold tracking-widest text-forge-text">NODE LOGIC ENGINE</h2>
        </div>
        <p className="text-[10px] font-mono text-forge-text-muted max-w-xs">
          Visual programming interface for defining AI behaviors, triggering 
          procedural Python jobs (OptiX, MaterialX), and orchestrating CAD events.
        </p>
      </div>
    </div>
  );
};
