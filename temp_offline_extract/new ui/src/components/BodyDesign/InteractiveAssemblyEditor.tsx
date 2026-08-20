import React, { useEffect, useRef } from 'react';
import * as THREE from 'three';
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { SceneObject, createBendedGeometry } from './InteractivePartEditor';
import { Joint } from './BodyDesignLayout';

interface Props {
  parts: SceneObject[];
  activeFastener: string;
  joints: Joint[];
  selectedForJoint: string[];
  setSelectedForJoint: React.Dispatch<React.SetStateAction<string[]>>;
  proposedJointSize: number;
  onProposedPositionChange: (pos: {x:number, y:number, z:number} | null) => void;
  isSimulating: boolean;
}

export const InteractiveAssemblyEditor: React.FC<Props> = ({ 
  parts, 
  activeFastener, 
  joints,
  selectedForJoint,
  setSelectedForJoint,
  proposedJointSize,
  onProposedPositionChange,
  isSimulating
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const transformControlRef = useRef<TransformControls | null>(null);
  const raycasterRef = useRef<THREE.Raycaster>(new THREE.Raycaster());
  const mouseRef = useRef<THREE.Vector2>(new THREE.Vector2());
  const frameRef = useRef<number>(0);
  
  const meshMapRef = useRef<Map<string, THREE.Mesh>>(new Map());
  const jointsGroupRef = useRef<THREE.Group>(new THREE.Group());
  const proposedJointMeshRef = useRef<THREE.Mesh | null>(null);

  const isSimulatingRef = useRef(isSimulating);
  const jointsRef = useRef(joints);
  const partsRef = useRef(parts);

  useEffect(() => {
    isSimulatingRef.current = isSimulating;
  }, [isSimulating]);

  useEffect(() => {
    jointsRef.current = joints;
  }, [joints]);
  
  useEffect(() => {
    partsRef.current = parts;
  }, [parts]);

  // Initialization
  useEffect(() => {
    if (!containerRef.current) return;
    const el = containerRef.current;

    const scene = new THREE.Scene();
    scene.background = null;
    sceneRef.current = scene;

    const camera = new THREE.PerspectiveCamera(50, el.clientWidth / Math.max(el.clientHeight, 1), 0.1, 1000);
    camera.position.set(300, 200, 300);
    cameraRef.current = camera;

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.setSize(el.clientWidth, el.clientHeight);
    el.appendChild(renderer.domElement);
    rendererRef.current = renderer;

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.target.set(0, 0, 0);
    controlsRef.current = controls;

    // Lighting
    scene.add(new THREE.AmbientLight(0xffffff, 0.6));
    const dirLight = new THREE.DirectionalLight(0x00ff88, 1.0);
    dirLight.position.set(200, 300, 200);
    scene.add(dirLight);

    const dirLight2 = new THREE.DirectionalLight(0x00d4ff, 0.8);
    dirLight2.position.set(-200, 100, -200);
    scene.add(dirLight2);

    scene.add(jointsGroupRef.current);

    const transformControl = new TransformControls(camera, renderer.domElement);
    transformControl.addEventListener('dragging-changed', (event) => {
      controls.enabled = !event.value;
    });
    transformControl.addEventListener('change', () => {
      if (proposedJointMeshRef.current) {
        onProposedPositionChange({
          x: proposedJointMeshRef.current.position.x,
          y: proposedJointMeshRef.current.position.y,
          z: proposedJointMeshRef.current.position.z,
        });
      }
    });
    scene.add(transformControl);
    transformControlRef.current = transformControl;

    const onPointerDown = (event: MouseEvent) => {
      if (transformControl.dragging) return;
      
      const rect = el.getBoundingClientRect();
      const x = ((event.clientX - rect.left) / el.clientWidth) * 2 - 1;
      const y = -((event.clientY - rect.top) / el.clientHeight) * 2 + 1;
      
      mouseRef.current.set(x, y);
      raycasterRef.current.setFromCamera(mouseRef.current, camera);
      
      const meshes = Array.from(meshMapRef.current.values());
      const intersects = raycasterRef.current.intersectObjects(meshes, false);
      
      if (intersects.length > 0) {
        const clickedMesh = intersects[0].object as THREE.Mesh;
        let clickedId: string | null = null;
        meshMapRef.current.forEach((mesh, id) => {
          if (mesh === clickedMesh) clickedId = id;
        });
        
        if (clickedId) {
          const validId = clickedId;
          setSelectedForJoint(prev => {
            if (prev.includes(validId)) {
              return prev.filter(id => id !== validId);
            }
            if (prev.length >= 2) {
              return [prev[1], validId];
            }
            return [...prev, validId];
          });
        }
      } else {
        setSelectedForJoint([]);
        onProposedPositionChange(null);
      }
    };
    
    el.addEventListener('pointerdown', onPointerDown);

    const clock = new THREE.Clock();

    const animate = () => {
      frameRef.current = requestAnimationFrame(animate);
      
      const time = clock.getElapsedTime();

      if (isSimulatingRef.current) {
        jointsRef.current.forEach(joint => {
          if (joint.type === 'motor' || joint.type === 'hinge') {
            const meshB = meshMapRef.current.get(joint.partBId);
            const partB = partsRef.current.find(p => p.id === joint.partBId);
            
            if (meshB && partB) {
              const initialPos = new THREE.Vector3(partB.position.x, partB.position.y, partB.position.z);
              const jointPos = new THREE.Vector3(joint.position.x, joint.position.y, joint.position.z);
              const offset = initialPos.clone().sub(jointPos);
              
              if (joint.type === 'motor') {
                offset.applyAxisAngle(new THREE.Vector3(0, 1, 0), time * 2.0);
                meshB.position.copy(jointPos).add(offset);
                meshB.rotation.y = time * 2.0;
              } else if (joint.type === 'hinge') {
                const angle = Math.sin(time * 3.0) * 0.5;
                offset.applyAxisAngle(new THREE.Vector3(0, 0, 1), angle);
                meshB.position.copy(jointPos).add(offset);
                meshB.rotation.z = angle;
              }
            }
          }
        });
      } else {
        jointsRef.current.forEach(joint => {
          if (joint.type === 'motor' || joint.type === 'hinge') {
            const meshB = meshMapRef.current.get(joint.partBId);
            const partB = partsRef.current.find(p => p.id === joint.partBId);
            if (meshB && partB) {
                meshB.position.set(partB.position.x, partB.position.y, partB.position.z);
                meshB.rotation.set(0, 0, 0);
            }
          }
        });
      }

      controls.update();
      renderer.render(scene, camera);
    };
    animate();

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
      el.removeEventListener('pointerdown', onPointerDown);
      cancelAnimationFrame(frameRef.current);
      ro.disconnect();
      transformControl.dispose();
      controls.dispose();
      
      meshMapRef.current.forEach(mesh => {
        if (sceneRef.current) sceneRef.current.remove(mesh);
        mesh.geometry.dispose();
        (mesh.material as THREE.Material).dispose();
      });
      meshMapRef.current.clear();
      
      if (jointsGroupRef.current) {
        scene.remove(jointsGroupRef.current);
      }
      
      renderer.dispose();
      el.removeChild(renderer.domElement);
    };
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []); // Run only once

  // Sync objects (read-only render)
  useEffect(() => {
    const scene = sceneRef.current;
    if (!scene) return;

    const currentIds = new Set(parts.map(o => o.id));
    
    // Remove deleted objects
    const toRemove: string[] = [];
    meshMapRef.current.forEach((mesh, id) => {
      if (!currentIds.has(id)) {
        scene.remove(mesh);
        mesh.geometry.dispose();
        (mesh.material as THREE.Material).dispose();
        toRemove.push(id);
      }
    });
    toRemove.forEach(id => meshMapRef.current.delete(id));

    // Add or update objects
    parts.forEach(obj => {
      let mesh = meshMapRef.current.get(obj.id);
      
      const requiresRebuild = !mesh || 
        mesh.userData.bendAngle !== obj.bendAngle || 
        mesh.userData.type !== obj.type ||
        mesh.userData.customGeometry !== obj.customGeometry ||
        (obj.bendAngle && obj.bendAngle > 0 && (
          mesh.userData.sizeX !== obj.size.x || 
          mesh.userData.sizeY !== obj.size.y || 
          mesh.userData.sizeZ !== obj.size.z
        ));

      if (requiresRebuild) {
        if (mesh) {
          scene.remove(mesh);
          mesh.geometry.dispose();
          while(mesh.children.length > 0){ 
            const child = mesh.children[0] as THREE.LineSegments;
            mesh.remove(child); 
            child.geometry.dispose();
            (child.material as THREE.Material).dispose();
          }
        }

        const geometry = createBendedGeometry(obj);

        const material = new THREE.MeshStandardMaterial({ 
          color: 0x4a4a5e, 
          metalness: 0.5, 
          roughness: 0.6,
        });
        
        mesh = new THREE.Mesh(geometry, material);
        mesh.userData = { 
          bendAngle: obj.bendAngle, 
          type: obj.type, 
          customGeometry: obj.customGeometry,
          sizeX: obj.size.x,
          sizeY: obj.size.y,
          sizeZ: obj.size.z
        };
        
        const edges = new THREE.EdgesGeometry(geometry);
        const line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0x1a1a24, opacity: 0.5, transparent: true }));
        mesh.add(line);
        
        scene.add(mesh);
        meshMapRef.current.set(obj.id, mesh);
      }

      mesh.position.set(obj.position.x, obj.position.y, obj.position.z);
      
      let targetScaleX = 1;
      let targetScaleY = 1;
      let targetScaleZ = 1;

      // Only scale if the geometry wasn't pre-scaled by createBendedGeometry
      if (!mesh.geometry.userData.isPreScaled && !obj.customGeometry) {
        targetScaleX = obj.size.x;
        targetScaleY = obj.size.y;
        targetScaleZ = obj.size.z;

        if (obj.type === 'cylinder') {
          targetScaleX = obj.size.x / 2;
          targetScaleZ = obj.size.z / 2;
        }
      }
      mesh.scale.set(targetScaleX, targetScaleY, targetScaleZ);
      
      // Highlight selected objects
      if (mesh.material instanceof THREE.MeshStandardMaterial) {
        if (selectedForJoint.includes(obj.id)) {
          mesh.material.color.setHex(0x00ff88); // Green for assembly selection
          mesh.material.emissive.setHex(0x004422);
        } else {
          mesh.material.color.setHex(0x4a4a5e);
          mesh.material.emissive.setHex(0x000000);
        }
      }
    });

  }, [parts, selectedForJoint]);

  // Render persistent joints and proposed joint
  useEffect(() => {
    const group = jointsGroupRef.current;
    
    // Clear old joints and markers
    while(group.children.length > 0){ 
      const child = group.children[0];
      group.remove(child);
      if (child instanceof THREE.Mesh) {
        child.geometry.dispose();
        (child.material as THREE.Material).dispose();
      } else if (child instanceof THREE.Line) {
        child.geometry.dispose();
        (child.material as THREE.Material).dispose();
      }
    }
    
    if (transformControlRef.current) {
      transformControlRef.current.detach();
    }
    proposedJointMeshRef.current = null;

    // 1. Render all persistent joints
    joints.forEach(joint => {
      let color = 0xffffff;
      if (joint.type === 'bolt') color = 0xffcc00;
      else if (joint.type === 'glue') color = 0xff00ff;
      else if (joint.type.includes('weld')) color = 0x00d4ff;
      else if (joint.type === 'motor') color = 0xff3300;
      else if (joint.type === 'hinge') color = 0x00ff00;

      const sphereGeo = new THREE.SphereGeometry(joint.size, 16, 16);
      const sphereMat = new THREE.MeshStandardMaterial({ color, metalness: 0.8, roughness: 0.2 });
      const sphere = new THREE.Mesh(sphereGeo, sphereMat);
      sphere.position.set(joint.position.x, joint.position.y, joint.position.z);
      group.add(sphere);
    });

    // 2. Render proposed joint if 2 parts are selected
    if (selectedForJoint.length === 2) {
      const meshA = meshMapRef.current.get(selectedForJoint[0]);
      const meshB = meshMapRef.current.get(selectedForJoint[1]);
      
      if (meshA && meshB) {
        const posA = meshA.position;
        const posB = meshB.position;

        // Draw a line connecting their centers (visual aid)
        const points = [];
        points.push(new THREE.Vector3(posA.x, posA.y, posA.z));
        points.push(new THREE.Vector3(posB.x, posB.y, posB.z));

        let color = 0xffffff;
        if (activeFastener === 'bolt') color = 0xffcc00;
        else if (activeFastener === 'glue') color = 0xff00ff;
        else if (activeFastener.includes('weld')) color = 0x00d4ff;
        else if (activeFastener === 'motor') color = 0xff3300;
        else if (activeFastener === 'hinge') color = 0x00ff00;

        const lineGeo = new THREE.BufferGeometry().setFromPoints(points);
        const lineMat = new THREE.LineBasicMaterial({ color, linewidth: 2, transparent: true, opacity: 0.3 });
        const line = new THREE.Line(lineGeo, lineMat);
        group.add(line);
        
        // Add a sphere in the middle to represent the PROPOSED joint
        const midPoint = new THREE.Vector3().addVectors(posA, posB).multiplyScalar(0.5);
        const sphereGeo = new THREE.SphereGeometry(proposedJointSize, 16, 16);
        const sphereMat = new THREE.MeshStandardMaterial({ color, metalness: 0.8, roughness: 0.2 });
        const sphere = new THREE.Mesh(sphereGeo, sphereMat);
        sphere.position.copy(midPoint);
        group.add(sphere);
        
        proposedJointMeshRef.current = sphere;
        
        if (transformControlRef.current) {
          transformControlRef.current.attach(sphere);
        }

        // Set initial proposed position if not already dragging
        onProposedPositionChange({
          x: midPoint.x,
          y: midPoint.y,
          z: midPoint.z,
        });
      }
    } else {
      onProposedPositionChange(null);
    }
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [joints, selectedForJoint, activeFastener, proposedJointSize]);

  return <div ref={containerRef} className="w-full h-full cursor-crosshair" />;
};
