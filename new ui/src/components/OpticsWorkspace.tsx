import React, { useState } from 'react';
import { useForgeStore } from '../store/ForgeStoreContext';
import { Lightbulb, Play, Aperture } from 'lucide-react';

export const OpticsWorkspace: React.FC = () => {
  const store = useForgeStore();
  const [focalLength, setFocalLength] = useState(50);
  const [wavelength, setWavelength] = useState(550);
  const [aperture, setAperture] = useState(25);
  const [rays, setRays] = useState(7);
  const [result, setResult] = useState<string | null>(null);
  const [running, setRunning] = useState(false);

  const runTrace = async () => {
    setRunning(true);
    setResult(null);
    try {
      const res = await fetch('/api/optics/raytrace', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          focal_length_mm: focalLength,
          wavelength_nm: wavelength,
          aperture_mm: aperture,
          ray_count: rays,
        }),
      });
      if (res.ok) {
        const data = await res.json();
        setResult(JSON.stringify(data, null, 2));
        store.showToast('Ray trace complete (backend)');
      } else {
        throw new Error('offline');
      }
    } catch {
      // Thin-lens deterministic fallback
      const f = focalLength;
      const NA = (aperture / 2) / Math.max(f, 1);
      const spot = (1.22 * wavelength * 1e-6 * f) / Math.max(aperture, 0.1);
      const lines = Array.from({ length: rays }, (_, i) => {
        const h = ((i / Math.max(rays - 1, 1)) - 0.5) * aperture;
        const angle = Math.atan2(-h, f);
        return `ray[${i}] h=${h.toFixed(2)} mm → θ=${(angle * 180 / Math.PI).toFixed(3)}°`;
      });
      setResult(
        [
          `Thin-lens fallback · f=${f} mm · λ=${wavelength} nm`,
          `NA ≈ ${NA.toFixed(3)} · Airy radius ≈ ${(spot * 1000).toFixed(2)} µm`,
          ...lines,
        ].join('\n'),
      );
      store.showToast('Optics raytrace (offline lens model)');
    } finally {
      setRunning(false);
    }
  };

  return (
    <div className="w-full h-full flex bg-forge-dark/50 relative overflow-hidden">
      <div className="w-[280px] border-r border-forge-border bg-forge-dark/90 p-4 flex flex-col gap-3 flex-shrink-0">
        <div className="flex items-center gap-2 text-forge-accent">
          <Aperture size={14} />
          <span className="text-[10px] font-bold tracking-widest">LENS / RAY PARAMS</span>
        </div>
        {([
          ['Focal length (mm)', focalLength, setFocalLength, 5, 500],
          ['Wavelength (nm)', wavelength, setWavelength, 380, 780],
          ['Aperture Ø (mm)', aperture, setAperture, 1, 100],
          ['Ray count', rays, setRays, 3, 21],
        ] as const).map(([label, value, setter, min, max]) => (
          <label key={label} className="flex flex-col gap-1">
            <span className="text-[8px] font-mono text-forge-text-muted">{label}</span>
            <input
              type="number"
              min={min}
              max={max}
              value={value}
              onChange={e => setter(Number(e.target.value))}
              className="bg-forge-black border border-forge-border rounded px-2 py-1 text-[10px] font-mono text-forge-text"
            />
          </label>
        ))}
        <button
          onClick={runTrace}
          disabled={running}
          className="mt-2 flex items-center justify-center gap-2 py-2 rounded bg-forge-accent/20 text-forge-accent border border-forge-accent/50 text-[10px] font-bold font-mono hover:bg-forge-accent/30 disabled:opacity-40"
        >
          <Play size={12} /> {running ? 'TRACING…' : 'RUN RAY TRACE'}
        </button>
        <button
          onClick={() => store.setImportDialogOpen(true)}
          className="py-2 rounded border border-forge-green/40 text-forge-green text-[10px] font-bold font-mono hover:bg-forge-green/10"
        >
          DROP IN COMPONENT
        </button>
      </div>

      <div className="flex-1 flex flex-col min-w-0 p-6">
        <div className="flex items-center gap-2 mb-4">
          <Lightbulb size={18} className="text-forge-accent" />
          <span className="text-forge-text font-mono text-sm tracking-wider">OPTICS & PHOTONICS</span>
        </div>
        <div className="flex-1 rounded-xl border border-forge-border/60 bg-forge-black/60 p-4 overflow-auto">
          {result ? (
            <pre className="text-[10px] font-mono text-forge-text-dim whitespace-pre-wrap">{result}</pre>
          ) : (
            <p className="text-sm text-forge-text-muted font-mono">
              Set lens parameters and run a ray trace. Backend `/api/optics/raytrace` is used when available.
            </p>
          )}
        </div>
      </div>
    </div>
  );
};
