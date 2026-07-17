import React, { useState } from 'react';
import { Viewport3D } from './Viewport3D';
import { ViewportThree } from './ViewportThree';
import type { ViewportRenderMode } from '../types';
import type { FurnitureType } from '../workshop/furnitureEngine';
import type { StructuralMember } from '../workshop/structuralAssembly';
import { useForgeStore } from '../store/ForgeStoreContext';

import { SceneObject } from './BodyDesign/InteractivePartEditor';
import { Joint } from './BodyDesign/BodyDesignLayout';

interface Props {
  renderMode: ViewportRenderMode;
  furnitureType?: FurnitureType;
  assemblyMembers?: StructuralMember[];
  bodyParts?: SceneObject[];
  bodyJoints?: Joint[];
  partStress?: Record<string, number>;
  loadKg?: number;
  hasModel?: boolean;
}

/** Switches between Three.js (codes/) and canvas isometric fallback. */
export const ViewportShell: React.FC<Props> = props => {
  const [engine, setEngine] = useState<'three' | 'canvas'>('three');
  const { showToast } = useForgeStore();

  return (
    <div className="relative w-full h-full flex flex-col">
      <div className="absolute top-2 right-2 z-20 flex gap-1">
        <button
          type="button"
          onClick={() => setEngine('three')}
          className={`px-2 py-0.5 rounded text-[7px] font-mono border ${
            engine === 'three' ? 'border-forge-accent text-forge-accent' : 'border-forge-border text-forge-text-muted'
          }`}
        >
          3D
        </button>
        <button
          type="button"
          onClick={() => {
            setEngine('canvas');
            showToast('Isometric canvas view');
          }}
          className={`px-2 py-0.5 rounded text-[7px] font-mono border ${
            engine === 'canvas' ? 'border-forge-accent text-forge-accent' : 'border-forge-border text-forge-text-muted'
          }`}
        >
          ISO
        </button>
      </div>
      <div className="flex-1 min-h-0">
        {engine === 'three' ? (
          <ViewportThree {...props} preferBackendScene={!props.assemblyMembers?.length} />
        ) : (
          <Viewport3D {...props} />
        )}
      </div>
    </div>
  );
};
