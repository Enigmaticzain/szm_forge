import { useState, useCallback } from 'react';

export interface ThermalAnalysisRequest {
  component_id: number;
  mode: 'steady_state' | 'transient';
  ambient_temp_c: number;
  heat_input_w: number;
  boundary_conditions?: Record<string, number>;
  convection_coefficient?: number;
  duration_seconds?: number;
  time_steps?: number;
  material_properties?: Record<string, number>;
}

export interface ThermalAnalysisResult {
  status: string;
  engine_used: string;
  component_id: number;
  analysis_type: string;
  max_temperature_c: number;
  ambient_temperature_c: number;
  temperature_rise_c: number;
  is_dangerous: boolean;
}

export interface ModalAnalysisRequest {
  component_id: number;
  num_modes?: number;
  material_properties?: Record<string, number>;
  constraints?: any[];
  min_frequency_hz?: number;
  max_frequency_hz?: number;
}

export interface ModalAnalysisResult {
  status: string;
  engine_used: string;
  component_id: number;
  num_modes_computed: number;
  natural_frequencies_hz: number[];
  first_natural_frequency_hz: number;
  frequency_range: {
    min_hz: number;
    max_hz: number;
  };
  lowest_frequency_hz: number;
  highest_frequency_hz: number;
}

const ANALYSIS_API_BASE = 'http://127.0.0.1:8003';

/**
 * Hook for thermal and modal analysis capabilities
 */
export function useAnalysisAPI() {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [thermalResults, setThermalResults] = useState<ThermalAnalysisResult | null>(null);
  const [modalResults, setModalResults] = useState<ModalAnalysisResult | null>(null);

  // Run thermal analysis
  const runThermalAnalysis = useCallback(async (request: ThermalAnalysisRequest) => {
    try {
      setLoading(true);
      setError(null);

      const response = await fetch(`${ANALYSIS_API_BASE}/simulation/thermal/run`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(request),
      });

      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      
      const data = await response.json();
      if (data.status === 'success') {
        setThermalResults(data);
        return data;
      } else {
        throw new Error(data.error || 'Unknown error');
      }
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Failed to run thermal analysis';
      setError(errorMsg);
      console.error('Thermal analysis error:', errorMsg);
      return null;
    } finally {
      setLoading(false);
    }
  }, []);

  // Run modal analysis
  const runModalAnalysis = useCallback(async (request: ModalAnalysisRequest) => {
    try {
      setLoading(true);
      setError(null);

      const response = await fetch(`${ANALYSIS_API_BASE}/simulation/modal/run`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(request),
      });

      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      
      const data = await response.json();
      if (data.status === 'success') {
        setModalResults(data);
        return data;
      } else {
        throw new Error(data.error || 'Unknown error');
      }
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Failed to run modal analysis';
      setError(errorMsg);
      console.error('Modal analysis error:', errorMsg);
      return null;
    } finally {
      setLoading(false);
    }
  }, []);

  return {
    loading,
    error,
    thermalResults,
    modalResults,
    runThermalAnalysis,
    runModalAnalysis,
  };
}
