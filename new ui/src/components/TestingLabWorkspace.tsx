import React, { useEffect, useState } from 'react';
import { useForgeStore } from '../store/ForgeStoreContext';
import { useProject } from '../store/ProjectContext';
import { TestTube, Play, Thermometer, Activity } from 'lucide-react';
import { runFea, runThermal } from '../services/forgeApi';

export const TestingLabWorkspace: React.FC = () => {
  const store = useForgeStore();
  const { bodyParts } = useProject();
  const [selectedId, setSelectedId] = useState(bodyParts[0]?.id ?? '');
  const [forceN, setForceN] = useState(1000);
  const [areaM2, setAreaM2] = useState('');
  const [thicknessM, setThicknessM] = useState('');
  const [tempC, setTempC] = useState(80);
  const [busy, setBusy] = useState(false);
  const [log, setLog] = useState<string[]>([]);

  const append = (msg: string) => setLog(prev => [msg, ...prev].slice(0, 40));
  const selectedPart = bodyParts.find(p => p.id === selectedId) ?? bodyParts[0] ?? null;

  useEffect(() => {
    if (!bodyParts.some(part => part.id === selectedId)) {
      setSelectedId(bodyParts[0]?.id ?? '');
    }
  }, [bodyParts, selectedId]);

  const runStress = async () => {
    if (!selectedPart) {
      const message = 'Load a component before running a stress test.';
      append(message);
      store.showToast(message);
      return;
    }
    const area = Number(areaM2);
    const thickness = Number(thicknessM);
    if (!Number.isFinite(forceN) || forceN <= 0 ||
      !Number.isFinite(area) || area <= 0 ||
      !Number.isFinite(thickness) || thickness <= 0) {
      const message = 'Enter a positive load, cross-section area, and thickness before running stress analysis.';
      append(message);
      store.showToast(message);
      return;
    }

    setBusy(true);
    const part = selectedPart;
    const componentId = bodyParts.findIndex(p => p.id === part.id) + 1;
    try {
      const data = await runFea(componentId, part.materialId ?? 'steel', forceN, area, thickness);
      append(`STRESS OK · ${part.name} · F=${forceN}N → ${JSON.stringify(data).slice(0, 180)}`);
      store.showToast('Stress test complete');
    } catch (e: unknown) {
      const message = `Stress test failed · ${part.name} · ${e instanceof Error ? e.message : String(e)}`;
      append(message);
      store.showToast(message);
    } finally {
      setBusy(false);
    }
  };

  const runThermalTest = async () => {
    if (!selectedPart) {
      const message = 'Load a component before running a thermal test.';
      append(message);
      store.showToast(message);
      return;
    }
    if (!Number.isFinite(tempC)) {
      const message = 'Enter a valid thermal set-point in °C.';
      append(message);
      store.showToast(message);
      return;
    }

    setBusy(true);
    const part = selectedPart;
    const componentId = bodyParts.findIndex(p => p.id === part.id) + 1;
    try {
      const data = await runThermal(componentId, tempC);
      append(`THERMAL OK · ${part.name} · T=${tempC}°C → ${JSON.stringify(data).slice(0, 180)}`);
      store.showToast('Thermal test complete');
    } catch (e: unknown) {
      const message = `Thermal test failed · ${part.name} · ${e instanceof Error ? e.message : String(e)}`;
      append(message);
      store.showToast(message);
    } finally {
      setBusy(false);
    }
  };

  return (
    <div className="w-full h-full flex bg-forge-dark/50 relative overflow-hidden">
      <div className="w-[300px] border-r border-forge-border bg-forge-dark/90 p-4 flex flex-col gap-3 flex-shrink-0">
        <div className="flex items-center gap-2 text-forge-accent">
          <TestTube size={14} />
          <span className="text-[10px] font-bold tracking-widest">TEST SETUP</span>
        </div>

        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Component under test</span>
          <select
            value={selectedPart?.id ?? ''}
            onChange={e => setSelectedId(e.target.value)}
            className="bg-forge-black border border-forge-border rounded px-2 py-1.5 text-[10px] font-mono text-forge-text"
          >
            {bodyParts.length === 0 && <option value="">No parts — load via import</option>}
            {bodyParts.map(p => (
              <option key={p.id} value={p.id}>{p.name}</option>
            ))}
          </select>
        </label>

        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Load force (N)</span>
          <input type="number" value={forceN} onChange={e => setForceN(Number(e.target.value))}
            className="bg-forge-black border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text" />
        </label>
        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Cross-section area (m²)</span>
          <input type="number" min="0" step="any" value={areaM2} onChange={e => setAreaM2(e.target.value)}
            className="bg-forge-black border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text" />
        </label>
        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Thickness (m)</span>
          <input type="number" min="0" step="any" value={thicknessM} onChange={e => setThicknessM(e.target.value)}
            className="bg-forge-black border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text" />
        </label>
        <label className="flex flex-col gap-1">
          <span className="text-[8px] font-mono text-forge-text-muted">Thermal set-point (°C)</span>
          <input type="number" value={tempC} onChange={e => setTempC(Number(e.target.value))}
            className="bg-forge-black border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text" />
        </label>

        <button onClick={runStress} disabled={busy || !selectedPart}
          className="flex items-center justify-center gap-2 py-2 rounded bg-forge-accent/20 text-forge-accent border border-forge-accent/50 text-[10px] font-bold font-mono disabled:opacity-40">
          <Activity size={12} /> RUN STRESS
        </button>
        <button onClick={runThermalTest} disabled={busy || !selectedPart}
          className="flex items-center justify-center gap-2 py-2 rounded bg-forge-orange/20 text-forge-orange border border-forge-orange/50 text-[10px] font-bold font-mono disabled:opacity-40">
          <Thermometer size={12} /> RUN THERMAL
        </button>
        <button onClick={() => store.setImportDialogOpen(true)}
          className="py-2 rounded border border-forge-green/40 text-forge-green text-[10px] font-bold font-mono hover:bg-forge-green/10">
          LOAD COMPONENT TO TEST
        </button>
      </div>

      <div className="flex-1 flex flex-col min-w-0 p-6">
        <div className="flex items-center gap-2 mb-4">
          <Play size={16} className="text-forge-accent" />
          <span className="text-forge-text font-mono text-sm tracking-wider">TESTING LAB</span>
        </div>
        <div className="flex-1 rounded-xl border border-forge-border/60 bg-forge-black/60 p-4 overflow-auto space-y-2">
          {log.length === 0 ? (
            <p className="text-sm text-forge-text-muted font-mono">
              Load and select a component before testing. Solver failures are reported directly; no offline estimate is substituted.
            </p>
          ) : (
            log.map((line, i) => (
              <div key={i} className="text-[10px] font-mono text-forge-text-dim border-b border-forge-border/30 pb-2">
                {line}
              </div>
            ))
          )}
        </div>
      </div>
    </div>
  );
};
