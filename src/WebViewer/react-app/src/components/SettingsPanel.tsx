import React, { useState } from 'react';
import {
  Settings, X, Monitor, Cpu, Bell,
  Cloud, Keyboard,
  Sun, Moon, Zap, HardDrive, Wifi, Globe
} from 'lucide-react';

interface Props {
  isOpen: boolean;
  onClose: () => void;
}

type SettingsTab = 'display' | 'solver' | 'performance' | 'notifications' | 'sync' | 'shortcuts';

const tabs: { id: SettingsTab; label: string; icon: React.ReactNode }[] = [
  { id: 'display', label: 'Display', icon: <Monitor size={14} /> },
  { id: 'solver', label: 'Solver', icon: <Cpu size={14} /> },
  { id: 'performance', label: 'Performance', icon: <Zap size={14} /> },
  { id: 'notifications', label: 'Notifications', icon: <Bell size={14} /> },
  { id: 'sync', label: 'Sync & Cloud', icon: <Cloud size={14} /> },
  { id: 'shortcuts', label: 'Shortcuts', icon: <Keyboard size={14} /> },
];

const Toggle: React.FC<{ enabled: boolean; onChange: () => void }> = ({ enabled, onChange }) => (
  <button
    onClick={onChange}
    className={`w-9 h-5 rounded-full transition-all ${enabled ? 'bg-forge-accent' : 'bg-forge-surface'}`}
  >
    <div className={`w-3.5 h-3.5 rounded-full bg-white transition-all transform ${enabled ? 'translate-x-5' : 'translate-x-1'}`} />
  </button>
);

export const SettingsPanel: React.FC<Props> = ({ isOpen, onClose }) => {
  const [activeTab, setActiveTab] = useState<SettingsTab>('display');
  const [settings, setSettings] = useState({
    darkMode: true,
    gridVisible: true,
    axesVisible: true,
    antiAliasing: true,
    shadows: true,
    autoMesh: true,
    parallelSolve: true,
    gpuAccel: true,
    adaptiveMesh: true,
    notifications: true,
    soundAlerts: false,
    criticalOnly: false,
    cloudSync: true,
    autoSave: true,
    telemetrySync: true,
  });

  const toggle = (key: keyof typeof settings) => {
    setSettings(prev => ({ ...prev, [key]: !prev[key] }));
  };

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-[100] flex items-center justify-center">
      <div className="absolute inset-0 bg-black/60 backdrop-blur-sm" onClick={onClose} />
      
      <div className="relative w-[700px] h-[500px] bg-forge-panel border border-forge-border rounded-lg shadow-2xl flex overflow-hidden animate-fade-in">
        {/* Sidebar */}
        <div className="w-[180px] bg-forge-dark border-r border-forge-border flex flex-col">
          <div className="p-4 border-b border-forge-border/50">
            <div className="flex items-center gap-2">
              <Settings size={16} className="text-forge-accent" />
              <span className="text-[11px] font-bold tracking-widest text-forge-text">SETTINGS</span>
            </div>
          </div>
          <div className="flex-1 p-2 space-y-0.5">
            {tabs.map(tab => (
              <button
                key={tab.id}
                onClick={() => setActiveTab(tab.id)}
                className={`w-full flex items-center gap-2 px-3 py-2 rounded text-[10px] font-medium transition-all ${
                  activeTab === tab.id
                    ? 'bg-forge-accent/10 text-forge-accent'
                    : 'text-forge-text-dim hover:text-forge-text hover:bg-forge-surface/50'
                }`}
              >
                {tab.icon}
                {tab.label}
              </button>
            ))}
          </div>
          <div className="p-3 border-t border-forge-border/50">
            <span className="text-[8px] font-mono text-forge-text-muted">SZM Forge v4.2.1</span>
          </div>
        </div>

        {/* Content */}
        <div className="flex-1 flex flex-col">
          <div className="h-[48px] px-4 border-b border-forge-border flex items-center justify-between">
            <span className="text-[12px] font-semibold text-forge-text">
              {tabs.find(t => t.id === activeTab)?.label} Settings
            </span>
            <button onClick={onClose} className="p-1 rounded hover:bg-forge-surface text-forge-text-muted hover:text-forge-text-dim">
              <X size={16} />
            </button>
          </div>

          <div className="flex-1 overflow-y-auto p-4">
            {activeTab === 'display' && (
              <div className="space-y-4">
                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">THEME</span>
                  <div className="flex items-center gap-3 mt-3">
                    <button className="flex-1 p-3 rounded-lg border border-forge-accent/30 bg-forge-accent/5 flex items-center gap-2">
                      <Moon size={14} className="text-forge-accent" />
                      <span className="text-[10px] font-medium text-forge-text">Dark</span>
                    </button>
                    <button className="flex-1 p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20 flex items-center gap-2 opacity-50">
                      <Sun size={14} className="text-forge-text-muted" />
                      <span className="text-[10px] font-medium text-forge-text-muted">Light</span>
                    </button>
                  </div>
                </div>

                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">VIEWPORT</span>
                  <div className="mt-3 space-y-3">
                    {[
                      { key: 'gridVisible', label: 'Show Grid', desc: 'Display engineering grid in viewport' },
                      { key: 'axesVisible', label: 'Show Axes', desc: 'Display coordinate axes indicator' },
                      { key: 'antiAliasing', label: 'Anti-Aliasing', desc: 'Smooth edges (impacts performance)' },
                      { key: 'shadows', label: 'Real-time Shadows', desc: 'Enable dynamic shadow rendering' },
                    ].map(item => (
                      <div key={item.key} className="flex items-center justify-between">
                        <div>
                          <div className="text-[10px] font-medium text-forge-text">{item.label}</div>
                          <div className="text-[8px] text-forge-text-muted">{item.desc}</div>
                        </div>
                        <Toggle enabled={settings[item.key as keyof typeof settings] as boolean} onChange={() => toggle(item.key as keyof typeof settings)} />
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            )}

            {activeTab === 'solver' && (
              <div className="space-y-4">
                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">SOLVER ENGINE</span>
                  <div className="mt-3 space-y-3">
                    {[
                      { key: 'parallelSolve', label: 'Parallel Processing', desc: 'Use all available CPU cores' },
                      { key: 'gpuAccel', label: 'GPU Acceleration', desc: 'Use CUDA/OpenCL for computation' },
                      { key: 'autoMesh', label: 'Auto Mesh Refinement', desc: 'Automatically refine mesh at stress concentrations' },
                      { key: 'adaptiveMesh', label: 'Adaptive Time Stepping', desc: 'Automatically adjust time step for convergence' },
                    ].map(item => (
                      <div key={item.key} className="flex items-center justify-between">
                        <div>
                          <div className="text-[10px] font-medium text-forge-text">{item.label}</div>
                          <div className="text-[8px] text-forge-text-muted">{item.desc}</div>
                        </div>
                        <Toggle enabled={settings[item.key as keyof typeof settings] as boolean} onChange={() => toggle(item.key as keyof typeof settings)} />
                      </div>
                    ))}
                  </div>
                </div>

                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">CONVERGENCE</span>
                  <div className="mt-3 space-y-3">
                    <div>
                      <div className="flex items-center justify-between mb-1">
                        <span className="text-[10px] text-forge-text">Convergence Tolerance</span>
                        <span className="text-[9px] font-mono text-forge-accent">1e-6</span>
                      </div>
                      <input type="range" className="w-full h-1 bg-forge-surface rounded-full appearance-none cursor-pointer" />
                    </div>
                    <div>
                      <div className="flex items-center justify-between mb-1">
                        <span className="text-[10px] text-forge-text">Max Iterations</span>
                        <span className="text-[9px] font-mono text-forge-accent">1000</span>
                      </div>
                      <input type="range" className="w-full h-1 bg-forge-surface rounded-full appearance-none cursor-pointer" />
                    </div>
                  </div>
                </div>
              </div>
            )}

            {activeTab === 'performance' && (
              <div className="space-y-4">
                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">RESOURCE ALLOCATION</span>
                  <div className="mt-3 space-y-4">
                    <div>
                      <div className="flex items-center justify-between mb-1">
                        <span className="text-[10px] text-forge-text">CPU Threads</span>
                        <span className="text-[9px] font-mono text-forge-accent">32 / 32</span>
                      </div>
                      <div className="w-full h-2 bg-forge-surface rounded-full overflow-hidden">
                        <div className="h-full bg-gradient-to-r from-forge-accent to-forge-cyan rounded-full" style={{ width: '100%' }} />
                      </div>
                    </div>
                    <div>
                      <div className="flex items-center justify-between mb-1">
                        <span className="text-[10px] text-forge-text">Memory Limit</span>
                        <span className="text-[9px] font-mono text-forge-accent">48 GB</span>
                      </div>
                      <div className="w-full h-2 bg-forge-surface rounded-full overflow-hidden">
                        <div className="h-full bg-gradient-to-r from-forge-green to-forge-teal rounded-full" style={{ width: '75%' }} />
                      </div>
                    </div>
                    <div>
                      <div className="flex items-center justify-between mb-1">
                        <span className="text-[10px] text-forge-text">GPU Memory</span>
                        <span className="text-[9px] font-mono text-forge-accent">12 GB</span>
                      </div>
                      <div className="w-full h-2 bg-forge-surface rounded-full overflow-hidden">
                        <div className="h-full bg-gradient-to-r from-forge-purple to-forge-magenta rounded-full" style={{ width: '50%' }} />
                      </div>
                    </div>
                  </div>
                </div>

                <div className="grid grid-cols-3 gap-3">
                  <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/10 text-center">
                    <HardDrive size={18} className="text-forge-accent mx-auto mb-1" />
                    <div className="text-[14px] font-bold font-mono text-forge-text">847 GB</div>
                    <div className="text-[8px] text-forge-text-muted">SSD Cache</div>
                  </div>
                  <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/10 text-center">
                    <Wifi size={18} className="text-forge-green mx-auto mb-1" />
                    <div className="text-[14px] font-bold font-mono text-forge-text">1 Gbps</div>
                    <div className="text-[8px] text-forge-text-muted">Network</div>
                  </div>
                  <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/10 text-center">
                    <Globe size={18} className="text-forge-purple mx-auto mb-1" />
                    <div className="text-[14px] font-bold font-mono text-forge-text">4 Nodes</div>
                    <div className="text-[8px] text-forge-text-muted">Cluster</div>
                  </div>
                </div>
              </div>
            )}

            {activeTab === 'notifications' && (
              <div className="space-y-4">
                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">NOTIFICATION PREFERENCES</span>
                  <div className="mt-3 space-y-3">
                    {[
                      { key: 'notifications', label: 'Enable Notifications', desc: 'Show system notifications' },
                      { key: 'soundAlerts', label: 'Sound Alerts', desc: 'Play sound for critical notifications' },
                      { key: 'criticalOnly', label: 'Critical Only', desc: 'Only show critical alerts' },
                    ].map(item => (
                      <div key={item.key} className="flex items-center justify-between">
                        <div>
                          <div className="text-[10px] font-medium text-forge-text">{item.label}</div>
                          <div className="text-[8px] text-forge-text-muted">{item.desc}</div>
                        </div>
                        <Toggle enabled={settings[item.key as keyof typeof settings] as boolean} onChange={() => toggle(item.key as keyof typeof settings)} />
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            )}

            {activeTab === 'sync' && (
              <div className="space-y-4">
                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">CLOUD SYNC</span>
                  <div className="mt-3 space-y-3">
                    {[
                      { key: 'cloudSync', label: 'Cloud Synchronization', desc: 'Sync projects to SZM Cloud' },
                      { key: 'autoSave', label: 'Auto-Save', desc: 'Automatically save changes every 5 minutes' },
                      { key: 'telemetrySync', label: 'Telemetry Sync', desc: 'Stream live telemetry to cloud' },
                    ].map(item => (
                      <div key={item.key} className="flex items-center justify-between">
                        <div>
                          <div className="text-[10px] font-medium text-forge-text">{item.label}</div>
                          <div className="text-[8px] text-forge-text-muted">{item.desc}</div>
                        </div>
                        <Toggle enabled={settings[item.key as keyof typeof settings] as boolean} onChange={() => toggle(item.key as keyof typeof settings)} />
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            )}

            {activeTab === 'shortcuts' && (
              <div className="space-y-4">
                <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/10">
                  <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">KEYBOARD SHORTCUTS</span>
                  <div className="mt-3 space-y-2">
                    {[
                      { action: 'Run Simulation', keys: ['Ctrl', 'Enter'] },
                      { action: 'Save Project', keys: ['Ctrl', 'S'] },
                      { action: 'Toggle Wireframe', keys: ['W'] },
                      { action: 'Center View', keys: ['F'] },
                      { action: 'Section Cut', keys: ['X'] },
                      { action: 'Measure Tool', keys: ['M'] },
                      { action: 'Toggle AI Panel', keys: ['Ctrl', 'I'] },
                      { action: 'Toggle Timeline', keys: ['T'] },
                    ].map((shortcut, i) => (
                      <div key={i} className="flex items-center justify-between py-1.5 border-b border-forge-border/20">
                        <span className="text-[10px] text-forge-text">{shortcut.action}</span>
                        <div className="flex items-center gap-1">
                          {shortcut.keys.map((key, j) => (
                            <span key={j} className="px-2 py-0.5 rounded bg-forge-surface text-[9px] font-mono text-forge-accent border border-forge-border/50">
                              {key}
                            </span>
                          ))}
                        </div>
                      </div>
                    ))}
                  </div>
                </div>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
