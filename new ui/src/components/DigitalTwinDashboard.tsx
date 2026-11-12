import React, { useState, useEffect } from 'react';
import { machineFleet } from '../data/mockData';
import type { MachineUnit } from '../types';
import { fetchJson, isOfflineMode, type SimulationStatusDto } from '../services/forgeApi';
import {
  AlertTriangle, CheckCircle, Wrench, XCircle,
  MapPin
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
          <span className="text-[10px] font-semibold text-forge-text truncate">${machine.name}</span>
        </div>
        <span className={`text-[7px] font-mono tracking-wider ${status.color}`}>{status.label}</span>
      </div>

      <div className="grid grid-cols-3 gap-2 mb-2">
        <div>
          <div className="text-[7px] font-mono text-forge-text-muted">HEALTH</div>
          <div className="text-[12px] font-bold font-mono text-forge-text">${machine.health}%</div>
        </div>
        <div>
          <div className="text-[7px] font-mono text-forge-text-muted">TEMP</div>
          <div className="text-[12px] font-bold font-mono text-forge-text">${machine.temperature}°</div>
        </div>
        <div>
          <div className="text-[7px] font-mono text-forge-text-muted">LOAD</div>
          <div className="text-[12px] font-bold font-mono text-forge-text">${machine.load}%</div>
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
  const [fleet, setFleet] = useState<MachineUnit[]>(machineFleet);

  useEffect(() => {
    if (isOfflineMode()) return;
    let cancelled = false;
    const pull = async () => {
      try {
        const data = await fetchJson<SimulationStatusDto>('/api/simulation/status');
        if (cancelled || !data) return;
        const components = Array.isArray(data.components) ? data.components : [];
        if (components.length > 0) {
          const nextLive: Record<string, number> = {};
          const mapped: MachineUnit[] = components.map((c, i) => {
            const base = machineFleet[i % machineFleet.length];
            const rawStress = Number(c.stressRatio ?? 0);
            const rawTemp = Number(c.tempRatio ?? 0);
            const stressRatio = rawStress > 1.5 ? rawStress / 100 : rawStress;
            const tempRatio = rawTemp > 1.5 ? rawTemp / 100 : rawTemp;
            const health = Math.max(5, Math.min(100, Math.round(100 - stressRatio * 40 - tempRatio * 30)));
            const status: MachineUnit['status'] =
              stressRatio > 0.85 ? 'critical' :
              stressRatio > 0.65 ? 'maintenance' :
              stressRatio > 0.4 ? 'degraded' :
              'online';
            const name = c.name || base.name;
            nextLive[`${name} Stress (MPa)`] = Number(Number(c.stress_MPa ?? 0).toFixed(2));
            nextLive[`${name} Temp (°C)`] = Number(Number(c.temperature_C ?? 0).toFixed(1));
            return {
              ...base,
              id: c.name || base.id,
              name,
              status,
              health,
              temperature: Number.isFinite(c.temperature_C) ? Math.round(c.temperature_C) : base.temperature,
              load: Math.round(Math.min(100, Math.max(0, stressRatio * 100))),
            };
          });
          setFleet(mapped);
          setLiveValues(nextLive);
        }
      } catch (err) {
        console.error('Failed to fetch simulation status:', err);
      }
    };
    const timer = setInterval(pull, 5000);
    pull();
    return () => {
      cancelled = true;
      clearInterval(timer);
    };
  }, []);

  return (
    <div className="p-6 bg-forge-dark rounded-lg shadow-md">
      <h2 className="text-2xl font-bold text-forge-text mb-4">Digital Twin Dashboard</h2>
      <div className="grid grid-cols-2 gap-4">
        <div className="bg-forge-surface rounded-lg p-4">
          <h3 className="text-lg font-semibold text-forge-text mb-2">Active Machines</h3>
          {fleet.length > 0 ? (
            fleet.map((machine, i) => (
              <MachineCard
                key={i}
                machine={machine}
                selected={selectedMachine === machine.name}
                onClick={() => setSelectedMachine(machine.name)}
              />
            ))
          ) : (
            <p className="text-forge-text-muted">No active machines</p>
          )}
        </div>

        <div className="bg-forge-surface rounded-lg p-4">
          <h3 className="text-lg font-semibold text-forge-text mb-2">Live Telemetry</h3>
          <div className="space-y-2">
            {Object.entries(liveValues).map(([key, value]) => (
              <div key={key} className="flex items-center space-x-2">
                <span className="text-[7px] font-mono text-forge-text-muted">{key}</span>
                <span className="text-[12px] font-bold text-forge-text">{value}</span>
              </div>
            ))}
            {Object.keys(liveValues).length === 0 && (
              <p className="text-forge-text-muted italic">No live telemetry yet</p>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};