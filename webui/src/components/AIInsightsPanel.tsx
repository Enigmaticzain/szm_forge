import { useState, useEffect } from 'react';

interface AIInsight {
  id: string;
  type: 'optimization' | 'risk' | 'warning' | 'prediction' | 'manufacturing';
  severity: 'info' | 'moderate' | 'high' | 'critical';
  title: string;
  description: string;
  confidence: number;
  metric?: string;
  recommendation?: string;
  timestamp: number;
}

const staticInsights: AIInsight[] = [
  {
    id: 'ai-1',
    type: 'risk',
    severity: 'high',
    title: 'Stress Concentration — Fillet Zone B3',
    description: 'Detected stress concentration exceeding 92% of yield strength at fillet radius R=1.5mm. Von Mises stress: 952 MPa.',
    confidence: 0.94,
    metric: '952 MPa / 1034 MPa yield',
    recommendation: 'Increase fillet radius to R≥3.0mm. Predicted stress reduction: 34%.',
    timestamp: Date.now() - 30000,
  },
  {
    id: 'ai-2',
    type: 'prediction',
    severity: 'moderate',
    title: 'Fatigue Life Prediction — Shaft Assembly',
    description: 'Cumulative fatigue damage analysis indicates potential crack initiation at 847,000 cycles under current loading.',
    confidence: 0.87,
    metric: '847K / 1M cycles',
    recommendation: 'Consider shot peening surface treatment. Est. improvement: +180K cycles.',
    timestamp: Date.now() - 120000,
  },
  {
    id: 'ai-3',
    type: 'optimization',
    severity: 'info',
    title: 'Topology Optimization — Weight Reduction',
    description: 'Generative design iteration 342 identified 23.4% material reduction while maintaining FoS ≥ 1.8.',
    confidence: 0.91,
    metric: '-23.4% mass / FoS 1.82',
    recommendation: 'Apply suggested topology. Manufacturing via DMLS compatible.',
    timestamp: Date.now() - 300000,
  },
  {
    id: 'ai-4',
    type: 'warning',
    severity: 'critical',
    title: 'Thermal Runaway Risk — Bearing Zone',
    description: 'Temperature gradient exceeding safe limits. Current: 487°C, threshold: 450°C. Coolant flow rate suboptimal.',
    confidence: 0.96,
    metric: '487°C / 450°C limit',
    recommendation: 'Increase coolant flow rate by 15% or add thermal barrier coating.',
    timestamp: Date.now() - 15000,
  },
  {
    id: 'ai-5',
    type: 'manufacturing',
    severity: 'moderate',
    title: 'Manufacturability — Undercut Detection',
    description: 'Feature at section C-7 requires 5-axis machining or split toolpath. Current setup: 3-axis CNC.',
    confidence: 0.89,
    metric: 'Undercut angle: 12.3°',
    recommendation: 'Modify geometry to 8° draft angle or upgrade to 5-axis toolpath.',
    timestamp: Date.now() - 180000,
  },
];

export default function AIInsightsPanel() {
  const [insights, setInsights] = useState(staticInsights);
  const [expandedId, setExpandedId] = useState<string | null>('ai-4');
  const [aiProcessing, setAiProcessing] = useState(false);

  useEffect(() => {
    const interval = setInterval(() => {
      setAiProcessing(true);
      setTimeout(() => setAiProcessing(false), 800);
    }, 8000);
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    setInsights(prev => prev.map(ins => ({
      ...ins,
      confidence: Math.min(0.99, Math.max(0.7, ins.confidence + (Math.random() - 0.5) * 0.01))
    })));
  }, [aiProcessing]);

  const severityConfig = {
    info: { color: 'text-forge-accent', bg: 'bg-forge-accent/10', border: 'border-forge-accent/20', icon: '◆' },
    moderate: { color: 'text-forge-warning', bg: 'bg-forge-warning/10', border: 'border-forge-warning/20', icon: '◈' },
    high: { color: 'text-orange-400', bg: 'bg-orange-400/10', border: 'border-orange-400/20', icon: '⬡' },
    critical: { color: 'text-forge-danger', bg: 'bg-forge-danger/10', border: 'border-forge-danger/20', icon: '⬢' },
  };

  const typeLabels = {
    optimization: 'OPTIMIZATION',
    risk: 'RISK ANALYSIS',
    warning: 'SYSTEM WARNING',
    prediction: 'PREDICTION',
    manufacturing: 'MFG ANALYSIS',
  };

  return (
    <div className="h-full flex flex-col bg-forge-surface overflow-hidden">
      <div className="px-3 py-2 border-b border-forge-border flex items-center justify-between">
        <div className="flex items-center gap-2">
          <div className={`w-2 h-2 rounded-full bg-forge-purple ${aiProcessing ? 'animate-pulse-glow' : ''}`} />
          <span className="text-[10px] font-bold text-forge-text tracking-wider">AI INTELLIGENCE</span>
        </div>
        <div className="flex items-center gap-2">
          {aiProcessing && (
            <span className="text-[8px] text-forge-purple animate-pulse-glow tracking-wider">ANALYZING...</span>
          )}
          <span className="text-[8px] text-forge-text-muted font-mono">
            {insights.filter(i => i.severity === 'critical').length} CRITICAL
          </span>
        </div>
      </div>

      <div className="flex-1 overflow-y-auto p-2 space-y-1.5">
        {insights.sort((a, b) => {
          const order = { critical: 0, high: 1, moderate: 2, info: 3 };
          return order[a.severity] - order[b.severity];
        }).map(insight => {
          const config = severityConfig[insight.severity];
          const isExpanded = expandedId === insight.id;

          return (
            <div
              key={insight.id}
              className={`rounded-lg border transition-all duration-300 cursor-pointer ${config.border} ${config.bg}`}
              onClick={() => setExpandedId(isExpanded ? null : insight.id)}
            >
              <div className="p-2.5">
                <div className="flex items-start justify-between mb-1">
                  <div className="flex items-center gap-1.5">
                    <span className={`text-xs ${config.color}`}>{config.icon}</span>
                    <span className={`text-[7px] font-bold tracking-wider ${config.color}`}>
                      {typeLabels[insight.type]}
                    </span>
                  </div>
                  <div className="flex items-center gap-1">
                    <span className="text-[7px] text-forge-text-muted">CONF</span>
                    <span className={`text-[8px] font-mono font-bold ${config.color}`}>
                      {(insight.confidence * 100).toFixed(1)}%
                    </span>
                  </div>
                </div>

                <h4 className="text-[10px] font-semibold text-forge-text mb-1">{insight.title}</h4>

                {isExpanded && (
                  <div className="animate-fade-in-up space-y-2 mt-2">
                    <p className="text-[9px] text-forge-text-dim leading-relaxed">{insight.description}</p>
                    
                    {insight.metric && (
                      <div className="flex items-center gap-2 p-1.5 bg-black/20 rounded">
                        <span className="text-[7px] text-forge-text-muted tracking-wider">METRIC</span>
                        <span className={`text-[9px] font-mono font-bold ${config.color}`}>{insight.metric}</span>
                      </div>
                    )}

                    {insight.recommendation && (
                      <div className="p-2 rounded bg-forge-purple/5 border border-forge-purple/20">
                        <div className="text-[7px] text-forge-purple font-bold tracking-wider mb-1">
                          AI RECOMMENDATION
                        </div>
                        <p className="text-[9px] text-forge-text-dim leading-relaxed">
                          {insight.recommendation}
                        </p>
                      </div>
                    )}

                    <div className="flex gap-1.5">
                      <button className="flex-1 py-1 text-[8px] font-bold tracking-wider bg-forge-accent/10 text-forge-accent border border-forge-accent/20 rounded hover:bg-forge-accent/20 transition-all">
                        APPLY
                      </button>
                      <button className="flex-1 py-1 text-[8px] font-bold tracking-wider bg-forge-panel text-forge-text-dim border border-forge-border rounded hover:border-forge-border-light transition-all">
                        DISMISS
                      </button>
                    </div>
                  </div>
                )}
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}
