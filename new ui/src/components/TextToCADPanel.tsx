import React, { useRef, useEffect, useState } from 'react';
import { Wand2, Loader2, Box, Cylinder, Circle, Triangle, Donut, Download, Plus, AlertCircle } from 'lucide-react';
import * as THREE from 'three';

const BRIDGE_BASE = 'http://localhost:8003';

interface CADSpec {
  shape: string;
  material: string;
  features: Record<string, boolean>;
  length?: number;
  width?: number;
  height?: number;
  radius?: number;
  radius1?: number;
  radius2?: number;
  major_radius?: number;
  minor_radius?: number;
}

interface T2CADResult {
  status: string;
  prompt: string;
  spec: CADSpec;
  step_file: string | null;
  step_available: boolean;
  geometry: Record<string, unknown>;
  engine: string;
}

const SHAPE_ICONS: Record<string, React.ReactNode> = {
  box: <Box size={14} />,
  cylinder: <Cylinder size={14} />,
  sphere: <Circle size={14} />,
  cone: <Triangle size={14} />,
  torus: <Donut size={14} />,
};

const EXAMPLE_PROMPTS = [
  'Steel bracket 120mm × 60mm × 15mm with fillets',
  'Aluminum cylinder shaft 25mm radius 200mm long',
  'Titanium sphere 40mm radius',
  'Hollow steel pipe 30mm radius 150mm tall',
  'Copper torus ring 50mm major radius 10mm tube',
];

// ── Three.js preview ──────────────────────────────────────────────────────────

function buildGeometry(geo: Record<string, unknown>): THREE.BufferGeometry {
  const type = geo.type as string;
  if (type === 'box')
    return new THREE.BoxGeometry(geo.width as number, geo.height as number, geo.depth as number);
  if (type === 'cylinder')
    return new THREE.CylinderGeometry(
      geo.radiusTop as number, geo.radiusBottom as number,
      geo.height as number, (geo.radialSegments as number) ?? 32
    );
  if (type === 'sphere')
    return new THREE.SphereGeometry(geo.radius as number, 32, 16);
  if (type === 'cone')
    return new THREE.CylinderGeometry(
      geo.radiusTop as number, geo.radiusBottom as number,
      geo.height as number, 32
    );
  if (type === 'torus')
    return new THREE.TorusGeometry(
      geo.radius as number, geo.tube as number,
      (geo.radialSegments as number) ?? 16, (geo.tubularSegments as number) ?? 48
    );
  return new THREE.BoxGeometry(1, 1, 1);
}

const GeometryPreview: React.FC<{ geometry: Record<string, unknown> }> = ({ geometry }) => {
  const mountRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const el = mountRef.current;
    if (!el) return;

    const w = el.clientWidth || 260;
    const h = el.clientHeight || 180;

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setSize(w, h);
    renderer.setPixelRatio(window.devicePixelRatio);
    el.appendChild(renderer.domElement);

    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(45, w / h, 0.1, 10000);

    const geo = buildGeometry(geometry);
    const mat = new THREE.MeshStandardMaterial({ color: 0x00d4ff, metalness: 0.6, roughness: 0.3, wireframe: false });
    const mesh = new THREE.Mesh(geo, mat);
    scene.add(mesh);

    // Scale to fit view
    geo.computeBoundingBox();
    const box = geo.boundingBox!;
    const size = new THREE.Vector3();
    box.getSize(size);
    const maxDim = Math.max(size.x, size.y, size.z);
    const scale = 120 / maxDim;
    mesh.scale.setScalar(scale);

    camera.position.set(200, 150, 200);
    camera.lookAt(0, 0, 0);

    scene.add(new THREE.AmbientLight(0xffffff, 0.4));
    const dir = new THREE.DirectionalLight(0xffffff, 1.2);
    dir.position.set(200, 300, 200);
    scene.add(dir);

    let frameId: number;
    const animate = () => {
      frameId = requestAnimationFrame(animate);
      mesh.rotation.y += 0.008;
      renderer.render(scene, camera);
    };
    animate();

    return () => {
      cancelAnimationFrame(frameId);
      renderer.dispose();
      geo.dispose();
      mat.dispose();
      el.removeChild(renderer.domElement);
    };
  }, [geometry]);

  return <div ref={mountRef} className="w-full h-[180px] rounded-lg overflow-hidden bg-forge-black/60" />;
};

// ── Main panel ────────────────────────────────────────────────────────────────

export const TextToCADPanel: React.FC = () => {
  const [prompt, setPrompt] = useState('');
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState<T2CADResult | null>(null);
  const [error, setError] = useState<string | null>(null);

  const generate = async (p = prompt) => {
    if (!p.trim()) return;
    setLoading(true);
    setError(null);
    setResult(null);
    try {
      const res = await fetch(`${BRIDGE_BASE}/cad/text-to-cad`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ prompt: p }),
      });
      if (!res.ok) throw new Error(`Server error ${res.status}`);
      const data: T2CADResult = await res.json();
      setResult(data);
    } catch (e: unknown) {
      setError(e instanceof Error ? e.message : 'Request failed');
    }
    setLoading(false);
  };

  const spec = result?.spec;

  return (
    <div className="h-full flex flex-col bg-forge-black overflow-hidden">
      {/* Header */}
      <div className="h-[48px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-2 flex-shrink-0">
        <Wand2 size={14} className="text-forge-accent" />
        <div>
          <div className="text-[10px] font-bold tracking-widest text-forge-accent">TEXT → CAD</div>
          <div className="text-[8px] font-mono text-forge-text-muted">NATURAL LANGUAGE TO 3D GEOMETRY</div>
        </div>
      </div>

      <div className="flex-1 overflow-y-auto p-4 space-y-4 min-h-0">
        {/* Prompt input */}
        <div>
          <div className="text-[9px] font-semibold tracking-widest text-forge-text-dim mb-2">DESCRIBE YOUR PART</div>
          <textarea
            value={prompt}
            onChange={e => setPrompt(e.target.value)}
            onKeyDown={e => { if (e.key === 'Enter' && (e.ctrlKey || e.metaKey)) generate(); }}
            placeholder="e.g. Steel bracket 120mm × 60mm × 15mm with fillets"
            rows={3}
            className="w-full px-3 py-2 rounded bg-forge-surface border border-forge-border text-[11px] text-forge-text placeholder-forge-text-muted focus:outline-none focus:border-forge-accent/50 resize-none transition-colors"
          />
          <div className="flex items-center justify-between mt-2">
            <span className="text-[8px] text-forge-text-muted font-mono">Ctrl+Enter to generate</span>
            <button
              onClick={() => generate()}
              disabled={loading || !prompt.trim()}
              className="flex items-center gap-1.5 px-4 py-1.5 rounded bg-forge-accent/15 text-forge-accent border border-forge-accent/30 text-[10px] font-bold tracking-wider hover:bg-forge-accent/25 transition-all disabled:opacity-40"
            >
              {loading ? <Loader2 size={11} className="animate-spin" /> : <Wand2 size={11} />}
              {loading ? 'GENERATING...' : 'GENERATE'}
            </button>
          </div>
        </div>

        {/* Example prompts */}
        <div>
          <div className="text-[9px] font-semibold tracking-widest text-forge-text-dim mb-2">EXAMPLES</div>
          <div className="flex flex-wrap gap-1.5">
            {EXAMPLE_PROMPTS.map(p => (
              <button
                key={p}
                onClick={() => { setPrompt(p); generate(p); }}
                className="px-2 py-1 rounded text-[8px] font-mono text-forge-text-muted border border-forge-border/50 hover:border-forge-accent/40 hover:text-forge-accent transition-all"
              >
                {p.length > 36 ? p.slice(0, 36) + '…' : p}
              </button>
            ))}
          </div>
        </div>

        {/* Error */}
        {error && (
          <div className="flex items-center gap-2 px-3 py-2 rounded bg-forge-red/10 border border-forge-red/30 text-[10px] text-forge-red">
            <AlertCircle size={12} />
            {error}
          </div>
        )}

        {/* Result */}
        {result && spec && (
          <>
            {/* 3D Preview */}
            <div>
              <div className="text-[9px] font-semibold tracking-widest text-forge-text-dim mb-2">3D PREVIEW</div>
              <GeometryPreview geometry={result.geometry} />
            </div>

            {/* Spec card */}
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/10 space-y-2">
              <div className="flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <span className="text-forge-accent">{SHAPE_ICONS[spec.shape] ?? <Box size={14} />}</span>
                  <span className="text-[11px] font-bold text-forge-text capitalize">{spec.shape}</span>
                  <span className="text-[8px] font-mono text-forge-text-muted px-1.5 py-0.5 rounded bg-forge-surface border border-forge-border/50 capitalize">{spec.material}</span>
                </div>
                <span className="text-[8px] font-mono text-forge-text-muted">{result.engine}</span>
              </div>

              {/* Dimensions */}
              <div className="grid grid-cols-3 gap-2">
                {spec.length !== undefined && (
                  <div className="text-center p-2 rounded bg-forge-dark border border-forge-border/30">
                    <div className="text-[10px] font-mono font-bold text-forge-accent">{spec.length.toFixed(1)}</div>
                    <div className="text-[7px] text-forge-text-muted">LENGTH mm</div>
                  </div>
                )}
                {spec.width !== undefined && (
                  <div className="text-center p-2 rounded bg-forge-dark border border-forge-border/30">
                    <div className="text-[10px] font-mono font-bold text-forge-accent">{spec.width.toFixed(1)}</div>
                    <div className="text-[7px] text-forge-text-muted">WIDTH mm</div>
                  </div>
                )}
                {spec.height !== undefined && (
                  <div className="text-center p-2 rounded bg-forge-dark border border-forge-border/30">
                    <div className="text-[10px] font-mono font-bold text-forge-accent">{spec.height.toFixed(1)}</div>
                    <div className="text-[7px] text-forge-text-muted">HEIGHT mm</div>
                  </div>
                )}
                {spec.radius !== undefined && (
                  <div className="text-center p-2 rounded bg-forge-dark border border-forge-border/30">
                    <div className="text-[10px] font-mono font-bold text-forge-accent">{spec.radius.toFixed(1)}</div>
                    <div className="text-[7px] text-forge-text-muted">RADIUS mm</div>
                  </div>
                )}
                {spec.major_radius !== undefined && (
                  <div className="text-center p-2 rounded bg-forge-dark border border-forge-border/30">
                    <div className="text-[10px] font-mono font-bold text-forge-accent">{spec.major_radius.toFixed(1)}</div>
                    <div className="text-[7px] text-forge-text-muted">MAJOR R mm</div>
                  </div>
                )}
                {spec.minor_radius !== undefined && (
                  <div className="text-center p-2 rounded bg-forge-dark border border-forge-border/30">
                    <div className="text-[10px] font-mono font-bold text-forge-accent">{spec.minor_radius.toFixed(1)}</div>
                    <div className="text-[7px] text-forge-text-muted">MINOR R mm</div>
                  </div>
                )}
              </div>

              {/* Features */}
              {Object.entries(spec.features).some(([, v]) => v) && (
                <div className="flex flex-wrap gap-1.5 pt-1">
                  {Object.entries(spec.features).filter(([, v]) => v).map(([k]) => (
                    <span key={k} className="px-2 py-0.5 rounded text-[8px] font-mono bg-forge-green/10 text-forge-green border border-forge-green/20 capitalize">
                      {k}
                    </span>
                  ))}
                </div>
              )}
            </div>

            {/* Actions */}
            <div className="flex gap-2">
              {result.step_available && result.step_file && (
                <a
                  href={`${BRIDGE_BASE}/cad/download?path=${encodeURIComponent(result.step_file)}`}
                  className="flex items-center gap-1.5 px-3 py-1.5 rounded bg-forge-green/10 text-forge-green border border-forge-green/20 text-[10px] font-semibold hover:bg-forge-green/20 transition-all"
                >
                  <Download size={11} />
                  DOWNLOAD STEP
                </a>
              )}
              <button
                onClick={() => {
                  const json = JSON.stringify(result.spec, null, 2);
                  const blob = new Blob([json], { type: 'application/json' });
                  const url = URL.createObjectURL(blob);
                  const a = document.createElement('a');
                  a.href = url; a.download = `szm_t2cad_${spec.shape}.json`;
                  a.click(); URL.revokeObjectURL(url);
                }}
                className="flex items-center gap-1.5 px-3 py-1.5 rounded bg-forge-surface text-forge-text-dim border border-forge-border/50 text-[10px] font-semibold hover:border-forge-border transition-all"
              >
                <Download size={11} />
                EXPORT SPEC
              </button>
              <button
                onClick={() => { setResult(null); setPrompt(''); }}
                className="flex items-center gap-1.5 px-3 py-1.5 rounded bg-forge-accent/10 text-forge-accent border border-forge-accent/20 text-[10px] font-semibold hover:bg-forge-accent/20 transition-all ml-auto"
              >
                <Plus size={11} />
                NEW PART
              </button>
            </div>
          </>
        )}
      </div>
    </div>
  );
};
