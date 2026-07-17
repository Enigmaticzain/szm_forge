import React, { useState } from 'react';
import { TreeNode } from '../types';
import { projectTree } from '../data/mockData';
import { useProject } from '../store/ProjectContext';
import {
  ChevronRight, ChevronDown, Box, Link2, Palette,
  Cpu, Radio, Brain, Layers, FolderOpen, Search, Filter,
  Wifi
} from 'lucide-react';
import { useTooltips } from '../store/TooltipContext';
import { tooltips } from '../data/tooltips';

interface Props {
  selectedNode: string | null;
  setSelectedNode: (id: string | null) => void;
}

const typeTips: Record<string, string> = {
  assembly: tooltips.hierarchy.assembly,
  part: tooltips.hierarchy.part,
  constraint: 'Mate, distance, or angle constraint between parts.',
  material: 'Material assignment from the SZM database.',
  simulation: tooltips.hierarchy.simulation,
  sensor: tooltips.hierarchy.sensor,
  agent: 'AI agent node for automated design tasks.',
  twin: 'Digital twin asset linked to live telemetry.',
  group: 'Organizational folder in the hierarchy.',
};

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
  const { t } = useTooltips();
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
        title={t(typeTips[node.type] ?? tooltips.hierarchy.panel)}
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

function buildFurnitureTree(
  projectName: string,
  furnitureLabel: string,
  parts: { name: string; status: string; stressRatio: number }[]
): TreeNode[] {
  const legNodes: TreeNode[] = parts
    .filter(p => p.name.startsWith('Leg-') || p.name.startsWith('Rung-') || p.name.startsWith('Rail-'))
    .map((p, i) => ({
      id: `leg-${i}`,
      label: `${p.name} [${(p.stressRatio * 100).toFixed(0)}% util]`,
      type: 'part' as const,
      status: p.status === 'FAIL' ? 'error' as const : p.status === 'WARN' ? 'warning' as const : 'active' as const,
    }));
  const mainPart = parts.find(p => !p.name.includes('Leg') && !p.name.includes('Rung') && !p.name.includes('Rail'));
  const groupLabel =
    legNodes.length === 4 ? 'Legs (×4)' : parts.some(p => p.name.startsWith('Rung')) ? 'Rungs & rails' : 'Members';

  return [
    {
      id: 'furniture-asm',
      label: `${projectName} — ${furnitureLabel}`,
      type: 'assembly',
      expanded: true,
      status: 'active',
      children: [
        ...(mainPart
          ? [{
              id: 'main-part',
              label: `${mainPart.name} [${(mainPart.stressRatio * 100).toFixed(0)}%]`,
              type: 'part' as const,
              status: mainPart.status === 'FAIL' ? 'error' as const : mainPart.status === 'WARN' ? 'warning' as const : 'active' as const,
            }]
          : []),
        {
          id: 'furniture-members',
          label: groupLabel,
          type: 'group',
          expanded: true,
          children: legNodes.length ? legNodes : parts.filter(p => p !== mainPart).map((p, i) => ({
            id: `p-${i}`,
            label: `${p.name} [${(p.stressRatio * 100).toFixed(0)}%]`,
            type: 'part' as const,
            status: p.status === 'FAIL' ? 'error' as const : p.status === 'WARN' ? 'warning' as const : 'active' as const,
          })),
        },
        {
          id: 'sim-load',
          label: 'Static Load Study',
          type: 'simulation',
          status: 'complete',
        },
      ],
    },
  ];
}

export const ProjectHierarchy: React.FC<Props> = ({ selectedNode, setSelectedNode }) => {
  const { t } = useTooltips();
  const { project, parts, furnitureType } = useProject();

  const hasFurniture = project?.hasFurniture ?? false;
  const furnitureLabel =
    furnitureType === 'chair' ? 'Chair' : furnitureType === 'ladder' ? 'Ladder' : 'Table';

  const tree: TreeNode[] =
    hasFurniture && parts.length > 0
      ? buildFurnitureTree(project!.projectName, furnitureLabel, parts)
      : projectTree;

  const partCount = hasFurniture ? parts.length : 8;

  return (
    <div className="h-full flex flex-col glass-panel border-r border-forge-border/40">
      {/* Header */}
      <div className="px-4 py-3 border-b border-forge-border/40 flex items-center justify-between">
        <div className="flex items-center gap-2">
          <Layers size={14} className="text-forge-accent" />
          <span className="text-[11px] font-semibold tracking-widest text-forge-text-dim">
            PROJECT HIERARCHY
          </span>
        </div>
        <div className="flex items-center gap-1">
          <button
            className="p-1 hover:bg-forge-surface rounded transition-colors"
            title={t(tooltips.hierarchy.filter)}
          >
            <Filter size={10} className="text-forge-text-muted" />
          </button>
        </div>
      </div>

      {/* Search */}
      <div className="px-3 py-2 border-b border-forge-border/40">
        <div
          className="flex items-center gap-2 px-2 py-1.5 bg-forge-surface/50 rounded-md border border-forge-border/30"
          title={t(tooltips.hierarchy.search)}
        >
          <Search size={12} className="text-forge-text-muted" />
          <input
            type="text"
            placeholder="Search components..."
            title={t(tooltips.hierarchy.search)}
            className="bg-transparent text-[11px] text-forge-text placeholder-forge-text-muted outline-none w-full font-mono"
          />
        </div>
      </div>

      {/* Tree */}
      <div className="flex-1 overflow-y-auto py-1">
        {tree.map(node => (
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
      <div className="px-4 py-3 border-t border-forge-border/40 bg-forge-dark/30 backdrop-blur-sm">
        <div className="grid grid-cols-3 gap-2">
          <div className="text-center">
            <div className="text-[10px] font-mono text-forge-text-muted">PARTS</div>
            <div className="text-xs font-bold text-forge-text font-mono">{partCount}</div>
          </div>
          <div className="text-center">
            <div className="text-[10px] font-mono text-forge-text-muted">SIMS</div>
            <div className="text-xs font-bold text-forge-accent font-mono">4</div>
          </div>
          <div className="text-center">
            <div className="text-[10px] font-mono text-forge-text-muted">TWINS</div>
            <div className="text-xs font-bold text-forge-green font-mono">2</div>
          </div>
        </div>
      </div>
    </div>
  );
};
