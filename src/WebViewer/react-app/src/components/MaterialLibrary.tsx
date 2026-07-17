import React, { useState } from 'react';
import {
  Palette, Search, Plus, Database,
  Thermometer, Zap, Shield, ChevronRight,
  Star, Check
} from 'lucide-react';

interface Material {
  id: string;
  name: string;
  category: string;
  density: number;
  youngsModulus: number;
  yieldStrength: number;
  thermalCond: number;
  cte: number;
  favorite: boolean;
  applied: boolean;
}

const materials: Material[] = [
  { id: 'm1', name: 'Ti-6Al-4V (Grade 5)', category: 'Titanium Alloy', density: 4430, youngsModulus: 113.8, yieldStrength: 880, thermalCond: 6.7, cte: 8.6, favorite: true, applied: true },
  { id: 'm2', name: 'Inconel 718', category: 'Nickel Superalloy', density: 8190, youngsModulus: 200, yieldStrength: 1100, thermalCond: 11.4, cte: 13, favorite: true, applied: true },
  { id: 'm3', name: 'AISI 4340 Steel', category: 'Alloy Steel', density: 7850, youngsModulus: 205, yieldStrength: 710, thermalCond: 44.5, cte: 12.3, favorite: false, applied: true },
  { id: 'm4', name: 'Al 7075-T6', category: 'Aluminum Alloy', density: 2810, youngsModulus: 71.7, yieldStrength: 503, thermalCond: 130, cte: 23.6, favorite: false, applied: false },
  { id: 'm5', name: 'CFRP (UD)', category: 'Composite', density: 1600, youngsModulus: 135, yieldStrength: 1500, thermalCond: 5, cte: -0.5, favorite: true, applied: true },
  { id: 'm6', name: 'Waspaloy', category: 'Nickel Superalloy', density: 8190, youngsModulus: 211, yieldStrength: 795, thermalCond: 10.7, cte: 12.7, favorite: false, applied: false },
  { id: 'm7', name: 'Hastelloy X', category: 'Nickel Superalloy', density: 8220, youngsModulus: 205, yieldStrength: 360, thermalCond: 9.1, cte: 13.3, favorite: false, applied: false },
  { id: 'm8', name: 'Ceramic Matrix (SiC/SiC)', category: 'CMC', density: 2700, youngsModulus: 250, yieldStrength: 350, thermalCond: 25, cte: 4.5, favorite: true, applied: false },
];

const categories = ['All', 'Titanium Alloy', 'Nickel Superalloy', 'Alloy Steel', 'Aluminum Alloy', 'Composite', 'CMC'];

export const MaterialLibrary: React.FC = () => {
  const [selectedMaterial, setSelectedMaterial] = useState<string>('m1');
  const [searchQuery, setSearchQuery] = useState('');
  const [activeCategory, setActiveCategory] = useState('All');
  const [showFavorites, setShowFavorites] = useState(false);

  const filteredMaterials = materials.filter(m => {
    if (showFavorites && !m.favorite) return false;
    if (activeCategory !== 'All' && m.category !== activeCategory) return false;
    if (searchQuery && !m.name.toLowerCase().includes(searchQuery.toLowerCase())) return false;
    return true;
  });

  const selected = materials.find(m => m.id === selectedMaterial);

  return (
    <div className="h-full flex flex-col bg-forge-black">
      {/* Header */}
      <div className="h-[48px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-4 flex-shrink-0">
        <Database size={14} className="text-forge-purple" />
        <span className="text-[10px] font-bold tracking-widest text-forge-purple">MATERIAL LIBRARY</span>
        <span className="text-[8px] font-mono text-forge-text-muted">{materials.length} materials</span>
        
        <div className="ml-auto flex items-center gap-2">
          <button className="flex items-center gap-1 px-2.5 py-1 rounded text-[9px] font-mono bg-forge-purple/10 text-forge-purple border border-forge-purple/20 hover:bg-forge-purple/20">
            <Plus size={10} /> ADD MATERIAL
          </button>
        </div>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Materials List */}
        <div className="w-[340px] border-r border-forge-border bg-forge-panel flex flex-col">
          {/* Search & Filter */}
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
            <div className="flex items-center gap-1 flex-wrap">
              {categories.slice(0, 4).map(cat => (
                <button
                  key={cat}
                  onClick={() => setActiveCategory(cat)}
                  className={`px-2 py-0.5 rounded text-[8px] font-mono transition-all ${
                    activeCategory === cat
                      ? 'bg-forge-purple/15 text-forge-purple border border-forge-purple/30'
                      : 'text-forge-text-muted hover:text-forge-text-dim border border-transparent'
                  }`}
                >
                  {cat}
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

          {/* Material List */}
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
                  <div className="flex items-center gap-1.5">
                    <Palette size={10} className="text-forge-purple" />
                    <span className="text-[10px] font-semibold text-forge-text">{mat.name}</span>
                  </div>
                  <div className="flex items-center gap-1">
                    {mat.favorite && <Star size={8} className="text-forge-yellow fill-forge-yellow" />}
                    {mat.applied && <Check size={8} className="text-forge-green" />}
                  </div>
                </div>
                <div className="text-[8px] text-forge-text-muted mb-1.5">{mat.category}</div>
                <div className="flex items-center gap-3 text-[8px] font-mono">
                  <span className="text-forge-text-dim">ρ: {mat.density}</span>
                  <span className="text-forge-text-dim">E: {mat.youngsModulus} GPa</span>
                  <span className="text-forge-text-dim">σy: {mat.yieldStrength} MPa</span>
                </div>
              </button>
            ))}
          </div>
        </div>

        {/* Material Detail */}
        {selected && (
          <div className="flex-1 overflow-y-auto p-4">
            <div className="flex items-center justify-between mb-4">
              <div>
                <h2 className="text-lg font-bold text-forge-text flex items-center gap-2">
                  {selected.name}
                  {selected.favorite && <Star size={14} className="text-forge-yellow fill-forge-yellow" />}
                </h2>
                <span className="text-[10px] font-mono text-forge-purple">{selected.category}</span>
              </div>
              <div className="flex gap-2">
                <button className="px-3 py-1.5 rounded text-[9px] font-mono bg-forge-green/10 text-forge-green border border-forge-green/20 hover:bg-forge-green/20">
                  APPLY TO SELECTION
                </button>
              </div>
            </div>

            {/* Property Cards */}
            <div className="grid grid-cols-3 gap-3 mb-4">
              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Shield size={12} className="text-forge-accent" />
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">MECHANICAL</span>
                </div>
                <div className="space-y-1.5">
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Young's Modulus</span>
                    <span className="text-[9px] font-mono text-forge-text">{selected.youngsModulus} GPa</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Yield Strength</span>
                    <span className="text-[9px] font-mono text-forge-text">{selected.yieldStrength} MPa</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Poisson's Ratio</span>
                    <span className="text-[9px] font-mono text-forge-text">0.342</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Shear Modulus</span>
                    <span className="text-[9px] font-mono text-forge-text">42.4 GPa</span>
                  </div>
                </div>
              </div>

              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Thermometer size={12} className="text-forge-orange" />
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">THERMAL</span>
                </div>
                <div className="space-y-1.5">
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Conductivity</span>
                    <span className="text-[9px] font-mono text-forge-text">{selected.thermalCond} W/m·K</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">CTE</span>
                    <span className="text-[9px] font-mono text-forge-text">{selected.cte} μm/m·°C</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Specific Heat</span>
                    <span className="text-[9px] font-mono text-forge-text">526 J/kg·K</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Max Temp</span>
                    <span className="text-[9px] font-mono text-forge-text">400 °C</span>
                  </div>
                </div>
              </div>

              <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
                <div className="flex items-center gap-1.5 mb-2">
                  <Zap size={12} className="text-forge-yellow" />
                  <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">PHYSICAL</span>
                </div>
                <div className="space-y-1.5">
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Density</span>
                    <span className="text-[9px] font-mono text-forge-text">{selected.density} kg/m³</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Melting Point</span>
                    <span className="text-[9px] font-mono text-forge-text">1660 °C</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Electrical Res.</span>
                    <span className="text-[9px] font-mono text-forge-text">1.7 μΩ·m</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-[9px] text-forge-text-dim">Hardness</span>
                    <span className="text-[9px] font-mono text-forge-text">36 HRC</span>
                  </div>
                </div>
              </div>
            </div>

            {/* Fatigue Data */}
            <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/20 mb-4">
              <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">S-N CURVE DATA</span>
              <div className="mt-3 h-32 flex items-end gap-1">
                {[95, 88, 78, 70, 62, 55, 50, 46, 43, 41, 40, 39, 38.5, 38, 37.8].map((h, i) => (
                  <div
                    key={i}
                    className="flex-1 bg-gradient-to-t from-forge-purple/60 to-forge-accent/40 rounded-t transition-all hover:from-forge-purple hover:to-forge-accent"
                    style={{ height: `${h}%` }}
                  />
                ))}
              </div>
              <div className="flex justify-between mt-2 text-[7px] font-mono text-forge-text-muted">
                <span>10³</span>
                <span>10⁴</span>
                <span>10⁵</span>
                <span>10⁶</span>
                <span>10⁷ cycles</span>
              </div>
            </div>

            {/* Applied Parts */}
            <div className="p-4 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-3">
                <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">APPLIED TO</span>
                <span className="text-[8px] font-mono text-forge-accent">3 parts</span>
              </div>
              <div className="space-y-1">
                {['Primary Blade', 'Blade Root Attachment', 'Hub Center Ring'].map((part, i) => (
                  <div key={i} className="flex items-center gap-2 px-2 py-1.5 rounded bg-forge-dark/50">
                    <ChevronRight size={8} className="text-forge-text-muted" />
                    <span className="text-[9px] text-forge-text">{part}</span>
                  </div>
                ))}
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};
