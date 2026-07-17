import React, { useState, useEffect } from 'react';
import {
  Factory, Package, Settings, CheckCircle,
  Ruler, Cpu,
  ArrowRight, Clock, Shield, Code, Download
} from 'lucide-react';

const bomItems = [
  { part: 'Primary Blade', material: 'Ti-6Al-4V', qty: 12, weight: '2.34 kg', process: 'Forging + 5-Axis CNC', status: 'ready', tolerance: '±0.02mm' },
  { part: 'Secondary Blade', material: 'IN718', qty: 12, weight: '1.87 kg', process: '5-Axis CNC', status: 'review', tolerance: '±0.015mm' },
  { part: 'Central Hub', material: 'Steel 4340', qty: 1, weight: '14.2 kg', process: 'Casting + CNC', status: 'ready', tolerance: '±0.05mm' },
  { part: 'Bearing Housing', material: 'Steel 4340', qty: 2, weight: '3.1 kg', process: '3-Axis CNC', status: 'ready', tolerance: '±0.01mm' },
  { part: 'Shaft Coupling', material: 'Steel 4340', qty: 1, weight: '5.8 kg', process: 'Turning + Grinding', status: 'processing', tolerance: '±0.005mm' },
  { part: 'Outer Nacelle', material: 'CFRP', qty: 1, weight: '8.4 kg', process: 'Layup + Autoclave', status: 'review', tolerance: '±0.1mm' },
  { part: 'Inner Shroud', material: 'Al 7075', qty: 1, weight: '4.2 kg', process: 'Sheet Metal + CNC', status: 'ready', tolerance: '±0.05mm' },
  { part: 'Blade Root Attach', material: 'Ti-6Al-4V', qty: 12, weight: '0.45 kg', process: 'Precision CNC', status: 'ready', tolerance: '±0.008mm' },
];

const processSteps = [
  { name: 'Design Validation', status: 'complete', progress: 100 },
  { name: 'DFM Analysis', status: 'complete', progress: 100 },
  { name: 'Tool Path Gen', status: 'active', progress: 78 },
  { name: 'Material Procurement', status: 'active', progress: 65 },
  { name: 'Quality Planning', status: 'pending', progress: 0 },
  { name: 'Production', status: 'pending', progress: 0 },
];

const statusBadge = {
  ready: { color: 'text-forge-green', bg: 'bg-forge-green/10', border: 'border-forge-green/20' },
  review: { color: 'text-forge-yellow', bg: 'bg-forge-yellow/10', border: 'border-forge-yellow/20' },
  processing: { color: 'text-forge-accent', bg: 'bg-forge-accent/10', border: 'border-forge-accent/20' },
  hold: { color: 'text-forge-red', bg: 'bg-forge-red/10', border: 'border-forge-red/20' },
};

export const ManufacturingDashboard: React.FC = () => {
  const [dfmResults, setDfmResults] = useState<any[]>([]);
  const [gcode, setGcode] = useState<string | null>(null);
  const [isGenerating, setIsGenerating] = useState(false);

  useEffect(() => {
    // Fetch DFM validation
    fetch('http://localhost:8001/api/manufacturability/validate', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        component_name: 'AI Hub Assembly',
        parameters: { thickness: 0.015 },
        process: 'CNC'
      })
    })
      .then(res => res.json())
      .then(data => {
        if (data && data.checks) {
          setDfmResults(data.checks);
        }
      })
      .catch(console.error);
  }, []);

  const handleGenerateGCode = async () => {
    setIsGenerating(true);
    try {
      const res = await fetch('http://localhost:8001/api/manufacturability/export-gcode', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          component_name: 'AI Hub Assembly',
          bounds: { length: 0.8, width: 0.4, depth: 0.15 }
        })
      });
      const data = await res.json();
      setGcode(data.gcode);
    } catch (e) {
      console.error(e);
    }
    setIsGenerating(false);
  };

  return (
    <div className="h-full flex flex-col bg-forge-black overflow-y-auto">
      {/* Header */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-6 flex-shrink-0">
        <div className="flex items-center gap-2">
          <Factory size={14} className="text-forge-accent" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-accent">MANUFACTURING INTELLIGENCE</div>
            <div className="text-[8px] font-mono text-forge-text-muted">BOM • DFM • TOLERANCE • CNC PREP • QUALITY</div>
          </div>
        </div>
        <div className="ml-auto flex items-center gap-4">
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">TOTAL PARTS</div>
            <div className="text-[14px] font-bold font-mono text-forge-text">42</div>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">READY</div>
            <div className="text-[14px] font-bold font-mono text-forge-green">36</div>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">REVIEW</div>
            <div className="text-[14px] font-bold font-mono text-forge-yellow">4</div>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">EST. COST</div>
            <div className="text-[14px] font-bold font-mono text-forge-accent">$47.2K</div>
          </div>
        </div>
      </div>

      <div className="flex-1 p-4 space-y-4 overflow-y-auto">
        {/* Process Pipeline */}
        <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
          <div className="flex items-center gap-2 mb-4">
            <ArrowRight size={12} className="text-forge-accent" />
            <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">MANUFACTURING PIPELINE</span>
          </div>
          <div className="flex items-center gap-2">
            {processSteps.map((step, i) => (
              <React.Fragment key={i}>
                <div className={`flex-1 p-3 rounded border transition-all ${
                  step.status === 'complete' ? 'border-forge-green/20 bg-forge-green/5' :
                  step.status === 'active' ? 'border-forge-accent/20 bg-forge-accent/5' :
                  'border-forge-border/50 bg-forge-surface/20'
                }`}>
                  <div className="flex items-center gap-1.5 mb-1.5">
                    {step.status === 'complete' ? <CheckCircle size={10} className="text-forge-green" /> :
                     step.status === 'active' ? <Cpu size={10} className="text-forge-accent animate-pulse" /> :
                     <Clock size={10} className="text-forge-text-muted" />}
                    <span className="text-[9px] font-medium text-forge-text-dim">{step.name}</span>
                  </div>
                  <div className="w-full h-1 bg-forge-dark rounded-full overflow-hidden">
                    <div
                      className={`h-full rounded-full transition-all duration-500 ${
                        step.status === 'complete' ? 'bg-forge-green' :
                        step.status === 'active' ? 'bg-forge-accent' : 'bg-forge-border'
                      }`}
                      style={{ width: `${step.progress}%` }}
                    />
                  </div>
                  <div className="text-[8px] font-mono text-forge-text-muted mt-1">{step.progress}%</div>
                </div>
                {i < processSteps.length - 1 && (
                  <ArrowRight size={12} className="text-forge-border flex-shrink-0" />
                )}
              </React.Fragment>
            ))}
          </div>
        </div>

        {/* BOM Table */}
        <div className="rounded-lg border border-forge-border bg-forge-panel overflow-hidden">
          <div className="px-4 py-3 border-b border-forge-border flex items-center gap-2">
            <Package size={12} className="text-forge-accent" />
            <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">BILL OF MATERIALS</span>
            <span className="text-[8px] font-mono text-forge-text-muted ml-2">{bomItems.length} items</span>
          </div>
          <div className="overflow-x-auto">
            <table className="w-full">
              <thead>
                <tr className="border-b border-forge-border/50">
                  {['Part', 'Material', 'Qty', 'Weight', 'Process', 'Tolerance', 'Status'].map(h => (
                    <th key={h} className="px-3 py-2 text-left text-[8px] font-mono tracking-wider text-forge-text-muted font-semibold">
                      {h.toUpperCase()}
                    </th>
                  ))}
                </tr>
              </thead>
              <tbody>
                {bomItems.map((item, i) => {
                  const st = statusBadge[item.status as keyof typeof statusBadge];
                  return (
                    <tr key={i} className="border-b border-forge-border/30 hover:bg-forge-surface/30 transition-colors">
                      <td className="px-3 py-2 text-[10px] font-medium text-forge-text">{item.part}</td>
                      <td className="px-3 py-2 text-[9px] font-mono text-forge-purple">{item.material}</td>
                      <td className="px-3 py-2 text-[10px] font-mono text-forge-text">{item.qty}</td>
                      <td className="px-3 py-2 text-[9px] font-mono text-forge-text-dim">{item.weight}</td>
                      <td className="px-3 py-2 text-[9px] text-forge-text-dim">{item.process}</td>
                      <td className="px-3 py-2 text-[9px] font-mono text-forge-accent">{item.tolerance}</td>
                      <td className="px-3 py-2">
                        <span className={`text-[7px] font-mono tracking-wider px-1.5 py-0.5 rounded ${st.bg} ${st.color} border ${st.border}`}>
                          {item.status.toUpperCase()}
                        </span>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        </div>

        {/* Bottom Grid */}
        <div className="grid grid-cols-3 gap-4">
          {/* DFM Analysis */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-1.5 mb-3">
              <Settings size={11} className="text-forge-accent" />
              <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">DFM ANALYSIS</span>
            </div>
            <div className="space-y-2">
              {dfmResults.length > 0 ? dfmResults.map((item, i) => (
                <div key={i} className="flex items-center justify-between">
                  <span className="text-[9px] text-forge-text-dim">{item.check}</span>
                  <div className="flex items-center gap-2">
                    <span className="text-[8px] font-mono text-forge-text-muted truncate max-w-[120px]">{item.value}</span>
                    <span className={`text-[7px] font-mono ${item.result === 'PASS' ? 'text-forge-green' : item.result === 'WARNING' ? 'text-forge-yellow' : 'text-forge-red'}`}>
                      {item.result}
                    </span>
                  </div>
                </div>
              )) : (
                <div className="text-[10px] text-forge-text-muted font-mono animate-pulse">Running analysis against Python backend...</div>
              )}
            </div>
          </div>

          {/* Tolerance Stack */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-1.5 mb-3">
              <Ruler size={11} className="text-forge-accent" />
              <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">TOLERANCE ANALYSIS</span>
            </div>
            <div className="space-y-2">
              {[
                { dim: 'Blade-Hub Interface', stack: '±0.035mm', margin: '82%' },
                { dim: 'Bearing Bore', stack: '±0.008mm', margin: '95%' },
                { dim: 'Shaft Alignment', stack: '±0.012mm', margin: '91%' },
                { dim: 'Nacelle Clearance', stack: '±0.15mm', margin: '78%' },
                { dim: 'Root Fillet Radius', stack: '±0.1mm', margin: '88%' },
              ].map((item, i) => (
                <div key={i}>
                  <div className="flex items-center justify-between mb-0.5">
                    <span className="text-[9px] text-forge-text-dim">{item.dim}</span>
                    <span className="text-[8px] font-mono text-forge-accent">{item.stack}</span>
                  </div>
                  <div className="flex items-center gap-2">
                    <div className="flex-1 h-1 bg-forge-dark rounded-full overflow-hidden">
                      <div className="h-full bg-forge-teal rounded-full" style={{ width: item.margin }} />
                    </div>
                    <span className="text-[7px] font-mono text-forge-text-muted">{item.margin}</span>
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* G-Code Generation */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel flex flex-col">
            <div className="flex items-center justify-between mb-3">
              <div className="flex items-center gap-1.5">
                <Code size={11} className="text-forge-accent" />
                <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">ROBOTIC PATH GEN</span>
              </div>
              {gcode && (
                <button className="text-[10px] text-forge-purple hover:text-forge-purple/80 transition-colors flex items-center gap-1">
                  <Download size={10} />
                  <span>DOWNLOAD .NC</span>
                </button>
              )}
            </div>
            
            {!gcode ? (
              <div className="flex-1 flex flex-col items-center justify-center border border-dashed border-forge-border rounded bg-forge-dark/50 p-4">
                <button
                  onClick={handleGenerateGCode}
                  disabled={isGenerating}
                  className="px-4 py-2 bg-forge-purple text-white rounded font-semibold text-[10px] hover:bg-forge-purple/80 transition-colors disabled:opacity-50"
                >
                  {isGenerating ? 'GENERATING...' : 'GENERATE CNC G-CODE'}
                </button>
              </div>
            ) : (
              <div className="flex-1 bg-forge-dark rounded border border-forge-border p-2 overflow-y-auto">
                <pre className="text-[8px] font-mono text-forge-green select-text">
                  {gcode}
                </pre>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
