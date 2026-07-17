import React, { createContext, useCallback, useContext, useState } from 'react';
import { pingBackend } from '../services/forgeApi';
import {
  createFurniture,
  createNewProject as apiNewProject,
  fetchProject,
  runAIImprove as apiAI,
  runLimitSweep as apiLimits,
  setLoad as apiSetLoad,
} from '../services/furnitureWorkshop';
import {
  type AIInsightDto,
  type FurniturePart,
  type FurnitureType,
  type LimitSweepRow,
  type ProjectState,
} from '../workshop/furnitureEngine';
import {
  attachPosition,
  createMember,
  presetChair,
  presetLadder,
  presetTable,
  type AttachEnd,
  type MemberKind,
  type StructuralMember,
} from '../workshop/structuralAssembly';
import { solveStructuralBackend } from '../services/structuralApi';
import type { SceneObject } from '../components/BodyDesign/InteractivePartEditor';
import type { Joint } from '../components/BodyDesign/BodyDesignLayout';

export interface CustomMaterial {
  id: string;
  name: string;
  density: number; // kg/m³
  yieldStrength: number; // MPa
  restitution: number; // 0 to 1
  color: string; // hex color
  category: string;
}

export const DEFAULT_MATERIALS: CustomMaterial[] = [
  { id: 'm1', name: 'AISI 1020 Steel', density: 7870, yieldStrength: 295, restitution: 0.1, color: '#4444ff', category: 'steel' },
  { id: 'm2', name: 'Al 7075-T6', density: 2810, yieldStrength: 503, restitution: 0.3, color: '#aaaaaa', category: 'alu' },
  { id: 'm3', name: 'Al 6061-T6', density: 2700, yieldStrength: 276, restitution: 0.3, color: '#bbbbbb', category: 'alu' },
  { id: 'm4', name: 'Carbon Fiber (UD)', density: 1600, yieldStrength: 1500, restitution: 0.4, color: '#222222', category: 'composite' },
  { id: 'm5', name: 'Titanium Ti-6Al-4V', density: 4430, yieldStrength: 880, restitution: 0.2, color: '#8888aa', category: 'steel' },
  { id: 'm6', name: 'ABS Plastic', density: 1040, yieldStrength: 40, restitution: 0.6, color: '#dddddd', category: 'plastic' }
];

interface ProjectContextValue {
  project: ProjectState | null;
  parts: FurniturePart[];
  bodyParts: SceneObject[];
  setBodyParts: React.Dispatch<React.SetStateAction<SceneObject[]>>;
  bodyJoints: Joint[];
  setBodyJoints: React.Dispatch<React.SetStateAction<Joint[]>>;
  limits: LimitSweepRow[];
  aiInsights: AIInsightDto[];
  loading: boolean;
  error: string | null;
  furnitureType: FurnitureType;
  setFurnitureType: (t: FurnitureType) => void;
  members: StructuralMember[];
  selectedMemberId: string | null;
  setSelectedMemberId: (id: string | null) => void;
  lastAction: string | null;
  newProject: (name: string) => Promise<void>;
  buildFurniture: (materialId: string, loadKg: number) => Promise<void>;
  loadPreset: (preset: 'table' | 'chair' | 'ladder', materialId: string, loadKg: number) => Promise<void>;
  addMember: (
    kind: MemberKind,
    dims: { length: number; width: number; height: number },
    materialId: string,
    attachTo: string | null,
    attachEnd: AttachEnd
  ) => void;
  removeMember: (id: string) => void;
  updateSelectedMember: (patch: Partial<StructuralMember>) => void;
  solveAssembly: (materialId: string, loadKg: number) => Promise<void>;
  applyLoad: (loadKg: number) => Promise<void>;
  measureLimits: () => Promise<void>;
  improveWithAI: () => Promise<void>;
  previewLoad: (loadKg: number) => void;
  refresh: () => Promise<void>;
  customMaterials: CustomMaterial[];
  addMaterial: (mat: Omit<CustomMaterial, 'id'>) => void;
}

// stressToParts removed since solveAssembly is removed.

const ProjectContext = createContext<ProjectContextValue | null>(null);

export const ProjectProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [project, setProject] = useState<ProjectState | null>(null);
  const [parts, setParts] = useState<FurniturePart[]>([]);
  const [bodyParts, setBodyParts] = useState<SceneObject[]>([]);
  const [bodyJoints, setBodyJoints] = useState<Joint[]>([]);
  const [limits, setLimits] = useState<LimitSweepRow[]>([]);
  const [aiInsights, setAiInsights] = useState<AIInsightDto[]>([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [furnitureType, setFurnitureType] = useState<FurnitureType>('ladder');
  const [lastAction, setLastAction] = useState<string | null>(null);
  const [members, setMembers] = useState<StructuralMember[]>([]);
  const [selectedMemberId, setSelectedMemberId] = useState<string | null>(null);
  const [useBackend, setUseBackend] = useState<boolean | null>(null);
  const [customMaterials, setCustomMaterials] = useState<CustomMaterial[]>(DEFAULT_MATERIALS);

  const addMaterial = useCallback((mat: Omit<CustomMaterial, 'id'>) => {
    setCustomMaterials(prev => [...prev, { ...mat, id: `mat_${Date.now()}` }]);
    setLastAction(`Synthesized new material: ${mat.name}`);
  }, []);

  const ensureBackend = useCallback(async () => {
    if (useBackend !== null) return useBackend;
    const ok = await pingBackend();
    setUseBackend(ok);
    return ok;
  }, [useBackend]);

  const runLocalSolve = useCallback(
    async (mList: StructuralMember[], materialId: string, loadKg: number) => {
      const online = await ensureBackend();
      if (!online) {
        setError('Backend is offline. Start the SZM_Forge C++ server to run simulations.');
        setParts([]);
        return null;
      }
      if (mList.length > 0) {
        const res = await solveStructuralBackend(mList, materialId, loadKg);
        if (res?.ok && res.parts?.length) {
          setParts(res.parts);
          return res.parts;
        } else {
          setError('Backend failed to solve the assembly.');
          setParts([]);
          return null;
        }
      }
      return null;
    },
    [ensureBackend]
  );

  const refresh = useCallback(async () => {
    const online = await ensureBackend();
    if (online) {
      const p = await fetchProject();
      if (p) {
        setProject({
          projectName: p.projectName,
          hasFurniture: p.hasFurniture ?? p.hasTable ?? false,
          furnitureType: (p.furnitureType as FurnitureType) ?? 'table',
          materialId: p.materialId,
          material: p.material,
          loadKg: p.loadKg,
          aiImproved: p.aiImproved,
        });
      }
    }
  }, [ensureBackend]);

  const newProject = useCallback(
    async (name: string) => {
      setLoading(true);
      setError(null);
      setParts([]);
      setLimits([]);
      setAiInsights([]);
      setMembers([]);
      setSelectedMemberId(null);
      const online = await ensureBackend();
      if (online) await apiNewProject(name);
      setProject({
        projectName: name,
        hasFurniture: false,
        furnitureType,
        materialId: 'MAT-STEEL-STRUCT',
        material: 'Structural Steel',
        loadKg: 200,
        aiImproved: false,
      });
      setLastAction(`Created project "${name}" — add members in KIT or pick a preset.`);
      setLoading(false);
    },
    [furnitureType, ensureBackend]
  );

  const loadPreset = useCallback(
    async (preset: 'table' | 'chair' | 'ladder', materialId: string, loadKg: number) => {
      setLoading(true);
      setError(null);
      const list =
        preset === 'table'
          ? presetTable(materialId)
          : preset === 'chair'
            ? presetChair(materialId)
            : presetLadder(materialId);
      setMembers(list);
      setFurnitureType(preset);
      await runLocalSolve(list, materialId, loadKg);
      setProject({
        projectName: `${preset.charAt(0).toUpperCase()}${preset.slice(1)} Study`,
        hasFurniture: true,
        furnitureType: preset,
        materialId,
        material: list[0]?.materialId ?? materialId,
        loadKg,
        aiImproved: false,
      });
      setLastAction(`Loaded ${preset} preset (${list.length} members) — drag load or SOLVE.`);
      setLoading(false);
    },
    [runLocalSolve]
  );

  const addMember = useCallback(
    (
      kind: MemberKind,
      dims: { length: number; width: number; height: number },
      materialId: string,
      attachTo: string | null,
      attachEnd: AttachEnd
    ) => {
      const name = `${kind}-${members.length + 1}`;
      const m = createMember(kind, name, dims, materialId, attachTo, attachEnd, members);
      const next = [...members, m];
      setMembers(next);
      setSelectedMemberId(m.id);
      const load = project?.loadKg ?? 200;
      void runLocalSolve(next, materialId, load);
      setProject(prev => ({
        projectName: prev?.projectName ?? 'Structural Assembly',
        hasFurniture: true,
        furnitureType: prev?.furnitureType ?? 'table',
        materialId,
        material: materialId,
        loadKg: load,
        aiImproved: false,
      }));
      setLastAction(`Added ${name} (${kind})${attachTo ? ' — attached' : ''}.`);
    },
    [members, project, runLocalSolve]
  );

  const removeMember = useCallback(
    (id: string) => {
      const next = members.filter(m => m.id !== id);
      setMembers(next);
      if (selectedMemberId === id) setSelectedMemberId(null);
      if (next.length) void runLocalSolve(next, project?.materialId ?? 'MAT-STEEL-STRUCT', project?.loadKg ?? 200);
      else setParts([]);
      setLastAction('Member removed.');
    },
    [members, selectedMemberId, project, runLocalSolve]
  );

  const updateSelectedMember = useCallback(
    (patch: Partial<StructuralMember>) => {
      if (!selectedMemberId) return;
      const next = members.map(m => {
        if (m.id !== selectedMemberId) return m;
        const updated = { ...m, ...patch };
        if (patch.attachTo !== undefined && patch.attachTo) {
          const parent = members.find(p => p.id === patch.attachTo);
          if (parent) Object.assign(updated, attachPosition(parent, updated, updated.attachEnd));
        }
        return updated;
      });
      setMembers(next);
      void runLocalSolve(next, project?.materialId ?? 'MAT-STEEL-STRUCT', project?.loadKg ?? 200);
      setLastAction('Member updated — stress recalculated.');
    },
    [selectedMemberId, members, project, runLocalSolve]
  );

  const solveAssemblyAction = useCallback(
    async (materialId: string, loadKg: number) => {
      setLoading(true);
      if (!members.length) {
        setError('Add members or load a preset first.');
        setLoading(false);
        return;
      }
      await runLocalSolve(members, materialId, loadKg);
      setProject(prev => ({
        projectName: prev?.projectName ?? 'Structural Assembly',
        hasFurniture: true,
        furnitureType: prev?.furnitureType ?? 'table',
        materialId,
        material: materialId,
        loadKg,
        aiImproved: false,
      }));
      setLastAction(`Solved ${members.length} members — von Mises + beam bending + rod buckling check.`);
      setLoading(false);
    },
    [members, runLocalSolve]
  );

  const buildFurniture = useCallback(
    async (materialId: string, loadKg: number) => {
      setLoading(true);
      setError(null);
      const online = await ensureBackend();
      if (!online) {
        setError('Backend is offline. Cannot build furniture.');
        setLoading(false);
        return;
      }

      let built: FurniturePart[] = [];
      const res = await createFurniture(furnitureType, materialId, loadKg);
      if (res?.parts) {
        built = res.parts;
      } else {
        setError('Backend failed to build furniture.');
        setLoading(false);
        return;
      }
      setParts(built);
      setProject(prev => ({
        projectName: prev?.projectName ?? `${furnitureType} Study`,
        hasFurniture: true,
        furnitureType,
        materialId,
        material: built[0]?.material ?? materialId,
        loadKg,
        aiImproved: false,
      }));
      setLastAction(
        `Built ${furnitureType} (${built.length} parts) with ${built[0]?.material ?? materialId}. Press APPLY LOAD or LIMITS.`
      );
      setLoading(false);
    },
    [furnitureType, ensureBackend]
  );

  const applyLoad = useCallback(
    async (loadKg: number) => {
      setLoading(true);
      const materialId = project?.materialId ?? 'MAT-STEEL-STRUCT';
      if (members.length > 0) {
        await runLocalSolve(members, materialId, loadKg);
        setProject(prev => (prev ? { ...prev, loadKg } : prev));
        setLastAction(`Applied ${loadKg} kg to assembly — see 3D stress colors.`);
        setLoading(false);
        return;
      }
      const online = await ensureBackend();
      if (!online) {
        setError('Backend is offline. Cannot apply load.');
        setLoading(false);
        return;
      }

      if (project?.hasFurniture) {
        const res = await apiSetLoad(loadKg);
        if (res?.parts) setParts(res.parts);
        else setError('Backend failed to apply load.');
      }
      setProject(prev =>
        prev ? { ...prev, loadKg } : prev
      );
      setLastAction(`Applied ${loadKg} kg — check viewport (Stress mode) and Analysis → RESULTS.`);
      setLoading(false);
    },
    [project, furnitureType, members, ensureBackend, runLocalSolve]
  );

  const runLimitSweep = useCallback(async () => {
    setLoading(true);
    setError(null);
    const materialId = project?.materialId ?? 'MAT-STEEL-STRUCT';
    if (members.length > 0) {
      setError('Limit sweep for custom assemblies is not supported by the backend yet.');
      setLoading(false);
      return;
    }

    const online = await ensureBackend();
    if (!online) {
      setError('Backend is offline. Cannot measure limits.');
      setLoading(false);
      return;
    }

    if (project?.hasFurniture) {
      const res = await apiLimits();
      if (res?.results) {
        setLimits(res.results);
        setLastAction(
          `Limit sweep: safe up to ${res.maxSafeLoadKg ?? '?'} kg` +
            (res.firstFailureLoadKg != null ? `; failure near ${res.firstFailureLoadKg} kg.` : '.')
        );
      } else {
        setError('Backend failed to measure limits.');
      }
    }
    setLoading(false);
  }, [project, furnitureType, members, ensureBackend]);

  const previewLoad = useCallback(
    (loadKg: number) => {
      const materialId = project?.materialId ?? 'MAT-STEEL-STRUCT';
      if (members.length > 0) {
        void runLocalSolve(members, materialId, loadKg);
        setProject(prev => (prev ? { ...prev, loadKg, hasFurniture: true } : prev));
        return;
      }
      if (!project?.hasFurniture) return;
      // Cannot preview load without backend. Just return.
      return;
      setProject(prev => (prev ? { ...prev, loadKg } : prev));
    },
    [project, furnitureType, members, runLocalSolve]
  );

  const runAIImprove = useCallback(async () => {
    setLoading(true);
    setError(null);
    
    if (members.length > 0) {
      setError('AI Improve for custom assemblies is not supported by the backend yet.');
      setLoading(false);
      return;
    }

    const online = await ensureBackend();
    if (!online) {
      setError('Backend is offline. Cannot run AI Improve.');
      setLoading(false);
      return;
    }

    if (project?.hasFurniture) {
      const res = await apiAI();
      if (res?.insights) setAiInsights(res.insights);
      if (res?.parts) setParts(res.parts);
      const mat = res as { materialId?: string; material?: string } | null;
      if (mat?.materialId) {
        setProject(prev =>
          prev
            ? {
                ...prev,
                materialId: mat.materialId!,
                material: mat.material ?? prev.material,
                aiImproved: true,
              }
            : prev
        );
        setLoading(false);
        setLastAction('AI improvements applied — open AI ENGINE workspace to review insights.');
        return;
      }
    }
    setProject(prev => (prev ? { ...prev, aiImproved: true } : prev));
    setLastAction('AI improvements applied.');
    setLoading(false);
  }, [project, furnitureType, parts, ensureBackend]);

  return (
    <ProjectContext.Provider
      value={{
        project,
        parts,
        bodyParts,
        setBodyParts,
        bodyJoints,
        setBodyJoints,
        limits,
        aiInsights,
        loading,
        error,
        furnitureType,
        setFurnitureType,
        members,
        selectedMemberId,
        setSelectedMemberId,
        lastAction,
        newProject,
        buildFurniture,
        loadPreset,
        addMember,
        removeMember,
        updateSelectedMember,
        solveAssembly: solveAssemblyAction,
        applyLoad,
        measureLimits: runLimitSweep,
        improveWithAI: runAIImprove,
        previewLoad,
        refresh,
        customMaterials,
        addMaterial,
      }}
    >
      {children}
    </ProjectContext.Provider>
  );
};

export function useProject() {
  const ctx = useContext(ProjectContext);
  if (!ctx) throw new Error('useProject must be used within ProjectProvider');
  return ctx;
}
