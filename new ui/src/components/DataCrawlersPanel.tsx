import React, { useState } from 'react';
import { searchArxiv, searchPatents, searchNasa } from '../services/forgeApi';
import { Search, ExternalLink, Database, Globe } from 'lucide-react';
import { useForgeStore } from '../store/ForgeStoreContext';

export const DataCrawlersPanel: React.FC = () => {
  const store = useForgeStore();
  const [query, setQuery] = useState('');
  const [source, setSource] = useState<'arxiv' | 'patents' | 'nasa'>('arxiv');
  const [results, setResults] = useState<any[]>([]);
  const [isSearching, setIsSearching] = useState(false);

  const handleSearch = async () => {
    if (!query) return;
    setIsSearching(true);
    try {
      let data: any = { results: [] };
      if (source === 'arxiv') data = await searchArxiv(query, 5);
      else if (source === 'patents') data = await searchPatents(query, 5);
      else if (source === 'nasa') data = await searchNasa(query, 5);
      
      setResults(data.results || []);
    } catch (e: any) {
      store.showToast(`Search failed: ${e.message}`, 'error');
    } finally {
      setIsSearching(false);
    }
  };

  return (
    <div className="h-full flex flex-col p-4 bg-forge-black text-forge-text">
      <div className="flex items-center gap-2 mb-4">
        <Globe size={18} className="text-forge-accent" />
        <h2 className="text-sm font-bold font-mono tracking-widest text-forge-text">EXTERNAL DATA CRAWLERS</h2>
      </div>
      
      <div className="flex gap-2 mb-6">
        <select 
          value={source}
          onChange={(e) => setSource(e.target.value as any)}
          className="bg-forge-dark border border-forge-border rounded p-2 text-sm font-mono focus:border-forge-accent outline-none"
        >
          <option value="arxiv">arXiv (Research Papers)</option>
          <option value="patents">US Patents</option>
          <option value="nasa">NASA Technical Reports</option>
        </select>
        
        <div className="flex-1 flex border border-forge-border rounded bg-forge-dark overflow-hidden focus-within:border-forge-accent">
          <input 
            type="text" 
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            onKeyDown={(e) => e.key === 'Enter' && handleSearch()}
            placeholder="Search engineering literature..."
            className="flex-1 bg-transparent p-2 text-sm font-mono outline-none"
          />
          <button 
            onClick={handleSearch}
            disabled={isSearching}
            className="px-4 bg-forge-surface hover:bg-forge-accent/20 flex items-center justify-center transition-colors"
          >
            {isSearching ? <div className="w-4 h-4 border-2 border-forge-accent border-t-transparent rounded-full animate-spin" /> : <Search size={16} className="text-forge-text-muted" />}
          </button>
        </div>
      </div>
      
      <div className="flex-1 overflow-y-auto space-y-3 pr-2">
        {results.length === 0 && !isSearching && (
          <div className="flex flex-col items-center justify-center h-full text-forge-text-muted opacity-50">
            <Database size={48} className="mb-4" />
            <span className="font-mono text-sm">Query engineering databases worldwide</span>
          </div>
        )}
        
        {results.map((res, i) => (
          <div key={i} className="p-4 border border-forge-border/50 rounded-lg bg-forge-dark/50 hover:border-forge-accent/50 transition-colors">
            <div className="flex justify-between items-start mb-2">
              <h3 className="text-sm font-bold text-forge-text break-words pr-4">{res.title || res.title_display}</h3>
              {res.pdf_url || res.url ? (
                <a href={res.pdf_url || res.url} target="_blank" rel="noreferrer" className="text-forge-accent hover:text-forge-green">
                  <ExternalLink size={14} />
                </a>
              ) : null}
            </div>
            
            {res.authors && (
              <p className="text-[10px] font-mono text-forge-purple mb-2">
                {Array.isArray(res.authors) ? res.authors.join(', ') : res.authors}
              </p>
            )}
            
            <p className="text-xs text-forge-text-dim line-clamp-3">
              {res.summary || res.abstract}
            </p>
          </div>
        ))}
      </div>
    </div>
  );
};
