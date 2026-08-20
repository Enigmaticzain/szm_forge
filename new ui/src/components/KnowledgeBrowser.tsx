import React, { useCallback, useEffect, useState } from 'react';
import { BookOpen, Search, Database, ChevronRight, AlertCircle } from 'lucide-react';
import {
  fetchKbDomains,
  fetchKbDomain,
  searchKnowledge,
  KbDomainDto,
  KbSearchHit,
} from '../services/forgeApi';

export const KnowledgeBrowser: React.FC = () => {
  const [domains, setDomains] = useState<KbDomainDto[]>([]);
  const [activeDomain, setActiveDomain] = useState<string>('equations');
  const [entries, setEntries] = useState<Record<string, unknown>[]>([]);
  const [selectedId, setSelectedId] = useState<string | null>(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [searchResults, setSearchResults] = useState<KbSearchHit[] | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    (async () => {
      const list = await fetchKbDomains();
      setDomains(list);
      if (list.length > 0 && list[0].id !== 'materials') {
        setActiveDomain(list.find(d => d.id === 'equations')?.id ?? list[0].id);
      }
      setLoading(false);
    })();
  }, []);

  const loadDomain = useCallback(async (domainId: string) => {
    setError(null);
    setSearchResults(null);
    const data = await fetchKbDomain(domainId);
    const list = (data?.entries as Record<string, unknown>[] | undefined) ?? [];
    setEntries(list);
    setSelectedId(list.length > 0 ? String(list[0].id ?? '') : null);
  }, []);

  useEffect(() => {
    if (!loading && activeDomain) {
      loadDomain(activeDomain);
    }
  }, [activeDomain, loading, loadDomain]);

  const runSearch = async () => {
    if (!searchQuery.trim()) {
      setSearchResults(null);
      return;
    }
    const hits = await searchKnowledge(searchQuery.trim(), activeDomain === 'all' ? '' : activeDomain);
    setSearchResults(hits);
    if (hits.length > 0) {
      setSelectedId(String(hits[0].entry.id ?? ''));
    }
  };

  const displayList: Record<string, unknown>[] = searchResults
    ? searchResults.map(h => ({ ...h.entry, _domain: h.domain }))
    : entries;

  const selected = displayList.find(e => String(e.id) === selectedId);

  if (loading) {
    return (
      <div className="h-full flex items-center justify-center text-forge-text-muted text-[10px] font-mono">
        Loading knowledge base…
      </div>
    );
  }

  if (domains.length === 0) {
    return (
      <div className="h-full flex flex-col items-center justify-center gap-2 p-6">
        <AlertCircle size={20} className="text-forge-yellow" />
        <p className="text-[10px] font-mono text-forge-text-muted text-center">
          Knowledge base unavailable. Ensure backend is running with data/knowledge loaded.
        </p>
      </div>
    );
  }

  return (
    <div className="h-full flex flex-col bg-forge-black">
      <div className="h-[48px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-4 flex-shrink-0">
        <BookOpen size={14} className="text-forge-accent" />
        <span className="text-[10px] font-bold tracking-widest text-forge-accent">KNOWLEDGE BASE</span>
        <span className="text-[8px] font-mono text-forge-text-muted">{domains.length} domains</span>
      </div>

      <div className="flex flex-1 overflow-hidden">
        <div className="w-[200px] border-r border-forge-border bg-forge-panel overflow-y-auto p-2 space-y-1">
          {domains.filter(d => d.id !== 'materials').map(d => (
            <button
              key={d.id}
              type="button"
              onClick={() => { setActiveDomain(d.id); setSearchResults(null); setSearchQuery(''); }}
              className={`w-full text-left px-2 py-1.5 rounded text-[9px] font-mono transition-all ${
                activeDomain === d.id
                  ? 'bg-forge-accent/15 text-forge-accent border border-forge-accent/30'
                  : 'text-forge-text-muted hover:text-forge-text border border-transparent'
              }`}
            >
              <div className="font-semibold truncate">{d.name}</div>
              <div className="text-[7px] opacity-70">{d.entryCount ?? 0} entries</div>
            </button>
          ))}
        </div>

        <div className="flex-1 flex flex-col min-w-0">
          <div className="p-2 border-b border-forge-border/50 flex gap-2">
            <div className="flex-1 flex items-center gap-1.5 px-2 py-1.5 bg-forge-surface rounded border border-forge-border/50">
              <Search size={10} className="text-forge-text-muted" />
              <input
                type="text"
                placeholder="Search equations, processes, standards…"
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                onKeyDown={(e) => e.key === 'Enter' && runSearch()}
                className="bg-transparent text-[10px] text-forge-text outline-none w-full font-mono"
              />
            </div>
            <button
              type="button"
              onClick={runSearch}
              className="px-3 py-1 rounded text-[9px] font-mono bg-forge-accent/10 text-forge-accent border border-forge-accent/20"
            >
              SEARCH
            </button>
          </div>

          {error && <p className="p-2 text-[9px] text-forge-red">{error}</p>}

          <div className="flex flex-1 overflow-hidden">
            <div className="w-[280px] border-r border-forge-border overflow-y-auto p-2 space-y-1">
              {displayList.map(entry => {
                const id = String(entry.id ?? '');
                const name = String(entry.name ?? id);
                return (
                  <button
                    key={`${entry._domain ?? activeDomain}-${id}`}
                    type="button"
                    onClick={() => setSelectedId(id)}
                    className={`w-full p-2 rounded border text-left text-[9px] ${
                      selectedId === id
                        ? 'border-forge-accent/40 bg-forge-accent/5'
                        : 'border-forge-border/50 hover:border-forge-accent/20'
                    }`}
                  >
                    <div className="font-semibold text-forge-text truncate">{name}</div>
                    {entry.category != null && (
                      <div className="text-[8px] text-forge-text-muted truncate">{String(entry.category)}</div>
                    )}
                    {entry.expression != null && (
                      <div className="text-[7px] font-mono text-forge-accent mt-1 truncate">{String(entry.expression)}</div>
                    )}
                  </button>
                );
              })}
              {displayList.length === 0 && (
                <p className="text-[9px] text-forge-text-muted p-2">No entries</p>
              )}
            </div>

            <div className="flex-1 overflow-y-auto p-4">
              {selected ? (
                <>
                  <h2 className="text-base font-bold text-forge-text mb-1">{String(selected.name ?? selected.id)}</h2>
                  {selected._domain != null && (
                    <span className="text-[8px] font-mono text-forge-accent">{String(selected._domain)}</span>
                  )}
                  <pre className="mt-4 p-3 rounded-lg bg-forge-surface/30 border border-forge-border/50 text-[9px] font-mono text-forge-text-dim whitespace-pre-wrap overflow-x-auto">
                    {JSON.stringify(
                      Object.fromEntries(
                        Object.entries(selected).filter(([k]) => k !== '_domain'),
                      ),
                      null,
                      2,
                    )}
                  </pre>
                </>
              ) : (
                <div className="flex flex-col items-center justify-center h-full text-forge-text-muted gap-2">
                  <Database size={24} />
                  <p className="text-[10px] font-mono">Select an entry</p>
                </div>
              )}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
