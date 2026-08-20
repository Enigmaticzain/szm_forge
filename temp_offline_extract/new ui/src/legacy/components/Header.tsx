import { useState, useEffect } from 'react';
import { useTheme } from '../../store/ThemeContext';
import { useTooltips } from '../../store/TooltipContext';
import { tooltips } from '../../data/tooltips';

interface HeaderProps {
  activeView: string;
  onViewChange: (view: string) => void;
}

const views: { id: string; label: string; icon: string; tipKey: keyof typeof tooltips.legacy }[] = [
  { id: 'overview', label: 'COMMAND CENTER', icon: '⬡', tipKey: 'overview' },
  { id: 'stress', label: 'FEA ANALYSIS', icon: '◆', tipKey: 'stress' },
  { id: 'thermal', label: 'THERMAL', icon: '◈', tipKey: 'thermal' },
  { id: 'electrical', label: 'ELECTRICAL', icon: '⚡', tipKey: 'electrical' },
  { id: 'digital-twin', label: 'DIGITAL TWIN', icon: '◎', tipKey: 'digitalTwin' },
  { id: 'manufacturing', label: 'MANUFACTURING', icon: '⚙', tipKey: 'manufacturing' },
];

export default function Header({ activeView, onViewChange }: HeaderProps) {
  const [time, setTime] = useState(new Date());
  const [gpuLoad, setGpuLoad] = useState(67);
  const { setTheme } = useTheme();
  const { t } = useTooltips();

  useEffect(() => {
    const t = setInterval(() => {
      setTime(new Date());
      setGpuLoad(prev => Math.max(40, Math.min(95, prev + (Math.random() - 0.5) * 5)));
    }, 1000);
    return () => clearInterval(t);
  }, []);

  return (
    <header className="h-12 bg-forge-surface border-b border-forge-border flex items-center px-4 gap-4 relative z-50">
      {/* Logo */}
      <div className="flex items-center gap-2.5 mr-4">
        <div className="w-7 h-7 rounded-md bg-gradient-to-br from-forge-accent to-forge-purple flex items-center justify-center relative">
          <span className="text-[10px] font-black text-white tracking-tighter">SZM</span>
          <div className="absolute inset-0 rounded-md bg-forge-accent/20 animate-pulse-glow" />
        </div>
        <div className="flex flex-col leading-none">
          <span className="text-[11px] font-bold text-forge-text tracking-wider">FORGE</span>
          <span className="text-[7px] text-forge-text-dim tracking-[0.2em] uppercase">Industrial Intelligence</span>
        </div>
      </div>

      {/* Separator */}
      <div className="w-px h-6 bg-forge-border" />

      {/* Navigation */}
      <nav className="flex items-center gap-0.5 flex-1">
        {views.map(view => (
          <button
            key={view.id}
            onClick={() => onViewChange(view.id)}
            title={t(tooltips.legacy[view.tipKey])}
            aria-label={t(tooltips.legacy[view.tipKey])}
            className={`px-3 py-1.5 text-[10px] font-semibold tracking-wider rounded transition-all duration-200 flex items-center gap-1.5 ${
              activeView === view.id
                ? 'bg-forge-accent/10 text-forge-accent border border-forge-accent/30'
                : 'text-forge-text-dim hover:text-forge-text hover:bg-forge-panel border border-transparent'
            }`}
          >
            <span className="text-xs">{view.icon}</span>
            {view.label}
          </button>
        ))}
      </nav>

      {/* Status Bar */}
      <div className="flex items-center gap-4 text-[9px] text-forge-text-dim tracking-wider">
        <div className="flex items-center gap-1.5">
          <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse-glow" />
          <span>SIMULATION ACTIVE</span>
        </div>
        <div className="flex items-center gap-1.5">
          <span className="text-forge-text-muted">GPU</span>
          <div className="w-16 h-1.5 bg-forge-panel rounded-full overflow-hidden">
            <div 
              className="h-full rounded-full transition-all duration-500"
              style={{ 
                width: `${gpuLoad}%`,
                background: gpuLoad > 80 ? '#ff3344' : gpuLoad > 60 ? '#ffaa00' : '#00ff88'
              }}
            />
          </div>
          <span className="text-forge-text w-7 text-right">{gpuLoad.toFixed(0)}%</span>
        </div>
        <div className="text-forge-text-dim font-mono">
          {time.toLocaleTimeString('en-US', { hour12: false })}
        </div>
        <button
          onClick={() => setTheme('theme2')}
          title="Switch to Modern Theme (Theme 2)"
          aria-label="Switch to Modern Theme"
          className="ml-2 px-2 py-1 rounded text-[9px] font-semibold tracking-wider text-forge-text-dim hover:text-forge-accent hover:bg-forge-accent/10 border border-transparent hover:border-forge-accent/30 transition-all"
        >
          ⚙ THEME
        </button>
      </div>
    </header>
  );
}
