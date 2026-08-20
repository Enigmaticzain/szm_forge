import React, { useState, useEffect, useCallback } from 'react';
import { Settings, Save, CheckCircle, AlertTriangle, XCircle, RefreshCw } from 'lucide-react';
import { fetchMachineConfig, saveMachineConfig, type AxisLimit, type MachineConfigDto } from '../services/forgeApi';

const DEFAULT_LINEAR: AxisLimit = { travel_mm: 500, feed_mm_min: 5000, accel_mm_s2: 500 };
const DEFAULT_ROTARY: AxisLimit = { travel_mm: 360, feed_mm_min: 3000, accel_mm_s2: 200 };

const defaultConfig = (): MachineConfigDto => ({
  id: 'default',
  name: 'CNC Machine',
  is_5axis: false,
  linear: [{ ...DEFAULT_LINEAR }, { ...DEFAULT_LINEAR }, { ...DEFAULT_LINEAR }],
  rotary: [{ ...DEFAULT_ROTARY }, { ...DEFAULT_ROTARY }],
});

interface ValidationResult {
  valid: boolean;
  errors: string[];
  warnings: string[];
  axis_count: number;
}

const AXIS_LABELS = ['X', 'Y', 'Z'];
const ROTARY_LABELS = ['A', 'B'];

function AxisRow({
  label, axis, onChange,
}: {
  label: string;
  axis: AxisLimit;
  onChange: (a: AxisLimit) => void;
}) {
  const field = (key: keyof AxisLimit, min: number, max: number, step: number, unit: string) => (
    <div className="flex flex-col gap-0.5">
      <div className="flex justify-between">
        <span className="text-[8px] font-mono text-forge-text-muted uppercase">{key.replace(/_/g, ' ')}</span>
        <span className="text-[8px] font-mono text-forge-accent">{axis[key]} {unit}</span>
      </div>
      <input
        type="range" min={min} max={max} step={step} value={axis[key]}
        onChange={e => onChange({ ...axis, [key]: parseFloat(e.target.value) })}
        className="w-full h-1 accent-forge-accent cursor-pointer"
      />
    </div>
  );

  return (
    <div className="p-3 rounded border border-forge-border bg-forge-surface/30">
      <div className="text-[9px] font-bold text-forge-text mb-2 tracking-widest">{label}-AXIS</div>
      <div className="space-y-2">
        {field('travel_mm',   50,  3000, 50,   'mm')}
        {field('feed_mm_min', 100, 60000, 100, 'mm/min')}
        {field('accel_mm_s2', 50,  5000, 50,   'mm/s²')}
      </div>
    </div>
  );
}

export const MachineConfigPanel: React.FC = () => {
  const [cfg, setCfg]           = useState<MachineConfigDto>(defaultConfig());
  const [validation, setVal]    = useState<ValidationResult | null>(null);
  const [saving, setSaving]     = useState(false);
  const [saveOk, setSaveOk]     = useState<boolean | null>(null);
  const [loading, setLoading]   = useState(true);

  useEffect(() => {
    fetchMachineConfig('default').then(data => {
      if (data) setCfg(data);
      setLoading(false);
    });
  }, []);

  const validate = useCallback(async (config: MachineConfigDto) => {
    try {
      const res = await fetch('http://127.0.0.1:8003/machine/config/validate', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ is_5axis: config.is_5axis, linear: config.linear, rotary: config.rotary }),
      });
      setVal(await res.json());
    } catch {
      setVal(null);
    }
  }, []);

  const updateLinear = (i: number, ax: AxisLimit) => {
    const next = { ...cfg, linear: cfg.linear.map((a, idx) => idx === i ? ax : a) };
    setCfg(next);
    validate(next);
  };

  const updateRotary = (i: number, ax: AxisLimit) => {
    const next = { ...cfg, rotary: cfg.rotary.map((a, idx) => idx === i ? ax : a) };
    setCfg(next);
    validate(next);
  };

  const toggle5Axis = () => {
    const next = { ...cfg, is_5axis: !cfg.is_5axis };
    setCfg(next);
    validate(next);
  };

  const handleSave = async () => {
    setSaving(true);
    const ok = await saveMachineConfig(cfg);
    setSaveOk(ok);
    setSaving(false);
    setTimeout(() => setSaveOk(null), 2500);
  };

  if (loading) {
    return (
      <div className="h-full flex items-center justify-center bg-forge-black">
        <RefreshCw size={16} className="text-forge-accent animate-spin" />
      </div>
    );
  }

  return (
    <div className="h-full flex flex-col bg-forge-black overflow-y-auto">
      {/* Header */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-3 flex-shrink-0">
        <Settings size={14} className="text-forge-accent" />
        <div>
          <div className="text-[10px] font-bold tracking-widest text-forge-accent">MACHINE CONFIGURATION</div>
          <div className="text-[8px] font-mono text-forge-text-muted">AXIS LIMITS • TRAVEL • FEED • ACCELERATION</div>
        </div>
        <div className="ml-auto flex items-center gap-3">
          {/* 3-axis / 5-axis toggle */}
          <button
            onClick={toggle5Axis}
            className={`px-3 py-1 rounded text-[9px] font-mono font-bold border transition-colors ${
              cfg.is_5axis
                ? 'bg-forge-accent/20 border-forge-accent text-forge-accent'
                : 'bg-forge-surface border-forge-border text-forge-text-dim hover:border-forge-accent/50'
            }`}
          >
            {cfg.is_5axis ? '5-AXIS' : '3-AXIS'}
          </button>
          <button
            onClick={handleSave}
            disabled={saving}
            className="flex items-center gap-1.5 px-3 py-1 rounded bg-forge-accent text-forge-black text-[9px] font-bold hover:bg-forge-accent/80 transition-colors disabled:opacity-50"
          >
            {saving ? <RefreshCw size={10} className="animate-spin" /> : <Save size={10} />}
            SAVE
          </button>
          {saveOk === true  && <CheckCircle size={14} className="text-forge-green" />}
          {saveOk === false && <XCircle     size={14} className="text-forge-red" />}
        </div>
      </div>

      <div className="flex-1 p-4 space-y-4 overflow-y-auto">
        {/* Machine name */}
        <div className="flex items-center gap-3">
          <span className="text-[9px] font-mono text-forge-text-muted w-20">NAME</span>
          <input
            value={cfg.name}
            onChange={e => setCfg({ ...cfg, name: e.target.value })}
            className="flex-1 bg-forge-surface border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text focus:outline-none focus:border-forge-accent"
          />
        </div>

        {/* Linear axes */}
        <div>
          <div className="text-[9px] font-semibold tracking-widest text-forge-text-dim mb-2">LINEAR AXES</div>
          <div className="grid grid-cols-3 gap-3">
            {AXIS_LABELS.map((label, i) => (
              <AxisRow key={label} label={label} axis={cfg.linear[i]} onChange={ax => updateLinear(i, ax)} />
            ))}
          </div>
        </div>

        {/* Rotary axes (5-axis only) */}
        {cfg.is_5axis && (
          <div>
            <div className="text-[9px] font-semibold tracking-widest text-forge-text-dim mb-2">ROTARY AXES (deg / deg·min⁻¹ / deg·s⁻²)</div>
            <div className="grid grid-cols-2 gap-3">
              {ROTARY_LABELS.map((label, i) => (
                <AxisRow key={label} label={label} axis={cfg.rotary[i]} onChange={ax => updateRotary(i, ax)} />
              ))}
            </div>
          </div>
        )}

        {/* Validation panel */}
        {validation && (
          <div className="p-3 rounded border border-forge-border bg-forge-panel space-y-1">
            <div className="flex items-center gap-2 mb-2">
              {validation.valid
                ? <CheckCircle size={11} className="text-forge-green" />
                : <XCircle     size={11} className="text-forge-red" />}
              <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">
                VALIDATION — {validation.axis_count}-AXIS
              </span>
            </div>
            {validation.errors.map((e, i) => (
              <div key={i} className="flex items-center gap-1.5">
                <XCircle size={9} className="text-forge-red flex-shrink-0" />
                <span className="text-[9px] font-mono text-forge-red">{e}</span>
              </div>
            ))}
            {validation.warnings.map((w, i) => (
              <div key={i} className="flex items-center gap-1.5">
                <AlertTriangle size={9} className="text-forge-yellow flex-shrink-0" />
                <span className="text-[9px] font-mono text-forge-yellow">{w}</span>
              </div>
            ))}
            {validation.valid && validation.warnings.length === 0 && (
              <span className="text-[9px] font-mono text-forge-green">All limits valid</span>
            )}
          </div>
        )}
      </div>
    </div>
  );
};
