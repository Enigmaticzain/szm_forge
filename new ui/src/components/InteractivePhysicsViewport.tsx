import React, { useEffect, useRef } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { useProject } from '../store/ProjectContext';
import { createBendedGeometry } from './BodyDesign/InteractivePartEditor';

interface Props {
  isPlaying: boolean;
  gravity: number;
  resetTrigger: number;
}

interface PhysicsBody {
  mesh: THREE.Mesh;
  velocity: THREE.Vector3;
  mass: number;
  restitution: number; // Bounciness
  radius: number;
}

export const InteractivePhysicsViewport: React.FC<Props> = ({ isPlaying, gravity, resetTrigger }) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const frameRef = useRef<number>(0);
  const bodiesRef = useRef<PhysicsBody[]>([]);
  const isPlayingRef = useRef(isPlaying);
  const gravityRef = useRef(gravity);
  const { bodyParts, customMaterials } = useProject();

  useEffect(() => {
    isPlayingRef.current = isPlaying;
  }, [isPlaying]);

  useEffect(() => {
    gravityRef.current = gravity;
  }, [gravity]);

  const initScene = () => {
    const el = containerRef.current;
    if (!el || !sceneRef.current) return;
    
    const scene = sceneRef.current;
    
    // Clear old bodies
    bodiesRef.current.forEach(b => {
      scene.remove(b.mesh);
      b.mesh.geometry.dispose();
      (b.mesh.material as THREE.Material).dispose();
    });
    bodiesRef.current = [];

    // Add Floor
    const floorGeo = new THREE.BoxGeometry(20, 0.5, 20);
    const floorMat = new THREE.MeshStandardMaterial({ color: 0x1e1e2a, metalness: 0.2, roughness: 0.8 });
    const floor = new THREE.Mesh(floorGeo, floorMat);
    floor.position.y = -0.25;
    scene.add(floor);

    // Create bodies from user design or fallback to test spheres
    if (bodyParts && bodyParts.length > 0) {
      bodyParts.forEach(part => {
        const assignedMat = customMaterials.find(m => m.id === part.materialId);
        const color = assignedMat ? assignedMat.color : '#00d4ff';
        const restitution = assignedMat ? assignedMat.restitution : 0.5;
        const mass = assignedMat ? (assignedMat.density * 0.001) : 1.0;

        const geo = createBendedGeometry(part);
        const mat = new THREE.MeshStandardMaterial({ color, metalness: 0.4, roughness: 0.4 });
        const mesh = new THREE.Mesh(geo, mat);

        const cadScale = 0.01;
        
        let targetScaleX = 1;
        let targetScaleY = 1;
        let targetScaleZ = 1;

        if (!geo.userData.isPreScaled && !part.customGeometry) {
          targetScaleX = part.size.x * cadScale;
          targetScaleY = part.size.y * cadScale;
          targetScaleZ = part.size.z * cadScale;

          if (part.type === 'cylinder') {
            targetScaleX = (part.size.x / 2) * cadScale;
            targetScaleZ = (part.size.z / 2) * cadScale;
          }
        } else {
           mesh.scale.set(cadScale, cadScale, cadScale);
        }
        
        if (!geo.userData.isPreScaled && !part.customGeometry) {
           mesh.scale.set(targetScaleX, targetScaleY, targetScaleZ);
        }

        // Initialize position based on part position, elevated slightly so they can drop
        mesh.position.set(part.position.x * cadScale, Math.max(part.position.y * cadScale, 0) + 5, part.position.z * cadScale);
        
        scene.add(mesh);
        
        // Approximate radius for simple floor collision based on the bounding size
        const radius = Math.max(part.size.x, part.size.y, part.size.z) * cadScale / 2;

        bodiesRef.current.push({
          mesh,
          velocity: new THREE.Vector3(0, 0, 0),
          mass,
          restitution,
          radius
        });
      });
    } else {
      // Fallback: Test Spheres
      const materials = [
        { color: 0xff4444, restitution: 0.8, name: 'Rubber' },
        { color: 0x44ff44, restitution: 0.3, name: 'Wood' },
        { color: 0x4444ff, restitution: 0.1, name: 'Steel' },
        { color: 0xffff44, restitution: 0.95, name: 'Superball' }
      ];

      materials.forEach((matDef, i) => {
        const radius = 0.5;
        const geo = new THREE.SphereGeometry(radius, 32, 32);
        const mat = new THREE.MeshStandardMaterial({ color: matDef.color, metalness: 0.4, roughness: 0.4 });
        const mesh = new THREE.Mesh(geo, mat);
        
        mesh.position.set(-3 + i * 2, 5 + Math.random() * 2, 0);
        
        scene.add(mesh);
        
        bodiesRef.current.push({
          mesh,
          velocity: new THREE.Vector3(0, 0, 0),
          mass: 1.0,
          restitution: matDef.restitution,
          radius
        });
      });
    }
  };

  useEffect(() => {
    const el = containerRef.current;
    if (!el) return;

    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0a0f);
    sceneRef.current = scene;

    const camera = new THREE.PerspectiveCamera(50, el.clientWidth / Math.max(el.clientHeight, 1), 0.1, 1000);
    camera.position.set(0, 5, 15);
    cameraRef.current = camera;

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.setSize(el.clientWidth, el.clientHeight);
    renderer.shadowMap.enabled = true;
    el.appendChild(renderer.domElement);
    rendererRef.current = renderer;

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.target.set(0, 2, 0);
    controlsRef.current = controls;

    scene.add(new THREE.AmbientLight(0x404060, 1.2));
    const dir = new THREE.DirectionalLight(0xffffff, 1.5);
    dir.position.set(5, 10, 5);
    dir.castShadow = true;
    scene.add(dir);

    const grid = new THREE.GridHelper(20, 20, 0x2a2a3a, 0x1a1a2a);
    grid.position.y = 0.01;
    scene.add(grid);

    initScene();

    let lastTime = performance.now();

    const animate = (time: number) => {
      frameRef.current = requestAnimationFrame(animate);
      
      const dt = Math.min((time - lastTime) / 1000, 0.1); // Cap dt
      lastTime = time;

      if (isPlayingRef.current) {
        // Simple Physics Integration (Euler / Semi-implicit Euler)
        bodiesRef.current.forEach(body => {
          // Apply gravity
          body.velocity.y += gravityRef.current * dt;
          
          // Update position
          body.mesh.position.addScaledVector(body.velocity, dt);

          // Floor collision detection
          if (body.mesh.position.y - body.radius < 0) {
            body.mesh.position.y = body.radius; // Penetration resolution
            body.velocity.y = Math.abs(body.velocity.y) * body.restitution; // Bounce
            
            // Friction
            body.velocity.x *= 0.98;
            body.velocity.z *= 0.98;
          }
        });
      }

      controls.update();
      renderer.render(scene, camera);
    };
    
    frameRef.current = requestAnimationFrame(animate);

    const onResize = () => {
      const w = el.clientWidth;
      const h = el.clientHeight;
      camera.aspect = w / Math.max(h, 1);
      camera.updateProjectionMatrix();
      renderer.setSize(w, h);
    };
    const ro = new ResizeObserver(onResize);
    ro.observe(el);

    return () => {
      cancelAnimationFrame(frameRef.current);
      ro.disconnect();
      controls.dispose();
      renderer.dispose();
      if (el && renderer.domElement.parentNode === el) {
        el.removeChild(renderer.domElement);
      }
    };
  }, []);

  useEffect(() => {
    initScene();
  }, [resetTrigger, bodyParts, customMaterials]);

  return <div ref={containerRef} className="w-full h-full min-h-[300px]" />;
};
