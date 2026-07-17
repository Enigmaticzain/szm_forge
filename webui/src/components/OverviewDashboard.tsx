import { useState, useEffect } from 'react';
import { SensorData } from '../hooks/useSimulationData';
import OptimizationChart from './OptimizationChart';
import { useOptimizationData } from '../hooks/useSimulationData';

interface OverviewDashboardProps {
  sensors: SensorData[];
  simulationTime: number;
}

export default function OverviewDashboard({ sensors, simulationTime }: OverviewDashboardProps) {
  const { generations } = useOptimizationData();
  const [systemMetrics, setSystemMetrics] = useState({
    cpuUsage: 78,
    gpuUsage: 67,
    memoryUsage: 62,
    networkIO: 340,
  });

  useEffect(() => {
    setSystemMetrics(prev => ({
      cpuUsage: Math.max(40, Math.min(95, prev.cpuUsage + (Math.random() - 0.5) * 5)),
      gpuUsage: Math.max(30, Math.min(90, prev.gpuUsage + (Math.random() - 0.5) * 4)),
      memoryUsage: Math.max(50, Math.min(85, prev.memoryUsage + (Math.random() - 0.5) * 2)),
      networkIO: Math.max(200, Math.min(500, prev.networkIO + (Math.random() - 0.5) * 30)),
    }));
  }, [simulationTime]);

  return (
    <div className="h-full grid grid-cols-4 grid-rows-3 gap-2 p-2 overflow-hidden">
      {/* System Health - Large Card */}
      <div className="col-span-1 row-span-1 glass-panel rounded-lg p-3 flex flex-col">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">SYSTEM HEALTH</div>
        <div className="flex-1 flex items-center justify-center">
          <div className="relative w-24 h-24">
            <svg viewBox="0 0 100 100" className="w-full h-full -rotate-90">
              <circle cx="50" cy="50" r="42" fill="none" stroke="rgba(30,34,48,0.5)" strokeWidth="6" />
              <circle cx="50" cy="50" r="42" fill="none" stroke="url(#healthGrad)" strokeWidth="6"
                strokeDasharray={`${94.2 * 263.89 / 100} 263.89`}
                strokeLinecap="round"
              />
              <defs>
                <linearGradient id="healthGrad" x1="0" y1="0" x2="1" y2="0">
                  <stop offset="0%" stopColor="#00ff88" />
                  <stop offset="100%" stopColor="#00c2ff" />
                </linearGradient>
              </defs>
            </svg>
            <div className="absolute inset-0 flex flex-col items-center justify-center">
              <span className="text-2xl font-bold text-forge-green font-mono">94.2</span>
              <span className="text-[7px] text-forge-text-muted tracking-wider">SCORE</span>
            </div>
          </div>
        </div>
        <div className="grid grid-cols-2 gap-1 mt-1">
          <div className="text-center p-1 bg-forge-panel/50 rounded">
            <div className="text-[7px] text-forge-text-muted">UPTIME</div>
            <div className="text-[10px] font-bold text-forge-green font-mono">99.7%</div>
          </div>
          <div className="text-center p-1 bg-forge-panel/50 rounded">
            <div className="text-[7px] text-forge-text-muted">ALERTS</div>
            <div className="text-[10px] font-bold text-forge-warning font-mono">2</div>
          </div>
        </div>
      </div>

      {/* Key Sensors Grid */}
      <div className="col-span-2 row-span-1 glass-panel rounded-lg p-3">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">PRIMARY TELEMETRY</div>
        <div className="grid grid-cols-4 gap-2 h-[calc(100%-20px)]">
          {sensors.slice(0, 4).map(sensor => {
            const statusColor = sensor.status === 'critical' ? '#ff3344' :
                               sensor.status === 'warning' ? '#ffaa00' : '#00ff88';
            const ratio = sensor.value / sensor.max;
            
            return (
              <div key={sensor.id} className="flex flex-col justify-between p-2 bg-forge-panel/30 rounded-lg border border-forge-border/30">
                <div className="flex items-center justify-between">
                  <span className="text-[7px] text-forge-text-muted tracking-wider uppercase">{sensor.name}</span>
                  <div className="w-1.5 h-1.5 rounded-full" style={{ backgroundColor: statusColor }} />
                </div>
                <div>
                  <div className="text-lg font-bold text-forge-text font-mono tabular-nums">{sensor.value.toFixed(1)}</div>
                  <div className="text-[8px] text-forge-text-muted">{sensor.unit}</div>
                </div>
                <div className="h-1 bg-forge-panel rounded-full overflow-hidden">
                  <div className="h-full rounded-full transition-all duration-500"
                    style={{ width: `${ratio * 100}%`, backgroundColor: statusColor }} />
                </div>
                {/* Mini sparkline */}
                <div className="h-6 mt-1">
                  <svg viewBox="0 0 100 30" className="w-full h-full" preserveAspectRatio="none">
                    <polyline
                      points={sensor.history.slice(-20).map((p, i) => {
                        const x = (i / 19) * 100;
                        const y = 30 - ((p.value - sensor.min) / (sensor.max - sensor.min)) * 30;
                        return `${x},${y}`;
                      }).join(' ')}
                      fill="none"
                      stroke={statusColor}
                      strokeWidth="1.5"
                      vectorEffect="non-scaling-stroke"
                    />
                  </svg>
                </div>
              </div>
            );
          })}
        </div>
      </div>

      {/* Compute Resources */}
      <div className="col-span-1 row-span-1 glass-panel rounded-lg p-3">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">COMPUTE RESOURCES</div>
        <div className="space-y-3">
          {[
            { label: 'CPU', value: systemMetrics.cpuUsage, color: '#00c2ff', detail: '64 cores' },
            { label: 'GPU', value: systemMetrics.gpuUsage, color: '#00ff88', detail: 'RTX 6000' },
            { label: 'MEM', value: systemMetrics.memoryUsage, color: '#8855ff', detail: '128 GB' },
            { label: 'NET', value: systemMetrics.networkIO / 5, color: '#ffaa00', detail: `${systemMetrics.networkIO.toFixed(0)} MB/s` },
          ].map(r => (
            <div key={r.label} className="space-y-1">
              <div className="flex justify-between text-[8px]">
                <span className="text-forge-text-dim font-bold">{r.label}</span>
                <span className="text-forge-text font-mono">{r.value.toFixed(0)}% — {r.detail}</span>
              </div>
              <div className="h-1.5 bg-forge-panel rounded-full overflow-hidden">
                <div className="h-full rounded-full transition-all duration-500"
                  style={{ width: `${r.value}%`, backgroundColor: r.color }} />
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Optimization Evolution */}
      <div className="col-span-2 row-span-1">
        <OptimizationChart generations={generations} />
      </div>

      {/* Simulation Status */}
      <div className="col-span-1 row-span-1 glass-panel rounded-lg p-3">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">SIMULATION STATUS</div>
        <div className="space-y-2">
          {[
            { label: 'FEA Solver', status: 'running', progress: 85 },
            { label: 'Thermal Analysis', status: 'running', progress: 72 },
            { label: 'CFD Simulation', status: 'queued', progress: 0 },
            { label: 'Fatigue Analysis', status: 'complete', progress: 100 },
            { label: 'Modal Analysis', status: 'complete', progress: 100 },
          ].map(sim => (
            <div key={sim.label} className="space-y-1">
              <div className="flex justify-between items-center">
                <span className="text-[8px] text-forge-text-dim">{sim.label}</span>
                <span className={`text-[7px] font-bold tracking-wider ${
                  sim.status === 'complete' ? 'text-forge-green' :
                  sim.status === 'running' ? 'text-forge-accent' :
                  'text-forge-text-muted'
                }`}>
                  {sim.status === 'complete' ? '✓ DONE' : sim.status === 'running' ? '▶ RUN' : '○ QUEUE'}
                </span>
              </div>
              <div className="h-1 bg-forge-panel rounded-full overflow-hidden">
                <div className="h-full rounded-full transition-all duration-700"
                  style={{
                    width: `${sim.progress}%`,
                    background: sim.status === 'complete' ? '#00ff88' :
                               sim.status === 'running' ? 'linear-gradient(90deg, #00c2ff, #8855ff)' :
                               '#333',
                  }} />
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Active Alerts */}
      <div className="col-span-1 row-span-1 glass-panel rounded-lg p-3 flex flex-col">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">ACTIVE ALERTS</div>
        <div className="flex-1 overflow-y-auto space-y-1.5">
          {[
            { severity: 'critical', msg: 'Thermal limit exceeded — Zone B3', time: '2m ago' },
            { severity: 'warning', msg: 'Shaft torque approaching limit', time: '8m ago' },
            { severity: 'info', msg: 'Bearing vibration trend upward', time: '15m ago' },
            { severity: 'info', msg: 'AI: Optimization gen. 342 complete', time: '22m ago' },
          ].map((alert, i) => (
            <div key={i} className={`p-2 rounded border ${
              alert.severity === 'critical' ? 'border-forge-danger/30 bg-forge-danger/5' :
              alert.severity === 'warning' ? 'border-forge-warning/20 bg-forge-warning/5' :
              'border-forge-accent/10 bg-forge-accent/3'
            }`}>
              <div className="flex items-center justify-between mb-0.5">
                <span className={`text-[7px] font-bold tracking-wider ${
                  alert.severity === 'critical' ? 'text-forge-danger' :
                  alert.severity === 'warning' ? 'text-forge-warning' :
                  'text-forge-accent'
                }`}>
                  {alert.severity.toUpperCase()}
                </span>
                <span className="text-[7px] text-forge-text-muted">{alert.time}</span>
              </div>
              <p className="text-[8px] text-forge-text-dim leading-relaxed">{alert.msg}</p>
            </div>
          ))}
        </div>
      </div>

      {/* Secondary Sensors */}
      <div className="col-span-2 row-span-1 glass-panel rounded-lg p-3">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">SECONDARY TELEMETRY</div>
        <div className="grid grid-cols-4 gap-2 h-[calc(100%-20px)]">
          {sensors.slice(4, 8).map(sensor => {
            const statusColor = sensor.status === 'critical' ? '#ff3344' :
                               sensor.status === 'warning' ? '#ffaa00' : '#00ff88';
            
            return (
              <div key={sensor.id} className="flex flex-col justify-between p-2 bg-forge-panel/30 rounded-lg border border-forge-border/30">
                <div className="flex items-center justify-between">
                  <span className="text-[7px] text-forge-text-muted tracking-wider uppercase">{sensor.name}</span>
                  <div className="w-1.5 h-1.5 rounded-full" style={{ backgroundColor: statusColor }} />
                </div>
                <div>
                  <div className="text-lg font-bold text-forge-text font-mono tabular-nums">{sensor.value.toFixed(1)}</div>
                  <div className="text-[8px] text-forge-text-muted">{sensor.unit}</div>
                </div>
                <div className="h-6 mt-1">
                  <svg viewBox="0 0 100 30" className="w-full h-full" preserveAspectRatio="none">
                    <polyline
                      points={sensor.history.slice(-20).map((p, i) => {
                        const x = (i / 19) * 100;
                        const y = 30 - ((p.value - sensor.min) / (sensor.max - sensor.min)) * 30;
                        return `${x},${y}`;
                      }).join(' ')}
                      fill="none"
                      stroke={statusColor}
                      strokeWidth="1.5"
                      vectorEffect="non-scaling-stroke"
                    />
                  </svg>
                </div>
              </div>
            );
          })}
        </div>
      </div>

      {/* Material Database */}
      <div className="col-span-1 row-span-1 glass-panel rounded-lg p-3">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">ACTIVE MATERIAL</div>
        <div className="text-center mb-2">
          <div className="text-sm font-bold text-forge-text">Inconel 718</div>
          <div className="text-[8px] text-forge-text-muted">Nickel-Chromium Superalloy</div>
        </div>
        <div className="space-y-1">
          {[
            { prop: 'Density', val: '8,190 kg/m³' },
            { prop: 'Yield', val: '1,034 MPa' },
            { prop: 'UTS', val: '1,241 MPa' },
            { prop: 'Elongation', val: '12%' },
            { prop: 'Hardness', val: '36 HRC' },
            { prop: 'Thermal Cond.', val: '11.4 W/m·K' },
            { prop: 'Max Service', val: '700 °C' },
          ].map(m => (
            <div key={m.prop} className="flex justify-between text-[8px]">
              <span className="text-forge-text-muted">{m.prop}</span>
              <span className="text-forge-text font-mono">{m.val}</span>
            </div>
          ))}
        </div>
      </div>

      {/* Lifecycle */}
      <div className="col-span-1 row-span-1 glass-panel rounded-lg p-3">
        <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">LIFECYCLE STATUS</div>
        <div className="space-y-2">
          {[
            { phase: 'Design', pct: 100, color: '#00c2ff' },
            { phase: 'Simulation', pct: 85, color: '#8855ff' },
            { phase: 'Validation', pct: 45, color: '#00ff88' },
            { phase: 'Manufacturing', pct: 0, color: '#ffaa00' },
            { phase: 'Deployment', pct: 0, color: '#ff6600' },
          ].map(p => (
            <div key={p.phase} className="space-y-0.5">
              <div className="flex justify-between text-[8px]">
                <span className="text-forge-text-dim">{p.phase}</span>
                <span className="text-forge-text font-mono">{p.pct}%</span>
              </div>
              <div className="h-1 bg-forge-panel rounded-full overflow-hidden">
                <div className="h-full rounded-full" style={{ width: `${p.pct}%`, backgroundColor: p.color }} />
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
