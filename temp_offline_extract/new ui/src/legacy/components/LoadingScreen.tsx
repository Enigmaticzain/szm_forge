import { useState, useEffect } from 'react';

interface LoadingScreenProps {
  onComplete: () => void;
}

export default function LoadingScreen({ onComplete }: LoadingScreenProps) {
  const [progress, setProgress] = useState(0);
  const [phase, setPhase] = useState('INITIALIZING RENDER ENGINE');

  const phases = [
    { at: 0, text: 'INITIALIZING RENDER ENGINE' },
    { at: 15, text: 'LOADING GPU PIPELINE' },
    { at: 30, text: 'COMPILING SHADER PROGRAMS' },
    { at: 45, text: 'LOADING SIMULATION DATA' },
    { at: 60, text: 'BUILDING MESH TOPOLOGY' },
    { at: 75, text: 'CONNECTING TELEMETRY STREAMS' },
    { at: 85, text: 'INITIALIZING AI MODULES' },
    { at: 95, text: 'READY' },
  ];

  useEffect(() => {
    const interval = setInterval(() => {
      setProgress(prev => {
        const next = prev + (Math.random() * 3 + 1);
        if (next >= 100) {
          clearInterval(interval);
          setTimeout(onComplete, 500);
          return 100;
        }
        return next;
      });
    }, 50);
    return () => clearInterval(interval);
  }, [onComplete]);

  useEffect(() => {
    const currentPhase = [...phases].reverse().find(p => progress >= p.at);
    if (currentPhase) setPhase(currentPhase.text);
  }, [progress]);

  return (
    <div className="fixed inset-0 bg-forge-bg z-[100] flex items-center justify-center">
      {/* Background grid */}
      <div className="absolute inset-0 grid-overlay opacity-30" />
      
      {/* Center content */}
      <div className="relative z-10 text-center space-y-8">
        {/* Logo */}
        <div className="flex items-center justify-center gap-4">
          <div className="w-16 h-16 rounded-xl bg-gradient-to-br from-forge-accent via-forge-purple to-forge-accent flex items-center justify-center relative overflow-hidden">
            <span className="text-xl font-black text-white tracking-tighter relative z-10">SZM</span>
            <div className="absolute inset-0 bg-gradient-to-br from-transparent to-black/30" />
            <div className="absolute inset-0 animate-rotate-slow opacity-20">
              <div className="absolute inset-2 border border-white/30 rounded-lg" />
              <div className="absolute inset-4 border border-white/20 rounded" />
            </div>
          </div>
          <div className="text-left">
            <h1 className="text-3xl font-black text-forge-text tracking-wider">
              SZM <span className="text-forge-accent">FORGE</span>
            </h1>
            <p className="text-[10px] text-forge-text-dim tracking-[0.3em] uppercase">
              Industrial Intelligence Platform
            </p>
          </div>
        </div>

        {/* Progress bar */}
        <div className="w-80 mx-auto">
          <div className="h-1 bg-forge-panel rounded-full overflow-hidden">
            <div 
              className="h-full rounded-full transition-all duration-100"
              style={{ 
                width: `${progress}%`,
                background: 'linear-gradient(90deg, #00c2ff, #8855ff, #00ff88)',
              }}
            />
          </div>
          <div className="flex justify-between mt-2">
            <span className="text-[9px] text-forge-accent tracking-wider animate-pulse-glow">
              {phase}
            </span>
            <span className="text-[9px] text-forge-text-dim font-mono">
              {progress.toFixed(0)}%
            </span>
          </div>
        </div>

        {/* System info */}
        <div className="flex items-center justify-center gap-6 text-[8px] text-forge-text-muted tracking-wider">
          <span>ENGINE v2.4.0</span>
          <span className="text-forge-border">|</span>
          <span>WebGPU RENDERER</span>
          <span className="text-forge-border">|</span>
          <span>64-BIT PRECISION</span>
        </div>
      </div>

      {/* Corner decorations */}
      {['top-4 left-4', 'top-4 right-4', 'bottom-4 left-4', 'bottom-4 right-4'].map((pos, i) => (
        <div key={i} className={`absolute ${pos}`}>
          <div className={`w-8 h-8 ${
            i === 0 ? 'border-t border-l' :
            i === 1 ? 'border-t border-r' :
            i === 2 ? 'border-b border-l' :
            'border-b border-r'
          } border-forge-border/40`} />
        </div>
      ))}
    </div>
  );
}
