import React from 'react';
import {
  Box, Palette, Ruler, Weight, Thermometer,
  ChevronDown, ChevronRight
} from 'lucide-react';

interface Props {
  selectedNode: string | null;
}

export const PropertyInspector: React.FC<Props> = ({ selectedNode }) => {
  if (!selectedNode) return null;

  return (
    <div className="border-t border-forge-border bg-forge-panel">
      <div className="px-3 py-2 border-b border-forge-border/50 flex items-center gap-1.5">
        <Box size={10} className="text-forge-blue" />
        <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">PROPERTIES</span>
      </div>

      <div className="p-2 space-y-2 max-h-[220px] overflow-y-auto">
        {/* Geometry Section */}
        <div>
          <button className="flex items-center gap-1 w-full text-left mb-1">
            <ChevronDown size={9} className="text-forge-text-muted" />
            <Ruler size={9} className="text-forge-accent" />
            <span className="text-[8px] font-semibold tracking-wider text-forge-text-dim">GEOMETRY</span>
          </button>
          <div className="pl-4 space-y-0.5">
            {[
              { label: 'Volume', value: '847.3 cm³' },
              { label: 'Surface Area', value: '1,247.8 cm²' },
              { label: 'Mass', value: '2.34 kg' },
              { label: 'Center of Mass', value: '[0, 12.4, -3.2]' },
              { label: 'Bounding Box', value: '142 × 38 × 12 mm' },
            ].map((prop, i) => (
              <div key={i} className="flex items-center justify-between py-0.5">
                <span className="text-[8px] text-forge-text-muted">{prop.label}</span>
                <span className="text-[8px] font-mono text-forge-text">{prop.value}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Material Section */}
        <div>
          <button className="flex items-center gap-1 w-full text-left mb-1">
            <ChevronDown size={9} className="text-forge-text-muted" />
            <Palette size={9} className="text-forge-purple" />
            <span className="text-[8px] font-semibold tracking-wider text-forge-text-dim">MATERIAL</span>
          </button>
          <div className="pl-4 space-y-0.5">
            {[
              { label: 'Name', value: 'Ti-6Al-4V (Grade 5)' },
              { label: 'Density', value: '4430 kg/m³' },
              { label: "Young's Modulus", value: '113.8 GPa' },
              { label: "Poisson's Ratio", value: '0.342' },
              { label: 'Yield Strength', value: '880 MPa' },
              { label: 'UTS', value: '950 MPa' },
              { label: 'Thermal Cond.', value: '6.7 W/(m·K)' },
              { label: 'CTE', value: '8.6 μm/(m·°C)' },
            ].map((prop, i) => (
              <div key={i} className="flex items-center justify-between py-0.5">
                <span className="text-[8px] text-forge-text-muted">{prop.label}</span>
                <span className="text-[8px] font-mono text-forge-purple">{prop.value}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Thermal Section */}
        <div>
          <button className="flex items-center gap-1 w-full text-left mb-1">
            <ChevronRight size={9} className="text-forge-text-muted" />
            <Thermometer size={9} className="text-forge-orange" />
            <span className="text-[8px] font-semibold tracking-wider text-forge-text-dim">THERMAL STATE</span>
          </button>
        </div>

        {/* Weight Section */}
        <div>
          <button className="flex items-center gap-1 w-full text-left mb-1">
            <ChevronRight size={9} className="text-forge-text-muted" />
            <Weight size={9} className="text-forge-teal" />
            <span className="text-[8px] font-semibold tracking-wider text-forge-text-dim">INERTIA PROPERTIES</span>
          </button>
        </div>
      </div>
    </div>
  );
};
