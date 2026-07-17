import { useState, useEffect } from 'react';

interface BottomBarProps {
  simulationTime: number;
}

export default function BottomBar({ simulationTime }: BottomBarProps) {
  const [isPlaying, setIsPlaying] = useState(true);
  const [speed, setSpeed] = useState(1);
  const [activeTab, setActiveTab] = useState<'timeline' | 'console' | 'metrics'>('timeline');
  const [consoleLines, setConsoleLines] = useState<string[]>([
    '[14:21:00] Simulation engine initialized — Implicit Nonlinear Solver',
    '[14:21:02] Mesh loaded: 3,892,104 elements | 1,247,832 nodes',
    '[14:21:05] Material: Inconel 718 — Temperature-dependent properties loaded',
    '[14:21:08] Boundary conditions applied: 4 fixed supports, 245 kN distributed load',
    '[14:21:15] Load step 1/5 — Converged in 42 iterations (residual: 3.2e-7)',
    '[14:21:30] Load step 2/5 — Converged in 67 iterations (residual: 8.1e-7)',
    '[14:22:00] Load step 3/5 — Converged in 89 iterations (residual: 5.4e-7)',
    '[14:22:30] ⚠ Warning: Stress concentration at node 847,231 — σ_vm = 952 MPa',
    '[14:22:45] AI Module: Analyzing stress distribution patterns...',
    '[14:22:58] Load step 4/5 — Converged in 103 iterations (residual: 6.8e-7)',
    '[14:23:01] ✓ All load steps converged — Solution complete',
  ]);

  useEffect(() => {
    if (simulationTime % 5 === 0 && simulationTime > 0) {
      const messages = [
        `[${new Date().toLocaleTimeString('en-US', { hour12: false })}] Sensor update: Core temp ${(340 + Math.random() * 10).toFixed(1)}°C`,
        `[${new Date().toLocaleTimeString('en-US', { hour12: false })}] AI: Monitoring fatigue accumulation — ${(Math.random() * 0.1).toFixed(4)}% increment`,
        `[${new Date().toLocaleTimeString('en-US', { hour12: false })}] Telemetry sync: 8/8 channels active`,
        `[${new Date().toLocaleTimeString('en-US', { hour12: false })}] GPU render: ${(16 + Math.random() * 2).toFixed(1)}ms frame time`,
      ];
      setConsoleLines(prev => [...prev.slice(-50), messages[Math.floor(Math.random() * messages.length)]]);
    }
  }, [simulationTime]);

  const formatTime = (s: number) => {
    const mins = Math.floor(s / 60);
    const secs = s % 60;
    return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
  };

  const totalDuration = 300;
  const progress = (simulationTime % totalDuration) / totalDuration;

  const loadSteps = [
    { pos: 0.2, label: 'LS1', done: progress > 0.2 },
    { pos: 0.4, label: 'LS2', done: progress > 0.4 },
    { pos: 0.6, label: 'LS3', done: progress > 0.6 },
    { pos: 0.8, label: 'LS4', done: progress > 0.8 },
    { pos: 1.0, label: 'LS5', done: progress > 1.0 },
  ];

  return (
    <div className="h-36 bg-forge-surface border-t border-forge-border flex flex-col">
      {/* Tabs */}
      <div className="flex items-center border-b border-forge-border px-3">
        {(['timeline', 'console', 'metrics'] as const).map(tab => (
          <button
            key={tab}
            onClick={() => setActiveTab(tab)}
            className={`px-3 py-1.5 text-[9px] font-bold tracking-wider transition-all ${
              activeTab === tab
                ? 'text-forge-accent border-b-2 border-forge-accent'
                : 'text-forge-text-muted hover:text-forge-text-dim'
            }`}
          >
            {tab.toUpperCase()}
          </button>
        ))}
        
        <div className="flex-1" />
        
        {/* Playback controls */}
        <div className="flex items-center gap-2">
          <button className="text-forge-text-dim hover:text-forge-text text-xs transition-all">⏮</button>
          <button className="text-forge-text-dim hover:text-forge-text text-xs transition-all">⏪</button>
          <button 
            onClick={() => setIsPlaying(!isPlaying)}
            className="w-6 h-6 rounded-full bg-forge-accent/20 text-forge-accent flex items-center justify-center text-xs hover:bg-forge-accent/30 transition-all"
          >
            {isPlaying ? '⏸' : '▶'}
          </button>
          <button className="text-forge-text-dim hover:text-forge-text text-xs transition-all">⏩</button>
          <button className="text-forge-text-dim hover:text-forge-text text-xs transition-all">⏭</button>
          
          <div className="w-px h-4 bg-forge-border mx-1" />
          
          <div className="flex items-center gap-1">
            {[0.5, 1, 2, 4].map(s => (
              <button
                key={s}
                onClick={() => setSpeed(s)}
                className={`px-1.5 py-0.5 text-[8px] font-mono rounded transition-all ${
                  speed === s
                    ? 'bg-forge-accent/20 text-forge-accent'
                    : 'text-forge-text-muted hover:text-forge-text-dim'
                }`}
              >
                {s}x
              </button>
            ))}
          </div>
          
          <span className="text-[9px] font-mono text-forge-text ml-2">
            {formatTime(simulationTime)} / {formatTime(totalDuration)}
          </span>
        </div>
      </div>

      {/* Content */}
      <div className="flex-1 overflow-hidden">
        {activeTab === 'timeline' && (
          <div className="h-full px-3 py-2">
            {/* Timeline bar */}
            <div className="relative h-4 mb-2">
              <div className="absolute inset-0 bg-forge-panel rounded-full overflow-hidden">
                <div 
                  className="h-full rounded-full transition-all duration-500"
                  style={{
                    width: `${progress * 100}%`,
                    background: 'linear-gradient(90deg, #00c2ff, #8855ff)',
                  }}
                />
              </div>
              
              {/* Load step markers */}
              {loadSteps.map((ls, i) => (
                <div
                  key={i}
                  className="absolute top-0 h-full flex flex-col items-center"
                  style={{ left: `${ls.pos * 100}%` }}
                >
                  <div className={`w-2 h-2 rounded-full border mt-1 ${
                    ls.done ? 'bg-forge-green border-forge-green' : 'bg-forge-panel border-forge-border-light'
                  }`} />
                  <span className="text-[7px] text-forge-text-muted mt-1">{ls.label}</span>
                </div>
              ))}
            </div>

            {/* Mini visualization bars */}
            <div className="grid grid-cols-5 gap-2 h-12">
              {['Stress', 'Thermal', 'Displacement', 'Strain', 'Energy'].map((label, idx) => (
                <div key={label} className="flex flex-col">
                  <span className="text-[7px] text-forge-text-muted tracking-wider mb-1">{label.toUpperCase()}</span>
                  <div className="flex-1 flex items-end gap-px">
                    {Array.from({ length: 20 }, (_, i) => {
                      const h = 20 + Math.sin(i * 0.5 + idx * 1.5 + simulationTime * 0.1) * 15 + Math.random() * 10;
                      return (
                        <div
                          key={i}
                          className="flex-1 rounded-sm transition-all duration-300"
                          style={{
                            height: `${h}%`,
                            backgroundColor: idx === 0 ? 'rgba(255, 51, 68, 0.6)' :
                                           idx === 1 ? 'rgba(255, 102, 0, 0.6)' :
                                           idx === 2 ? 'rgba(0, 194, 255, 0.6)' :
                                           idx === 3 ? 'rgba(136, 85, 255, 0.6)' :
                                           'rgba(0, 255, 136, 0.6)',
                          }}
                        />
                      );
                    })}
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {activeTab === 'console' && (
          <div className="h-full overflow-y-auto p-2 font-mono text-[9px] space-y-0.5">
            {consoleLines.map((line, i) => (
              <div 
                key={i} 
                className={`leading-relaxed ${
                  line.includes('⚠') ? 'text-forge-warning' :
                  line.includes('✓') ? 'text-forge-green' :
                  line.includes('AI') ? 'text-forge-purple' :
                  'text-forge-text-dim'
                }`}
              >
                {line}
              </div>
            ))}
          </div>
        )}

        {activeTab === 'metrics' && (
          <div className="h-full p-2 grid grid-cols-6 gap-2">
            {[
              { label: 'SOLVER ITERATIONS', value: '342', sub: 'Converged' },
              { label: 'RESIDUAL NORM', value: '8.2e-7', sub: '< 1e-6 target' },
              { label: 'WALL CLOCK', value: '02:01', sub: 'minutes' },
              { label: 'CPU CORES', value: '64/64', sub: 'Active' },
              { label: 'GPU MEMORY', value: '18.4 GB', sub: '/ 24 GB' },
              { label: 'MESH QUALITY', value: '97.3%', sub: 'Excellent' },
            ].map(m => (
              <div key={m.label} className="glass-panel rounded-lg p-2 flex flex-col justify-center items-center text-center">
                <span className="text-[7px] text-forge-text-muted tracking-wider mb-1">{m.label}</span>
                <span className="text-sm font-bold text-forge-text font-mono">{m.value}</span>
                <span className="text-[7px] text-forge-green">{m.sub}</span>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
