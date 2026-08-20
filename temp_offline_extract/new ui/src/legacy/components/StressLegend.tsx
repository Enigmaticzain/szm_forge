import { useState } from 'react';

interface StressLegendProps {
  mode: string;
}

const stressColors = [
  { value: '0', color: '#003366', label: 'Minimal' },
  { value: '50', color: '#005599', label: '' },
  { value: '100', color: '#0088cc', label: 'Low' },
  { value: '150', color: '#00cc66', label: '' },
  { value: '200', color: '#66cc00', label: 'Moderate' },
  { value: '250', color: '#cccc00', label: '' },
  { value: '300', color: '#ffaa00', label: 'Elevated' },
  { value: '350', color: '#ff6600', label: '' },
  { value: '400', color: '#ff3300', label: 'Critical' },
  { value: '450+', color: '#cc0000', label: 'OVERLOAD' },
];

const thermalColors = [
  { value: '20°C', color: '#0044ff', label: 'Ambient' },
  { value: '100°C', color: '#0088ff', label: '' },
  { value: '200°C', color: '#00ccaa', label: 'Warm' },
  { value: '300°C', color: '#88cc00', label: '' },
  { value: '400°C', color: '#ffcc00', label: 'Hot' },
  { value: '500°C', color: '#ff6600', label: '' },
  { value: '600°C', color: '#ff3300', label: 'Critical' },
  { value: '700°C+', color: '#cc0000', label: 'MELTDOWN' },
];

export default function StressLegend({ mode }: StressLegendProps) {
  const [selectedType, setSelectedType] = useState(mode === 'thermal' ? 'thermal' : 'vonMises');
  
  const colors = mode === 'thermal' ? thermalColors : stressColors;
  
  const types = mode === 'thermal' 
    ? ['thermal', 'heatFlux', 'expansion']
    : ['vonMises', 'principal', 'strain', 'displacement'];

  const typeLabels: Record<string, string> = {
    vonMises: 'Von Mises Stress',
    principal: 'Principal Stress',
    strain: 'Equiv. Strain',
    displacement: 'Total Displacement',
    thermal: 'Temperature',
    heatFlux: 'Heat Flux',
    expansion: 'Thermal Expansion',
  };

  const typeUnits: Record<string, string> = {
    vonMises: 'MPa',
    principal: 'MPa',
    strain: 'mm/mm',
    displacement: 'mm',
    thermal: '°C',
    heatFlux: 'W/m²',
    expansion: 'mm',
  };

  return (
    <div className="glass-panel rounded-lg p-3 w-[180px]">
      <div className="text-[8px] font-bold text-forge-accent tracking-wider mb-2">
        {mode === 'thermal' ? 'THERMAL FIELD' : 'STRESS FIELD'}
      </div>
      
      {/* Type selector */}
      <div className="flex flex-wrap gap-1 mb-3">
        {types.map(t => (
          <button
            key={t}
            onClick={() => setSelectedType(t)}
            className={`px-1.5 py-0.5 text-[7px] font-bold tracking-wider rounded transition-all ${
              selectedType === t
                ? 'bg-forge-accent/20 text-forge-accent border border-forge-accent/30'
                : 'text-forge-text-muted hover:text-forge-text-dim border border-transparent'
            }`}
          >
            {t.toUpperCase()}
          </button>
        ))}
      </div>

      <div className="text-[9px] text-forge-text-dim mb-2">{typeLabels[selectedType]}</div>
      
      {/* Color scale */}
      <div className="space-y-0">
        {colors.map((c, i) => (
          <div key={i} className="flex items-center gap-2">
            <div 
              className="w-full h-3 flex-shrink-0"
              style={{ backgroundColor: c.color }}
            />
            <span className="text-[8px] text-forge-text-muted font-mono w-12 text-right flex-shrink-0">{c.value}</span>
          </div>
        ))}
      </div>

      <div className="mt-2 pt-2 border-t border-forge-border space-y-1">
        <div className="flex justify-between text-[8px]">
          <span className="text-forge-text-muted">Unit</span>
          <span className="text-forge-text font-mono">{typeUnits[selectedType]}</span>
        </div>
        <div className="flex justify-between text-[8px]">
          <span className="text-forge-text-muted">Max</span>
          <span className="text-forge-danger font-mono font-bold">
            {mode === 'thermal' ? '687.3 °C' : '448.2 MPa'}
          </span>
        </div>
        <div className="flex justify-between text-[8px]">
          <span className="text-forge-text-muted">Min</span>
          <span className="text-forge-green font-mono">
            {mode === 'thermal' ? '21.4 °C' : '12.7 MPa'}
          </span>
        </div>
        <div className="flex justify-between text-[8px]">
          <span className="text-forge-text-muted">Safety Factor</span>
          <span className={`font-mono font-bold ${mode === 'thermal' ? 'text-forge-warning' : 'text-forge-warning'}`}>
            1.42
          </span>
        </div>
      </div>
    </div>
  );
}
