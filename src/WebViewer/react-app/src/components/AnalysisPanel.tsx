import React, { useState } from 'react';
import { telemetryData, simulationResults, aiInsights, solverMetrics } from '../data/mockData';
import {
  Activity, TrendingUp, TrendingDown, Minus,
  AlertTriangle, CheckCircle, Info, XCircle,
  Brain, ChevronDown, ChevronRight, Cpu,
  BarChart3, Gauge, Lightbulb, Shield
} from 'lucide-react';

type PanelTab = 'results' | 'telemetry' | 'ai' | 'solver';

const severityIcons = {
  info: <Info size={11} className="text-forge-accent" />,
  success: <CheckCircle size={11} className="text-forge-green" />,
  warning: <AlertTriangle size={11} className="text-forge-yellow" />,
  critical: <XCircle size={11} className="text-forge-red" />,
};

const severityColors = {
  info: 'border-forge-accent/20 bg-forge-accent/5',
  success: 'border-forge-green/20 bg-forge-green/5',
  warning: 'border-forge-yellow/20 bg-forge-yellow/5',
  critical: 'border-forge-red/20 bg-forge-red/5',
};

const trendIcons = {
  up: <TrendingUp size={10} className="text-forge-red" />,
  down: <TrendingDown size={10} className="text-forge-yellow" />,
  stable: <Minus size={10} className="text-forge-green" />,
};

const MiniSparkline: React.FC<{ data: number[]; color: string }> = ({ data, color }) => {
  const max = Math.max(...data);
  const min = Math.min(...data);
  const range = max - min || 1;
  const w = 48;
  const h = 16;
  const points = data.map((v, i) => `${(i / (data.length - 1)) * w},${h - ((v - min) / range) * h}`).join(' ');

  return (
    <svg width={w} height={h} className="flex-shrink-0">
      <polyline points={points} fill="none" stroke={color} strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round" />
    </svg>
  );
};

export const AnalysisPanel: React.FC = () => {
  const [activeTab, setActiveTab] = useState<PanelTab>('results');
  const [expandedInsight, setExpandedInsight] = useState<string | null>('ai-1');

  const tabs: { id: PanelTab; label: string; icon: React.ReactNode }[] = [
    { id: 'results', label: 'RESULTS', icon: <BarChart3 size={11} /> },
    { id: 'telemetry', label: 'TELEMETRY', icon: <Activity size={11} /> },
    { id: 'ai', label: 'AI', icon: <Brain size={11} /> },
    { id: 'solver', label: 'SOLVER', icon: <Cpu size={11} /> },
  ];

  return (
    <div className="h-full flex flex-col bg-forge-panel border-l border-forge-border">
      {/* Header */}
      <div className="px-3 py-2.5 border-b border-forge-border flex items-center gap-1.5">
        <Gauge size={12} className="text-forge-accent" />
        <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">
          ANALYSIS ENGINE
        </span>
      </div>

      {/* Tabs */}
      <div className="flex border-b border-forge-border">
        {tabs.map(tab => (
          <button
            key={tab.id}
            onClick={() => setActiveTab(tab.id)}
            className={`
              flex-1 flex items-center justify-center gap-1 py-2 text-[9px] font-semibold tracking-wider
              transition-all duration-200 border-b-2
              ${activeTab === tab.id
                ? 'text-forge-accent border-forge-accent bg-forge-accent/5'
                : 'text-forge-text-muted border-transparent hover:text-forge-text-dim hover:bg-forge-surface/50'
              }
            `}
          >
            {tab.icon}
            {tab.label}
          </button>
        ))}
      </div>

      {/* Content */}
      <div className="flex-1 overflow-y-auto">
        {activeTab === 'results' && (
          <div className="p-2 space-y-1">
            {simulationResults.map((result, i) => (
              <div
                key={i}
                className={`p-2 rounded border ${severityColors[result.status]} transition-all`}
              >
                <div className="flex items-center justify-between mb-0.5">
                  <div className="flex items-center gap-1.5">
                    {severityIcons[result.status]}
                    <span className="text-[9px] font-medium text-forge-text-dim">{result.metric}</span>
                  </div>
                </div>
                <div className="flex items-baseline justify-between">
                  <span className="text-[13px] font-bold font-mono text-forge-text">{result.value}</span>
                </div>
                {result.detail && (
                  <span className="text-[8px] text-forge-text-muted mt-0.5 block">{result.detail}</span>
                )}
              </div>
            ))}
          </div>
        )}

        {activeTab === 'telemetry' && (
          <div className="p-2 space-y-1">
            {telemetryData.map(item => {
              const sparkColor = item.status === 'critical' ? '#ff3355' :
                item.status === 'warning' ? '#ffcc00' :
                item.status === 'success' ? '#00ff88' : '#00d4ff';
              return (
                <div
                  key={item.id}
                  className="p-2 rounded border border-forge-border/50 bg-forge-surface/30 hover:bg-forge-surface/60 transition-all"
                >
                  <div className="flex items-center justify-between mb-1">
                    <span className="text-[9px] text-forge-text-dim">{item.label}</span>
                    {trendIcons[item.trend]}
                  </div>
                  <div className="flex items-center justify-between">
                    <div className="flex items-baseline gap-1">
                      <span className="text-[14px] font-bold font-mono text-forge-text">
                        {typeof item.value === 'number' && item.value > 10000
                          ? item.value.toExponential(1)
                          : item.value}
                      </span>
                      <span className="text-[8px] text-forge-text-muted font-mono">{item.unit}</span>
                    </div>
                    <MiniSparkline data={item.history} color={sparkColor} />
                  </div>
                </div>
              );
            })}
          </div>
        )}

        {activeTab === 'ai' && (
          <div className="p-2 space-y-1.5">
            <div className="flex items-center gap-1.5 px-1 mb-2">
              <Lightbulb size={10} className="text-forge-accent" />
              <span className="text-[9px] font-mono text-forge-text-muted">
                {aiInsights.length} ACTIVE INSIGHTS
              </span>
            </div>
            {aiInsights.map(insight => (
              <div
                key={insight.id}
                className={`rounded border ${severityColors[insight.severity]} transition-all`}
              >
                <button
                  onClick={() => setExpandedInsight(expandedInsight === insight.id ? null : insight.id)}
                  className="w-full p-2 flex items-start gap-2 text-left"
                >
                  <div className="flex-shrink-0 mt-0.5">
                    {severityIcons[insight.severity]}
                  </div>
                  <div className="flex-1 min-w-0">
                    <div className="flex items-center justify-between gap-2">
                      <span className="text-[10px] font-semibold text-forge-text leading-tight">
                        {insight.title}
                      </span>
                      {expandedInsight === insight.id ? (
                        <ChevronDown size={10} className="text-forge-text-muted flex-shrink-0" />
                      ) : (
                        <ChevronRight size={10} className="text-forge-text-muted flex-shrink-0" />
                      )}
                    </div>
                    <div className="flex items-center gap-2 mt-1">
                      <span className="text-[8px] font-mono text-forge-text-muted">{insight.timestamp}</span>
                      <span className="text-[8px] font-mono text-forge-accent">{insight.confidence}% conf</span>
                    </div>
                  </div>
                </button>
                {expandedInsight === insight.id && (
                  <div className="px-2 pb-2 pt-0">
                    <p className="text-[9px] text-forge-text-dim leading-relaxed pl-5">
                      {insight.description}
                    </p>
                    <div className="flex gap-1 mt-2 pl-5">
                      <button className="px-2 py-0.5 rounded text-[8px] font-mono bg-forge-accent/10 text-forge-accent border border-forge-accent/20 hover:bg-forge-accent/20 transition-all">
                        APPLY
                      </button>
                      <button className="px-2 py-0.5 rounded text-[8px] font-mono text-forge-text-muted hover:bg-forge-surface transition-all">
                        DISMISS
                      </button>
                      <button className="px-2 py-0.5 rounded text-[8px] font-mono text-forge-text-muted hover:bg-forge-surface transition-all">
                        DETAILS
                      </button>
                    </div>
                  </div>
                )}
              </div>
            ))}
          </div>
        )}

        {activeTab === 'solver' && (
          <div className="p-2 space-y-2">
            {/* Solver Status */}
            <div className="p-2.5 rounded border border-forge-green/20 bg-forge-green/5">
              <div className="flex items-center gap-1.5 mb-2">
                <Shield size={11} className="text-forge-green" />
                <span className="text-[10px] font-semibold text-forge-green">SOLVER CONVERGED</span>
              </div>
              <div className="grid grid-cols-2 gap-2">
                <div>
                  <div className="text-[8px] text-forge-text-muted font-mono">ITERATIONS</div>
                  <div className="text-[13px] font-bold font-mono text-forge-text">{solverMetrics.iterations}</div>
                </div>
                <div>
                  <div className="text-[8px] text-forge-text-muted font-mono">CONVERGENCE</div>
                  <div className="text-[13px] font-bold font-mono text-forge-green">{solverMetrics.convergence}%</div>
                </div>
                <div>
                  <div className="text-[8px] text-forge-text-muted font-mono">RESIDUAL</div>
                  <div className="text-[13px] font-bold font-mono text-forge-text">{solverMetrics.residual.toExponential(1)}</div>
                </div>
                <div>
                  <div className="text-[8px] text-forge-text-muted font-mono">WALL TIME</div>
                  <div className="text-[13px] font-bold font-mono text-forge-text">{solverMetrics.wallTime}</div>
                </div>
              </div>
            </div>

            {/* Resource Usage */}
            <div className="p-2.5 rounded border border-forge-border/50 bg-forge-surface/30">
              <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">RESOURCES</span>
              <div className="space-y-2 mt-2">
                <div>
                  <div className="flex justify-between text-[8px] font-mono text-forge-text-muted mb-0.5">
                    <span>CPU ({solverMetrics.cpuCores} cores)</span>
                    <span>87%</span>
                  </div>
                  <div className="w-full h-1.5 bg-forge-dark rounded-full overflow-hidden">
                    <div className="h-full bg-gradient-to-r from-forge-accent to-forge-cyan rounded-full" style={{ width: '87%' }} />
                  </div>
                </div>
                <div>
                  <div className="flex justify-between text-[8px] font-mono text-forge-text-muted mb-0.5">
                    <span>GPU</span>
                    <span>{solverMetrics.gpuUtil}%</span>
                  </div>
                  <div className="w-full h-1.5 bg-forge-dark rounded-full overflow-hidden">
                    <div className="h-full bg-gradient-to-r from-forge-purple to-forge-magenta rounded-full" style={{ width: `${solverMetrics.gpuUtil}%` }} />
                  </div>
                </div>
                <div>
                  <div className="flex justify-between text-[8px] font-mono text-forge-text-muted mb-0.5">
                    <span>MEMORY</span>
                    <span>{solverMetrics.memUsage} / 64 GB</span>
                  </div>
                  <div className="w-full h-1.5 bg-forge-dark rounded-full overflow-hidden">
                    <div className="h-full bg-gradient-to-r from-forge-green to-forge-teal rounded-full" style={{ width: `${(solverMetrics.memUsage / 64) * 100}%` }} />
                  </div>
                </div>
              </div>
            </div>

            {/* Mesh Info */}
            <div className="p-2.5 rounded border border-forge-border/50 bg-forge-surface/30">
              <span className="text-[9px] font-semibold tracking-wider text-forge-text-dim">MESH</span>
              <div className="grid grid-cols-1 gap-1.5 mt-2">
                <div className="flex justify-between">
                  <span className="text-[8px] font-mono text-forge-text-muted">Nodes</span>
                  <span className="text-[10px] font-mono text-forge-text">{solverMetrics.meshNodes.toLocaleString()}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-[8px] font-mono text-forge-text-muted">Elements</span>
                  <span className="text-[10px] font-mono text-forge-text">{solverMetrics.meshElements.toLocaleString()}</span>
                </div>
                <div className="flex justify-between">
                  <span className="text-[8px] font-mono text-forge-text-muted">DOFs</span>
                  <span className="text-[10px] font-mono text-forge-text">{solverMetrics.dofs.toLocaleString()}</span>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};
