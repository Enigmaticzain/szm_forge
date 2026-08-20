import React, { useState } from 'react';
import { MaterialLibrary } from './MaterialLibrary';
import { KnowledgeBrowser } from './KnowledgeBrowser';

type Tab = 'materials' | 'reference';

export const KnowledgeWorkspace: React.FC = () => {
  const [tab, setTab] = useState<Tab>('materials');

  return (
    <div className="h-full flex flex-col bg-forge-black">
      <div className="flex border-b border-forge-border bg-forge-dark px-2 gap-1 flex-shrink-0">
        {([
          ['materials', 'MATERIALS'],
          ['reference', 'ENGINEERING REFERENCE'],
        ] as const).map(([id, label]) => (
          <button
            key={id}
            type="button"
            onClick={() => setTab(id)}
            className={`px-4 py-2 text-[9px] font-mono tracking-wider border-b-2 transition-all ${
              tab === id
                ? 'border-forge-purple text-forge-purple'
                : 'border-transparent text-forge-text-muted hover:text-forge-text-dim'
            }`}
          >
            {label}
          </button>
        ))}
      </div>
      <div className="flex-1 min-h-0">
        {tab === 'materials' ? <MaterialLibrary /> : <KnowledgeBrowser />}
      </div>
    </div>
  );
};
