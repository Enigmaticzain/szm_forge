import React, { useState } from 'react';
import { useAnalysisAPI, ThermalAnalysisRequest, ModalAnalysisRequest } from '../hooks/useAnalysisAPI';

export function ThermalAnalysisPanel() {
  const { loading, error, thermalResults, runThermalAnalysis } = useAnalysisAPI();
  
  const [componentId, setComponentId] = useState(1);
  const [mode, setMode] = useState<'steady_state' | 'transient'>('steady_state');
  const [ambientTemp, setAmbientTemp] = useState(20);
  const [heatInput, setHeatInput] = useState(100);
  const [convection, setConvection] = useState(10);

  const handleRun = async () => {
    const request: ThermalAnalysisRequest = {
      component_id: componentId,
      mode,
      ambient_temp_c: ambientTemp,
      heat_input_w: heatInput,
      convection_coefficient: convection,
      material_properties: {
        density_kg_m3: 7850,
        thermal_conductivity_w_mk: 50,
        specific_heat_j_kgk: 500,
      },
    };

    await runThermalAnalysis(request);
  };

  return (
    <div className="h-full flex flex-col bg-forge-surface overflow-hidden">
      <div className="px-3 py-2 border-b border-forge-border">
        <h2 className="text-sm font-bold text-forge-text tracking-wider">THERMAL ANALYSIS</h2>
        <p className="text-xs text-forge-text-muted mt-1">CalculiX Heat Transfer Simulation</p>
      </div>

      <div className="flex-1 overflow-y-auto p-4 space-y-4">
        {/* Analysis Configuration */}
        <div className="space-y-3">
          <div>
            <label className="text-xs text-forge-text-dim">Component ID</label>
            <input
              type="number"
              value={componentId}
              onChange={(e) => setComponentId(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            />
          </div>

          <div>
            <label className="text-xs text-forge-text-dim">Analysis Mode</label>
            <select
              value={mode}
              onChange={(e) => setMode(e.target.value as 'steady_state' | 'transient')}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            >
              <option value="steady_state">Steady State</option>
              <option value="transient">Transient</option>
            </select>
          </div>

          <div>
            <label className="text-xs text-forge-text-dim">Ambient Temperature (°C)</label>
            <input
              type="number"
              value={ambientTemp}
              onChange={(e) => setAmbientTemp(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            />
          </div>

          <div>
            <label className="text-xs text-forge-text-dim">Heat Input (W)</label>
            <input
              type="number"
              value={heatInput}
              onChange={(e) => setHeatInput(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            />
          </div>

          <div>
            <label className="text-xs text-forge-text-dim">Convection Coefficient (W/m²·K)</label>
            <input
              type="number"
              value={convection}
              onChange={(e) => setConvection(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            />
          </div>
        </div>

        {/* Run Button */}
        <button
          onClick={handleRun}
          disabled={loading}
          className="w-full px-4 py-2 bg-forge-accent text-forge-text text-sm font-bold rounded hover:bg-forge-accent-hover disabled:opacity-50"
        >
          {loading ? 'Running Analysis...' : 'Run Thermal Analysis'}
        </button>

        {/* Error Message */}
        {error && (
          <div className="p-3 bg-red-500/10 border border-red-500/20 rounded">
            <p className="text-xs text-red-400">{error}</p>
          </div>
        )}

        {/* Results */}
        {thermalResults && (
          <div className="p-3 bg-forge-panel border border-forge-border rounded space-y-2">
            <h3 className="text-xs font-bold text-forge-text">RESULTS</h3>
            <div className="space-y-1 text-xs text-forge-text-muted">
              <div className="flex justify-between">
                <span>Engine Used:</span>
                <span className="font-mono">{thermalResults.engine_used}</span>
              </div>
              <div className="flex justify-between">
                <span>Max Temperature:</span>
                <span className={`font-mono font-bold ${thermalResults.is_dangerous ? 'text-red-400' : 'text-forge-green'}`}>
                  {thermalResults.max_temperature_c}°C
                </span>
              </div>
              <div className="flex justify-between">
                <span>Temperature Rise:</span>
                <span className="font-mono">{thermalResults.temperature_rise_c}°C</span>
              </div>
              {thermalResults.is_dangerous && (
                <div className="mt-2 p-2 bg-red-500/20 border border-red-500/30 rounded">
                  <p className="text-red-300 font-bold">⚠ DANGEROUS CONDITIONS</p>
                </div>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

export default ThermalAnalysisPanel;
