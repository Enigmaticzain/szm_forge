import { useState, useCallback } from 'react';
import Header from './components/Header';
import SidePanel from './components/SidePanel';
import Viewport3D from './components/Viewport3D';
import TelemetryPanel from './components/TelemetryPanel';
import StressLegend from './components/StressLegend';
import AIInsightsPanel from './components/AIInsightsPanel';
import BottomBar from './components/BottomBar';
import OverviewDashboard from './components/OverviewDashboard';
import DigitalTwinView from './components/DigitalTwinView';
import ManufacturingView from './components/ManufacturingView';
import ElectricalView from './components/ElectricalView';
import CircuitDesigner from './components/CircuitDesigner';
import ChemistryLabView from './components/ChemistryLabView';
import LoadingScreen from './components/LoadingScreen';
import { useSimulationData } from './hooks/useSimulationData';

export default function App() {
  const [isLoading, setIsLoading] = useState(true);
  const [activeView, setActiveView] = useState('overview');
  const [viewHistory, setViewHistory] = useState<string[]>(['overview']);
  const [historyIndex, setHistoryIndex] = useState(0);
  const { sensors, electricalField, simulationTime } = useSimulationData();

  const handleLoadComplete = useCallback(() => {
    setIsLoading(false);
  }, []);

  const changeActiveView = useCallback((nextView: string) => {
    if (!nextView || nextView === activeView) {
      return;
    }

    const nextHistory = historyIndex < viewHistory.length - 1
      ? viewHistory.slice(0, historyIndex + 1).concat(nextView)
      : viewHistory.concat(nextView);

    setViewHistory(nextHistory);
    setHistoryIndex(nextHistory.length - 1);
    setActiveView(nextView);
  }, [activeView, historyIndex, viewHistory]);

  const handleUndoView = useCallback(() => {
    if (historyIndex === 0) {
      return;
    }

    const previousIndex = historyIndex - 1;
    setHistoryIndex(previousIndex);
    setActiveView(viewHistory[previousIndex]);
  }, [historyIndex, viewHistory]);

  const handleRedoView = useCallback(() => {
    if (historyIndex >= viewHistory.length - 1) {
      return;
    }

    const nextIndex = historyIndex + 1;
    setHistoryIndex(nextIndex);
    setActiveView(viewHistory[nextIndex]);
  }, [historyIndex, viewHistory]);

  if (isLoading) {
    return <LoadingScreen onComplete={handleLoadComplete} />;
  }

  const renderMainContent = () => {
    switch (activeView) {
      case 'overview':
        return <OverviewDashboard sensors={sensors} simulationTime={simulationTime} />;
      
      case 'stress':
        return (
          <div className="flex h-full">
            <div className="flex-1 relative">
              <Viewport3D mode="stress" />
              <div className="absolute bottom-3 left-3">
                <StressLegend mode="stress" />
              </div>
            </div>
            <div className="w-64">
              <AIInsightsPanel />
            </div>
          </div>
        );
      
      case 'thermal':
        return (
          <div className="flex h-full">
            <div className="flex-1 relative">
              <Viewport3D mode="thermal" />
              <div className="absolute bottom-3 left-3">
                <StressLegend mode="thermal" />
              </div>
            </div>
            <div className="w-64">
              <TelemetryPanel sensors={sensors.filter(s => 
                s.id.includes('temp') || s.id.includes('flow') || s.id.includes('press')
              )} />
            </div>
          </div>
        );
      
      case 'electrical':
        return <ElectricalView />;
      
      case 'circuit-designer':
        return <CircuitDesigner />;
      
      case 'digital-twin':
        return <DigitalTwinView sensors={sensors} simulationTime={simulationTime} />;
      
      case 'manufacturing':
        return <ManufacturingView />;

      case 'chemistry-lab':
        return <ChemistryLabView />;
      
      default:
        return <OverviewDashboard sensors={sensors} simulationTime={simulationTime} />;
    }
  };

  return (
    <div className="h-screen w-screen flex flex-col bg-forge-bg overflow-hidden">
      <Header
        activeView={activeView}
        onViewChange={changeActiveView}
        onUndo={handleUndoView}
        onRedo={handleRedoView}
        canUndo={historyIndex > 0}
        canRedo={historyIndex < viewHistory.length - 1}
      />
      
      <div className="flex-1 flex overflow-hidden">
        <SidePanel activeView={activeView} />
        
        <div className="flex-1 flex flex-col overflow-hidden">
          {/* Main viewport area */}
          <div className="flex-1 overflow-hidden relative">
            {renderMainContent()}
            
            {/* Subtle corner vignette overlay */}
            <div className="absolute inset-0 pointer-events-none" 
              style={{
                background: 'radial-gradient(ellipse at center, transparent 50%, rgba(10,12,16,0.3) 100%)',
              }}
            />
          </div>
          
          {/* Bottom bar */}
          <BottomBar simulationTime={simulationTime} />
        </div>
      </div>

      {/* Status bar at the very bottom */}
      <StatusBar simulationTime={simulationTime} sensors={sensors} />
    </div>
  );
}

function StatusBar({ simulationTime, sensors }: { simulationTime: number; sensors: { status: string }[] }) {
  const criticalCount = sensors.filter(s => s.status === 'critical').length;
  const warningCount = sensors.filter(s => s.status === 'warning').length;

  return (
    <div className="h-6 bg-forge-surface border-t border-forge-border flex items-center px-4 gap-6 text-[8px] text-forge-text-muted">
      <div className="flex items-center gap-1.5">
        <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse-glow" />
        <span className="tracking-wider">ENGINE: ACTIVE</span>
      </div>
      
      <div className="w-px h-3 bg-forge-border" />
      
      <span className="font-mono">SIM TIME: {simulationTime}s</span>
      
      <div className="w-px h-3 bg-forge-border" />
      
      <span className="font-mono">MESH: 3.89M ELEM</span>
      
      <div className="w-px h-3 bg-forge-border" />
      
      <span className="font-mono">SOLVER: CONVERGED</span>
      
      <div className="w-px h-3 bg-forge-border" />
      
      <div className="flex items-center gap-1">
        {criticalCount > 0 && (
          <span className="text-forge-danger font-bold">{criticalCount} CRITICAL</span>
        )}
        {warningCount > 0 && (
          <span className="text-forge-warning font-bold ml-2">{warningCount} WARNING</span>
        )}
        {criticalCount === 0 && warningCount === 0 && (
          <span className="text-forge-green">ALL NOMINAL</span>
        )}
      </div>
      
      <div className="flex-1" />
      
      <span className="tracking-wider">SZM FORGE v2.4.0</span>
      <span className="text-forge-accent">|</span>
      <span className="tracking-wider">GPU: NVIDIA RTX 6000</span>
      <span className="text-forge-accent">|</span>
      <span className="tracking-wider">RENDER: WebGPU</span>
    </div>
  );
}
