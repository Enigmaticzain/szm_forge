import React from 'react';
import {
  ArrowRight,
  Brain,
  Cpu,
  Factory,
  Layers,
  PlayCircle,
  ShieldCheck,
  Sparkles,
  Workflow,
} from 'lucide-react';

const focusAreas = [
  {
    title: 'Design review',
    description: 'Check geometry readiness and resolve high-risk assembly conflicts.',
    accent: 'text-forge-accent',
  },
  {
    title: 'Simulation sweep',
    description: 'Run stress, thermal, and optimization studies before release.',
    accent: 'text-forge-green',
  },
  {
    title: 'Manufacturing handoff',
    description: 'Align DFM checks, tooling assumptions, and inspection readiness.',
    accent: 'text-forge-purple',
  },
];

const memorySignals = [
  'Current project context is synchronized with the active assembly and CAD intent.',
  'SZM AI is tracking design decisions, materials, and solver insights for fast recall.',
  'Recent outputs are available for review, reuse, and downstream engineering handoff.',
];

const healthMetrics = [
  { label: 'Model readiness', value: '96%', tone: 'text-forge-green' },
  { label: 'AI confidence', value: '92%', tone: 'text-forge-accent' },
  { label: 'Production readiness', value: '88%', tone: 'text-forge-purple' },
];

export const CommandCenterWorkspace: React.FC = () => {
  return (
    <div className="h-full w-full overflow-auto bg-[radial-gradient(circle_at_top_left,rgba(0,212,255,0.09),transparent_30%),linear-gradient(180deg,rgba(10,10,15,0.96),rgba(7,8,12,1))] text-forge-text">
      <div className="mx-auto flex max-w-7xl flex-col gap-6 px-6 py-6 lg:px-8">
        <header className="rounded-2xl border border-forge-border/70 bg-forge-panel/80 p-6 shadow-[0_20px_80px_rgba(0,0,0,0.25)]">
          <div className="flex flex-col gap-4 lg:flex-row lg:items-end lg:justify-between">
            <div className="max-w-2xl">
              <div className="mb-3 inline-flex items-center gap-2 rounded-full border border-forge-accent/30 bg-forge-accent/10 px-3 py-1 text-[10px] font-semibold uppercase tracking-[0.28em] text-forge-accent">
                <Sparkles size={12} />
                SZM AI operating system
              </div>
              <h1 className="text-3xl font-semibold tracking-tight text-forge-text">Engineering Command Center</h1>
              <p className="mt-3 text-sm leading-7 text-forge-text-dim">
                Coordinate design, simulation, manufacturing, and AI reasoning from a single professional workspace.
              </p>
            </div>
            <div className="flex items-center gap-2 rounded-full border border-forge-border/70 bg-forge-surface/70 px-3 py-2 text-[11px] font-mono uppercase tracking-[0.24em] text-forge-text-muted">
              <ShieldCheck size={14} className="text-forge-green" />
              Secure execution mode
            </div>
          </div>
        </header>

        <section className="grid gap-4 lg:grid-cols-[1.3fr_0.7fr]">
          <div className="rounded-2xl border border-forge-border/70 bg-forge-panel/75 p-5">
            <div className="flex items-center justify-between">
              <div>
                <p className="text-[10px] font-semibold uppercase tracking-[0.3em] text-forge-text-muted">Launch critical workflows</p>
                <h2 className="mt-2 text-lg font-semibold text-forge-text">Operational focus</h2>
              </div>
              <div className="rounded-full border border-forge-accent/20 bg-forge-accent/10 p-2 text-forge-accent">
                <Workflow size={16} />
              </div>
            </div>
            <div className="mt-4 grid gap-3 md:grid-cols-3">
              {focusAreas.map(area => (
                <button
                  key={area.title}
                  type="button"
                  className="flex flex-col items-start rounded-xl border border-forge-border/60 bg-forge-surface/70 p-3 text-left transition-all duration-200 hover:-translate-y-0.5 hover:border-forge-accent/30"
                >
                  <div className={`mb-2 flex items-center gap-2 text-sm font-semibold ${area.accent}`}>
                    <PlayCircle size={14} />
                    {area.title}
                  </div>
                  <p className="text-[12px] leading-6 text-forge-text-dim">{area.description}</p>
                </button>
              ))}
            </div>
          </div>

          <div className="rounded-2xl border border-forge-border/70 bg-forge-panel/75 p-5">
            <div className="flex items-center gap-2">
              <Cpu size={16} className="text-forge-accent" />
              <p className="text-[10px] font-semibold uppercase tracking-[0.3em] text-forge-text-muted">System health</p>
            </div>
            <div className="mt-4 space-y-3">
              {healthMetrics.map(metric => (
                <div key={metric.label} className="rounded-xl border border-forge-border/60 bg-forge-surface/70 px-3 py-3">
                  <div className="flex items-center justify-between">
                    <span className="text-[11px] uppercase tracking-[0.24em] text-forge-text-muted">{metric.label}</span>
                    <span className={`text-sm font-semibold ${metric.tone}`}>{metric.value}</span>
                  </div>
                </div>
              ))}
            </div>
          </div>
        </section>

        <section className="grid gap-4 xl:grid-cols-[1.05fr_0.95fr]">
          <div className="rounded-2xl border border-forge-border/70 bg-forge-panel/75 p-5">
            <div className="flex items-center gap-2">
              <Brain size={16} className="text-forge-purple" />
              <p className="text-[10px] font-semibold uppercase tracking-[0.3em] text-forge-text-muted">SZM AI memory</p>
            </div>
            <div className="mt-4 space-y-3">
              {memorySignals.map(signal => (
                <div key={signal} className="flex items-start gap-3 rounded-xl border border-forge-border/60 bg-forge-surface/70 px-3 py-3">
                  <div className="mt-1 h-2 w-2 flex-shrink-0 rounded-full bg-forge-accent" />
                  <p className="text-sm leading-6 text-forge-text-dim">{signal}</p>
                </div>
              ))}
            </div>
          </div>

          <div className="rounded-2xl border border-forge-border/70 bg-forge-panel/75 p-5">
            <div className="flex items-center gap-2">
              <Layers size={16} className="text-forge-green" />
              <p className="text-[10px] font-semibold uppercase tracking-[0.3em] text-forge-text-muted">Execution stream</p>
            </div>
            <div className="mt-4 space-y-3">
              {[ 
                { title: 'Design synthesis', detail: 'AI context and geometry review are aligned for the next iteration.' },
                { title: 'Manufacturing readiness', detail: 'Inspection checkpoints and tolerances are prepared for release.' },
                { title: 'Knowledge handoff', detail: 'Reports and decision records are ready for the next engineering team.' },
              ].map(item => (
                <div key={item.title} className="rounded-xl border border-forge-border/60 bg-forge-surface/70 p-3">
                  <div className="flex items-center justify-between">
                    <span className="text-sm font-semibold text-forge-text">{item.title}</span>
                    <ArrowRight size={14} className="text-forge-text-muted" />
                  </div>
                  <p className="mt-2 text-sm leading-6 text-forge-text-dim">{item.detail}</p>
                </div>
              ))}
            </div>
          </div>
        </section>
      </div>
    </div>
  );
};
