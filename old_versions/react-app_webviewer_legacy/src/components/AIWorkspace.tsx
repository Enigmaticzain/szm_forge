import React, { useState, useEffect } from 'react';
import { aiInsights } from '../data/mockData';
import {
  Brain, Cpu, Activity, Target, Lightbulb,
  AlertTriangle, CheckCircle, Info, XCircle,
  TrendingUp, Gauge, Shield,
  Zap, ArrowRight, ChevronRight
} from 'lucide-react';

const severityIcons = {
  info: <Info size={12} className="text-forge-accent" />,
  success: <CheckCircle size={12} className="text-forge-green" />,
  warning: <AlertTriangle size={12} className="text-forge-yellow" />,
  critical: <XCircle size={12} className="text-forge-red" />,
};

const typeLabels = {
  optimization: { label: 'OPTIMIZATION', color: 'text-forge-purple', bg: 'bg-forge-purple/10' },
  warning: { label: 'WARNING', color: 'text-forge-yellow', bg: 'bg-forge-yellow/10' },
  recommendation: { label: 'RECOMMENDATION', color: 'text-forge-accent', bg: 'bg-forge-accent/10' },
  diagnostic: { label: 'DIAGNOSTIC', color: 'text-forge-teal', bg: 'bg-forge-teal/10' },
};

const optimizationTargets = [
  { name: 'Mass Reduction', current: '12.4%', target: '15%', progress: 83, trend: 'up' },
  { name: 'Stress Distribution', current: '847 MPa', target: '<800 MPa', progress: 65, trend: 'improving' },
  { name: 'Thermal Efficiency', current: '94.2%', target: '96%', progress: 78, trend: 'up' },
  { name: 'Fatigue Life', current: '1.2M', target: '2M cycles', progress: 60, trend: 'working' },
  { name: 'Manufacturing Cost', current: '$47.2K', target: '<$45K', progress: 72, trend: 'improving' },
];

const agentStatus = [
  { name: 'Topology Optimizer', status: 'active', task: 'Iteration 12/20 — Hub geometry', cpu: 34, confidence: 89 },
  { name: 'Failure Predictor', status: 'active', task: 'Scanning blade root stress concentration', cpu: 12, confidence: 94 },
  { name: 'Material Selector', status: 'processing', task: 'Evaluating IN718 alternatives', cpu: 8, confidence: 76 },
  { name: 'Mesh Advisor', status: 'idle', task: 'Awaiting next simulation', cpu: 0, confidence: 91 },
  { name: 'Thermal Analyst', status: 'active', task: 'Gradient mapping on blade array', cpu: 22, confidence: 88 },
];

export const AIWorkspace: React.FC = () => {
  const [activeAgent, setActiveAgent] = useState(0);
  const [pulseVals, setPulseVals] = useState<number[]>([]);

  useEffect(() => {
    const interval = setInterval(() => {
      setPulseVals(prev => {
        const next = [...prev, Math.random() * 100];
        if (next.length > 40) next.shift();
        return next;
      });
    }, 200);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="h-full flex flex-col bg-forge-black overflow-hidden">
      {/* Header */}
      <div className="h-[56px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-6 flex-shrink-0">
        <div className="flex items-center gap-2">
          <Brain size={14} className="text-forge-purple" />
          <div>
            <div className="text-[10px] font-bold tracking-widest text-forge-purple">AI ENGINEERING INTELLIGENCE</div>
            <div className="text-[8px] font-mono text-forge-text-muted">AUTONOMOUS AGENTS • OPTIMIZATION • DIAGNOSTICS • PREDICTIONS</div>
          </div>
        </div>
        <div className="ml-auto flex items-center gap-4">
          <div className="flex items-center gap-1.5">
            <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse" />
            <span className="text-[9px] font-mono text-forge-text-dim">5 AGENTS DEPLOYED</span>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">AI CPU</div>
            <div className="text-[14px] font-bold font-mono text-forge-purple">76%</div>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">INSIGHTS</div>
            <div className="text-[14px] font-bold font-mono text-forge-accent">{aiInsights.length}</div>
          </div>
        </div>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Left - Agent Status */}
        <div className="w-[320px] border-r border-forge-border bg-forge-panel overflow-y-auto p-3">
          <div className="flex items-center gap-1.5 mb-3">
            <Cpu size={11} className="text-forge-purple" />
            <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">AI AGENTS</span>
          </div>

          <div className="space-y-2 mb-4">
            {agentStatus.map((agent, i) => (
              <button
                key={i}
                onClick={() => setActiveAgent(i)}
                className={`w-full p-3 rounded-lg border text-left transition-all ${
                  activeAgent === i
                    ? 'border-forge-purple/40 bg-forge-purple/5'
                    : 'border-forge-border/50 bg-forge-surface/20 hover:border-forge-purple/20'
                }`}
              >
                <div className="flex items-center justify-between mb-1.5">
                  <div className="flex items-center gap-1.5">
                    <div className={`w-1.5 h-1.5 rounded-full ${
                      agent.status === 'active' ? 'bg-forge-green animate-pulse' :
                      agent.status === 'processing' ? 'bg-forge-accent animate-pulse' :
                      'bg-forge-text-muted'
                    }`} />
                    <span className="text-[10px] font-semibold text-forge-text">{agent.name}</span>
                  </div>
                  <span className="text-[7px] font-mono text-forge-text-muted">{agent.status.toUpperCase()}</span>
                </div>
                <p className="text-[8px] text-forge-text-muted mb-2">{agent.task}</p>
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <span className="text-[7px] font-mono text-forge-text-muted">CPU {agent.cpu}%</span>
                    <span className="text-[7px] font-mono text-forge-purple">CONF {agent.confidence}%</span>
                  </div>
                  <ChevronRight size={10} className="text-forge-text-muted" />
                </div>
              </button>
            ))}
          </div>

          {/* Neural Activity */}
          <div className="p-3 rounded-lg border border-forge-purple/20 bg-forge-purple/5">
            <div className="flex items-center gap-1.5 mb-2">
              <Activity size={10} className="text-forge-purple" />
              <span className="text-[9px] font-semibold text-forge-text-dim">NEURAL ACTIVITY</span>
            </div>
            <div className="h-12 flex items-end gap-px">
              {pulseVals.map((v, i) => (
                <div
                  key={i}
                  className="flex-1 bg-forge-purple/60 rounded-t transition-all duration-100"
                  style={{ height: `${v}%` }}
                />
              ))}
            </div>
          </div>
        </div>

        {/* Center - Insights & Optimization */}
        <div className="flex-1 overflow-y-auto p-4 space-y-4">
          {/* Optimization Targets */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-2 mb-4">
              <Target size={12} className="text-forge-accent" />
              <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">OPTIMIZATION TARGETS</span>
            </div>
            <div className="space-y-3">
              {optimizationTargets.map((target, i) => (
                <div key={i}>
                  <div className="flex items-center justify-between mb-1">
                    <span className="text-[10px] font-medium text-forge-text">{target.name}</span>
                    <div className="flex items-center gap-3">
                      <span className="text-[9px] font-mono text-forge-text-dim">{target.current}</span>
                      <ArrowRight size={8} className="text-forge-text-muted" />
                      <span className="text-[9px] font-mono text-forge-accent">{target.target}</span>
                    </div>
                  </div>
                  <div className="flex items-center gap-2">
                    <div className="flex-1 h-1.5 bg-forge-dark rounded-full overflow-hidden">
                      <div
                        className="h-full bg-gradient-to-r from-forge-purple to-forge-accent rounded-full transition-all"
                        style={{ width: `${target.progress}%` }}
                      />
                    </div>
                    <span className="text-[8px] font-mono text-forge-text-muted w-8 text-right">{target.progress}%</span>
                  </div>
                </div>
              ))}
            </div>
          </div>

          {/* Active Insights */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-2 mb-4">
              <Lightbulb size={12} className="text-forge-yellow" />
              <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">AI INSIGHTS & ACTIONS</span>
              <span className="text-[8px] font-mono text-forge-text-muted ml-auto">{aiInsights.length} active</span>
            </div>
            <div className="space-y-2">
              {aiInsights.map(insight => {
                const tl = typeLabels[insight.type];
                return (
                  <div
                    key={insight.id}
                    className="p-3 rounded border border-forge-border/50 bg-forge-surface/20 hover:bg-forge-surface/40 transition-all"
                  >
                    <div className="flex items-start gap-2">
                      {severityIcons[insight.severity]}
                      <div className="flex-1">
                        <div className="flex items-center gap-2 mb-1">
                          <span className="text-[10px] font-semibold text-forge-text">{insight.title}</span>
                          <span className={`text-[7px] font-mono tracking-wider px-1.5 py-0.5 rounded ${tl.bg} ${tl.color}`}>
                            {tl.label}
                          </span>
                        </div>
                        <p className="text-[9px] text-forge-text-dim leading-relaxed mb-2">{insight.description}</p>
                        <div className="flex items-center gap-4">
                          <span className="text-[8px] font-mono text-forge-text-muted">{insight.timestamp}</span>
                          <span className="text-[8px] font-mono text-forge-purple">Confidence: {insight.confidence}%</span>
                          <div className="flex gap-1 ml-auto">
                            <button className="px-2 py-0.5 rounded text-[8px] font-mono bg-forge-accent/10 text-forge-accent border border-forge-accent/20 hover:bg-forge-accent/20 transition-all">
                              APPLY
                            </button>
                            <button className="px-2 py-0.5 rounded text-[8px] font-mono text-forge-text-muted hover:bg-forge-surface transition-all">
                              EXPLORE
                            </button>
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>

          {/* AI Performance Grid */}
          <div className="grid grid-cols-4 gap-3">
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">INFERENCES</span>
                <Zap size={14} className="text-forge-accent" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-accent">14,847</div>
            </div>
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">OPTIMIZATIONS</span>
                <TrendingUp size={14} className="text-forge-purple" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-purple">342</div>
            </div>
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">WARNINGS CAUGHT</span>
                <Shield size={14} className="text-forge-yellow" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-yellow">28</div>
            </div>
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">ACCURACY</span>
                <Gauge size={14} className="text-forge-green" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-green">96.7%</div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
