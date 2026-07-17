import React, { useState, useEffect } from 'react';
import { Hexagon } from 'lucide-react';

interface Props {
  onComplete: () => void;
}

const bootMessages = [
  'Initializing SZM Forge Kernel...',
  'Loading GPU Compute Pipeline...',
  'Establishing Solver Connections...',
  'Mounting Engineering Modules...',
  'Synchronizing Digital Twin Network...',
  'Deploying AI Agents...',
  'Calibrating Viewport Renderer...',
  'Connecting Fleet Telemetry...',
  'System Ready.',
];

export const LoadingScreen: React.FC<Props> = ({ onComplete }) => {
  const [progress, setProgress] = useState(0);
  const [currentMsg, setCurrentMsg] = useState(0);
  const [opacity, setOpacity] = useState(1);

  useEffect(() => {
    const interval = setInterval(() => {
      setProgress(prev => {
        const next = prev + 2 + Math.random() * 3;
        if (next >= 100) {
          clearInterval(interval);
          setTimeout(() => {
            setOpacity(0);
            setTimeout(onComplete, 500);
          }, 400);
          return 100;
        }
        return next;
      });
    }, 60);

    return () => clearInterval(interval);
  }, [onComplete]);

  useEffect(() => {
    const msgIndex = Math.min(Math.floor((progress / 100) * bootMessages.length), bootMessages.length - 1);
    setCurrentMsg(msgIndex);
  }, [progress]);

  return (
    <div
      className="fixed inset-0 z-[9999] bg-forge-black flex flex-col items-center justify-center transition-opacity duration-500"
      style={{ opacity }}
    >
      {/* Animated hex background */}
      <div className="absolute inset-0 overflow-hidden opacity-10">
        {Array.from({ length: 20 }).map((_, i) => (
          <div
            key={i}
            className="absolute border border-forge-accent/20 rounded-lg"
            style={{
              width: `${60 + Math.random() * 100}px`,
              height: `${60 + Math.random() * 100}px`,
              left: `${Math.random() * 100}%`,
              top: `${Math.random() * 100}%`,
              transform: `rotate(${Math.random() * 45}deg)`,
              animation: `fadeIn ${1 + Math.random() * 2}s ease-out forwards`,
              animationDelay: `${Math.random() * 1.5}s`,
              opacity: 0,
            }}
          />
        ))}
      </div>

      {/* Logo */}
      <div className="relative mb-8">
        <div className="relative">
          <Hexagon size={64} className="text-forge-accent" strokeWidth={1.5} />
          <div className="absolute inset-0 flex items-center justify-center">
            <Hexagon size={32} className="text-forge-accent/40" strokeWidth={1} />
          </div>
          <div className="absolute inset-0 flex items-center justify-center">
            <div className="w-3 h-3 bg-forge-accent rounded-full animate-pulse" />
          </div>
        </div>
        {/* Glow ring */}
        <div
          className="absolute inset-0 rounded-full"
          style={{
            boxShadow: '0 0 40px rgba(0, 212, 255, 0.2), 0 0 80px rgba(0, 212, 255, 0.1)',
          }}
        />
      </div>

      {/* Title */}
      <h1 className="text-2xl font-bold tracking-[0.4em] text-forge-accent text-glow-accent mb-1">
        SZM FORGE
      </h1>
      <p className="text-[10px] tracking-[0.3em] text-forge-text-muted font-mono mb-8">
        INDUSTRIAL ENGINEERING INTELLIGENCE PLATFORM
      </p>

      {/* Progress bar */}
      <div className="w-80 mb-4">
        <div className="w-full h-[2px] bg-forge-surface rounded-full overflow-hidden">
          <div
            className="h-full bg-gradient-to-r from-forge-accent via-forge-cyan to-forge-accent rounded-full transition-all duration-100"
            style={{ width: `${Math.min(progress, 100)}%` }}
          />
        </div>
      </div>

      {/* Boot messages */}
      <div className="h-5">
        <p className="text-[9px] font-mono text-forge-accent/70 tracking-wider animate-fade-in" key={currentMsg}>
          {bootMessages[currentMsg]}
        </p>
      </div>

      {/* Progress percentage */}
      <p className="text-[11px] font-mono text-forge-text-muted mt-2">
        {Math.min(Math.floor(progress), 100)}%
      </p>

      {/* Version info */}
      <div className="absolute bottom-6 flex items-center gap-4 text-[8px] font-mono text-forge-text-muted/50">
        <span>v4.2.1-alpha</span>
        <span>•</span>
        <span>WebGPU Runtime</span>
        <span>•</span>
        <span>Build 20260115</span>
        <span>•</span>
        <span>© SZM Systems</span>
      </div>
    </div>
  );
};
