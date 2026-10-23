import React, { useState, useRef, useEffect } from 'react';
import { useForgeStore } from '../store/ForgeStoreContext';
import { createGeometryPart, analyzeGeometryPart, geometryBridgeUrl } from '../services/forgeApi';
import { PenTool, Move, Layers, Settings, Maximize2, Play, Download, AlertTriangle } from 'lucide-react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { STLLoader } from 'three/examples/jsm/loaders/STLLoader.js';

export const GeometryWorkspace: React.FC = () => {
  const store = useForgeStore();
  const [activeTab, setActiveTab] = useState<'create' | 'analyze'>('create');
  const [partType, setPartType] = useState('bracket');
  const [isGenerating, setIsGenerating] = useState(false);
  const [stlUrl, setStlUrl] = useState<string | null>(null);
  const [analysis, setAnalysis] = useState<any>(null);
  const [isOmniverseStreaming, setIsOmniverseStreaming] = useState(false);
  
  // Params
  const [params, setParams] = useState<Record<string, any>>({
    length: 80, width: 40, thickness: 5, holes: 2
  });

  const handleParamChange = (key: string, value: any) => {
    setParams(p => ({ ...p, [key]: value }));
  };

  const handlePartTypeChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    const type = e.target.value;
    setPartType(type);
    if (type === 'bracket') setParams({ length: 80, width: 40, thickness: 5, holes: 2, fillet_r: 1 });
    else if (type === 'shaft') setParams({ diameter: 20, length: 100, keyway: true, chamfer: true });
    else if (type === 'flange') setParams({ outer_d: 80, inner_d: 30, thickness: 10, bolt_holes: 6, bolt_d: 8 });
    else if (type === 'housing') setParams({ length: 120, width: 80, height: 60, wall_thickness: 5, bore_d: 30 });
    else if (type === 'gear') setParams({ module: 2, num_teeth: 20, thickness: 10, bore_d: 10 });
    else if (type === 'plate') setParams({ length: 200, width: 100, thickness: 5, hole_pattern: "grid", hole_rows: 2, hole_cols: 4 });
  };

  const generatePart = async () => {
    setIsGenerating(true);
    setAnalysis(null);
    try {
      const data = await createGeometryPart(partType, params, 'stl');
      if (!data?.render_url) throw new Error('Could not generate part');
      setStlUrl(geometryBridgeUrl(data.render_url as string));
      store.showToast(`Successfully generated ${partType}!`);
      analyzePart();
    } catch (e: any) {
      store.showToast(`Error: ${e.message}`, 'error');
    } finally {
      setIsGenerating(false);
    }
  };

  const analyzePart = async () => {
    try {
      const data = await analyzeGeometryPart(partType, params);
      if (data) setAnalysis(data);
    } catch (e) {
      console.error(e);
    }
  };

  return (
    <div className="w-full h-full flex bg-forge-dark relative overflow-hidden">
      
      {/* 3D Canvas Area */}
      <div className="flex-1 relative">
        {isOmniverseStreaming ? (
          <div className="w-full h-full bg-forge-dark flex flex-col items-center justify-center relative">
             <div className="absolute top-4 right-4 bg-black/60 px-3 py-1 rounded text-forge-accent font-mono text-xs animate-pulse border border-forge-accent/30 z-10 flex items-center gap-2">
                <span className="w-2 h-2 rounded-full bg-forge-accent"></span>
                NUCLEUS LIVE SYNC
             </div>
             {/* Using standard Omniverse WebRTC streaming port 8211 for local development */}
             <iframe src="http://localhost:8211/streaming/webrtc-demo/?server=localhost" className="w-full h-full border-none" title="Omniverse Stream" />
          </div>
        ) : stlUrl ? (
          <StlViewer url={stlUrl} />
        ) : (
          <div className="flex-1 w-full h-full flex items-center justify-center bg-[url('/grid-pattern.svg')] opacity-30">
            <div className="flex flex-col items-center opacity-50">
              <Maximize2 size={48} className="text-forge-accent mb-4" />
              <span className="text-forge-text font-mono text-sm tracking-wider">GEOMETRY ENGINE READY</span>
              <span className="text-forge-text-muted font-mono text-xs mt-2">Select a part and click Generate</span>
            </div>
          </div>
        )}
      </div>
      
      {/* Sidebar Control Panel */}
      <div className="w-[340px] bg-forge-panel border-l border-forge-border flex flex-col shadow-xl z-20">
        <div className="p-4 border-b border-forge-border">
          <h2 className="text-forge-text font-mono font-bold flex items-center gap-2">
            <Layers size={18} className="text-forge-accent" />
            GEOMETRY ENGINE
          </h2>
          <p className="text-xs text-forge-text-muted mt-1">Offline parametric engine (Three.js)</p>
        </div>
        
        <div className="flex-1 overflow-y-auto p-4 flex flex-col gap-4">
          <div className="flex flex-col gap-1">
            <label className="text-xs font-mono text-forge-text-muted">PART TYPE</label>
            <select 
              className="bg-forge-dark border border-forge-border rounded p-2 text-sm text-forge-text font-mono outline-none focus:border-forge-accent"
              value={partType}
              onChange={handlePartTypeChange}
            >
              <option value="bracket">L-Bracket</option>
              <option value="shaft">Cylindrical Shaft</option>
              <option value="flange">Mounting Flange</option>
              <option value="housing">Box Housing</option>
              <option value="gear">Spur Gear</option>
              <option value="plate">Mounting Plate</option>
            </select>
          </div>
          
          <div className="border-t border-forge-border/50 pt-4 flex flex-col gap-3">
            <label className="text-xs font-mono text-forge-text-muted">PARAMETERS (mm)</label>
            {Object.entries(params).map(([k, v]) => (
              <div key={k} className="flex items-center justify-between">
                <span className="text-sm font-mono text-forge-text/80">{k}</span>
                {typeof v === 'boolean' ? (
                  <input type="checkbox" checked={v} onChange={e => handleParamChange(k, e.target.checked)} />
                ) : typeof v === 'string' ? (
                  <input 
                    type="text" 
                    value={v} 
                    onChange={e => handleParamChange(k, e.target.value)}
                    className="w-24 bg-forge-dark border border-forge-border rounded p-1 text-right text-sm font-mono text-forge-text outline-none focus:border-forge-accent"
                  />
                ) : (
                  <input 
                    type="number" 
                    value={v} 
                    onChange={e => handleParamChange(k, Number(e.target.value))}
                    className="w-24 bg-forge-dark border border-forge-border rounded p-1 text-right text-sm font-mono text-forge-text outline-none focus:border-forge-accent"
                  />
                )}
              </div>
            ))}
          </div>
          
          <div className="flex flex-col gap-2 mt-4">
            <button 
              onClick={generatePart}
              disabled={isGenerating}
              className="w-full py-3 bg-forge-accent/20 hover:bg-forge-accent/40 text-forge-accent border border-forge-accent/50 rounded flex items-center justify-center gap-2 font-mono font-bold transition-all disabled:opacity-50"
            >
              {isGenerating ? <div className="w-4 h-4 border-2 border-forge-accent border-t-transparent rounded-full animate-spin" /> : <Play size={16} />}
              {isGenerating ? 'GENERATING...' : 'GENERATE 3D PART'}
            </button>
            <button 
              onClick={async () => {
                const { exportUsd } = await import('../services/forgeApi');
                try {
                  await exportUsd('component_1');
                  alert('Successfully exported to USD format for Omniverse!');
                } catch(e: any) {
                  alert('Failed to export USD: ' + e.message);
                }
              }}
              className="w-full py-2 bg-forge-purple/20 hover:bg-forge-purple/40 text-forge-purple border border-forge-purple/50 rounded flex items-center justify-center gap-2 font-mono font-bold transition-all"
            >
              EXPORT USD (OMNIVERSE)
            </button>
            <button 
              onClick={() => setIsOmniverseStreaming(!isOmniverseStreaming)}
              className={`w-full py-2 ${isOmniverseStreaming ? 'bg-forge-accent text-black' : 'bg-forge-dark hover:bg-forge-accent/20 text-forge-accent'} border border-forge-accent/50 rounded flex items-center justify-center gap-2 font-mono font-bold transition-all mt-1`}
            >
              <Maximize2 size={16} />
              {isOmniverseStreaming ? 'DISABLE NUCLEUS STREAM' : 'ENABLE NUCLEUS STREAM'}
            </button>
          </div>
          
          {analysis && (
            <div className="mt-4 border border-forge-border rounded p-3 bg-forge-dark/50">
              <h3 className="text-xs font-bold font-mono text-forge-text mb-2 flex items-center gap-2">
                <Settings size={14} /> ANALYSIS RESULTS
              </h3>
              <div className="text-xs font-mono text-forge-text-muted flex flex-col gap-1">
                <div className="flex justify-between">
                  <span>Volume:</span>
                  <span className="text-forge-text">{(analysis.bounding_box.volume / 1000).toFixed(2)} cm³</span>
                </div>
                <div className="flex justify-between">
                  <span>Bounds (X):</span>
                  <span className="text-forge-text">{analysis.bounding_box.extents[0].toFixed(1)} mm</span>
                </div>
                <div className="flex justify-between">
                  <span>Bounds (Y):</span>
                  <span className="text-forge-text">{analysis.bounding_box.extents[1].toFixed(1)} mm</span>
                </div>
                <div className="flex justify-between">
                  <span>Bounds (Z):</span>
                  <span className="text-forge-text">{analysis.bounding_box.extents[2].toFixed(1)} mm</span>
                </div>
              </div>
              
              {analysis.manufacturability_warnings?.length > 0 && (
                <div className="mt-3 p-2 bg-forge-red/10 border border-forge-red/30 rounded">
                  <div className="flex items-center gap-1 text-forge-red text-xs font-bold mb-1">
                    <AlertTriangle size={12} /> DFM WARNINGS
                  </div>
                  <ul className="list-disc pl-4 text-[10px] text-forge-red/80">
                    {analysis.manufacturability_warnings.map((w: string, i: number) => (
                      <li key={i}>{w}</li>
                    ))}
                  </ul>
                </div>
              )}
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

// ── STL Viewer Component ────────────────────────────────────────────────────
const StlViewer: React.FC<{ url: string }> = ({ url }) => {
  const containerRef = useRef<HTMLDivElement>(null);
  
  useEffect(() => {
    if (!containerRef.current) return;
    const el = containerRef.current;
    
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0a0f);
    
    const camera = new THREE.PerspectiveCamera(45, el.clientWidth / el.clientHeight, 0.1, 1000);
    camera.position.set(150, 150, 150);
    
    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(el.clientWidth, el.clientHeight);
    renderer.setPixelRatio(window.devicePixelRatio);
    el.appendChild(renderer.domElement);
    
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    
    scene.add(new THREE.AmbientLight(0x404040, 2.0));
    const dirLight = new THREE.DirectionalLight(0xffffff, 2.0);
    dirLight.position.set(1, 1, 1);
    scene.add(dirLight);
    
    const gridHelper = new THREE.GridHelper(200, 50, 0x1e1e2a, 0x1e1e2a);
    scene.add(gridHelper);
    
    let mesh: THREE.Mesh | null = null;
    const loader = new STLLoader();
    loader.load(url, (geometry) => {
      geometry.center();
      geometry.computeVertexNormals();
      
      const material = new THREE.MeshStandardMaterial({ 
        color: 0x00d4ff, 
        metalness: 0.5, 
        roughness: 0.2 
      });
      mesh = new THREE.Mesh(geometry, material);
      
      // Add edges
      const edges = new THREE.EdgesGeometry(geometry);
      const line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0x00d4ff, opacity: 0.3, transparent: true }));
      mesh.add(line);
      
      scene.add(mesh);
      
      // Auto-fit camera
      geometry.computeBoundingSphere();
      const radius = geometry.boundingSphere?.radius || 100;
      camera.position.set(radius * 1.5, radius * 1.5, radius * 1.5);
      controls.target.set(0, 0, 0);
    });
    
    const animate = () => {
      requestAnimationFrame(animate);
      controls.update();
      renderer.render(scene, camera);
    };
    animate();
    
    const handleResize = () => {
      camera.aspect = el.clientWidth / el.clientHeight;
      camera.updateProjectionMatrix();
      renderer.setSize(el.clientWidth, el.clientHeight);
    };
    window.addEventListener('resize', handleResize);
    
    return () => {
      window.removeEventListener('resize', handleResize);
      if (mesh) {
        mesh.geometry.dispose();
        (mesh.material as THREE.Material).dispose();
      }
      renderer.dispose();
      if (el.contains(renderer.domElement)) el.removeChild(renderer.domElement);
    };
  }, [url]);

  return <div ref={containerRef} className="w-full h-full" />;
};
