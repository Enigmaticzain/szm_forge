import React, { useCallback, useEffect, useRef } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import { TransformControls } from 'three/examples/jsm/controls/TransformControls.js';
import { Evaluator, Brush, SUBTRACTION } from 'three-bvh-csg';

const disposeObjectTree = (obj: THREE.Object3D) => {
  if (obj instanceof THREE.Mesh || obj instanceof THREE.LineSegments || obj instanceof THREE.Line) {
    (obj as any).geometry?.dispose();
    const mat = (obj as any).material;
    if (Array.isArray(mat)) {
      mat.forEach(m => m.dispose());
    } else {
      mat?.dispose();
    }
  }
  obj.children.forEach(disposeObjectTree);
};

const getGeometrySize = (geo: THREE.BufferGeometry): GeometrySize => {
  geo.computeBoundingBox();
  const box = geo.boundingBox;
  if (!box) return { x: 1, y: 1, z: 1 };
  const size = new THREE.Vector3();
  box.getSize(size);
  return { x: size.x, y: size.y, z: size.z };
};

export type ShapeType = 'box' | 'cylinder' | 'plate' | 'custom';
export type DeformSelectionMode = 'object' | 'point' | 'line' | 'plane';

type Axis = 'x' | 'y' | 'z';
type DeformHandleKind = Exclude<DeformSelectionMode, 'object'>;
type AxisLocks = Partial<Record<Axis, number>>;

export interface SceneObject {
  id: string;
  name: string;
  type: ShapeType;
  position: { x: number; y: number; z: number };
  size: { x: number; y: number; z: number };
  bendAngle?: number; // 0 to Math.PI * 2
  bendAxis?: 'x' | 'y' | 'z';
  customGeometry?: THREE.BufferGeometry; // For storing CSG/deformed results
  materialId?: string;
}

interface GeometrySize {
  x: number;
  y: number;
  z: number;
}

interface Props {
  objects: SceneObject[];
  selectedId: string | null;
  transformMode: 'translate' | 'scale';
  deformSelectionMode?: DeformSelectionMode;
  booleanToolMode?: boolean; // If true, clicking selects a tool to cut the selectedId
  isSnappingEnabled?: boolean;
  deformBrushSize?: number;
  onObjectChange: (id: string, newSize: GeometrySize, newPos: GeometrySize) => void;
  onGeometryChange?: (id: string, geometry: THREE.BufferGeometry, newSize: GeometrySize) => void;
  onSelect: (id: string | null) => void;
  onPerformCut?: (targetId: string, toolId: string) => void;
}

interface DeformDragState {
  objectId: string;
  mesh: THREE.Mesh;
  kind: DeformSelectionMode;
  startPositions: Float32Array;
  startWorldHit: THREE.Vector3;
  dragPlane: THREE.Plane;
  pointerId: number;
  vertexWeights: Float32Array;
  brushSize: number;
}


// Helper to create a bended geometry
export const createBendedGeometry = (obj: SceneObject) => {
  let geometry: THREE.BufferGeometry;
  
  if (obj.customGeometry) {
    geometry = obj.customGeometry.clone();
  } else if (obj.type === 'cylinder') {
    geometry = new THREE.CylinderGeometry(0.5, 0.5, 1, 32);
  } else {
    geometry = new THREE.BoxGeometry(1, 1, 1, 64, 64, 64);
  }

  // Apply bending if it's a plate (or box) and bendAngle > 0
  if ((obj.type === 'plate' || obj.type === 'box') && obj.bendAngle && obj.bendAngle > 0) {
    const angle = obj.bendAngle; // in radians
    const axis = obj.bendAxis || 'x';
    
    if (!obj.customGeometry) {
      geometry.scale(obj.size.x, obj.size.y, obj.size.z);
    }
    const positions = geometry.attributes.position;
    
    if (axis === 'x') {
      const W = obj.size.x;
      const R = W / angle;
      for (let i = 0; i < positions.count; i++) {
        const x = positions.getX(i);
        const y = positions.getY(i);
        const z = positions.getZ(i);
        const alpha = x / R;
        positions.setXYZ(i, (R + z) * Math.sin(alpha), y, (R + z) * Math.cos(alpha) - R);
      }
    } else if (axis === 'y') {
      const W = obj.size.y;
      const R = W / angle;
      for (let i = 0; i < positions.count; i++) {
        const x = positions.getX(i);
        const y = positions.getY(i);
        const z = positions.getZ(i);
        const alpha = y / R;
        // Bend Y into Z
        positions.setXYZ(i, x, (R + z) * Math.sin(alpha), (R + z) * Math.cos(alpha) - R);
      }
    } else if (axis === 'z') {
      const W = obj.size.z;
      const R = W / angle;
      for (let i = 0; i < positions.count; i++) {
        const x = positions.getX(i);
        const y = positions.getY(i);
        const z = positions.getZ(i);
        const alpha = z / R;
        // Bend Z into X
        positions.setXYZ(i, (R + x) * Math.cos(alpha) - R, y, (R + x) * Math.sin(alpha));
      }
    }
    
    geometry.computeVertexNormals();
    geometry.userData.isPreScaled = true;
  }
  
  return geometry;
};

export const InteractivePartEditor: React.FC<Props> = ({ 
  objects, 
  selectedId, 
  transformMode,
  deformSelectionMode = 'object',
  booleanToolMode,
  isSnappingEnabled,
  deformBrushSize = 30,
  onObjectChange,
  onGeometryChange,
  onSelect,
  onPerformCut
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const transformRef = useRef<TransformControls | null>(null);
  const raycasterRef = useRef<THREE.Raycaster>(new THREE.Raycaster());
  const mouseRef = useRef<THREE.Vector2>(new THREE.Vector2());
  const frameRef = useRef<number>(0);
  const ghostMeshRef = useRef<THREE.Mesh | null>(null);
  const handleGroupRef = useRef<THREE.Group>(new THREE.Group());
  const activeDragRef = useRef<DeformDragState | null>(null);
  const meshMapRef = useRef<Map<string, THREE.Mesh>>(new Map());

  const objectsRef = useRef(objects);
  const selectedIdRef = useRef(selectedId);
  const booleanToolModeRef = useRef(booleanToolMode);
  const deformSelectionModeRef = useRef(deformSelectionMode);
  const deformBrushSizeRef = useRef(deformBrushSize);
  const onObjectChangeRef = useRef(onObjectChange);
  const onGeometryChangeRef = useRef(onGeometryChange);
  const onSelectRef = useRef(onSelect);
  const onPerformCutRef = useRef(onPerformCut);

  objectsRef.current = objects;
  selectedIdRef.current = selectedId;
  booleanToolModeRef.current = booleanToolMode;
  deformSelectionModeRef.current = deformSelectionMode;
  deformBrushSizeRef.current = deformBrushSize;
  onObjectChangeRef.current = onObjectChange;
  onGeometryChangeRef.current = onGeometryChange;
  onSelectRef.current = onSelect;
  onPerformCutRef.current = onPerformCut;

  const getSnapPoints = (mesh: THREE.Mesh): THREE.Vector3[] => {
    mesh.updateMatrixWorld();
    const box = new THREE.Box3().setFromObject(mesh);
    const points: THREE.Vector3[] = [];
    points.push(new THREE.Vector3(box.min.x, box.min.y, box.min.z));
    points.push(new THREE.Vector3(box.max.x, box.min.y, box.min.z));
    points.push(new THREE.Vector3(box.min.x, box.max.y, box.min.z));
    points.push(new THREE.Vector3(box.max.x, box.max.y, box.min.z));
    points.push(new THREE.Vector3(box.min.x, box.min.y, box.max.z));
    points.push(new THREE.Vector3(box.max.x, box.min.y, box.max.z));
    points.push(new THREE.Vector3(box.min.x, box.max.y, box.max.z));
    points.push(new THREE.Vector3(box.max.x, box.max.y, box.max.z));
    points.push(new THREE.Vector3((box.min.x + box.max.x) / 2, box.min.y, (box.min.z + box.max.z) / 2));
    points.push(new THREE.Vector3((box.min.x + box.max.x) / 2, box.max.y, (box.min.z + box.max.z) / 2));
    points.push(new THREE.Vector3(box.min.x, (box.min.y + box.max.y) / 2, (box.min.z + box.max.z) / 2));
    points.push(new THREE.Vector3(box.max.x, (box.min.y + box.max.y) / 2, (box.min.z + box.max.z) / 2));
    points.push(new THREE.Vector3((box.min.x + box.max.x) / 2, (box.min.y + box.max.y) / 2, box.min.z));
    points.push(new THREE.Vector3((box.min.x + box.max.x) / 2, (box.min.y + box.max.y) / 2, box.max.z));
    points.push(box.getCenter(new THREE.Vector3()));
    return points;
  };

  

  const rebuildDeformHandles = useCallback(() => {
    handleGroupRef.current.visible = false;
  }, []);

  const setRaycasterFromPointer = useCallback((event: PointerEvent, el: HTMLElement, camera: THREE.PerspectiveCamera) => {
    const rect = el.getBoundingClientRect();
    const x = ((event.clientX - rect.left) / Math.max(el.clientWidth, 1)) * 2 - 1;
    const y = -((event.clientY - rect.top) / Math.max(el.clientHeight, 1)) * 2 + 1;
    mouseRef.current.set(x, y);
    raycasterRef.current.setFromCamera(mouseRef.current, camera);
  }, []);

  

  const startDeformDrag = useCallback((event: PointerEvent, el: HTMLElement, camera: THREE.PerspectiveCamera, objectId: string, mesh: THREE.Mesh, intersect: THREE.Intersection) => {
    const controls = controlsRef.current;
    if (!controls) return false;

    const geometry = mesh.geometry as THREE.BufferGeometry;
    const position = geometry.getAttribute('position') as THREE.BufferAttribute | undefined;
    if (!position) return false;

    mesh.updateMatrixWorld();
    const anchorWorld = intersect.point.clone();
    const anchorLocal = mesh.worldToLocal(anchorWorld.clone());
    const normalLocal = intersect.face?.normal?.clone() || new THREE.Vector3(0,1,0);

    const kind = deformSelectionModeRef.current;
    if (kind === 'object') return false;

    const brushSize = deformBrushSizeRef.current;
    const vertexWeights = new Float32Array(position.count);
    const vertex = new THREE.Vector3();
    let hasNonZeroWeight = false;

    let lineDir = new THREE.Vector3();
    if (kind === 'line') {
      lineDir.crossVectors(normalLocal, new THREE.Vector3(0,1,0));
      if (lineDir.lengthSq() < 0.01) lineDir.crossVectors(normalLocal, new THREE.Vector3(1,0,0));
      lineDir.normalize();
    }

    for (let i = 0; i < position.count; i += 1) {
      vertex.fromBufferAttribute(position, i);
      let dist = 0;
      
      if (kind === 'point') {
        dist = vertex.distanceTo(anchorLocal);
      } else if (kind === 'plane') {
        const plane = new THREE.Plane().setFromNormalAndCoplanarPoint(normalLocal, anchorLocal);
        dist = Math.abs(plane.distanceToPoint(vertex));
      } else if (kind === 'line') {
        const line = new THREE.Line3(anchorLocal.clone().sub(lineDir.clone().multiplyScalar(1000)), anchorLocal.clone().add(lineDir.clone().multiplyScalar(1000)));
        const closest = new THREE.Vector3();
        line.closestPointToPoint(vertex, false, closest);
        dist = vertex.distanceTo(closest);
      }

      if (dist <= brushSize) {
        const t = 1.0 - (dist / brushSize) * (dist / brushSize);
        vertexWeights[i] = t * t;
        hasNonZeroWeight = true;
      } else {
        vertexWeights[i] = 0;
      }
    }

    if (!hasNonZeroWeight) return false;

    const cameraNormal = camera.getWorldDirection(new THREE.Vector3()).normalize();
    const dragPlane = new THREE.Plane().setFromNormalAndCoplanarPoint(cameraNormal, anchorWorld);
    
    activeDragRef.current = {
      objectId,
      mesh,
      kind,
      startPositions: new Float32Array(position.array as Float32Array),
      startWorldHit: anchorWorld,
      dragPlane,
      pointerId: event.pointerId,
      vertexWeights,
      brushSize
    };

    controls.enabled = false;
    transformRef.current?.detach();
    if (!el.hasPointerCapture(event.pointerId)) {
      el.setPointerCapture(event.pointerId);
    }
    event.preventDefault();
    return true;
  }, []);

  const moveDeformDrag = useCallback((event: PointerEvent, el: HTMLElement, camera: THREE.PerspectiveCamera) => {
    const drag = activeDragRef.current;
    if (!drag) return;

    setRaycasterFromPointer(event, el, camera);
    const currentWorldHit = raycasterRef.current.ray.intersectPlane(drag.dragPlane, new THREE.Vector3());
    if (!currentWorldHit) return;

    drag.mesh.updateMatrixWorld();
    const startLocal = drag.mesh.worldToLocal(drag.startWorldHit.clone());
    const currentLocal = drag.mesh.worldToLocal(currentWorldHit);
    const deltaLocal = currentLocal.sub(startLocal);
    
    const geometry = drag.mesh.geometry as THREE.BufferGeometry;
    const position = geometry.getAttribute('position') as THREE.BufferAttribute;

    for (let i = 0; i < position.count; i++) {
      const w = drag.vertexWeights[i];
      if (w > 0) {
        const base = i * 3;
        position.setXYZ(
          i,
          drag.startPositions[base] + deltaLocal.x * w,
          drag.startPositions[base + 1] + deltaLocal.y * w,
          drag.startPositions[base + 2] + deltaLocal.z * w
        );
      }
    }

    position.needsUpdate = true;
    geometry.computeVertexNormals();
    geometry.computeBoundingBox();
    geometry.computeBoundingSphere();
    event.preventDefault();
  }, [setRaycasterFromPointer]);

  const finishDeformDrag = useCallback((event: PointerEvent | null, el: HTMLElement | null) => {
    const drag = activeDragRef.current;
    if (!drag) return;

    activeDragRef.current = null;
    controlsRef.current!.enabled = true;

    const sourceGeometry = drag.mesh.geometry as THREE.BufferGeometry;
    const finalGeometry = sourceGeometry.clone();
    finalGeometry.userData = { ...sourceGeometry.userData, isDeformed: true };

    if (!sourceGeometry.userData.isPreScaled) {
      finalGeometry.scale(drag.mesh.scale.x, drag.mesh.scale.y, drag.mesh.scale.z);
      finalGeometry.userData.isPreScaled = true;
    }

    finalGeometry.computeVertexNormals();
    finalGeometry.computeBoundingBox();
    finalGeometry.computeBoundingSphere();

    onGeometryChangeRef.current?.(drag.objectId, finalGeometry, getGeometrySize(finalGeometry));
    handleGroupRef.current.visible = true;
    rebuildDeformHandles();

    if (event && el?.hasPointerCapture(event.pointerId)) {
      el.releasePointerCapture(event.pointerId);
    }
    event?.preventDefault();
  }, [rebuildDeformHandles]);

  // Initialization
  useEffect(() => {
    if (!containerRef.current) return;
    const el = containerRef.current;

    const scene = new THREE.Scene();
    scene.background = null;
    sceneRef.current = scene;

    const camera = new THREE.PerspectiveCamera(50, el.clientWidth / Math.max(el.clientHeight, 1), 0.1, 1000);
    camera.position.set(200, 150, 200);
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

    scene.add(new THREE.AmbientLight(0xffffff, 0.6));
    const dirLight = new THREE.DirectionalLight(0x00d4ff, 1.2);
    dirLight.position.set(200, 300, 200);
    scene.add(dirLight);
    const dirLight2 = new THREE.DirectionalLight(0x9d4edd, 0.8);
    dirLight2.position.set(-200, 100, -200);
    scene.add(dirLight2);

    // Ghost mesh for snapping preview
    const ghostMat = new THREE.MeshBasicMaterial({ color: 0xffcc00, wireframe: true, transparent: true, opacity: 0.5 });
    const ghostMesh = new THREE.Mesh(new THREE.BufferGeometry(), ghostMat);
    ghostMesh.visible = false;
    scene.add(ghostMesh);
    ghostMeshRef.current = ghostMesh;

    scene.add(handleGroupRef.current);
    raycasterRef.current.params.Line = { threshold: 8 };

    const transform = new TransformControls(camera, renderer.domElement);
    scene.add(transform.getHelper());
    transformRef.current = transform;

    transform.addEventListener('dragging-changed', event => {
      const isDragging = Boolean((event as { value?: boolean }).value);
      controls.enabled = !isDragging;
      
      const mesh = transform.object as THREE.Mesh | undefined;
      const ghost = ghostMeshRef.current;
      
      if (isDragging && mesh && ghost) {
        // Drag started
        ghost.geometry = mesh.geometry;
        ghost.scale.copy(mesh.scale);
        ghost.visible = false;
      } else if (!isDragging && mesh && ghost && ghost.visible) {
        // Drag ended, snap if ghost is visible
        mesh.position.copy(ghost.position);
        ghost.visible = false;
        
        let attachedId: string | null = null;
        meshMapRef.current.forEach((m, id) => {
          if (m === mesh) attachedId = id;
        });
        
        if (attachedId) {
          const currentPos = mesh.position;
          onObjectChangeRef.current(
            attachedId,
            { x: mesh.userData.sizeX, y: mesh.userData.sizeY, z: mesh.userData.sizeZ },
            { x: currentPos.x, y: currentPos.y, z: currentPos.z }
          );
        }
      } else if (!isDragging && ghost) {
        ghost.visible = false;
      }
    });

    const onPointerDown = (event: PointerEvent) => {
      if (event.button !== 0 || transform.dragging || activeDragRef.current) return;
      
      setRaycasterFromPointer(event, el, camera);

      const meshes = Array.from(meshMapRef.current.values());
      const intersects = raycasterRef.current.intersectObjects(meshes, false);

      if (deformSelectionModeRef.current !== 'object' && selectedIdRef.current && !booleanToolModeRef.current) {
        const selectedMesh = meshMapRef.current.get(selectedIdRef.current);
        if (selectedMesh) {
          const selectedIntersects = raycasterRef.current.intersectObject(selectedMesh, false);
          if (selectedIntersects.length > 0) {
            if (startDeformDrag(event, el, camera, selectedIdRef.current, selectedMesh, selectedIntersects[0])) {
              return;
            }
          }
        }
      }
      
      if (intersects.length > 0) {
        const clickedMesh = intersects[0].object as THREE.Mesh;
        let clickedId: string | null = null;
        meshMapRef.current.forEach((mesh, id) => {
          if (mesh === clickedMesh) clickedId = id;
        });
        
        if (clickedId) {
          if (booleanToolModeRef.current && selectedIdRef.current && clickedId !== selectedIdRef.current) {
            onPerformCutRef.current?.(selectedIdRef.current, clickedId);
          } else {
            onSelectRef.current(clickedId);
          }
        }
      } else if (!booleanToolModeRef.current) {
        onSelectRef.current(null);
      }
    };

    const onPointerMove = (event: PointerEvent) => {
      if (activeDragRef.current) {
        moveDeformDrag(event, el, camera);
      }
    };

    const onPointerUp = (event: PointerEvent) => {
      if (activeDragRef.current) {
        finishDeformDrag(event, el);
      }
    };
    
    el.addEventListener('pointerdown', onPointerDown);
    el.addEventListener('pointermove', onPointerMove);
    el.addEventListener('pointerup', onPointerUp);
    el.addEventListener('pointercancel', onPointerUp);

    const animate = () => {
      frameRef.current = requestAnimationFrame(animate);
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
      rebuildDeformHandles();
    };
    const ro = new ResizeObserver(onResize);
    ro.observe(el);

    return () => {
      el.removeEventListener('pointerdown', onPointerDown);
      el.removeEventListener('pointermove', onPointerMove);
      el.removeEventListener('pointerup', onPointerUp);
      el.removeEventListener('pointercancel', onPointerUp);
      finishDeformDrag(null, null);
      cancelAnimationFrame(frameRef.current);
      ro.disconnect();
      controls.dispose();
      transform.dispose();
      
      if (sceneRef.current && transformRef.current) {
        sceneRef.current.remove(transformRef.current.getHelper());
      }
      
      meshMapRef.current.forEach(mesh => {
        if (sceneRef.current) sceneRef.current.remove(mesh);
        disposeObjectTree(mesh);
      });
      meshMapRef.current.clear();
      handleGroupRef.current.children.forEach(disposeObjectTree);
      handleGroupRef.current.clear();
      
      renderer.dispose();
      el.removeChild(renderer.domElement);
    };
  }, [
        finishDeformDrag,
    moveDeformDrag,
    rebuildDeformHandles,
    setRaycasterFromPointer,
    startDeformDrag,
  ]);

  useEffect(() => {
    const transform = transformRef.current;
    if (!transform) return;

    const handleChange = () => {
      const mesh = transform.object as THREE.Mesh | undefined;
      if (!mesh) return;

      let attachedId: string | null = null;
      meshMapRef.current.forEach((m, id) => {
        if (m === mesh) attachedId = id;
      });

      if (!attachedId) return;

      const obj = objectsRef.current.find(o => o.id === attachedId);
      if (!obj) return;

      const currentScale = mesh.scale;
      const currentPos = mesh.position;

      let newX = obj.size.x;
      let newY = obj.size.y;
      let newZ = obj.size.z;

      if (transformMode === 'scale' && !mesh.geometry.userData.isPreScaled) {
        if (obj.type === 'cylinder') {
          newX = currentScale.x * 2;
          newY = currentScale.y;
          newZ = currentScale.z * 2;
        } else {
          newX = currentScale.x;
          newY = currentScale.y;
          newZ = currentScale.z;
        }
      }

      if (transformMode === 'translate' && isSnappingEnabled) {
        const ghost = ghostMeshRef.current;
        if (ghost) {
          const dragPoints = getSnapPoints(mesh);
          let bestOffset: THREE.Vector3 | null = null;
          let minDistance = 15; // Snapping threshold

          meshMapRef.current.forEach((targetMesh, id) => {
            if (id === attachedId) return; // Don't snap to self
            const targetPoints = getSnapPoints(targetMesh);

            for (const dp of dragPoints) {
              for (const tp of targetPoints) {
                const dist = dp.distanceTo(tp);
                if (dist < minDistance) {
                  minDistance = dist;
                  bestOffset = tp.clone().sub(dp);
                }
              }
            }
          });

          if (bestOffset) {
            ghost.position.copy(currentPos).add(bestOffset);
            ghost.visible = true;
          } else {
            ghost.visible = false;
          }
        }
      }

      if (transform.axis && !ghostMeshRef.current?.visible) {
        onObjectChangeRef.current(attachedId, { x: newX, y: newY, z: newZ }, { x: currentPos.x, y: currentPos.y, z: currentPos.z });
      }
    };

    transform.addEventListener('change', handleChange);
    return () => {
      transform.removeEventListener('change', handleChange);
    };
  }, [transformMode, isSnappingEnabled]);

  // Sync objects
  useEffect(() => {
    const scene = sceneRef.current;
    if (!scene) return;

    const currentIds = new Set(objects.map(o => o.id));
    
    // Remove deleted objects
    const toRemove: string[] = [];
    meshMapRef.current.forEach((mesh, id) => {
      if (!currentIds.has(id)) {
        scene.remove(mesh);
        disposeObjectTree(mesh);
        toRemove.push(id);
      }
    });
    toRemove.forEach(id => meshMapRef.current.delete(id));

    // Add or update objects
    objects.forEach(obj => {
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
          disposeObjectTree(mesh);
        }

        const geometry = createBendedGeometry(obj);
        const material = new THREE.MeshStandardMaterial({ 
          color: 0x00d4ff, 
          metalness: 0.3, 
          roughness: 0.4,
          transparent: true,
          opacity: 0.9,
          side: THREE.DoubleSide
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
        const line = new THREE.LineSegments(edges, new THREE.LineBasicMaterial({ color: 0x000000, transparent: true, opacity: 0.3 }));
        mesh.add(line);
        
        scene.add(mesh);
        meshMapRef.current.set(obj.id, mesh);
        
        // Re-attach transform controls if this was the selected object
        if (selectedId === obj.id && transformRef.current && deformSelectionMode === 'object') {
          transformRef.current.attach(mesh);
        }
      }

      // Sync position and scale if not currently being dragged by transform controls
      const transform = transformRef.current;
      const isBeingDragged = transform && transform.object === mesh && transform.dragging;
      
      if (!isBeingDragged && mesh) {
        mesh.position.set(obj.position.x, obj.position.y, obj.position.z);
        
        if (!mesh.geometry.userData.isPreScaled) {
          let targetScaleX = obj.size.x;
          let targetScaleY = obj.size.y;
          let targetScaleZ = obj.size.z;

          if (obj.type === 'cylinder') {
            targetScaleX = obj.size.x / 2;
            targetScaleZ = obj.size.z / 2;
          }
          mesh.scale.set(targetScaleX, targetScaleY, targetScaleZ);
        } else {
          mesh.scale.set(1, 1, 1);
        }
      }
      
      // Highlight selected object
      if (mesh && mesh.material instanceof THREE.MeshStandardMaterial) {
        if (booleanToolMode && selectedId !== obj.id) {
          // Pulse or highlight potential tools red
          mesh.material.color.setHex(0xff3366);
        } else {
          mesh.material.color.setHex(obj.id === selectedId ? 0x9d4edd : 0x00d4ff);
        }
        mesh.material.opacity = obj.id === selectedId ? 1.0 : 0.7;
      }
    });

    rebuildDeformHandles();
  }, [objects, selectedId, booleanToolMode, deformSelectionMode, rebuildDeformHandles]);

  // Sync transform mode and attachment
  useEffect(() => {
    const transform = transformRef.current;
    if (!transform) return;

    transform.setMode(transformMode);
    
    // Hide transform handles during boolean tool selection or sub-object deformation.
    if (booleanToolMode || deformSelectionMode !== 'object') {
      transform.detach();
      return;
    }

    if (selectedId) {
      const mesh = meshMapRef.current.get(selectedId);
      if (mesh && transform.object !== mesh) {
        transform.attach(mesh);
      }
    } else {
      transform.detach();
    }
  }, [selectedId, transformMode, booleanToolMode, deformSelectionMode]);

  useEffect(() => {
    rebuildDeformHandles();
  }, [deformSelectionMode, selectedId, booleanToolMode, rebuildDeformHandles]);
  
  // Expose CSG and bend helpers for the parent panel.
  useEffect(() => {
    (window as any)._performCSG = (targetId: string, toolId: string) => {
      const targetMesh = meshMapRef.current.get(targetId);
      const toolMesh = meshMapRef.current.get(toolId);
      if (!targetMesh || !toolMesh) return null;

      targetMesh.updateMatrixWorld();
      toolMesh.updateMatrixWorld();

      const brush1 = new Brush(targetMesh.geometry, targetMesh.material as THREE.Material);
      brush1.matrix.copy(targetMesh.matrixWorld);
      
      const brush2 = new Brush(toolMesh.geometry, toolMesh.material as THREE.Material);
      brush2.matrix.copy(toolMesh.matrixWorld);

      const evaluator = new Evaluator();
      const result = evaluator.evaluate(brush1, brush2, SUBTRACTION);
      const finalGeo = result.geometry.clone();
      
      const inverseMatrix = new THREE.Matrix4().copy(targetMesh.matrixWorld).invert();
      finalGeo.applyMatrix4(inverseMatrix);
      
      return finalGeo;
    };
    
    (window as any)._bakeBend = (targetId: string) => {
      const targetMesh = meshMapRef.current.get(targetId);
      if (!targetMesh) return null;
      const finalGeo = targetMesh.geometry.clone();
      // Ensure the scale is baked in so we don't double scale
      finalGeo.userData.isPreScaled = true;
      return finalGeo;
    };
    
    return () => {
      delete (window as any)._performCSG;
      delete (window as any)._bakeBend;
    };
  }, []);

  const cursorClass = deformSelectionMode === 'object' ? 'cursor-crosshair' : 'cursor-grab';

  return <div ref={containerRef} className={`w-full h-full ${cursorClass}`} />;
};
