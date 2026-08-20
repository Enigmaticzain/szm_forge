import { useState, useCallback } from 'react';

export interface CircuitComponent {
  name: string;
  type: 'Resistor' | 'Capacitor' | 'Inductor' | 'VoltageSource' | 'CurrentSource' | 'Diode';
  value: number;
  node1: number;
  node2: number;
  ac?: number;
}

export interface CircuitSimulationRequest {
  component_id: number;
  circuit_type: 'dc' | 'ac' | 'transient';
  frequency_hz?: number;
  temperature_c?: number;
  duration_seconds?: number;
  time_steps?: number;
  netlist?: string;
  components?: CircuitComponent[];
  parameters?: Record<string, number>;
}

export interface CircuitSimulationResult {
  status: string;
  engine_used: string;
  component_id: number;
  circuit_type: string;
  temperature_c?: number;
  
  // DC Results
  node_voltages?: Record<string, number>;
  currents?: Record<string, number>;
  power_dissipation?: Record<string, number>;
  
  // AC Results
  frequency_hz?: number;
  impedance_ohms?: number;
  phase_degrees?: number;
  reactance_xl_ohms?: number;
  reactance_xc_ohms?: number;
  resonant_frequency_hz?: number;
  
  // Transient Results
  time_constant_s?: number;
  time_points_s?: number[];
  voltage_rise?: number[];
  final_voltage_v?: number;
  settling_time_s?: number;
  
  analysis_successful?: boolean;
}

const ANALYSIS_API_BASE = 'http://127.0.0.1:8003';

/**
 * Hook for circuit simulation via ngspice
 */
export function useCircuitAPI() {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [results, setResults] = useState<CircuitSimulationResult | null>(null);

  const runCircuitSimulation = useCallback(async (request: CircuitSimulationRequest) => {
    try {
      setLoading(true);
      setError(null);

      const response = await fetch(`${ANALYSIS_API_BASE}/simulation/circuit/run`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(request),
      });

      if (!response.ok) throw new Error(`HTTP ${response.status}`);

      const data = await response.json();
      if (data.status === 'success') {
        setResults(data);
        return data;
      } else {
        throw new Error(data.error || 'Unknown error');
      }
    } catch (err) {
      const errorMsg = err instanceof Error ? err.message : 'Failed to run circuit simulation';
      setError(errorMsg);
      console.error('Circuit analysis error:', errorMsg);
      return null;
    } finally {
      setLoading(false);
    }
  }, []);

  return {
    loading,
    error,
    results,
    runCircuitSimulation,
  };
}
