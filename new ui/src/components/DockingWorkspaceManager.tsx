import React, { useState } from 'react';
import { X, Save, Trash2, LayoutGrid, Check } from 'lucide-react';
import { useWorkspacePresets, WorkspacePreset } from '../store/useWorkspacePresets';
import { useForgeStore } from '../store/ForgeStoreContext';

interface Props {
  isOpen: boolean;
  onClose: () => void;
}

export const DockingWorkspaceManager: React.FC<Props> = ({ isOpen, onClose }) => {
  const store = useForgeStore();
  const { presets, savePreset, deletePreset, renamePreset } = useWorkspacePresets();
  const [newName, setNewName] = useState('');
  const [editingId, setEditingId] = useState<string | null>(null);
  const [editName, setEditName] = useState('');
  const [savedId, setSavedId] = useState<string | null>(null);

  if (!isOpen) return null;

  const handleSave = () => {
    const name = newName.trim() || `Workspace ${presets.length + 1}`;
    const p = savePreset({
      name,
      workspace: store.workspace,
      renderMode: store.renderMode,
      leftPanelWidth: store.leftPanelWidth,
      rightPanelWidth: store.rightPanelWidth,
      timelineHeight: store.timelineHeight,
      showAIPanel: store.showAIPanel,
      showTimeline: store.showTimeline,
    });
    setNewName('');
    setSavedId(p.id);
    setTimeout(() => setSavedId(null), 1500);
  };

  const handleLoad = (p: WorkspacePreset) => {
    store.setWorkspace(p.workspace);
    store.setRenderMode(p.renderMode);
    store.setLeftPanelWidth(p.leftPanelWidth);
    store.setRightPanelWidth(p.rightPanelWidth);
    store.setTimelineHeight(p.timelineHeight);
    if (p.showAIPanel !== store.showAIPanel) store.toggleAIPanel();
    if (p.showTimeline !== store.showTimeline) store.toggleTimeline();
    store.showToast(`Loaded: ${p.name}`);
    onClose();
  };

  const commitRename = (id: string) => {
    if (editName.trim()) renamePreset(id, editName.trim());
    setEditingId(null);
  };

  return (
    <div className="fixed inset-0 z-[110] flex items-center justify-center">
      <div className="absolute inset-0 bg-black/60 backdrop-blur-sm" onClick={onClose} />
      <div className="relative w-[520px] max-h-[520px] bg-forge-panel border border-forge-border rounded-lg shadow-2xl flex flex-col animate-fade-in">
        {/* Header */}
        <div className="h-[48px] px-4 border-b border-forge-border flex items-center justify-between flex-shrink-0">
          <div className="flex items-center gap-2">
            <LayoutGrid size={15} className="text-forge-accent" />
            <span className="text-[12px] font-bold tracking-widest text-forge-text">WORKSPACE PRESETS</span>
          </div>
          <button onClick={onClose} className="p-1 rounded hover:bg-forge-surface text-forge-text-muted hover:text-forge-text-dim">
            <X size={15} />
          </button>
        </div>

        {/* Save current */}
        <div className="p-4 border-b border-forge-border/50 flex-shrink-0">
          <div className="text-[9px] font-semibold tracking-widest text-forge-text-dim mb-2">SAVE CURRENT LAYOUT</div>
          <div className="flex gap-2">
            <input
              value={newName}
              onChange={e => setNewName(e.target.value)}
              onKeyDown={e => e.key === 'Enter' && handleSave()}
              placeholder={`Workspace ${presets.length + 1}`}
              className="flex-1 px-3 py-1.5 rounded bg-forge-surface border border-forge-border text-[11px] text-forge-text placeholder-forge-text-muted focus:outline-none focus:border-forge-accent/50"
            />
            <button
              onClick={handleSave}
              className="flex items-center gap-1.5 px-3 py-1.5 rounded bg-forge-accent/15 text-forge-accent border border-forge-accent/30 text-[10px] font-semibold hover:bg-forge-accent/25 transition-all"
            >
              {savedId ? <Check size={12} /> : <Save size={12} />}
              {savedId ? 'SAVED' : 'SAVE'}
            </button>
          </div>
          <div className="mt-1.5 text-[8px] text-forge-text-muted font-mono">
            Current: {store.workspace.toUpperCase()} · {store.renderMode.toUpperCase()} · L:{store.leftPanelWidth}px R:{store.rightPanelWidth}px
          </div>
        </div>

        {/* Preset list */}
        <div className="flex-1 overflow-y-auto p-4 space-y-2 min-h-0">
          {presets.length === 0 ? (
            <div className="text-center py-8 text-[10px] text-forge-text-muted">
              No saved presets yet. Save your current layout above.
            </div>
          ) : (
            presets.map(p => (
              <div
                key={p.id}
                className="flex items-center gap-2 p-3 rounded-lg border border-forge-border/50 bg-forge-surface/10 hover:border-forge-border group"
              >
                <LayoutGrid size={13} className="text-forge-accent/60 flex-shrink-0" />

                <div className="flex-1 min-w-0">
                  {editingId === p.id ? (
                    <input
                      autoFocus
                      value={editName}
                      onChange={e => setEditName(e.target.value)}
                      onBlur={() => commitRename(p.id)}
                      onKeyDown={e => { if (e.key === 'Enter') commitRename(p.id); if (e.key === 'Escape') setEditingId(null); }}
                      className="w-full bg-forge-surface border border-forge-accent/40 rounded px-2 py-0.5 text-[11px] text-forge-text focus:outline-none"
                    />
                  ) : (
                    <button
                      className="text-[11px] font-medium text-forge-text text-left truncate w-full hover:text-forge-accent transition-colors"
                      onDoubleClick={() => { setEditingId(p.id); setEditName(p.name); }}
                      onClick={() => handleLoad(p)}
                    >
                      {p.name}
                    </button>
                  )}
                  <div className="text-[8px] text-forge-text-muted font-mono mt-0.5">
                    {p.workspace} · {p.renderMode} · {new Date(p.createdAt).toLocaleDateString()}
                  </div>
                </div>

                <button
                  onClick={() => handleLoad(p)}
                  className="px-2 py-1 rounded text-[9px] font-mono bg-forge-accent/10 text-forge-accent border border-forge-accent/20 hover:bg-forge-accent/20 transition-all opacity-0 group-hover:opacity-100"
                >
                  LOAD
                </button>
                <button
                  onClick={() => deletePreset(p.id)}
                  className="p-1 rounded text-forge-text-muted hover:text-forge-red hover:bg-forge-red/10 transition-all opacity-0 group-hover:opacity-100"
                >
                  <Trash2 size={12} />
                </button>
              </div>
            ))
          )}
        </div>

        <div className="px-4 py-2 border-t border-forge-border/50 flex-shrink-0">
          <span className="text-[8px] text-forge-text-muted font-mono">
            {presets.length} preset{presets.length !== 1 ? 's' : ''} · Double-click name to rename · Click row or LOAD to apply
          </span>
        </div>
      </div>
    </div>
  );
};
