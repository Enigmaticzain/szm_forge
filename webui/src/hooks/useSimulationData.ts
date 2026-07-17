import { useState, useEffect, useRef } from 'react';

export interface TelemetryPoint {
  time: number;
  value: number;
  label?: string;
}

export interface SensorData {
  id: string;
  name: string;
  value: number;
  unit: string;
  min: number;
  max: number;
  status: 'normal' | 'warning' | 'critical';
  history: TelemetryPoint[];
}

export interface StressNode {
  x: number;
  y: number;
  vonMises: number;
  principal: number;
  strain: number;
  displacement: number;
}

export interface ThermalNode {
  x: number;
  y: number;
  temperature: number;
  heatFlux: number;
}

export interface ElectricalNode {
  x: number;
  y: number;
  voltage: number;
  current: number;
  power: number;
  phase: number;
}

function generateHistory(baseValue: number, variance: number, points: number): TelemetryPoint[] {
  const history: TelemetryPoint[] = [];
  let value = baseValue;
  for (let i = 0; i < points; i++) {
    value += (Math.random() - 0.5) * variance;
    value = Math.max(baseValue - variance * 2, Math.min(baseValue + variance * 2, value));
    history.push({ time: Date.now() - (points - i) * 1000, value });
  }
  return history;
}

export function useSimulationData() {
  const timeRef = useRef(0);
  
  const [sensors, setSensors] = useState<SensorData[]>([
    { id: 'temp-1', name: 'Core Temperature', value: 342, unit: '°C', min: 0, max: 600, status: 'normal', history: generateHistory(342, 5, 60) },
    { id: 'press-1', name: 'Chamber Pressure', value: 14.7, unit: 'MPa', min: 0, max: 25, status: 'normal', history: generateHistory(14.7, 0.3, 60) },
    { id: 'vib-1', name: 'Vibration RMS', value: 2.3, unit: 'mm/s', min: 0, max: 10, status: 'normal', history: generateHistory(2.3, 0.2, 60) },
    { id: 'rpm-1', name: 'Rotor Speed', value: 3450, unit: 'RPM', min: 0, max: 5000, status: 'normal', history: generateHistory(3450, 30, 60) },
    { id: 'flow-1', name: 'Flow Rate', value: 127.4, unit: 'L/min', min: 0, max: 200, status: 'normal', history: generateHistory(127.4, 2, 60) },
    { id: 'torq-1', name: 'Shaft Torque', value: 845, unit: 'N·m', min: 0, max: 1200, status: 'warning', history: generateHistory(845, 15, 60) },
    { id: 'volt-1', name: 'Bus Voltage', value: 672, unit: 'V', min: 0, max: 800, status: 'normal', history: generateHistory(672, 10, 60) },
    { id: 'curr-1', name: 'Phase Current', value: 32.1, unit: 'A', min: 0, max: 50, status: 'normal', history: generateHistory(32.1, 0.8, 60) },
    { id: 'pf-1', name: 'Power Factor', value: 0.94, unit: '', min: 0, max: 1, status: 'normal', history: generateHistory(0.94, 0.015, 60) },
    { id: 'thd-1', name: 'THD', value: 4.2, unit: '%', min: 0, max: 20, status: 'normal', history: generateHistory(4.2, 0.5, 60) },
    { id: 'freq-1', name: 'Frequency', value: 60.01, unit: 'Hz', min: 57, max: 63, status: 'normal', history: generateHistory(60.01, 0.03, 60) },
    { id: 'active-1', name: 'Active Power', value: 45.2, unit: 'kW', min: 0, max: 60, status: 'normal', history: generateHistory(45.2, 1.2, 60) },
    { id: 'reactive-1', name: 'Reactive Power', value: 12.8, unit: 'kVAR', min: 0, max: 30, status: 'normal', history: generateHistory(12.8, 0.8, 60) },
    { id: 'eff-1', name: 'Efficiency', value: 96.3, unit: '%', min: 0, max: 100, status: 'normal', history: generateHistory(96.3, 0.4, 60) },
    { id: 'temp-2', name: 'Temperature Rise', value: 42, unit: '°C', min: 0, max: 120, status: 'normal', history: generateHistory(42, 1.2, 60) },
  ]);

  const [stressField, setStressField] = useState<StressNode[]>([]);
  const [thermalField, setThermalField] = useState<ThermalNode[]>([]);
  const [electricalField, setElectricalField] = useState<ElectricalNode[]>([]);
  const [simulationTime, setSimulationTime] = useState(0);

  useEffect(() => {
    const nodes: StressNode[] = [];
    for (let x = 0; x < 20; x++) {
      for (let y = 0; y < 20; y++) {
        const cx = x - 10, cy = y - 10;
        const dist = Math.sqrt(cx * cx + cy * cy);
        nodes.push({
          x, y,
          vonMises: Math.max(0, 450 - dist * 30 + Math.random() * 50),
          principal: Math.max(0, 380 - dist * 25 + Math.random() * 40),
          strain: Math.max(0, 0.003 - dist * 0.0002 + Math.random() * 0.0005),
          displacement: Math.max(0, 2.5 - dist * 0.15 + Math.random() * 0.3),
        });
      }
    }
    setStressField(nodes);

    const thermal: ThermalNode[] = [];
    for (let x = 0; x < 20; x++) {
      for (let y = 0; y < 20; y++) {
        const hotspot1 = Math.sqrt((x - 5) ** 2 + (y - 5) ** 2);
        const hotspot2 = Math.sqrt((x - 15) ** 2 + (y - 12) ** 2);
        thermal.push({
          x, y,
          temperature: 200 + Math.max(300 / (1 + hotspot1), 250 / (1 + hotspot2)) + Math.random() * 20,
          heatFlux: Math.max(0, 5000 - hotspot1 * 300 + Math.random() * 500),
        });
      }
    }
    setThermalField(thermal);

    const electrical: ElectricalNode[] = [];
    for (let x = 0; x < 20; x++) {
      for (let y = 0; y < 20; y++) {
        const cx = x - 10;
        const cy = y - 10;
        const dist = Math.sqrt(cx * cx + cy * cy);
        const voltage = 520 + Math.max(0, 180 - dist * 8) + Math.random() * 12;
        const current = 8 + Math.max(0, 28 - dist * 1.1) + Math.random() * 1.5;
        const phase = ((x + y) * 18) % 360;
        electrical.push({
          x,
          y,
          voltage,
          current,
          power: Math.max(0, voltage * current * 0.9 + (Math.random() - 0.5) * 20),
          phase,
        });
      }
    }
    setElectricalField(electrical);
  }, []);

  useEffect(() => {
    const interval = setInterval(() => {
      timeRef.current += 1;
      setSimulationTime(t => t + 1);
      
      setSensors(prev => prev.map(sensor => {
        const variance = (sensor.max - sensor.min) * 0.005;
        let newValue = sensor.value + (Math.random() - 0.5) * variance * 2;
        newValue = Math.max(sensor.min, Math.min(sensor.max, newValue));
        
        const ratio = newValue / sensor.max;
        let status: 'normal' | 'warning' | 'critical' = 'normal';
        if (ratio > 0.85) status = 'critical';
        else if (ratio > 0.7) status = 'warning';

        const newHistory = [...sensor.history.slice(-59), { time: Date.now(), value: newValue }];

        return { ...sensor, value: newValue, status, history: newHistory };
      }));

      setStressField(prev => prev.map(node => ({
        ...node,
        vonMises: node.vonMises + (Math.random() - 0.5) * 5,
        principal: node.principal + (Math.random() - 0.5) * 4,
      })));

      setThermalField(prev => prev.map(node => ({
        ...node,
        temperature: node.temperature + (Math.random() - 0.5) * 2,
        heatFlux: Math.max(0, node.heatFlux + (Math.random() - 0.5) * 100),
      })));

      setElectricalField(prev => prev.map(node => {
        const voltageShift = (Math.random() - 0.5) * 4;
        const currentShift = (Math.random() - 0.5) * 0.3;
        const voltage = Math.max(420, Math.min(720, node.voltage + voltageShift));
        const current = Math.max(5, Math.min(45, node.current + currentShift));
        return {
          ...node,
          voltage,
          current,
          power: Math.max(0, voltage * current * 0.92 + (Math.random() - 0.5) * 10),
          phase: (node.phase + (Math.random() - 0.5) * 2 + 360) % 360,
        };
      }));
    }, 1000);

    return () => clearInterval(interval);
  }, []);

  return { sensors, stressField, thermalField, electricalField, simulationTime };
}

export function useOptimizationData() {
  const [generations, setGenerations] = useState<{ gen: number; fitness: number; diversity: number }[]>([]);
  
  useEffect(() => {
    const data = [];
    let fitness = 0.3;
    for (let i = 0; i < 50; i++) {
      fitness += (1 - fitness) * 0.04 + (Math.random() - 0.4) * 0.02;
      fitness = Math.min(0.98, fitness);
      data.push({ gen: i, fitness, diversity: Math.max(0.1, 1 - fitness * 0.8 + Math.random() * 0.1) });
    }
    setGenerations(data);
  }, []);

  useEffect(() => {
    const interval = setInterval(() => {
      setGenerations(prev => {
        if (prev.length === 0) return prev;
        const last = prev[prev.length - 1];
        const newFitness = Math.min(0.99, last.fitness + (1 - last.fitness) * 0.02 + (Math.random() - 0.4) * 0.005);
        return [...prev.slice(-99), {
          gen: last.gen + 1,
          fitness: newFitness,
          diversity: Math.max(0.05, 1 - newFitness * 0.85 + Math.random() * 0.05)
        }];
      });
    }, 2000);
    return () => clearInterval(interval);
  }, []);

  return { generations };
}
