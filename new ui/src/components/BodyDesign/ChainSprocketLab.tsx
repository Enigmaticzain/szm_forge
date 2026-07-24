import React, { useState, useEffect, useRef, useMemo } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { FlaskConical, Play, CheckCircle2, ShieldAlert } from 'lucide-react';
import { useProject, CustomMaterial } from '../../store/ProjectContext';
import { BarChart, Bar, XAxis, YAxis, Tooltip, ResponsiveContainer, CartesianGrid } from 'recharts';

interface ExperimentResult {
  materialId: string;
  materialName: string;
  color: string;
  friction: number;
  efficiency: number;
  wearRate: number;
  weight: number;
  score: number;
}

export const ChainSprocketLab: React.FC = () => {
  const { customMaterials } = useProject();
  const [driverTeeth, setDriverTeeth] = useState(15);
  const [drivenTeeth, setDrivenTeeth] = useState(45);
  const [rpm, setRpm] = useState(1000);
  const [torque, setTorque] = useState(50);
  const [results, setResults] = useState<ExperimentResult[]>([]);
  const [activeMaterialId, setActiveMaterialId] = useState<string | null>(null);
  const [isSimulating, setIsSimulating] = useState(false);

  const containerRef = useRef<HTMLDivElement>(null);
  const sceneRef = useRef<THREE.Scene | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const animationRef = useRef<number>(0);
  
  const mechanismGroupRef = useRef<THREE.Group>(new THREE.Group());
  const driverRef = useRef<THREE.Group | null>(null);
  const drivenRef = useRef<THREE.Group | null>(null);
  const chainRef = useRef<THREE.Group | null>(null);

  const runExperiment = () => {
    const newResults: ExperimentResult[] = customMaterials.map(mat => {
      let baseFriction = 0.1;
      if (mat.category === 'steel') baseFriction = 0.05;
      else if (mat.category === 'alu') baseFriction = 0.12;
      else if (mat.category === 'composite') baseFriction = 0.08;
      else if (mat.category === 'plastic') baseFriction = 0.18;

      const loadFactor = torque / (mat.yieldStrength * 10);
      const speedFactor = rpm / 10000;
      const totalFriction = baseFriction + loadFactor + speedFactor;
      const efficiency = Math.max(0, 100 - (totalFriction * 100));
      
      const wearRate = (torque * rpm * totalFriction) / (mat.yieldStrength * 100);
      
      // Volume proxy based on teeth
      const volProxy = (driverTeeth * driverTeeth + drivenTeeth * drivenTeeth) * 0.00001;
      const weight = volProxy * mat.density;
      
      const score = efficiency - wearRate * 10 - weight * 0.5;

      return {
        materialId: mat.id,
        materialName: mat.name,
        color: mat.color,
        friction: totalFriction,
        efficiency,
        wearRate,
        weight,
        score
      };
    });

    newResults.sort((a, b) => b.score - a.score);
    setResults(newResults);
    if (newResults.length > 0) {
      setActiveMaterialId(newResults[0].materialId);
    }
  };

  useEffect(() => {
    if (!containerRef.current) return;
    const el = containerRef.current;

    const scene = new THREE.Scene();
    sceneRef.current = scene;

    const camera = new THREE.PerspectiveCamera(50, el.clientWidth / Math.max(el.clientHeight, 1), 0.1, 1000);
    camera.position.set(0, 0, 150);
    cameraRef.current = camera;

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(el.clientWidth, el.clientHeight);
    el.appendChild(renderer.domElement);
    rendererRef.current = renderer;

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controlsRef.current = controls;

    scene.add(new THREE.AmbientLight(0xffffff, 0.6));
    const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight.position.set(50, 50, 50);
    scene.add(dirLight);

    scene.add(mechanismGroupRef.current);

    const onResize = () => {
      camera.aspect = el.clientWidth / Math.max(el.clientHeight, 1);
      camera.updateProjectionMatrix();
      renderer.setSize(el.clientWidth, el.clientHeight);
    };
    window.addEventListener('resize', onResize);

    return () => {
      window.removeEventListener('resize', onResize);
      cancelAnimationFrame(animationRef.current);
      controls.dispose();
      renderer.dispose();
      el.removeChild(renderer.domElement);
    };
  }, []);

  useEffect(() => {
    const group = mechanismGroupRef.current;
    while(group.children.length > 0){ 
        const child = group.children[0];
        group.remove(child);
    }

    const matInfo = customMaterials.find(m => m.id === activeMaterialId);
    const color = matInfo ? matInfo.color : '#888888';
    const material = new THREE.MeshStandardMaterial({ color, metalness: 0.6, roughness: 0.4 });

    const createSprocket = (teeth: number, radius: number) => {
      const spGroup = new THREE.Group();
      const bodyGeo = new THREE.CylinderGeometry(radius - 2, radius - 2, 4, 32);
      bodyGeo.rotateX(Math.PI / 2);
      spGroup.add(new THREE.Mesh(bodyGeo, material));
      
      for (let i = 0; i < teeth; i++) {
        const angle = (i / teeth) * Math.PI * 2;
        const toothGeo = new THREE.BoxGeometry(3, 4, 3);
        const tooth = new THREE.Mesh(toothGeo, material);
        tooth.position.set(Math.cos(angle) * radius, Math.sin(angle) * radius, 0);
        tooth.rotation.z = angle;
        spGroup.add(tooth);
      }
      return spGroup;
    };

    const r1 = driverTeeth * 0.8;
    const r2 = drivenTeeth * 0.8;
    const distance = r1 + r2 + 20;

    const d1 = createSprocket(driverTeeth, r1);
    d1.position.set(-distance/2, 0, 0);
    group.add(d1);
    driverRef.current = d1;

    const d2 = createSprocket(drivenTeeth, r2);
    d2.position.set(distance/2, 0, 0);
    group.add(d2);
    drivenRef.current = d2;
    
    // Fake chain
    const chainGroup = new THREE.Group();
    const chainMat = new THREE.MeshStandardMaterial({ color: 0x333333, metalness: 0.8 });
    const chainGeo = new THREE.CylinderGeometry(1, 1, 6, 8);
    chainGeo.rotateX(Math.PI/2);
    
    // Top span
    const topLink = new THREE.Mesh(new THREE.BoxGeometry(distance, 2, 6), chainMat);
    topLink.position.set(0, r1 + (r2-r1)/2, 0);
    topLink.rotation.z = Math.atan2(r2-r1, distance);
    chainGroup.add(topLink);
    
    // Bottom span
    const botLink = new THREE.Mesh(new THREE.BoxGeometry(distance, 2, 6), chainMat);
    botLink.position.set(0, -(r1 + (r2-r1)/2), 0);
    botLink.rotation.z = -Math.atan2(r2-r1, distance);
    chainGroup.add(botLink);
    
    group.add(chainGroup);
    chainRef.current = chainGroup;

  }, [driverTeeth, drivenTeeth, activeMaterialId, customMaterials]);

  useEffect(() => {
    const clock = new THREE.Clock();
    const animate = () => {
      animationRef.current = requestAnimationFrame(animate);
      const delta = clock.getDelta();
      
      if (isSimulating && driverRef.current && drivenRef.current) {
        const speed = (rpm / 60) * Math.PI * 2 * delta; // rad per frame
        driverRef.current.rotation.z -= speed;
        // Gear ratio
        const ratio = driverTeeth / drivenTeeth;
        drivenRef.current.rotation.z -= speed * ratio;
      }
      
      controlsRef.current?.update();
      rendererRef.current?.render(sceneRef.current!, cameraRef.current!);
    };
    animate();
    
    return () => cancelAnimationFrame(animationRef.current);
  }, [isSimulating, driverTeeth, drivenTeeth, rpm]);


  return (
    <div className="h-full w-full flex bg-forge-black relative">
      {/* Sidebar */}
      <div className="w-[300px] border-r border-forge-border bg-forge-dark/80 flex flex-col flex-shrink-0 z-10 p-4 overflow-y-auto">
        <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-4 flex items-center gap-2">
          <FlaskConical size={12} className="text-forge-accent" />
          EXPERIMENT SETUP
        </h3>

        <div className="space-y-4 mb-6">
          <div>
            <div className="flex justify-between mb-1">
              <span className="text-[9px] font-mono text-forge-text-muted">DRIVER TEETH</span>
              <span className="text-[9px] font-mono text-forge-accent">{driverTeeth}</span>
            </div>
            <input type="range" min="10" max="40" value={driverTeeth} onChange={e => setDriverTeeth(Number(e.target.value))} className="w-full" />
          </div>
          <div>
            <div className="flex justify-between mb-1">
              <span className="text-[9px] font-mono text-forge-text-muted">DRIVEN TEETH</span>
              <span className="text-[9px] font-mono text-forge-accent">{drivenTeeth}</span>
            </div>
            <input type="range" min="10" max="80" value={drivenTeeth} onChange={e => setDrivenTeeth(Number(e.target.value))} className="w-full" />
          </div>
          <div>
            <div className="flex justify-between mb-1">
              <span className="text-[9px] font-mono text-forge-text-muted">RPM</span>
              <span className="text-[9px] font-mono text-forge-accent">{rpm}</span>
            </div>
            <input type="range" min="100" max="5000" step="100" value={rpm} onChange={e => setRpm(Number(e.target.value))} className="w-full" />
          </div>
          <div>
            <div className="flex justify-between mb-1">
              <span className="text-[9px] font-mono text-forge-text-muted">TORQUE (N·m)</span>
              <span className="text-[9px] font-mono text-forge-accent">{torque}</span>
            </div>
            <input type="range" min="10" max="1000" step="10" value={torque} onChange={e => setTorque(Number(e.target.value))} className="w-full" />
          </div>
        </div>

        <button 
          onClick={runExperiment}
          className="w-full py-2 bg-forge-accent/20 text-forge-accent border border-forge-accent/40 rounded flex items-center justify-center gap-2 hover:bg-forge-accent/30 transition-colors mb-4"
        >
          <FlaskConical size={14} />
          <span className="text-[10px] font-bold">RUN EXPERIMENT (ALL MATERIALS)</span>
        </button>
        
        <button 
          onClick={() => setIsSimulating(!isSimulating)}
          className={`w-full py-2 border rounded flex items-center justify-center gap-2 transition-colors mb-6 ${
            isSimulating ? 'bg-forge-red/20 text-forge-red border-forge-red/40' : 'bg-forge-green/20 text-forge-green border-forge-green/40'
          }`}
        >
          <Play size={14} />
          <span className="text-[10px] font-bold">{isSimulating ? 'STOP SIMULATION' : 'START SIMULATION'}</span>
        </button>

        {results.length > 0 && (
          <div>
            <h3 className="text-[10px] font-bold tracking-widest text-forge-text-dim mb-2">RESULTS (RANKED)</h3>
            <div className="space-y-2">
              {results.map((res, idx) => (
                <div 
                  key={res.materialId}
                  onClick={() => setActiveMaterialId(res.materialId)}
                  className={`p-2 border rounded cursor-pointer transition-colors ${
                    activeMaterialId === res.materialId ? 'bg-forge-surface/80 border-forge-accent' : 'bg-forge-surface/30 border-forge-border hover:border-forge-accent/50'
                  }`}
                >
                  <div className="flex justify-between items-center mb-1">
                    <span className="text-[10px] font-bold text-forge-text" style={{ color: res.color }}>
                      {idx + 1}. {res.materialName}
                    </span>
                    {idx === 0 && <CheckCircle2 size={12} className="text-forge-green" />}
                  </div>
                  <div className="grid grid-cols-2 gap-1 text-[8px] font-mono text-forge-text-muted">
                    <div>Eff: {res.efficiency.toFixed(1)}%</div>
                    <div>Wear: {res.wearRate.toFixed(2)}</div>
                    <div>Wt: {res.weight.toFixed(1)}kg</div>
                    <div>Score: {res.score.toFixed(0)}</div>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}
      </div>

      {/* Main Viewport */}
      <div className="flex-1 flex flex-col relative overflow-hidden">
        <div ref={containerRef} className="absolute inset-0 z-10" />
        
        {/* Background grid */}
        <div className="absolute inset-0 z-0 opacity-10 pointer-events-none"
             style={{ 
               backgroundImage: 'linear-gradient(rgba(255,255,255,0.2) 1px, transparent 1px), linear-gradient(90deg, rgba(255,255,255,0.2) 1px, transparent 1px)', 
               backgroundSize: '40px 40px',
               backgroundPosition: 'center center'
             }} 
        />

        {results.length > 0 && (
          <div className="absolute bottom-4 left-4 right-4 h-48 bg-forge-panel border border-forge-border rounded p-3 z-20 shadow-lg flex gap-4">
            <div className="flex-1">
              <div className="text-[9px] font-mono text-forge-text-muted mb-2 text-center">EFFICIENCY COMPARISON (%)</div>
              <ResponsiveContainer width="100%" height="80%">
                <BarChart data={results}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                  <XAxis dataKey="materialName" tick={{fontSize: 8, fill: '#888'}} interval={0} angle={-15} textAnchor="end" />
                  <YAxis domain={['auto', 100]} tick={{fontSize: 8, fill: '#888'}} />
                  <Tooltip contentStyle={{backgroundColor: '#1a1a24', border: '1px solid #333', fontSize: '10px'}} />
                  <Bar dataKey="efficiency" fill="#00ff88" radius={[2,2,0,0]} />
                </BarChart>
              </ResponsiveContainer>
            </div>
            
            <div className="flex-1">
              <div className="text-[9px] font-mono text-forge-text-muted mb-2 text-center">WEAR RATE COMPARISON</div>
              <ResponsiveContainer width="100%" height="80%">
                <BarChart data={results}>
                  <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                  <XAxis dataKey="materialName" tick={{fontSize: 8, fill: '#888'}} interval={0} angle={-15} textAnchor="end" />
                  <YAxis tick={{fontSize: 8, fill: '#888'}} />
                  <Tooltip contentStyle={{backgroundColor: '#1a1a24', border: '1px solid #333', fontSize: '10px'}} />
                  <Bar dataKey="wearRate" fill="#ff3366" radius={[2,2,0,0]} />
                </BarChart>
              </ResponsiveContainer>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};
