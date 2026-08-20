import React, { useEffect, useMemo, useState } from 'react';
import {
  Palette, Search, Database, Thermometer, Shield, Star, Check, AlertCircle,
} from 'lucide-react';
import { fetchMaterials, MaterialDto } from '../services/forgeApi';

const FAVORITES_KEY = 'szm_material_favorites';

function loadFavorites(): Set<string> {
  try {
    const raw = localStorage.getItem(FAVORITES_KEY);
    if (!raw) return new Set();
    return new Set(JSON.parse(raw) as string[]);
  } catch {
    return new Set();
  }
}

function saveFavorites(favs: Set<string>) {
  localStorage.setItem(FAVORITES_KEY, JSON.stringify([...favs]));
}

export const MaterialLibrary: React.FC = () => {
  const [materials, setMaterials] = useState<MaterialDto[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [selectedMaterial, setSelectedMaterial] = useState<string>('');
  const [searchQuery, setSearchQuery] = useState('');
  const [activeCategory, setActiveCategory] = useState('All');
  const [showFavorites, setShowFavorites] = useState(false);
  const [favorites, setFavorites] = useState<Set<string>>(loadFavorites);

  useEffect(() => {
    let cancelled = false;
    (async () => {
      setLoading(true);
      setError(null);
      const mats = await fetchMaterials();
      if (cancelled) return;
      if (mats.length === 0) {
        setError('No materials returned. Start the SZM Forge backend on port 8000.');
      } else {
        setMaterials(mats);
        setSelectedMaterial(mats[0].id);
      }
      setLoading(false);
    })();
    return () => { cancelled = true; };
  }, []);

  const categories = useMemo(() => {
    const cats = new Set<string>();
    materials.forEach(m => { if (m.category) cats.add(m.category); });
    return ['All', ...Array.from(cats).sort()];
  }, [materials]);

  const filteredMaterials = materials.filter(m => {
    if (showFavorites && !favorites.has(m.id)) return false;
    if (activeCategory !== 'All' && m.category !== activeCategory) return false;
    if (searchQuery && !m.name.toLowerCase().includes(searchQuery.toLowerCase())) return false;
    return true;
  });

  const selected = materials.find(m => m.id === selectedMaterial);

  const toggleFavorite = (id: string) => {
    setFavorites(prev => {
      const next = new Set(prev);
      if (next.has(id)) next.delete(id);
      else next.add(id);
      saveFavorites(next);
      return next;
    });
  };

  if (loading) {
    return (
      <div className="h-full flex items-center justify-center bg-forge-black text-forge-text-muted text-[10px] font-mono">
        Loading materials from backend…
      </div>
    );
  }

  if (error) {
    return (
      <div className="h-full flex flex-col items-center justify-center gap-2 bg-forge-black p-6">
        <AlertCircle size={20} className="text-forge-yellow" />
        <p className="text-[10px] font-mono text-forge-text-muted text-center max-w-sm">{error}</p>
      </div>
    );
  }

  return (
    <div className="h-full flex flex-col bg-forge-black">
      <div className="h-[48px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-4 flex-shrink-0">
        <Database size={14} className="text-forge-purple" />
        <span className="text-[10px] font-bold tracking-widest text-forge-purple">MATERIAL LIBRARY</span>
        <span className="text-[8px] font-mono text-forge-text-muted">{materials.length} materials</span>
      </div>

      <div className="flex-1 flex overflow-hidden">
        <div className="w-[340px] border-r border-forge-border bg-forge-panel flex flex-col">
          <div className="p-2 border-b border-forge-border/50 space-y-2">
            <div className="flex items-center gap-1.5 px-2 py-1.5 bg-forge-surface rounded border border-forge-border/50">
              <Search size={10} className="text-forge-text-muted" />
              <input
                type="text"
                placeholder="Search materials..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="bg-transparent text-[10px] text-forge-text placeholder-forge-text-muted outline-none w-full font-mono"
              />
            </div>
            <div className="flex items-center gap-1 flex-wrap max-h-16 overflow-y-auto">
              {categories.slice(0, 6).map(cat => (
                <button
                  key={cat}
                  onClick={() => setActiveCategory(cat)}
                  className={`px-2 py-0.5 rounded text-[8px] font-mono transition-all ${
                    activeCategory === cat
                      ? 'bg-forge-purple/15 text-forge-purple border border-forge-purple/30'
                      : 'text-forge-text-muted hover:text-forge-text-dim border border-transparent'
                  }`}
                >
                  {cat.length > 18 ? `${cat.slice(0, 16)}…` : cat}
                </button>
              ))}
              <button
                onClick={() => setShowFavorites(!showFavorites)}
                className={`px-2 py-0.5 rounded text-[8px] font-mono transition-all flex items-center gap-1 ${
                  showFavorites
                    ? 'bg-forge-yellow/15 text-forge-yellow border border-forge-yellow/30'
                    : 'text-forge-text-muted hover:text-forge-text-dim border border-transparent'
                }`}
              >
                <Star size={8} /> Favorites
              </button>
            </div>
          </div>

          <div className="flex-1 overflow-y-auto p-2 space-y-1">
            {filteredMaterials.map(mat => (
              <button
                key={mat.id}
                onClick={() => setSelectedMaterial(mat.id)}
                className={`w-full p-2.5 rounded-lg border text-left transition-all ${
                  selectedMaterial === mat.id
                    ? 'border-forge-purple/40 bg-forge-purple/5'
                    : 'border-forge-border/50 bg-forge-surface/20 hover:border-forge-purple/20'
                }`}
              >
                <div className="flex items-center justify-between mb-1">
                  <div className="flex items-center gap-1.5 min-w-0">
                    <Palette size={10} className="text-forge-purple flex-shrink-0" />
                    <span className="text-[10px] font-semibold text-forge-text truncate">{mat.name}</span>
                  </div>
                  <div className="flex items-center gap-1 flex-shrink-0">
                    {favorites.has(mat.id) && (
                      <Star size={8} className="text-forge-yellow fill-forge-yellow" />
                    )}
                    {selectedMaterial === mat.id && (
                      <Check size={8} className="text-forge-green" />
                    )}
                  </div>
                </div>
                <div className="text-[8px] text-forge-text-muted mb-1.5 truncate">{mat.category ?? '—'}</div>
                <div className="flex items-center gap-3 text-[8px] font-mono flex-wrap">
                  <span className="text-forge-text-dim">ρ: {mat.density_kg_m3}</span>
                  <span className="text-forge-text-dim">E: {mat.youngsModulus_GPa} GPa</span>
                  <span className="text-forge-text-dim">σy: {mat.yieldStrength_MPa} MPa</span>
                </div>
              </button>
            ))}
          </div>
        </div>

        {selected && (
          <div className="flex-1 overflow-y-auto p-4">
            <div className="flex items-center justify-between mb-4">
              <div>
                <h2 className="text-lg font-bold text-forge-text flex items-center gap-2">
                  {selected.name}
                  <button
                    type="button"
                    onClick={() => toggleFavorite(selected.id)}
                    className="p-1 rounded hover:bg-forge-surface"
                    aria-label="Toggle favorite"
                  >
                    <Star
                      size={14}
                      className={favorites.has(selected.id) ? 'text-forge-yellow fill-forge-yellow' : 'text-forge-text-muted'}
                    />
                  </button>
                </h2>
                <span className="text-[10px] font-mono text-forge-purple">{selected.category ?? 'Uncategorized'}</span>
                <div className="text-[8px] font-mono text-forge-text-muted mt-1">{selected.id}</div>
              </div>
            </div>

            <div className="grid grid-cols-2 gap-3 mb-4">
              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Shield size={12} className="text-forge-accent" />
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">MECHANICAL</span>
                </div>
                <div className="space-y-1.5 text-[9px]">
                  <div className="flex justify-between"><span className="text-forge-text-dim">E</span><span className="font-mono">{selected.youngsModulus_GPa} GPa</span></div>
                  <div className="flex justify-between"><span className="text-forge-text-dim">ν</span><span className="font-mono">{selected.poissonsRatio ?? '—'}</span></div>
                  <div className="flex justify-between"><span className="text-forge-text-dim">Yield</span><span className="font-mono">{selected.yieldStrength_MPa} MPa</span></div>
                  <div className="flex justify-between"><span className="text-forge-text-dim">UTS</span><span className="font-mono">{selected.ultimateStrength_MPa ?? '—'} MPa</span></div>
                  <div className="flex justify-between"><span className="text-forge-text-dim">Density</span><span className="font-mono">{selected.density_kg_m3} kg/m³</span></div>
                </div>
              </div>
              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Thermometer size={12} className="text-forge-orange" />
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">THERMAL</span>
                </div>
                <div className="space-y-1.5 text-[9px]">
                  <div className="flex justify-between"><span className="text-forge-text-dim">k</span><span className="font-mono">{selected.thermalConductivity_W_mK ?? '—'} W/m·K</span></div>
                  <div className="flex justify-between"><span className="text-forge-text-dim">α</span><span className="font-mono">{selected.thermalExpansion_1_K != null ? `${(selected.thermalExpansion_1_K * 1e6).toFixed(1)} μm/m·K` : '—'}</span></div>
                </div>
              </div>
            </div>

            {selected.tags && selected.tags.length > 0 && (
              <div className="flex flex-wrap gap-1 mb-3">
                {selected.tags.map(tag => (
                  <span key={tag} className="px-2 py-0.5 rounded text-[8px] font-mono bg-forge-purple/10 text-forge-purple border border-forge-purple/20">
                    {tag}
                  </span>
                ))}
              </div>
            )}

            {selected.notes && (
              <p className="text-[9px] text-forge-text-dim leading-relaxed">{selected.notes}</p>
            )}
          </div>
        )}
      </div>
    </div>
  );
};
