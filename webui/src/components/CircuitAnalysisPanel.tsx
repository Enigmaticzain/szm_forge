import React, { useState } from 'react';
import { useCircuitAPI, CircuitSimulationRequest } from '../hooks/useCircuitAPI';

export function CircuitAnalysisPanel() {
  const { loading, error, results, runCircuitSimulation } = useCircuitAPI();

  const [componentId, setComponentId] = useState(1);
  const [analysisType, setAnalysisType] = useState<'dc' | 'ac' | 'transient'>('dc');
  const [temperature, setTemperature] = useState(27);
  const [frequency, setFrequency] = useState(1000);
  const [duration, setDuration] = useState(1);
  const [timeSteps, setTimeSteps] = useState(100);

  const handleRunDC = async () => {
    const request: CircuitSimulationRequest = {
      component_id: componentId,
      circuit_type: 'dc',
      temperature_c: temperature,
    };
    await runCircuitSimulation(request);
  };

  const handleRunAC = async () => {
    const request: CircuitSimulationRequest = {
      component_id: componentId,
      circuit_type: 'ac',
      frequency_hz: frequency,
      temperature_c: temperature,
    };
    await runCircuitSimulation(request);
  };

  const handleRunTransient = async () => {
    const request: CircuitSimulationRequest = {
      component_id: componentId,
      circuit_type: 'transient',
      duration_seconds: duration,
      time_steps: timeSteps,
      temperature_c: temperature,
    };
    await runCircuitSimulation(request);
  };

  return (
    <div className="h-full flex flex-col bg-forge-surface overflow-hidden">
      <div className="px-3 py-2 border-b border-forge-border">
        <h2 className="text-sm font-bold text-forge-text tracking-wider">CIRCUIT ANALYSIS</h2>
        <p className="text-xs text-forge-text-muted mt-1">Ngspice Electronics Simulation</p>
      </div>

      <div className="flex-1 overflow-y-auto p-4 space-y-4">
        {/* Basic Configuration */}
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
            <label className="text-xs text-forge-text-dim">Temperature (°C)</label>
            <input
              type="number"
              value={temperature}
              onChange={(e) => setTemperature(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            />
          </div>
        </div>

        {/* Analysis Type Tabs */}
        <div className="space-y-3">
          <label className="text-xs text-forge-text-dim block">Analysis Type</label>

          {/* DC Analysis */}
          <div className="p-3 bg-forge-panel border border-forge-border rounded">
            <h3 className="text-xs font-bold text-forge-text mb-2">DC Operating Point</h3>
            <p className="text-xs text-forge-text-muted mb-2">
              Find steady-state node voltages and component currents
            </p>
            <button
              onClick={handleRunDC}
              disabled={loading}
              className="w-full px-3 py-2 bg-forge-accent text-forge-text text-xs font-bold rounded hover:bg-forge-accent-hover disabled:opacity-50"
            >
              {loading && analysisType === 'dc' ? 'Computing...' : 'Run DC Analysis'}
            </button>
          </div>

          {/* AC Analysis */}
          <div className="p-3 bg-forge-panel border border-forge-border rounded">
            <h3 className="text-xs font-bold text-forge-text mb-2">AC Frequency Response</h3>
            <div className="mb-2">
              <label className="text-xs text-forge-text-dim">Frequency (Hz)</label>
              <input
                type="number"
                value={frequency}
                onChange={(e) => setFrequency(Number(e.target.value))}
                className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
              />
            </div>
            <button
              onClick={handleRunAC}
              disabled={loading}
              className="w-full px-3 py-2 bg-forge-accent text-forge-text text-xs font-bold rounded hover:bg-forge-accent-hover disabled:opacity-50"
            >
              {loading && analysisType === 'ac' ? 'Computing...' : 'Run AC Analysis'}
            </button>
          </div>

          {/* Transient Analysis */}
          <div className="p-3 bg-forge-panel border border-forge-border rounded">
            <h3 className="text-xs font-bold text-forge-text mb-2">Transient Response</h3>
            <div className="grid grid-cols-2 gap-2 mb-2">
              <div>
                <label className="text-xs text-forge-text-dim">Duration (s)</label>
                <input
                  type="number"
                  value={duration}
                  onChange={(e) => setDuration(Number(e.target.value))}
                  step="0.1"
                  className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
                />
              </div>
              <div>
                <label className="text-xs text-forge-text-dim">Time Steps</label>
                <input
                  type="number"
                  value={timeSteps}
                  onChange={(e) => setTimeSteps(Number(e.target.value))}
                  min="10"
                  max="1000"
                  className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
                />
              </div>
            </div>
            <button
              onClick={handleRunTransient}
              disabled={loading}
              className="w-full px-3 py-2 bg-forge-accent text-forge-text text-xs font-bold rounded hover:bg-forge-accent-hover disabled:opacity-50"
            >
              {loading && analysisType === 'transient' ? 'Computing...' : 'Run Transient Analysis'}
            </button>
          </div>
        </div>

        {/* Error Message */}
        {error && (
          <div className="p-3 bg-red-500/10 border border-red-500/20 rounded">
            <p className="text-xs text-red-400">{error}</p>
          </div>
        )}

        {/* Results */}
        {results && (
          <div className="p-3 bg-forge-panel border border-forge-accent/40 rounded space-y-2">
            <h3 className="text-xs font-bold text-forge-text">ANALYSIS RESULTS</h3>
            <div className="space-y-1 text-xs text-forge-text-muted">
              <div className="flex justify-between">
                <span>Engine:</span>
                <span className="font-mono text-forge-green">{results.engine_used}</span>
              </div>
              <div className="flex justify-between">
                <span>Type:</span>
                <span className="font-mono">{results.circuit_type.toUpperCase()}</span>
              </div>

              {/* DC Results */}
              {results.circuit_type === 'dc' && results.node_voltages && (
                <div className="mt-2 pt-2 border-t border-forge-border">
                  <p className="font-bold text-forge-green mb-1">Node Voltages (V)</p>
                  {Object.entries(results.node_voltages).map(([node, voltage]) => (
                    <div key={node} className="flex justify-between text-xs text-forge-text-muted">
                      <span>{node}:</span>
                      <span className="font-mono">{Number(voltage).toFixed(3)}</span>
                    </div>
                  ))}
                </div>
              )}

              {/* AC Results */}
              {results.circuit_type === 'ac' && (
                <div className="mt-2 pt-2 border-t border-forge-border">
                  <p className="font-bold text-forge-green mb-1">AC Response @ {results.frequency_hz} Hz</p>
                  <div className="flex justify-between text-xs text-forge-text-muted">
                    <span>Impedance:</span>
                    <span className="font-mono">{results.impedance_ohms?.toFixed(2)} Ω</span>
                  </div>
                  <div className="flex justify-between text-xs text-forge-text-muted">
                    <span>Phase:</span>
                    <span className="font-mono">{results.phase_degrees?.toFixed(2)}°</span>
                  </div>
                  {results.resonant_frequency_hz && (
                    <div className="flex justify-between text-xs text-forge-text-muted">
                      <span>Resonant Freq:</span>
                      <span className="font-mono">{results.resonant_frequency_hz.toFixed(2)} Hz</span>
                    </div>
                  )}
                </div>
              )}

              {/* Transient Results */}
              {results.circuit_type === 'transient' && (
                <div className="mt-2 pt-2 border-t border-forge-border">
                  <p className="font-bold text-forge-green mb-1">Transient Response</p>
                  <div className="flex justify-between text-xs text-forge-text-muted">
                    <span>Time Constant:</span>
                    <span className="font-mono">{results.time_constant_s?.toFixed(6)} s</span>
                  </div>
                  <div className="flex justify-between text-xs text-forge-text-muted">
                    <span>Final Voltage:</span>
                    <span className="font-mono">{results.final_voltage_v?.toFixed(3)} V</span>
                  </div>
                  <div className="flex justify-between text-xs text-forge-text-muted">
                    <span>Settling Time:</span>
                    <span className="font-mono">{results.settling_time_s?.toFixed(6)} s</span>
                  </div>
                </div>
              )}
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

export default CircuitAnalysisPanel;
