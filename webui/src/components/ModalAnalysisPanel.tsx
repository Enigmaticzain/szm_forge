import React, { useState } from 'react';
import { useAnalysisAPI, ModalAnalysisRequest } from '../hooks/useAnalysisAPI';

export function ModalAnalysisPanel() {
  const { loading, error, modalResults, runModalAnalysis } = useAnalysisAPI();
  
  const [componentId, setComponentId] = useState(1);
  const [numModes, setNumModes] = useState(10);
  const [youngsModulus, setYoungsModulus] = useState(210e9);
  const [minFreq, setMinFreq] = useState(0);
  const [maxFreq, setMaxFreq] = useState(10000);

  const handleRun = async () => {
    const request: ModalAnalysisRequest = {
      component_id: componentId,
      num_modes: numModes,
      material_properties: {
        density_kg_m3: 7850,
        youngs_modulus_pa: youngsModulus,
        poisson_ratio: 0.3,
      },
      min_frequency_hz: minFreq,
      max_frequency_hz: maxFreq,
    };

    await runModalAnalysis(request);
  };

  return (
    <div className="h-full flex flex-col bg-forge-surface overflow-hidden">
      <div className="px-3 py-2 border-b border-forge-border">
        <h2 className="text-sm font-bold text-forge-text tracking-wider">MODAL ANALYSIS</h2>
        <p className="text-xs text-forge-text-muted mt-1">Eigenvalue & Natural Frequencies</p>
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
            <label className="text-xs text-forge-text-dim">Number of Modes</label>
            <input
              type="number"
              min="1"
              max="20"
              value={numModes}
              onChange={(e) => setNumModes(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
            />
          </div>

          <div>
            <label className="text-xs text-forge-text-dim">Young's Modulus (Pa)</label>
            <input
              type="number"
              value={youngsModulus}
              onChange={(e) => setYoungsModulus(Number(e.target.value))}
              className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text font-mono"
            />
          </div>

          <div className="grid grid-cols-2 gap-2">
            <div>
              <label className="text-xs text-forge-text-dim">Min Frequency (Hz)</label>
              <input
                type="number"
                value={minFreq}
                onChange={(e) => setMinFreq(Number(e.target.value))}
                className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
              />
            </div>
            <div>
              <label className="text-xs text-forge-text-dim">Max Frequency (Hz)</label>
              <input
                type="number"
                value={maxFreq}
                onChange={(e) => setMaxFreq(Number(e.target.value))}
                className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text"
              />
            </div>
          </div>
        </div>

        {/* Run Button */}
        <button
          onClick={handleRun}
          disabled={loading}
          className="w-full px-4 py-2 bg-forge-accent text-forge-text text-sm font-bold rounded hover:bg-forge-accent-hover disabled:opacity-50"
        >
          {loading ? 'Computing Frequencies...' : 'Run Modal Analysis'}
        </button>

        {/* Error Message */}
        {error && (
          <div className="p-3 bg-red-500/10 border border-red-500/20 rounded">
            <p className="text-xs text-red-400">{error}</p>
          </div>
        )}

        {/* Results */}
        {modalResults && (
          <div className="p-3 bg-forge-panel border border-forge-border rounded space-y-3">
            <h3 className="text-xs font-bold text-forge-text">RESULTS</h3>
            
            {/* Summary */}
            <div className="space-y-1 text-xs text-forge-text-muted">
              <div className="flex justify-between">
                <span>Engine Used:</span>
                <span className="font-mono">{modalResults.engine_used}</span>
              </div>
              <div className="flex justify-between">
                <span>Modes Computed:</span>
                <span className="font-mono">{modalResults.num_modes_computed}</span>
              </div>
              <div className="flex justify-between">
                <span>First Natural Frequency:</span>
                <span className="font-mono font-bold text-forge-green">
                  {modalResults.first_natural_frequency_hz.toFixed(2)} Hz
                </span>
              </div>
            </div>

            {/* Frequency List */}
            <div>
              <p className="text-xs font-bold text-forge-text mb-2">Mode Frequencies:</p>
              <div className="bg-forge-input/50 rounded p-2 max-h-32 overflow-y-auto space-y-1">
                {modalResults.natural_frequencies_hz.map((freq, idx) => (
                  <div key={idx} className="flex justify-between text-xs text-forge-text-muted font-mono">
                    <span>Mode {idx + 1}:</span>
                    <span className="text-forge-green">{freq.toFixed(2)} Hz</span>
                  </div>
                ))}
              </div>
            </div>

            {/* Frequency Range */}
            <div className="p-2 bg-forge-accent/10 border border-forge-accent/20 rounded text-xs">
              <p className="text-forge-text-muted">
                Frequency range: <span className="font-mono">{modalResults.frequency_range.min_hz.toFixed(2)}</span> −{' '}
                <span className="font-mono">{modalResults.frequency_range.max_hz.toFixed(2)} Hz</span>
              </p>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}

export default ModalAnalysisPanel;
