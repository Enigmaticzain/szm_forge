import React, { useState, useCallback } from 'react';
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
import { DesigningWorkspace } from './components/DesigningWorkspace';
import { PhysicsWorkspace } from './components/PhysicsWorkspace';
import ChemistryLabView from './components/ChemistryLabView';
import CircuitDesigner from './components/CircuitDesigner';
import { CommandCenterWorkspace } from './components/CommandCenterWorkspace';
import { NodeProgrammingWorkspace } from './components/NodeProgrammingWorkspace';
import { TextToCADPanel } from './components/TextToCADPanel';
import { NotificationCenter } from './components/NotificationCenter';
import { SettingsPanel } from './components/SettingsPanel';
import { DockingWorkspaceManager } from './components/DockingWorkspaceManager';
import { Resizer } from './components/Resizer';
import { WorkspaceLayout } from './components/WorkspaceLayout';
import { useBackend } from './store/BackendContext';
import { useProject } from './store/ProjectContext';
import { useTooltips } from './store/TooltipContext';
import { tooltips } from './data/tooltips';
import { useAuth } from './store/AuthContext';
import { LoginScreen } from './components/LoginScreen';
import { ActionObserver } from './components/ActionObserver';

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
  } = useProject();
  const furnitureLoadKg = project?.loadKg ?? 0;
  const { t } = useTooltips();
  const handleLoadComplete = useCallback(() => setLoading(false), []);

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
      case 'designing':
        return (
          <WorkspaceLayout
            showLeft={false}
            center={<DesigningWorkspace />}
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
          <span className="text-[9px] font-mono text-forge-green">● 60 FPS</span>
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
    </div>
  );
};

export default ModernApp;
