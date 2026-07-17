import React, { useEffect, useRef } from 'react';
import {
  Copy, Trash2, Eye, EyeOff, Edit3, Link2,
  Download, Palette, Cpu,
  ChevronRight, Box, Settings, Scissors, Move
} from 'lucide-react';

interface MenuItem {
  id: string;
  label: string;
  icon?: React.ReactNode;
  shortcut?: string;
  disabled?: boolean;
  danger?: boolean;
  submenu?: MenuItem[];
  divider?: boolean;
}

interface Props {
  x: number;
  y: number;
  items: MenuItem[];
  onClose: () => void;
  onSelect: (id: string) => void;
}

const defaultItems: MenuItem[] = [
  { id: 'edit', label: 'Edit Properties', icon: <Edit3 size={12} />, shortcut: 'E' },
  { id: 'rename', label: 'Rename', icon: <Edit3 size={12} />, shortcut: 'F2' },
  { id: 'divider1', label: '', divider: true },
  { id: 'copy', label: 'Copy', icon: <Copy size={12} />, shortcut: '⌘C' },
  { id: 'cut', label: 'Cut', icon: <Scissors size={12} />, shortcut: '⌘X' },
  { id: 'paste', label: 'Paste', icon: <Box size={12} />, shortcut: '⌘V' },
  { id: 'duplicate', label: 'Duplicate', icon: <Copy size={12} />, shortcut: '⌘D' },
  { id: 'divider2', label: '', divider: true },
  { id: 'transform', label: 'Transform', icon: <Move size={12} />, submenu: [
    { id: 'move', label: 'Move', shortcut: 'G' },
    { id: 'rotate', label: 'Rotate', shortcut: 'R' },
    { id: 'scale', label: 'Scale', shortcut: 'S' },
    { id: 'mirror', label: 'Mirror', shortcut: 'M' },
  ]},
  { id: 'visibility', label: 'Visibility', icon: <Eye size={12} />, submenu: [
    { id: 'show', label: 'Show', icon: <Eye size={11} /> },
    { id: 'hide', label: 'Hide', icon: <EyeOff size={11} /> },
    { id: 'isolate', label: 'Isolate Selection' },
    { id: 'show-all', label: 'Show All' },
  ]},
  { id: 'divider3', label: '', divider: true },
  { id: 'material', label: 'Assign Material', icon: <Palette size={12} /> },
  { id: 'constraints', label: 'Add Constraint', icon: <Link2 size={12} /> },
  { id: 'simulation', label: 'Add to Simulation', icon: <Cpu size={12} /> },
  { id: 'divider4', label: '', divider: true },
  { id: 'export', label: 'Export Selection', icon: <Download size={12} /> },
  { id: 'properties', label: 'Properties Panel', icon: <Settings size={12} />, shortcut: 'P' },
  { id: 'divider5', label: '', divider: true },
  { id: 'delete', label: 'Delete', icon: <Trash2 size={12} />, shortcut: 'Del', danger: true },
];

export const ContextMenu: React.FC<Props> = ({ x, y, items = defaultItems, onClose, onSelect }) => {
  const menuRef = useRef<HTMLDivElement>(null);
  const [hoveredSubmenu, setHoveredSubmenu] = React.useState<string | null>(null);

  useEffect(() => {
    const handleClickOutside = (e: MouseEvent) => {
      if (menuRef.current && !menuRef.current.contains(e.target as Node)) {
        onClose();
      }
    };

    const handleEscape = (e: KeyboardEvent) => {
      if (e.key === 'Escape') onClose();
    };

    document.addEventListener('mousedown', handleClickOutside);
    document.addEventListener('keydown', handleEscape);
    return () => {
      document.removeEventListener('mousedown', handleClickOutside);
      document.removeEventListener('keydown', handleEscape);
    };
  }, [onClose]);

  // Adjust position to keep menu on screen
  const adjustedX = Math.min(x, window.innerWidth - 220);
  const adjustedY = Math.min(y, window.innerHeight - 400);

  return (
    <div
      ref={menuRef}
      className="fixed z-[200] min-w-[200px] py-1 bg-forge-panel border border-forge-border rounded-lg shadow-2xl animate-fade-in"
      style={{ left: adjustedX, top: adjustedY }}
    >
      {items.map((item, i) => {
        if (item.divider) {
          return <div key={i} className="my-1 border-t border-forge-border/50" />;
        }

        return (
          <div
            key={item.id}
            className="relative"
            onMouseEnter={() => item.submenu && setHoveredSubmenu(item.id)}
            onMouseLeave={() => setHoveredSubmenu(null)}
          >
            <button
              onClick={() => {
                if (!item.submenu && !item.disabled) {
                  onSelect(item.id);
                  onClose();
                }
              }}
              disabled={item.disabled}
              className={`
                w-full flex items-center gap-2 px-3 py-1.5 text-left transition-all
                ${item.disabled 
                  ? 'text-forge-text-muted cursor-not-allowed' 
                  : item.danger 
                    ? 'text-forge-red hover:bg-forge-red/10' 
                    : 'text-forge-text-dim hover:bg-forge-accent/10 hover:text-forge-text'
                }
              `}
            >
              <span className="w-4 flex items-center justify-center text-forge-text-muted">
                {item.icon}
              </span>
              <span className="flex-1 text-[10px] font-medium">{item.label}</span>
              {item.shortcut && (
                <span className="text-[8px] font-mono text-forge-text-muted">{item.shortcut}</span>
              )}
              {item.submenu && (
                <ChevronRight size={10} className="text-forge-text-muted" />
              )}
            </button>

            {/* Submenu */}
            {item.submenu && hoveredSubmenu === item.id && (
              <div 
                className="absolute left-full top-0 ml-1 min-w-[160px] py-1 bg-forge-panel border border-forge-border rounded-lg shadow-2xl"
              >
                {item.submenu.map((subItem, j) => (
                  <button
                    key={j}
                    onClick={() => {
                      onSelect(subItem.id);
                      onClose();
                    }}
                    className="w-full flex items-center gap-2 px-3 py-1.5 text-left text-forge-text-dim hover:bg-forge-accent/10 hover:text-forge-text transition-all"
                  >
                    {subItem.icon && (
                      <span className="w-4 flex items-center justify-center text-forge-text-muted">
                        {subItem.icon}
                      </span>
                    )}
                    <span className="flex-1 text-[10px] font-medium">{subItem.label}</span>
                    {subItem.shortcut && (
                      <span className="text-[8px] font-mono text-forge-text-muted">{subItem.shortcut}</span>
                    )}
                  </button>
                ))}
              </div>
            )}
          </div>
        );
      })}
    </div>
  );
};

// Export default items for reference
export { defaultItems as contextMenuItems };
