import React, { useState } from 'react';
import { queryGraphRAG, queryReasoning } from '../services/forgeApi';
import { Network, Search, MessageSquare, Zap, Target } from 'lucide-react';

export const GraphRAGPanel: React.FC = () => {
  const [query, setQuery] = useState('');
  const [mode, setMode] = useState<'graphrag' | 'reasoning'>('graphrag');
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState<any>(null);

  const handleQuery = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!query.trim()) return;

    setLoading(true);
    setResult(null);
    try {
      if (mode === 'graphrag') {
        const data = await queryGraphRAG(query);
        setResult(data);
      } else {
        const data = await queryReasoning(query, 'forward_chain');
        setResult(data);
      }
    } catch (err) {
      console.error(err);
    }
    setLoading(false);
  };

  return (
    <div className="h-full flex flex-col bg-forge-panel p-4">
      <div className="flex items-center gap-2 mb-4">
        <Network size={16} className="text-forge-purple" />
        <span className="text-[12px] font-bold tracking-widest text-forge-text-dim">GraphRAG & Causal Reasoning</span>
      </div>

      <div className="flex gap-2 mb-4">
        <button
          onClick={() => setMode('graphrag')}
          className={`px-3 py-1.5 text-[10px] font-mono rounded ${mode === 'graphrag' ? 'bg-forge-purple text-white' : 'bg-forge-surface text-forge-text-muted hover:text-forge-text'}`}
        >
          GraphRAG
        </button>
        <button
          onClick={() => setMode('reasoning')}
          className={`px-3 py-1.5 text-[10px] font-mono rounded ${mode === 'reasoning' ? 'bg-forge-accent text-white' : 'bg-forge-surface text-forge-text-muted hover:text-forge-text'}`}
        >
          Causal Reasoning
        </button>
      </div>

      <form onSubmit={handleQuery} className="flex gap-2 mb-4">
        <input
          type="text"
          value={query}
          onChange={e => setQuery(e.target.value)}
          placeholder={mode === 'graphrag' ? "Ask the engineering knowledge graph..." : "Enter a design problem..."}
          className="flex-1 bg-forge-dark border border-forge-border rounded px-3 py-2 text-[11px] font-mono text-forge-text focus:outline-none focus:border-forge-purple/50"
        />
        <button
          type="submit"
          disabled={loading}
          className="px-4 py-2 bg-forge-purple text-white rounded font-mono text-[10px] hover:bg-forge-purple/80 transition-colors flex items-center gap-2 disabled:opacity-50"
        >
          {loading ? 'PROCESSING...' : <><Search size={12} /> QUERY</>}
        </button>
      </form>

      <div className="flex-1 bg-forge-dark rounded border border-forge-border p-4 overflow-y-auto custom-scrollbar">
        {!result && !loading && (
          <div className="h-full flex flex-col items-center justify-center text-forge-text-muted opacity-50">
            <MessageSquare size={32} className="mb-2" />
            <p className="text-[11px] font-mono">
              {mode === 'graphrag' ? 'Ask questions to traverse the engineering ontology.' : 'Analyze failure modes and design constraints.'}
            </p>
          </div>
        )}

        {loading && (
          <div className="text-[11px] font-mono text-forge-text-muted animate-pulse">
            <Zap size={14} className="inline mr-2 text-forge-accent" /> Analyzing cognitive graph...
          </div>
        )}

        {result && mode === 'graphrag' && (
          <div className="space-y-4 text-forge-text">
            <div className="text-[12px] whitespace-pre-wrap">{result.answer || result.error || 'No direct answer found.'}</div>
            {result.context && result.context.length > 0 && (
              <div className="mt-4 pt-4 border-t border-forge-border/50">
                <span className="text-[10px] font-bold text-forge-text-dim block mb-2">GRAPH CONTEXT USED</span>
                <ul className="space-y-1">
                  {result.context.map((ctx: any, i: number) => (
                    <li key={i} className="text-[10px] font-mono text-forge-text-muted bg-forge-surface/30 p-2 rounded">
                      {JSON.stringify(ctx)}
                    </li>
                  ))}
                </ul>
              </div>
            )}
          </div>
        )}

        {result && mode === 'reasoning' && (
          <div className="space-y-4 text-forge-text">
            <div className="flex items-center gap-2 text-forge-accent text-[11px] font-bold uppercase">
              <Target size={14} /> Mode: {result.mode || result.error}
            </div>
            {result.derived_facts && (
              <div className="mt-2">
                <span className="text-[10px] font-bold text-forge-text-dim block mb-2">DERIVED CONCLUSIONS</span>
                <ul className="list-disc pl-4 space-y-1 text-[11px] text-forge-green font-mono">
                  {Array.isArray(result.derived_facts) 
                    ? result.derived_facts.map((fact: string, i: number) => <li key={i}>{fact}</li>) 
                    : <li>{JSON.stringify(result.derived_facts)}</li>}
                </ul>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
};
