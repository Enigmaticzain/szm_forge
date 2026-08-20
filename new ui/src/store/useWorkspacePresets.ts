import { useState, useCallback } from 'react';
import { WorkspaceMode, ViewportRenderMode } from '../types';

export interface WorkspacePreset {
  id: string;
  name: string;
  workspace: WorkspaceMode;
  renderMode: ViewportRenderMode;
  leftPanelWidth: number;
  rightPanelWidth: number;
  timelineHeight: number;
  showAIPanel: boolean;
  showTimeline: boolean;
  createdAt: number;
}

const STORAGE_KEY = 'szm-workspace-presets';

function load(): WorkspacePreset[] {
  try {
    return JSON.parse(localStorage.getItem(STORAGE_KEY) ?? '[]');
  } catch {
    return [];
  }
}

function save(presets: WorkspacePreset[]) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(presets));
}

export function useWorkspacePresets() {
  const [presets, setPresets] = useState<WorkspacePreset[]>(load);

  const savePreset = useCallback((preset: Omit<WorkspacePreset, 'id' | 'createdAt'>) => {
    const next: WorkspacePreset = {
      ...preset,
      id: `ws-${Date.now()}`,
      createdAt: Date.now(),
    };
    setPresets(prev => {
      const updated = [...prev, next];
      save(updated);
      return updated;
    });
    return next;
  }, []);

  const deletePreset = useCallback((id: string) => {
    setPresets(prev => {
      const updated = prev.filter(p => p.id !== id);
      save(updated);
      return updated;
    });
  }, []);

  const renamePreset = useCallback((id: string, name: string) => {
    setPresets(prev => {
      const updated = prev.map(p => (p.id === id ? { ...p, name } : p));
      save(updated);
      return updated;
    });
  }, []);

  return { presets, savePreset, deletePreset, renamePreset };
}
