import { useState } from 'react';

interface SidePanelProps {
  activeView: string;
}

interface LayerConfig {
  id: string;
  name: string;
  active: boolean;
  color: string;
  opacity: number;
}

const defaultLayers: LayerConfig[] = [
  { id: 'geometry', name: 'CAD Geometry', active: true, color: '#00c2ff', opacity: 100 },
  { id: 'stress', name: 'Stress Overlay', active: true, color: '#ff3344', opacity: 80 },
  { id: 'thermal', name: 'Thermal Map', active: false, color: '#ff6600', opacity: 75 },
  { id: 'electrical', name: 'Electrical Flow', active: false, color: '#00eeff', opacity: 70 },
  { id: 'sensors', name: 'Sensor Network', active: true, color: '#00ff88', opacity: 90 },
  { id: 'ai-insights', name: 'AI Insights', active: true, color: '#8855ff', opacity: 85 },
  { id: 'annotations', name: 'Annotations', active: true, color: '#ffaa00', opacity: 100 },
  { id: 'manufacturing', name: 'Mfg Overlay', active: false, color: '#ff55aa', opacity: 70 },
];

export default function SidePanel({ activeView }: SidePanelProps) {
  const [layers, setLayers] = useState(defaultLayers);
  const [expandedSection, setExpandedSection] = useState<string>('layers');

  const toggleLayer = (id: string) => {
    setLayers(prev => prev.map(l => l.id === id ? { ...l, active: !l.active } : l));
  };

  const sections = [
    { id: 'layers', label: 'RENDER LAYERS', icon: '◇' },
    { id: 'properties', label: 'PROPERTIES', icon: '◆' },
    { id: 'analysis', label: 'ANALYSIS', icon: '◈' },
    { id: 'history', label: 'SIM HISTORY', icon: '◉' },
  ];

  return (
    <aside className="w-56 bg-forge-surface border-r border-forge-border flex flex-col overflow-hidden">
      {/* Section Tabs */}
      <div className="flex border-b border-forge-border">
        {sections.map(s => (
          <button
            key={s.id}
            onClick={() => setExpandedSection(s.id)}
            className={`flex-1 py-2 text-[8px] font-bold tracking-wider text-center transition-all ${
              expandedSection === s.id
                ? 'text-forge-accent border-b-2 border-forge-accent bg-forge-accent/5'
                : 'text-forge-text-muted hover:text-forge-text-dim'
            }`}
          >
            <div className="text-sm mb-0.5">{s.icon}</div>
            {s.label}
          </button>
        ))}
      </div>

      <div className="flex-1 overflow-y-auto p-3 space-y-3">
        {expandedSection === 'layers' && (
          <>
            <div className="text-[9px] text-forge-text-muted tracking-wider mb-2">
              RENDERING PIPELINE — {layers.filter(l => l.active).length}/{layers.length} ACTIVE
            </div>
            {layers.map(layer => (
              <div key={layer.id} className="glass-panel rounded-lg p-2.5 space-y-2">
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <button
                      onClick={() => toggleLayer(layer.id)}
                      className={`w-3 h-3 rounded-sm border transition-all ${
                        layer.active
                          ? 'border-forge-accent bg-forge-accent'
                          : 'border-forge-border-light bg-transparent'
                      }`}
                    />
                    <span className="text-[10px] font-medium text-forge-text">{layer.name}</span>
                  </div>
                  <div className="w-2 h-2 rounded-full" style={{ backgroundColor: layer.color, opacity: layer.active ? 1 : 0.3 }} />
                </div>
                {layer.active && (
                  <div className="flex items-center gap-2">
                    <span className="text-[8px] text-forge-text-muted w-6">α</span>
                    <div className="flex-1 h-1 bg-forge-panel rounded-full overflow-hidden">
                      <div 
                        className="h-full rounded-full transition-all"
                        style={{ width: `${layer.opacity}%`, backgroundColor: layer.color }}
                      />
                    </div>
                    <span className="text-[8px] text-forge-text-dim w-7 text-right">{layer.opacity}%</span>
                  </div>
                )}
              </div>
            ))}
          </>
        )}

        {expandedSection === 'properties' && (
          <div className="space-y-3">
            <div className="text-[9px] text-forge-text-muted tracking-wider mb-2">
              ACTIVE COMPONENT PROPERTIES
            </div>
            <PropertyGroup label="GEOMETRY" items={[
              { key: 'Type', value: 'Turbine Assembly' },
              { key: 'Nodes', value: '1,247,832' },
              { key: 'Elements', value: '3,892,104' },
              { key: 'Material', value: 'Inconel 718' },
            ]} />
            <PropertyGroup label="MATERIAL" items={[
              { key: 'Yield Strength', value: '1,034 MPa' },
              { key: 'UTS', value: '1,241 MPa' },
              { key: "Young's Mod.", value: '205 GPa' },
              { key: "Poisson's", value: '0.294' },
              { key: 'Density', value: '8,190 kg/m³' },
              { key: 'Thermal Cond.', value: '11.4 W/m·K' },
            ]} />
            <PropertyGroup label="BOUNDARY CONDITIONS" items={[
              { key: 'Fixed Supports', value: '4' },
              { key: 'Applied Load', value: '245 kN' },
              { key: 'Temperature', value: '342 °C' },
              { key: 'Pressure', value: '14.7 MPa' },
            ]} />
          </div>
        )}

        {expandedSection === 'analysis' && (
          <div className="space-y-3">
            <div className="text-[9px] text-forge-text-muted tracking-wider mb-2">
              {activeView.toUpperCase()} ANALYSIS CONFIG
            </div>
            <PropertyGroup label="SOLVER" items={[
              { key: 'Method', value: 'Implicit Nonlinear' },
              { key: 'Convergence', value: '1e-6' },
              { key: 'Max Iterations', value: '500' },
              { key: 'Status', value: 'Converged ✓', highlight: true },
            ]} />
            <PropertyGroup label="MESH QUALITY" items={[
              { key: 'Aspect Ratio', value: '< 3.2' },
              { key: 'Jacobian', value: '> 0.65' },
              { key: 'Skewness', value: '< 0.72' },
              { key: 'Quality', value: 'Excellent', highlight: true },
            ]} />
          </div>
        )}

        {expandedSection === 'history' && (
          <div className="space-y-2">
            <div className="text-[9px] text-forge-text-muted tracking-wider mb-2">SIMULATION LOG</div>
            {[
              { time: '14:23:01', msg: 'Stress convergence achieved', type: 'success' },
              { time: '14:22:58', msg: 'Iteration 342/500 — residual: 8.2e-7', type: 'info' },
              { time: '14:22:45', msg: 'Thermal coupling updated', type: 'info' },
              { time: '14:22:30', msg: 'Warning: Local stress concentration detected', type: 'warning' },
              { time: '14:22:15', msg: 'Mesh refinement zone activated', type: 'info' },
              { time: '14:21:58', msg: 'AI: Recommend fillet radius increase R≥3mm', type: 'ai' },
              { time: '14:21:30', msg: 'Load step 3/5 complete', type: 'success' },
              { time: '14:21:00', msg: 'Nonlinear solver initialized', type: 'info' },
            ].map((log, i) => (
              <div key={i} className="glass-panel rounded p-2 flex items-start gap-2">
                <div className={`w-1.5 h-1.5 rounded-full mt-1 flex-shrink-0 ${
                  log.type === 'success' ? 'bg-forge-green' :
                  log.type === 'warning' ? 'bg-forge-warning' :
                  log.type === 'ai' ? 'bg-forge-purple' :
                  'bg-forge-accent'
                }`} />
                <div>
                  <div className="text-[8px] text-forge-text-muted font-mono">{log.time}</div>
                  <div className="text-[9px] text-forge-text-dim leading-tight">{log.msg}</div>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Bottom Status */}
      <div className="border-t border-forge-border p-3 space-y-2">
        <div className="flex items-center justify-between">
          <span className="text-[8px] text-forge-text-muted tracking-wider">VIEWPORT</span>
          <span className="text-[9px] text-forge-text-dim font-mono">60 FPS</span>
        </div>
        <div className="flex gap-1">
          {['X', 'Y', 'Z'].map(axis => (
            <div key={axis} className="flex-1 text-center py-1 bg-forge-panel rounded text-[8px] text-forge-text-muted">
              {axis}: {(Math.random() * 100).toFixed(1)}
            </div>
          ))}
        </div>
      </div>
    </aside>
  );
}

function PropertyGroup({ label, items }: { label: string; items: { key: string; value: string; highlight?: boolean }[] }) {
  return (
    <div className="glass-panel rounded-lg p-2.5">
      <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">{label}</div>
      <div className="space-y-1.5">
        {items.map(item => (
          <div key={item.key} className="flex justify-between items-center">
            <span className="text-[9px] text-forge-text-muted">{item.key}</span>
            <span className={`text-[9px] font-mono ${item.highlight ? 'text-forge-green' : 'text-forge-text'}`}>
              {item.value}
            </span>
          </div>
        ))}
      </div>
    </div>
  );
}
