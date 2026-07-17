import React, { useState, useEffect } from 'react';
import { machineFleet } from '../data/mockData';
import type { MachineUnit } from '../types';
import {
  Radio, Activity, Thermometer, Gauge, Zap,
  AlertTriangle, CheckCircle, Wrench, XCircle,
  TrendingUp, TrendingDown, Shield, Wifi,
  MapPin, Clock, BarChart3, ArrowUpRight, Brain
} from 'lucide-react';

const statusConfig = {
  online: { color: 'text-forge-green', bg: 'bg-forge-green/10', border: 'border-forge-green/20', label: 'ONLINE', icon: <CheckCircle size={10} /> },
  degraded: { color: 'text-forge-yellow', bg: 'bg-forge-yellow/10', border: 'border-forge-yellow/20', label: 'DEGRADED', icon: <AlertTriangle size={10} /> },
  maintenance: { color: 'text-forge-orange', bg: 'bg-forge-orange/10', border: 'border-forge-orange/20', label: 'MAINT', icon: <Wrench size={10} /> },
  offline: { color: 'text-forge-text-muted', bg: 'bg-forge-surface', border: 'border-forge-border', label: 'OFFLINE', icon: <XCircle size={10} /> },
  critical: { color: 'text-forge-red', bg: 'bg-forge-red/10', border: 'border-forge-red/20', label: 'CRITICAL', icon: <XCircle size={10} /> },
};

const HealthBar: React.FC<{ value: number; className?: string }> = ({ value, className }) => {
  const color = value > 80 ? 'from-forge-green to-forge-teal' :
    value > 50 ? 'from-forge-yellow to-forge-orange' : 'from-forge-red to-forge-warm';
  return (
    <div className={`h-1 bg-forge-dark rounded-full overflow-hidden ${className}`}>
      <div className={`h-full bg-gradient-to-r ${color} rounded-full transition-all duration-500`} style={{ width: `${value}%` }} />
    </div>
  );
};

const MachineCard: React.FC<{ machine: MachineUnit; selected: boolean; onClick: () => void }> = ({ machine, selected, onClick }) => {
  const status = statusConfig[machine.status];
  return (
    <button
      onClick={onClick}
      className={`
        p-3 rounded-lg border text-left transition-all duration-200 w-full
        ${selected
          ? 'border-forge-accent/40 bg-forge-accent/5 glow-accent'
          : `${status.border} ${status.bg} hover:border-forge-accent/20`
        }
      `}
    >
      <div className="flex items-center justify-between mb-2">
        <div className="flex items-center gap-1.5">
          <div className={`w-2 h-2 rounded-full ${status.color.replace('text-', 'bg-')} ${machine.status === 'online' ? 'animate-pulse' : ''}`} />
          <span className="text-[10px] font-semibold text-forge-text truncate">{machine.name}</span>
        </div>
        <span className={`text-[7px] font-mono tracking-wider ${status.color}`}>{status.label}</span>
      </div>

      <div className="grid grid-cols-3 gap-2 mb-2">
        <div>
          <div className="text-[7px] font-mono text-forge-text-muted">HEALTH</div>
          <div className="text-[12px] font-bold font-mono text-forge-text">{machine.health}%</div>
        </div>
        <div>
          <div className="text-[7px] font-mono text-forge-text-muted">TEMP</div>
          <div className="text-[12px] font-bold font-mono text-forge-text">{machine.temperature}°</div>
        </div>
        <div>
          <div className="text-[7px] font-mono text-forge-text-muted">LOAD</div>
          <div className="text-[12px] font-bold font-mono text-forge-text">{machine.load}%</div>
        </div>
      </div>

      <HealthBar value={machine.health} />

      <div className="flex items-center justify-between mt-2">
        <span className="text-[7px] font-mono text-forge-text-muted flex items-center gap-0.5">
          <MapPin size={7} />{machine.location}
        </span>
        <span className="text-[7px] font-mono text-forge-text-muted">{machine.type}</span>
      </div>
    </button>
  );
};

export const DigitalTwinDashboard: React.FC = () => {
  const [selectedMachine, setSelectedMachine] = useState<string>('m1');
  const [liveValues, setLiveValues] = useState<Record<string, number>>({});

  useEffect(() => {
    const interval = setInterval(() => {
      setLiveValues({
        rpm: 3200 + Math.random() * 100,
        pressure: 142 + Math.random() * 5,
        vibration: 0.04 + Math.random() * 0.02,
        flowRate: 847 + Math.random() * 20,
        oilTemp: 78 + Math.random() * 3,
        bearingTemp: 124 + Math.random() * 5,
      });
    }, 1500);
    return () => clearInterval(interval);
  }, []);

  const selected = machineFleet.find(m => m.id === selectedMachine) || machineFleet[0];
  const selStatus = statusConfig[selected.status];

  const fleetStats = {
    online: machineFleet.filter(m => m.status === 'online').length,
    degraded: machineFleet.filter(m => m.status === 'degraded').length,
    critical: machineFleet.filter(m => m.status === 'critical' || m.status === 'maintenance').length,
    avgHealth: Math.round(machineFleet.reduce((a, m) => a + m.health, 0) / machineFleet.length),
    avgEfficiency: (machineFleet.filter(m => m.efficiency > 0).reduce((a, m) => a + m.efficiency, 0) / machineFleet.filter(m => m.efficiency > 0).length).toFixed(1),
  };

  return (
    <div className="h-full flex flex-col bg-forge-black">
      {/* Fleet Overview Bar */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-6">
        <div className="flex items-center gap-2">
          <Radio size={14} className="text-forge-accent" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-accent">DIGITAL TWIN COMMAND CENTER</div>
            <div className="text-[8px] font-mono text-forge-text-muted">INDUSTRIAL FLEET MANAGEMENT • REAL-TIME TELEMETRY</div>
          </div>
        </div>

        <div className="ml-auto flex items-center gap-6">
          <div className="flex items-center gap-4">
            <div className="text-center">
              <div className="text-[8px] font-mono text-forge-text-muted">FLEET</div>
              <div className="text-[14px] font-bold font-mono text-forge-text">{machineFleet.length}</div>
            </div>
            <div className="text-center">
              <div className="text-[8px] font-mono text-forge-text-muted">ONLINE</div>
              <div className="text-[14px] font-bold font-mono text-forge-green">{fleetStats.online}</div>
            </div>
            <div className="text-center">
              <div className="text-[8px] font-mono text-forge-text-muted">DEGRADED</div>
              <div className="text-[14px] font-bold font-mono text-forge-yellow">{fleetStats.degraded}</div>
            </div>
            <div className="text-center">
              <div className="text-[8px] font-mono text-forge-text-muted">CRITICAL</div>
              <div className="text-[14px] font-bold font-mono text-forge-red">{fleetStats.critical}</div>
            </div>
          </div>
          <div className="w-px h-8 bg-forge-border" />
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">AVG HEALTH</div>
            <div className="text-[14px] font-bold font-mono text-forge-accent">{fleetStats.avgHealth}%</div>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">AVG EFFICIENCY</div>
            <div className="text-[14px] font-bold font-mono text-forge-green">{fleetStats.avgEfficiency}%</div>
          </div>
        </div>
      </div>

      {/* Main Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* Machine Fleet Grid */}
        <div className="w-[380px] border-r border-forge-border bg-forge-panel overflow-y-auto p-3">
          <div className="flex items-center justify-between mb-3">
            <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">MACHINE FLEET</span>
            <span className="text-[8px] font-mono text-forge-text-muted">{machineFleet.length} units</span>
          </div>
          <div className="grid grid-cols-1 gap-2">
            {machineFleet.map(machine => (
              <MachineCard
                key={machine.id}
                machine={machine}
                selected={selectedMachine === machine.id}
                onClick={() => setSelectedMachine(machine.id)}
              />
            ))}
          </div>
        </div>

        {/* Selected Machine Detail */}
        <div className="flex-1 overflow-y-auto">
          <div className="p-4">
            {/* Machine Header */}
            <div className="flex items-center justify-between mb-4">
              <div>
                <div className="flex items-center gap-2 mb-1">
                  <div className={`w-2.5 h-2.5 rounded-full ${selStatus.color.replace('text-', 'bg-')} ${selected.status === 'online' ? 'animate-pulse' : ''}`} />
                  <h2 className="text-lg font-bold text-forge-text">{selected.name}</h2>
                  <span className={`text-[8px] font-mono tracking-wider px-2 py-0.5 rounded ${selStatus.bg} ${selStatus.color} border ${selStatus.border}`}>
                    {selStatus.label}
                  </span>
                </div>
                <div className="flex items-center gap-3 text-[9px] font-mono text-forge-text-muted">
                  <span className="flex items-center gap-1"><MapPin size={9} />{selected.location}</span>
                  <span>{selected.type}</span>
                  <span className="flex items-center gap-1"><Wifi size={9} />CONNECTED</span>
                </div>
              </div>
              <div className="flex gap-2">
                <button className="px-3 py-1.5 rounded text-[9px] font-mono bg-forge-accent/10 text-forge-accent border border-forge-accent/20 hover:bg-forge-accent/20 transition-all">
                  DIAGNOSTICS
                </button>
                <button className="px-3 py-1.5 rounded text-[9px] font-mono bg-forge-surface text-forge-text-dim border border-forge-border hover:bg-forge-surface/80 transition-all">
                  HISTORY
                </button>
              </div>
            </div>

            {/* Key Metrics */}
            <div className="grid grid-cols-4 gap-3 mb-4">
              <div className={`p-3 rounded-lg border ${selected.health > 80 ? 'border-forge-green/20 bg-forge-green/5' : 'border-forge-yellow/20 bg-forge-yellow/5'}`}>
                <div className="flex items-center justify-between mb-2">
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">HEALTH INDEX</span>
                  <Shield size={14} className={selected.health > 80 ? 'text-forge-green' : 'text-forge-yellow'} />
                </div>
                <div className={`text-[22px] font-bold font-mono ${selected.health > 80 ? 'text-forge-green' : 'text-forge-yellow'}`}>{selected.health}%</div>
              </div>
              <div className="p-3 rounded-lg border border-forge-accent/20 bg-forge-accent/5">
                <div className="flex items-center justify-between mb-2">
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">UPTIME</span>
                  <TrendingUp size={14} className="text-forge-accent" />
                </div>
                <div className="text-[22px] font-bold font-mono text-forge-accent">{selected.uptime}%</div>
              </div>
              <div className="p-3 rounded-lg border border-forge-teal/20 bg-forge-teal/5">
                <div className="flex items-center justify-between mb-2">
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">EFFICIENCY</span>
                  <Gauge size={14} className="text-forge-teal" />
                </div>
                <div className="text-[22px] font-bold font-mono text-forge-teal">{selected.efficiency}%</div>
              </div>
              <div className={`p-3 rounded-lg border ${selected.load > 90 ? 'border-forge-orange/20 bg-forge-orange/5' : 'border-forge-blue/20 bg-forge-blue/5'}`}>
                <div className="flex items-center justify-between mb-2">
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">LOAD</span>
                  <Zap size={14} className={selected.load > 90 ? 'text-forge-orange' : 'text-forge-blue'} />
                </div>
                <div className={`text-[22px] font-bold font-mono ${selected.load > 90 ? 'text-forge-orange' : 'text-forge-blue'}`}>{selected.load}%</div>
              </div>
            </div>

            {/* Live Telemetry */}
            <div className="mb-4">
              <div className="flex items-center gap-2 mb-3">
                <Activity size={12} className="text-forge-accent" />
                <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">LIVE TELEMETRY</span>
                <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse" />
                <span className="text-[8px] font-mono text-forge-green">STREAMING</span>
              </div>
              <div className="grid grid-cols-3 gap-2">
                {[
                  { label: 'RPM', value: liveValues.rpm?.toFixed(0) || '3245', unit: 'rpm', icon: <BarChart3 size={10} /> },
                  { label: 'PRESSURE', value: liveValues.pressure?.toFixed(1) || '144.2', unit: 'bar', icon: <Gauge size={10} /> },
                  { label: 'VIBRATION', value: liveValues.vibration?.toFixed(3) || '0.048', unit: 'mm/s', icon: <Activity size={10} /> },
                  { label: 'FLOW RATE', value: liveValues.flowRate?.toFixed(0) || '855', unit: 'L/min', icon: <ArrowUpRight size={10} /> },
                  { label: 'OIL TEMP', value: liveValues.oilTemp?.toFixed(1) || '79.5', unit: '°C', icon: <Thermometer size={10} /> },
                  { label: 'BEARING TEMP', value: liveValues.bearingTemp?.toFixed(1) || '126.8', unit: '°C', icon: <Thermometer size={10} /> },
                ].map((item, i) => (
                  <div key={i} className="p-2.5 rounded border border-forge-border/50 bg-forge-surface/30">
                    <div className="flex items-center gap-1 mb-1">
                      <span className="text-forge-text-muted">{item.icon}</span>
                      <span className="text-[8px] font-mono text-forge-text-muted">{item.label}</span>
                    </div>
                    <div className="flex items-baseline gap-1">
                      <span className="text-[16px] font-bold font-mono text-forge-text">{item.value}</span>
                      <span className="text-[8px] font-mono text-forge-text-muted">{item.unit}</span>
                    </div>
                  </div>
                ))}
              </div>
            </div>

            {/* Maintenance Info */}
            <div className="grid grid-cols-2 gap-3">
              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Clock size={11} className="text-forge-accent" />
                  <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">MAINTENANCE</span>
                </div>
                <div className="space-y-2">
                  <div className="flex justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">Last Service</span>
                    <span className="text-[9px] font-mono text-forge-text">{selected.lastMaintenance}</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">Next Service</span>
                    <span className={`text-[9px] font-mono ${selected.nextMaintenance === 'OVERDUE' ? 'text-forge-red' : 'text-forge-text'}`}>
                      {selected.nextMaintenance}
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">Operating Hours</span>
                    <span className="text-[9px] font-mono text-forge-text">12,847 hrs</span>
                  </div>
                </div>
              </div>
              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Brain size={11} className="text-forge-purple" />
                  <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">AI PREDICTIONS</span>
                </div>
                <div className="space-y-2">
                  <div className="flex justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">Failure Risk</span>
                    <span className="text-[9px] font-mono text-forge-green">LOW (4%)</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">Remaining Life</span>
                    <span className="text-[9px] font-mono text-forge-text">~18,200 hrs</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] font-mono text-forge-text-muted">Degradation</span>
                    <span className="text-[9px] font-mono text-forge-yellow flex items-center gap-0.5">
                      <TrendingDown size={8} /> 0.02%/day
                    </span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
