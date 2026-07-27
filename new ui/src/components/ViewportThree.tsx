/**
 * True 3D viewport using Three.js (aligned with codes/three.js-dev + backend scene.json).
 */
import React, { useEffect, useRef, useCallback } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/examples/jsm/controls/OrbitControls.js';
import type { StructuralMember } from '../workshop/structuralAssembly';
import { membersToViewportBoxes } from '../workshop/structuralAssembly';
import { SceneObject, createBendedGeometry } from './BodyDesign/InteractivePartEditor';
import { Joint } from './BodyDesign/BodyDesignLayout';
import type { FurnitureType } from '../workshop/furnitureEngine';
import {
  boxesForChair,
  boxesForLadder,
  boxesForTable,
} from '../utils/viewportFurniture3d';
import { fetchSceneJson } from '../services/forgeApi';
import { stressRatioToHex } from '../utils/stressColor';
import { useForgeStore } from '../store/ForgeStoreContext';
import type { ViewportRenderMode } from '../types';

interface Props {
  renderMode: ViewportRenderMode;
  furnitureType?: FurnitureType;
  assemblyMembers?: StructuralMember[];
  bodyParts?: SceneObject[];
  bodyJoints?: Joint[];
  partStress?: Record<string, number>;
  loadKg?: number;
  preferBackendScene?: boolean;
}

export const ViewportThree: React.FC<Props> = ({
  renderMode,
  furnitureType,
  assemblyMembers,
  bodyParts,
  bodyJoints,
  partStress = {},
  loadKg = 0,
  preferBackendScene = false,
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const rendererRef = useRef<THREE.WebGLRenderer | null>(null);
  const sceneRef = useRef<THREE.Scene | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const meshesRef = useRef<THREE.Object3D[]>([]);
  const frameRef = useRef<number>(0);
  const gridRef = useRef<THREE.GridHelper | null>(null);
  const buildVersionRef = useRef(0);
  const mountedRef = useRef(true);
  const { viewportZoom, showViewportGrid } = useForgeStore();

  const clearMeshes = useCallback((scene: THREE.Scene) => {
    for (const m of meshesRef.current) {
      scene.remove(m);
      if (m instanceof THREE.Mesh) {
        m.geometry.dispose();
        const mat = m.material;
        if (Array.isArray(mat)) mat.forEach(x => x.dispose());
        else mat.dispose();
      }
    }
    meshesRef.current = [];
  }, []);

  const rebuildModel = useCallback(
    async (scene: THREE.Scene) => {
      const buildVersion = ++buildVersionRef.current;
      clearMeshes(scene);

      try {
        if (preferBackendScene && !assemblyMembers?.length) {
          const data = (await fetchSceneJson()) as {
            geometries?: { uuid: string; type: string; width?: number; height?: number; depth?: number }[];
            materials?: { uuid: string; color?: number }[];
            object?: {
              children?: {
                name: string;
                geometry: string;
                material: string;
                userData?: { stressRatio?: number };
              }[];
            };
          } | null;

          if (data?.geometries?.length && data.object?.children?.length) {
            const geomMap = new Map(data.geometries.map(g => [g.uuid, g]));
            const matMap = new Map(data.materials?.map(m => [m.uuid, m]) ?? []);
            for (const child of data.object.children) {
              if (!mountedRef.current || buildVersion !== buildVersionRef.current) return;
              const g = geomMap.get(child.geometry);
              if (!g || g.type !== 'BoxGeometry') continue;
              const geo = new THREE.BoxGeometry(g.width ?? 0.5, g.height ?? 0.5, g.depth ?? 0.5);
              const matEntry = matMap.get(child.material);
              const ratio = child.userData?.stressRatio ?? 0.3;
              const color = matEntry?.color ?? stressRatioToHex(ratio);
              const mat =
                renderMode === 'wireframe'
                  ? new THREE.MeshBasicMaterial({ color, wireframe: true })
                  : new THREE.MeshStandardMaterial({ color, metalness: 0.4, roughness: 0.5 });
              const mesh = new THREE.Mesh(geo, mat);
              mesh.name = child.name;
              scene.add(mesh);
              meshesRef.current.push(mesh);
            }
            return;
          }
        }

        let boxes = assemblyMembers?.length
          ? membersToViewportBoxes(assemblyMembers, partStress)
          : [];

        if (!boxes.length && furnitureType && (!bodyParts || bodyParts.length === 0)) {
          const ratio = (name: string) => partStress[name] ?? 0.25;
          if (furnitureType === 'table') boxes = boxesForTable(ratio);
          else if (furnitureType === 'chair') boxes = boxesForChair(ratio);
          else boxes = boxesForLadder(ratio);
        }

        for (const b of boxes) {
          if (!mountedRef.current || buildVersion !== buildVersionRef.current) return;
          const geo = new THREE.BoxGeometry(b.w, b.h, b.d);
          const color = stressRatioToHex(b.stressRatio);
          const mat =
            renderMode === 'wireframe'
              ? new THREE.MeshBasicMaterial({ color, wireframe: true })
              : new THREE.MeshStandardMaterial({ color, metalness: 0.35, roughness: 0.45 });
          const mesh = new THREE.Mesh(geo, mat);
          mesh.position.set(b.cx, b.cy + b.h / 2, b.cz);
          mesh.name = b.label;
          scene.add(mesh);
          meshesRef.current.push(mesh);
        }

        if (loadKg > 0 && boxes.length) {
          if (!mountedRef.current || buildVersion !== buildVersionRef.current) return;
          const topY = Math.max(...boxes.map(b => b.cy + b.h));
          const arrow = new THREE.ArrowHelper(
            new THREE.Vector3(0, -1, 0),
            new THREE.Vector3(0, topY + 0.35, 0),
            0.4,
            0xffcc00
          );
          scene.add(arrow);
          meshesRef.current.push(arrow);
        }

        if (bodyParts && bodyParts.length > 0) {
          const cadScale = 0.01;

          for (const obj of bodyParts) {
            if (!mountedRef.current || buildVersion !== buildVersionRef.current) return;
            const geo = createBendedGeometry(obj);
            const color = stressRatioToHex(0.2);
            const mat =
              renderMode === 'wireframe'
                ? new THREE.MeshBasicMaterial({ color, wireframe: true })
                : new THREE.MeshStandardMaterial({ color, metalness: 0.35, roughness: 0.45 });

            const mesh = new THREE.Mesh(geo, mat);
            mesh.position.set(obj.position.x * cadScale, obj.position.y * cadScale, obj.position.z * cadScale);

            let targetScaleX = 1;
            let targetScaleY = 1;
            let targetScaleZ = 1;

            if (!geo.userData.isPreScaled && !obj.customGeometry) {
              targetScaleX = obj.size.x * cadScale;
              targetScaleY = obj.size.y * cadScale;
              targetScaleZ = obj.size.z * cadScale;

              if (obj.type === 'cylinder') {
                targetScaleX = (obj.size.x / 2) * cadScale;
                targetScaleZ = (obj.size.z / 2) * cadScale;
              }
            } else {
              mesh.scale.set(cadScale, cadScale, cadScale);
            }

            if (!geo.userData.isPreScaled && !obj.customGeometry) {
              mesh.scale.set(targetScaleX, targetScaleY, targetScaleZ);
            }

            scene.add(mesh);
            meshesRef.current.push(mesh);
          }
        }

        if (bodyJoints && bodyJoints.length > 0) {
          const cadScale = 0.01;
          for (const joint of bodyJoints) {
            if (!mountedRef.current || buildVersion !== buildVersionRef.current) return;
            let color = 0xffffff;
            if (joint.type === 'bolt') color = 0xffcc00;
            else if (joint.type === 'glue') color = 0xff00ff;
            else if (joint.type.includes('weld')) color = 0x00d4ff;

            const sphereGeo = new THREE.SphereGeometry(joint.size * cadScale, 16, 16);
            const sphereMat = new THREE.MeshStandardMaterial({ color, metalness: 0.8, roughness: 0.2 });
            const sphere = new THREE.Mesh(sphereGeo, sphereMat);
            sphere.position.set(joint.position.x * cadScale, joint.position.y * cadScale, joint.position.z * cadScale);

            scene.add(sphere);
            meshesRef.current.push(sphere);
          }
        }
      } catch (error) {
        console.warn('ViewportThree failed to rebuild scene', error);
      }
    },
    [
      assemblyMembers,
      bodyParts,
      bodyJoints,
      partStress,
      furnitureType,
      renderMode,
      loadKg,
      preferBackendScene,
      clearMeshes,
    ]
  );

  useEffect(() => {
    const el = containerRef.current;
    if (!el || rendererRef.current) return;

    mountedRef.current = true;
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0a0f);
    sceneRef.current = scene;

    const camera = new THREE.PerspectiveCamera(50, el.clientWidth / Math.max(el.clientHeight, 1), 0.01, 200);
    camera.position.set(2.2, 1.6, 2.4);
    cameraRef.current = camera;

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.setSize(el.clientWidth, el.clientHeight);
    el.appendChild(renderer.domElement);
    rendererRef.current = renderer;

    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.target.set(0, 0.5, 0);
    controlsRef.current = controls;

    scene.add(new THREE.AmbientLight(0x404060, 0.9));
    const dir = new THREE.DirectionalLight(0x00d4ff, 1.1);
    dir.position.set(3, 5, 2);
    scene.add(dir);

    const grid = new THREE.GridHelper(4, 20, 0x1e1e2a, 0x16161f);
    grid.visible = showViewportGrid;
    scene.add(grid);
    gridRef.current = grid;

    const animate = () => {
      if (!mountedRef.current) return;
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
    };
    const ro = new ResizeObserver(onResize);
    ro.observe(el);

    return () => {
      mountedRef.current = false;
      cancelAnimationFrame(frameRef.current);
      ro.disconnect();
      controls.dispose();
      clearMeshes(scene);
      if (gridRef.current) scene.remove(gridRef.current);
      renderer.dispose();
      rendererRef.current = null;
      sceneRef.current = null;
      cameraRef.current = null;
      controlsRef.current = null;
      gridRef.current = null;
      meshesRef.current = [];
      if (el.contains(renderer.domElement)) {
        el.removeChild(renderer.domElement);
      }
    };
  }, []);

  useEffect(() => {
    const scene = sceneRef.current;
    if (scene) void rebuildModel(scene);
  }, [rebuildModel]);

  useEffect(() => {
    if (gridRef.current) gridRef.current.visible = showViewportGrid;
  }, [showViewportGrid]);

  useEffect(() => {
    const cam = cameraRef.current;
    if (!cam) return;
    const dist = 2.8 / viewportZoom;
    cam.position.normalize().multiplyScalar(dist);
  }, [viewportZoom]);

  return (
    <div ref={containerRef} className="w-full h-full min-h-[200px] relative bg-forge-black">
      <div className="absolute top-2 left-2 z-10 px-2 py-0.5 rounded bg-forge-panel/80 border border-forge-border text-[8px] font-mono text-forge-accent">
        THREE.JS · codes/
      </div>
    </div>
  );
};
