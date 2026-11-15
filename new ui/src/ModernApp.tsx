import React, { useState, useCallback, useEffect, useRef } from 'react';
import * as THREE from 'three';
import { STLLoader } from 'three/examples/jsm/loaders/STLLoader.js';
import { useForgeStore } from './store/ForgeStoreContext';
import { TopCommandBar } from './components/TopCommandBar';
import { ProjectHierarchy } from './components/ProjectHierarchy';
import { ViewportShell } from './components/ViewportShell';
import { AnalysisPanel } from './components/AnalysisPanel';
import { TimelineBar } from './components/TimelineBar';
import { DigitalTwinDashboard } from './components/DigitalTwinDashboard';
import { ManufacturingDashboard } from './components/ManufacturingDashboard';
import { AIWorkspace } from './components/AIWorkspace';
import { FleetCommand } from './components/FleetCommand';
import { SimulationWorkspace } from './components/SimulationWorkspace';
import { LoadingScreen } from './components/LoadingScreen';
import { PropertyInspector } from './components/PropertyInspector';
import { UnifiedGeometryWorkspace } from './components/geometry/UnifiedGeometryWorkspace';
import { PhysicsWorkspace } from './components/PhysicsWorkspace';
import ChemistryLabView from './components/ChemistryLabView';
import CircuitDesigner from './components/CircuitDesigner';
import { CommandCenterWorkspace } from './components/CommandCenterWorkspace';
import { NodeProgrammingWorkspace } from './components/NodeProgrammingWorkspace';
import { TextToCADPanel } from './components/TextToCADPanel';
import { NotificationCenter } from './components/NotificationCenter';
import { SettingsPanel } from './components/SettingsPanel';
import { GeometryWorkspace } from './components/GeometryWorkspace';
import { MechanicalWorkspace } from './components/MechanicalWorkspace';
import { OpticsWorkspace } from './components/OpticsWorkspace';
import { CombinedAssemblyWorkspace } from './components/CombinedAssemblyWorkspace';
import { TestingLabWorkspace } from './components/TestingLabWorkspace';
import { DockingWorkspaceManager } from './components/DockingWorkspaceManager';
import { Resizer } from './components/Resizer';
import { WorkspaceLayout } from './components/WorkspaceLayout';
import { useBackend } from './store/BackendContext';
import { useProject } from './store/ProjectContext';
import { useTooltips } from './store/TooltipContext';
import { tooltips } from './data/tooltips';
import { useAuth } from './store/AuthContext';
import { LoginScreen } from './components/LoginScreen';
import { KnowledgeWorkspace } from './components/KnowledgeWorkspace';
import { ActionObserver } from './components/ActionObserver';
import { RoboticsWorkspace } from './components/RoboticsWorkspace';

const ModernApp: React.FC = () => {
  const [loading, setLoading] = useState(true);
  const [showNotifications, setShowNotifications] = useState(false);
  const [showSettings, setShowSettings] = useState(false);
  const [showWorkspaces, setShowWorkspaces] = useState(false);
  const store = useForgeStore();
  const { user, logout } = useAuth();
  const { connected, refresh } = useBackend();
  const {
    project,
    parts,
    members,
    measureLimits,
    solveAssembly,
    refresh: refreshProject,
    bodyParts,
    bodyJoints,
    setBodyParts,
  } = useProject();
  const furnitureLoadKg = project?.loadKg ?? 0;
  const { t } = useTooltips();
  const handleLoadComplete = useCallback(() => setLoading(false), []);
  const [fps, setFps] = useState(0);
  const [importFile, setImportFile] = useState<File | null>(null);
  const [importDragging, setImportDragging] = useState(false);
  const importInputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    let frames = 0;
    let last = performance.now();
    let raf = 0;
    const tick = (now: number) => {
      frames += 1;
      if (now - last >= 1000) {
        setFps(frames);
        frames = 0;
        last = now;
      }
      raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, []);

  const handleImportFile = useCallback((file: File | null) => {
    if (!file) return;
    const ext = file.name.split('.').pop()?.toLowerCase() ?? '';
    if (!['step', 'stp', 'stl', 'dxf'].includes(ext)) {
      store.showToast('Unsupported format — use .step .stp or .stl');
      return;
    }
    if (ext === 'dxf') {
      store.showToast('DXF files are 2D profiles — not importable as solids yet. Use STEP or STL.');
      return;
    }
    setImportFile(file);
  }, [store]);

  const addImportedToScene = useCallback(async () => {
    if (!importFile) {
      store.showToast('Select a CAD file first');
      return;
    }
    const ext = importFile.name.split('.').pop()?.toLowerCase() ?? '';
    const id = `import_${Date.now()}`;
    const name = importFile.name.replace(/\.[^.]+$/, '');

    try {
      if (ext === 'stl') {
        const buffer = await importFile.arrayBuffer();
        const geo = new STLLoader().parse(buffer);
        geo.computeBoundingBox();
        const box = geo.boundingBox!;
        const size = new THREE.Vector3();
        box.getSize(size);
        setBodyParts(prev => [
          ...prev,
          {
            id,
            name,
            type: 'box',
            position: { x: 0, y: 0, z: 0 },
            size: {
              x: Math.max(size.x, 1),
              y: Math.max(size.y, 1),
              z: Math.max(size.z, 1),
            },
            customGeometry: geo,
            bendAngle: 0,
          },
        ]);
      } else if (ext === 'step' || ext === 'stp') {
        // Real STEP import: OpenCascade (WASM) B-Rep → triangulated meshes.
        const { parseStepBytes, looksLikeStep } = await import('./offline/stepImport');
        const bytes = new Uint8Array(await importFile.arrayBuffer());
        if (!looksLikeStep(bytes)) {
          throw new Error(`"${importFile.name}" is not a valid STEP file (missing ISO-10303-21 header).`);
        }
        const summary = await parseStepBytes(bytes, name);
        if (summary.parts.length === 0) {
          throw new Error(summary.warnings[0] ?? 'No readable solids in this STEP file.');
        }
        setBodyParts(prev => [
          ...prev,
          ...summary.parts.map((p, i) => {
            const bbox = p.geometry.boundingBox;
            const sx = bbox ? bbox.max.x - bbox.min.x : 0.1;
            const sy = bbox ? bbox.max.y - bbox.min.y : 0.1;
            const sz = bbox ? bbox.max.z - bbox.min.z : 0.1;
            return {
              id: summary.parts.length > 1 ? `${id}_${i}` : id,
              name: p.name,
              type: 'custom' as const,
              position: { x: 0, y: 0, z: 0 },
              size: { x: sx, y: sy, z: sz },
              customGeometry: p.geometry,
              bendAngle: 0,
            };
          }),
        ]);
        const warnSuffix = summary.warnings.length ? ` (${summary.warnings.join('; ')})` : '';
        store.showToast(
          `Imported STEP: ${summary.parts.length} solid(s), ${Math.round(summary.totalTriangles)} triangles${warnSuffix}`,
        );
      } else {
        // DXF is filtered at selection time; this branch is unreachable but
        // must still refuse rather than create placeholder geometry.
        store.showToast('DXF is a 2D profile format — no placeholder solid was created.');
        setImportFile(null);
        return;
      }
      store.setWorkspace('geometry');
      store.setImportDialogOpen(false);
      setImportFile(null);
      if (ext === 'stl') store.showToast(`Added ${name} to scene`);
    } catch (e: unknown) {
      store.showToast(`Import failed: ${e instanceof Error ? e.message : String(e)}`);
    }
  }, [importFile, setBodyParts, store]);

  const partStress = Object.fromEntries(parts.map(p => [p.name, p.stressRatio]));
  const hasModel = (project?.hasFurniture && parts.length > 0) || members.length > 0 || bodyParts.length > 0;

  const viewport = (
    <ViewportShell
      renderMode={store.renderMode}
      hasModel={hasModel}
      furnitureType={
        members.length
          ? undefined
          : project?.hasFurniture && project.furnitureType
            ? project.furnitureType
            : undefined
      }
      assemblyMembers={members.length ? members : undefined}
      bodyParts={bodyParts}
      bodyJoints={bodyJoints}
      partStress={partStress}
      loadKg={furnitureLoadKg}
    />
  );

  const leftPanel = (
    <>
      <div className="flex-1 overflow-hidden min-h-0">
        <ProjectHierarchy
          selectedNode={store.selectedNode}
          setSelectedNode={store.setSelectedNode}
        />
      </div>
      <PropertyInspector selectedNode={store.selectedNode} />
    </>
  );

  const rightPanel = <AnalysisPanel />;

  const runSolver = async () => {
    if (!members.length && !project?.hasFurniture) {
      const message = bodyParts.length
        ? 'Simulation unavailable: imported scene parts are not connected to the structural solver yet.'
        : 'Simulation unavailable: create or load a supported model before running.';
      store.showToast(message);
      return;
    }

    store.setSimulationStatus('solving');
    store.setSolverProgress(0);
    for (let i = 1; i <= 10; i++) {
      await new Promise(r => setTimeout(r, 80));
      if (store.simulationPaused) return;
      store.setSolverProgress(i * 10);
    }
    await refresh();
    await refreshProject();
    if (members.length) {
      await solveAssembly(project?.materialId ?? 'MAT-STEEL-STRUCT', project?.loadKg ?? 200);
    } else if (project?.hasFurniture) {
      await measureLimits();
    }
    store.setSimulationStatus('complete');
    store.showToast('Solver finished');
  };

  const renderMainContent = () => {
    switch (store.workspace) {
      case 'digital-twin':
        return (
          <WorkspaceLayout
            left={leftPanel}
            center={<DigitalTwinDashboard />}
            right={rightPanel}
          />
        );
      case 'manufacturing':
        return (
          <WorkspaceLayout
            left={leftPanel}
            center={<ManufacturingDashboard />}
            right={rightPanel}
          />
        );
      case 'ai-assistant':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<AIWorkspace />}
            right={rightPanel}
          />
        );
      case 'fleet-command':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<FleetCommand />}
            right={rightPanel}
          />
        );
      case 'physics':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<PhysicsWorkspace />}
            showRight={false}
          />
        );
      case 'chemistry-lab':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<ChemistryLabView />}
            showRight={false}
          />
        );
      case 'circuit-design':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<CircuitDesigner />}
            showRight={false}
          />
        );
      case 'simulation':
        return (
          <WorkspaceLayout
            left={leftPanel}
            center={<SimulationWorkspace renderMode={store.renderMode} viewport={viewport} />}
            right={rightPanel}
          />
        );
      case 'geometry':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<UnifiedGeometryWorkspace />}
            showRight={false}
          />
        );
      case 'node-programming':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<NodeProgrammingWorkspace />}
            showRight={false}
          />
        );
      case 'text-to-cad':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<TextToCADPanel />}
            showRight={false}
          />
        );
      case 'command-center':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<CommandCenterWorkspace />}
            showRight={false}
          />
        );
      case 'knowledge-base':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<KnowledgeWorkspace />}
            showRight={false}
          />
        );
      case 'components':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<GeometryWorkspace />}
            showRight={false}
          />
        );
      case 'mechanical':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<MechanicalWorkspace />}
            showRight={false}
          />
        );
      case 'optics':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<OpticsWorkspace />}
            showRight={false}
          />
        );
      case 'robotics':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<RoboticsWorkspace />}
            showRight={false}
          />
        );
      case 'combined-assembly':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<CombinedAssemblyWorkspace />}
            showRight={false}
          />
        );
      case 'testing-lab':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<TestingLabWorkspace />}
            showRight={false}
          />
        );
      case 'engineering':
      default:
        return (
          <WorkspaceLayout left={leftPanel} center={viewport} right={rightPanel} />
        );
    }
  };

  const showBottomTimeline =
    store.workspace === 'engineering' ||
    store.workspace === 'simulation' ||
    store.workspace === 'digital-twin' ||
    store.workspace === 'manufacturing';

  if (loading) {
    return <LoadingScreen onComplete={handleLoadComplete} />;
  }

  if (!user) {
    return <LoginScreen />;
  }

  return (
    <div className="w-full h-full flex flex-col bg-forge-black overflow-hidden">
      <ActionObserver />
      <TopCommandBar
        workspace={store.workspace}
        setWorkspace={store.setWorkspace}
        renderMode={store.renderMode}
        setRenderMode={store.setRenderMode}
        solverProgress={store.solverProgress}
        simulationRunning={store.simulationStatus === 'solving' || store.simulationStatus === 'meshing'}
        simulationPaused={store.simulationPaused}
        onOpenNotifications={() => setShowNotifications(true)}
        onOpenSettings={() => setShowSettings(true)}
        onOpenWorkspaces={() => setShowWorkspaces(true)}
        onRunSimulation={runSolver}
        onPauseSimulation={store.pauseSimulation}
        onResumeSimulation={store.resumeSimulation}
        onResetSimulation={store.resetSimulation}
        onToggleAIPanel={store.toggleAIPanel}
        showAIPanel={store.showAIPanel}
        notificationCount={3}
      />

      <div className="flex-1 flex flex-col overflow-hidden min-h-0">
        <div className="flex-1 flex overflow-hidden min-h-0">{renderMainContent()}</div>

        {showBottomTimeline && store.showTimeline && (
          <>
            <Resizer
              orientation="horizontal"
              onResize={delta =>
                store.setTimelineHeight(prev => Math.max(64, Math.min(280, prev - delta)))
              }
            />
            <div style={{ height: store.timelineHeight }} className="flex-shrink-0 min-h-0">
              <TimelineBar visible onToggle={store.toggleTimeline} />
            </div>
          </>
        )}
        {showBottomTimeline && !store.showTimeline && (
          <TimelineBar visible={false} onToggle={store.toggleTimeline} />
        )}
      </div>

      <div className="h-[26px] bg-forge-dark/90 backdrop-blur-md border-t border-forge-border/40 flex items-center px-4 justify-between flex-shrink-0 z-50">
        <div className="flex items-center gap-4">
          <div
            className="flex items-center gap-1.5"
            title={t(connected ? tooltips.backend.connected : tooltips.backend.offline)}
          >
            <div className={`w-2 h-2 rounded-full ${connected ? 'bg-forge-green glow-green' : 'bg-forge-yellow'}`} />
            <span className="text-[9px] font-mono text-forge-text-muted">
              {connected ? 'BACKEND LIVE' : 'DEMO MODE'}
            </span>
          </div>
          <span className="text-[9px] font-mono text-forge-text-muted">
            PROJECT: {project?.projectName ?? '—'}
          </span>
          <span className="text-[9px] font-mono text-forge-text-muted">
            MEMBERS: {members.length || parts.length || '—'}
          </span>
        </div>
        <div className="flex items-center gap-4">
          <span className="text-[9px] font-mono text-forge-text-muted">
            RENDER: {store.renderMode.toUpperCase()}
          </span>
          <span className="text-[9px] font-mono text-forge-text-muted">
            {hasModel ? 'MODEL LIVE' : 'NO MODEL'}
          </span>
          <span className="text-[9px] font-mono text-forge-green">● {fps} FPS</span>
          {user && (
            <>
              <span className="text-[9px] font-mono text-forge-accent">{user.username.toUpperCase()}</span>
              <button
                onClick={logout}
                className="text-[8px] font-mono text-forge-text-muted hover:text-forge-red transition-colors"
                title="Sign out"
              >
                SIGN OUT
              </button>
            </>
          )}
        </div>
      </div>

      {store.toast && (
        <div className="fixed bottom-10 left-1/2 -translate-x-1/2 z-[100] px-4 py-2 rounded-lg bg-forge-accent/20 border border-forge-accent/50 text-[11px] font-mono text-forge-accent shadow-lg">
          {store.toast}
        </div>
      )}

      <NotificationCenter isOpen={showNotifications} onClose={() => setShowNotifications(false)} />
      <SettingsPanel isOpen={showSettings} onClose={() => setShowSettings(false)} />
      <DockingWorkspaceManager isOpen={showWorkspaces} onClose={() => setShowWorkspaces(false)} />

      {store.importDialogOpen && (
        <div className="fixed inset-0 z-[200] bg-black/60 flex items-center justify-center p-8 backdrop-blur-sm">
          <div className="bg-forge-dark/95 border border-forge-border/40 rounded-xl w-full max-w-2xl overflow-hidden shadow-2xl flex flex-col">
            <div className="flex items-center justify-between p-4 border-b border-forge-border/30">
              <h2 className="text-forge-text font-mono text-sm tracking-wider">IMPORT COMPONENT / MATERIAL</h2>
              <button
                onClick={() => { store.setImportDialogOpen(false); setImportFile(null); }}
                className="text-forge-text-muted hover:text-forge-red transition-colors"
              >
                ✕
              </button>
            </div>
            <div className="p-6 flex flex-col gap-4">
              <p className="text-forge-text-dim font-sans text-xs">
                Import STEP or STL into the active scene. STEP solids are triangulated by the embedded OpenCascade kernel (mm → m); STL loads mesh geometry. DXF (2D profiles) is not yet supported — no placeholder will be created.
              </p>
              <input
                ref={importInputRef}
                type="file"
                accept=".step,.stp,.stl,.dxf"
                className="hidden"
                onChange={e => handleImportFile(e.target.files?.[0] ?? null)}
              />
              <div
                onDragOver={e => { e.preventDefault(); setImportDragging(true); }}
                onDragLeave={() => setImportDragging(false)}
                onDrop={e => {
                  e.preventDefault();
                  setImportDragging(false);
                  handleImportFile(e.dataTransfer.files?.[0] ?? null);
                }}
                onClick={() => importInputRef.current?.click()}
                className={`cursor-pointer rounded-xl border-2 border-dashed px-6 py-10 text-center transition-colors ${
                  importDragging
                    ? 'border-forge-accent bg-forge-accent/10'
                    : 'border-forge-border/60 bg-forge-surface/40 hover:border-forge-accent/40'
                }`}
              >
                <div className="text-forge-text font-mono text-xs mb-2">
                  {importFile ? importFile.name : 'Drop file here or click to browse'}
                </div>
                <div className="text-forge-text-muted font-mono text-[10px]">
                  Accepts .step · .stp · .stl · .dxf
                </div>
              </div>
            </div>
            <div className="p-4 border-t border-forge-border/30 bg-forge-surface/50 flex justify-end gap-2">
              <button
                onClick={() => { store.setImportDialogOpen(false); setImportFile(null); }}
                className="px-4 py-2 text-forge-text-muted border border-forge-border/40 rounded hover:text-forge-text text-xs font-bold font-mono transition-all"
              >
                CANCEL
              </button>
              <button
                onClick={addImportedToScene}
                disabled={!importFile}
                className="px-4 py-2 bg-forge-accent/20 text-forge-accent border border-forge-accent/40 rounded hover:bg-forge-accent/30 text-xs font-bold font-mono transition-all disabled:opacity-40"
              >
                ADD TO SCENE
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default ModernApp;
