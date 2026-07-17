import React, { useState, useCallback } from 'react';
import { useForgeStore } from './store/useForgeStore';
import { TopCommandBar } from './components/TopCommandBar';
import { ProjectHierarchy } from './components/ProjectHierarchy';
import { Viewport3D } from './components/Viewport3D';
import { AnalysisPanel } from './components/AnalysisPanel';
import { TimelineBar } from './components/TimelineBar';
import { DigitalTwinDashboard } from './components/DigitalTwinDashboard';
import { ManufacturingDashboard } from './components/ManufacturingDashboard';
import { AIWorkspace } from './components/AIWorkspace';
import { FleetCommand } from './components/FleetCommand';
import { SimulationWorkspace } from './components/SimulationWorkspace';
import { LoadingScreen } from './components/LoadingScreen';
import { PropertyInspector } from './components/PropertyInspector';
import { NotificationCenter } from './components/NotificationCenter';
import { SettingsPanel } from './components/SettingsPanel';

const App: React.FC = () => {
  const [loading, setLoading] = useState(true);
  const [showNotifications, setShowNotifications] = useState(false);
  const [showSettings, setShowSettings] = useState(false);
  const store = useForgeStore();
  const handleLoadComplete = useCallback(() => setLoading(false), []);

  const renderMainContent = () => {
    switch (store.workspace) {
      case 'digital-twin':
        return <DigitalTwinDashboard />;
      case 'manufacturing':
        return <ManufacturingDashboard />;
      case 'ai-assistant':
        return <AIWorkspace />;
      case 'fleet-command':
        return <FleetCommand />;
      case 'simulation':
        return <SimulationWorkspace renderMode={store.renderMode} />;
      case 'engineering':
      default:
        return (
          <div className="flex-1 flex overflow-hidden">
            {/* Left Panel - Project Hierarchy + Properties */}
            <div className="flex-shrink-0 flex flex-col" style={{ width: store.leftPanelWidth }}>
              <div className="flex-1 overflow-hidden">
                <ProjectHierarchy
                  selectedNode={store.selectedNode}
                  setSelectedNode={store.setSelectedNode}
                />
              </div>
              <PropertyInspector selectedNode={store.selectedNode} />
            </div>

            {/* Center - 3D Viewport */}
            <div className="flex-1 relative">
              <Viewport3D renderMode={store.renderMode} />
            </div>

            {/* Right Panel - Analysis */}
            {store.showAIPanel && (
              <div className="flex-shrink-0" style={{ width: store.rightPanelWidth }}>
                <AnalysisPanel />
              </div>
            )}
          </div>
        );
    }
  };

  // Determine if we should show the bottom timeline
  const showBottomTimeline = store.workspace === 'engineering' || store.workspace === 'simulation';

  if (loading) {
    return <LoadingScreen onComplete={handleLoadComplete} />;
  }

  return (
    <div className="w-full h-full flex flex-col bg-forge-black overflow-hidden">
      {/* Top Command Bar */}
      <TopCommandBar
        workspace={store.workspace}
        setWorkspace={store.setWorkspace}
        renderMode={store.renderMode}
        setRenderMode={store.setRenderMode}
        solverProgress={store.solverProgress}
        simulationRunning={store.simulationStatus === 'solving' || store.simulationStatus === 'meshing'}
        onOpenNotifications={() => setShowNotifications(true)}
        onOpenSettings={() => setShowSettings(true)}
        notificationCount={3}
      />

      {/* Main Content Area */}
      <div className="flex-1 flex flex-col overflow-hidden">
        <div className="flex-1 flex overflow-hidden">
          {renderMainContent()}
        </div>

        {/* Bottom Timeline */}
        {showBottomTimeline && (
          <TimelineBar
            visible={store.showTimeline}
            onToggle={store.toggleTimeline}
          />
        )}
      </div>

      {/* Status Bar */}
      <div className="h-[22px] bg-forge-dark border-t border-forge-border flex items-center px-3 justify-between flex-shrink-0">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-1">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green" />
            <span className="text-[8px] font-mono text-forge-text-muted">SYSTEM ONLINE</span>
          </div>
          <span className="text-[8px] font-mono text-forge-text-muted">
            PROJECT: Turbine Assembly v4.2
          </span>
          <span className="text-[8px] font-mono text-forge-text-muted">
            WORKSPACE: {store.workspace.toUpperCase().replace('-', ' ')}
          </span>
        </div>
        <div className="flex items-center gap-4">
          <span className="text-[8px] font-mono text-forge-text-muted">
            RENDER: {store.renderMode.toUpperCase()}
          </span>
          <span className="text-[8px] font-mono text-forge-text-muted">
            NODES: 2.8M
          </span>
          <span className="text-[8px] font-mono text-forge-green">
            ● 60 FPS
          </span>
          <span className="text-[8px] font-mono text-forge-text-muted">
            SZM FORGE v4.2.1
          </span>
        </div>
      </div>

      {/* Modals */}
      <NotificationCenter isOpen={showNotifications} onClose={() => setShowNotifications(false)} />
      <SettingsPanel isOpen={showSettings} onClose={() => setShowSettings(false)} />
    </div>
  );
};

export default App;
