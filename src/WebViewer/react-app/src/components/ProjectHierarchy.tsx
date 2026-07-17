import React, { useState } from 'react';
import { TreeNode } from '../types';
import { projectTree } from '../data/mockData';
import {
  ChevronRight, ChevronDown, Box, Link2, Palette,
  Cpu, Radio, Brain, Layers, FolderOpen, Search, Filter,
  Wifi
} from 'lucide-react';

interface Props {
  selectedNode: string | null;
  setSelectedNode: (id: string | null) => void;
}

const typeIcons: Record<string, React.ReactNode> = {
  assembly: <Layers size={12} className="text-forge-accent" />,
  part: <Box size={12} className="text-forge-blue" />,
  constraint: <Link2 size={12} className="text-forge-yellow" />,
  material: <Palette size={12} className="text-forge-purple" />,
  simulation: <Cpu size={12} className="text-forge-cyan" />,
  sensor: <Wifi size={12} className="text-forge-teal" />,
  agent: <Brain size={12} className="text-forge-magenta" />,
  twin: <Radio size={12} className="text-forge-green" />,
  group: <FolderOpen size={12} className="text-forge-text-dim" />,
};

const statusColors: Record<string, string> = {
  active: 'bg-forge-green',
  warning: 'bg-forge-yellow',
  error: 'bg-forge-red',
  processing: 'bg-forge-accent',
  complete: 'bg-forge-teal',
};

const TreeItem: React.FC<{
  node: TreeNode;
  depth: number;
  selectedNode: string | null;
  setSelectedNode: (id: string | null) => void;
}> = ({ node, depth, selectedNode, setSelectedNode }) => {
  const [expanded, setExpanded] = useState(node.expanded ?? false);
  const hasChildren = node.children && node.children.length > 0;
  const isSelected = selectedNode === node.id;

  return (
    <div>
      <button
        onClick={() => {
          if (hasChildren) setExpanded(!expanded);
          setSelectedNode(node.id);
        }}
        className={`
          w-full flex items-center gap-1.5 py-[5px] pr-2 text-left transition-all duration-150 group
          hover:bg-forge-surface/80
          ${isSelected ? 'bg-forge-accent/8 border-r-2 border-forge-accent' : ''}
        `}
        style={{ paddingLeft: `${depth * 14 + 8}px` }}
      >
        {/* Expand/Collapse */}
        <span className="w-3 flex-shrink-0 flex items-center justify-center">
          {hasChildren ? (
            expanded ? (
              <ChevronDown size={10} className="text-forge-text-muted" />
            ) : (
              <ChevronRight size={10} className="text-forge-text-muted" />
            )
          ) : (
            <span className="w-1 h-1 rounded-full bg-forge-border-light" />
          )}
        </span>

        {/* Icon */}
        {typeIcons[node.type]}

        {/* Label */}
        <span className={`
          text-[10px] font-medium truncate flex-1
          ${isSelected ? 'text-forge-text' : 'text-forge-text-dim group-hover:text-forge-text'}
        `}>
          {node.label}
        </span>

        {/* Status Dot */}
        {node.status && (
          <span className={`
            w-1.5 h-1.5 rounded-full flex-shrink-0
            ${statusColors[node.status] || 'bg-forge-text-muted'}
            ${node.status === 'processing' ? 'animate-pulse' : ''}
          `} />
        )}
      </button>

      {/* Children */}
      {expanded && hasChildren && (
        <div>
          {node.children!.map(child => (
            <TreeItem
              key={child.id}
              node={child}
              depth={depth + 1}
              selectedNode={selectedNode}
              setSelectedNode={setSelectedNode}
            />
          ))}
        </div>
      )}
    </div>
  );
};

export const ProjectHierarchy: React.FC<Props> = ({ selectedNode, setSelectedNode }) => {
  return (
    <div className="h-full flex flex-col bg-forge-panel border-r border-forge-border">
      {/* Header */}
      <div className="px-3 py-2.5 border-b border-forge-border flex items-center justify-between">
        <div className="flex items-center gap-1.5">
          <Layers size={12} className="text-forge-accent" />
          <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">
            PROJECT HIERARCHY
          </span>
        </div>
        <div className="flex items-center gap-1">
          <button className="p-1 hover:bg-forge-surface rounded transition-colors">
            <Filter size={10} className="text-forge-text-muted" />
          </button>
        </div>
      </div>

      {/* Search */}
      <div className="px-2 py-1.5 border-b border-forge-border/50">
        <div className="flex items-center gap-1.5 px-2 py-1 bg-forge-surface rounded border border-forge-border/50">
          <Search size={10} className="text-forge-text-muted" />
          <input
            type="text"
            placeholder="Search components..."
            className="bg-transparent text-[10px] text-forge-text placeholder-forge-text-muted outline-none w-full font-mono"
          />
        </div>
      </div>

      {/* Tree */}
      <div className="flex-1 overflow-y-auto py-1">
        {projectTree.map(node => (
          <TreeItem
            key={node.id}
            node={node}
            depth={0}
            selectedNode={selectedNode}
            setSelectedNode={setSelectedNode}
          />
        ))}
      </div>

      {/* Footer Stats */}
      <div className="px-3 py-2 border-t border-forge-border bg-forge-dark/50">
        <div className="grid grid-cols-3 gap-2">
          <div className="text-center">
            <div className="text-[9px] font-mono text-forge-text-muted">PARTS</div>
            <div className="text-[11px] font-bold text-forge-text font-mono">8</div>
          </div>
          <div className="text-center">
            <div className="text-[9px] font-mono text-forge-text-muted">SIMS</div>
            <div className="text-[11px] font-bold text-forge-accent font-mono">4</div>
          </div>
          <div className="text-center">
            <div className="text-[9px] font-mono text-forge-text-muted">TWINS</div>
            <div className="text-[11px] font-bold text-forge-green font-mono">2</div>
          </div>
        </div>
      </div>
    </div>
  );
};
