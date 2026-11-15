import React, { useState, useEffect } from 'react';
import { aiInsights as sampleInsights } from '../data/mockData';
import {
  Brain, Cpu, Activity, Target, Lightbulb,
  AlertTriangle, CheckCircle, Info, XCircle,
  TrendingUp, Shield,
  ChevronRight, Eye, EyeOff
} from 'lucide-react';
import { AIChatBot } from './AIChatBot';
import { Resizer } from './Resizer';
import { useBackend } from '../store/BackendContext';
import { useProject } from '../store/ProjectContext';
import { useForgeStore } from '../store/ForgeStoreContext';
import { getApiBaseUrl, runBatchSimulation, mlIngestData, mlLearnPatterns, mlGenerateMachine, queryReasoning, GeneratedMachineCode } from '../services/forgeApi';

const severityIcons: Record<string, React.ReactNode> = {
  info: <Info size={12} className="text-forge-accent" />,
  success: <CheckCircle size={12} className="text-forge-green" />,
  warning: <AlertTriangle size={12} className="text-forge-yellow" />,
  critical: <XCircle size={12} className="text-forge-red" />,
};

const typeLabels: Record<string, { label: string; color: string; bg: string }> = {
  optimization: { label: 'OPTIMIZATION', color: 'text-forge-purple', bg: 'bg-forge-purple/10' },
  warning: { label: 'WARNING', color: 'text-forge-yellow', bg: 'bg-forge-yellow/10' },
  recommendation: { label: 'RECOMMENDATION', color: 'text-forge-accent', bg: 'bg-forge-accent/10' },
  diagnostic: { label: 'DIAGNOSTIC', color: 'text-forge-teal', bg: 'bg-forge-teal/10' },
};

// Honest initial agent state: nothing is "active scanning" until the backend
// reports real work. CPU/confidence start at zero instead of fabricated values.
const initialAgentStatus = [
  { name: 'SZM Design Loop', status: 'idle', task: 'Awaiting design problem…', cpu: 0, confidence: 0 },
  { name: 'Failure Predictor', status: 'idle', task: 'Needs solver results from a loaded model', cpu: 0, confidence: 0 },
  { name: 'Material Selector', status: 'idle', task: 'Awaiting requirements', cpu: 0, confidence: 0 },
];

export const AIWorkspace: React.FC = () => {
  const backend = useBackend();
  const store = useForgeStore();
  const { bodyParts, members, parts, aiInsights: sessionInsights } = useProject();

  const [activeAgent, setActiveAgent] = useState(0);
  const [pulseVals, setPulseVals] = useState<number[]>([]);
  const [leftWidth, setLeftWidth] = useState(320);
  const [rightWidth, setRightWidth] = useState(380);
  const [agents, setAgents] = useState(initialAgentStatus);
  const [batchStatus, setBatchStatus] = useState('Idle');

  const [mlOutput, setMlOutput] = useState<GeneratedMachineCode[]>([]);
  const [isGeneratingML, setIsGeneratingML] = useState(false);
  const [isLearningML, setIsLearningML] = useState(false);

  const [observationActive, setObservationActive] = useState(false);
  const [obsStats, setObsStats] = useState({ total_actions: 0, total_patterns: 0 });

  const [showSampleInsights, setShowSampleInsights] = useState(false);
  const [reasoning, setReasoning] = useState<null | { prompt: string; answer: string }>(null);
  const [reasoningBusy, setReasoningBusy] = useState<string | null>(null);

  // A "model" exists only when the user actually loaded/built something.
  const hasModelContent = bodyParts.length > 0 || members.length > 0 || parts.length > 0;
  const backendLinked = backend.remoteEnabled && backend.remoteReachable === true;

  // Agent activity bar: flat when idle, animated only while the backend
  // reports an active design task. No perpetual random "brain noise".
  const agentActive = agents[0]?.status === 'active';
  useEffect(() => {
    const interval = setInterval(() => {
      setPulseVals(prev => {
        const next = [...prev, agentActive ? 30 + Math.random() * 70 : 2];
        if (next.length > 40) next.shift();
        return next;
      });
    }, 200);
    return () => clearInterval(interval);
  }, [agentActive]);

  // Poll Python backend for training status — only when a remote backend is
  // actually enabled; offline mode skips the polling entirely.
  useEffect(() => {
    if (!backend.remoteEnabled) return undefined;
    const pollStatus = async () => {
      try {
        const res = await fetch(`${getApiBaseUrl()}/api/status`);
        if (res.ok) {
          const data = await res.json();
          const activeTask = data.training?.active_status || 'Idle';
          setBatchStatus(data.training?.batch_status || 'Idle');

          setAgents(prev => {
            const newAgents = [...prev];
            newAgents[0] = {
              ...newAgents[0],
              status: activeTask === 'Idle' ? 'idle' : 'active',
              task: activeTask,
              cpu: activeTask === 'Idle' ? 0 : 85 + Math.random() * 10,
            };
            return newAgents;
          });
        }
      } catch (err) {
        // silently ignore fetch errors to avoid console spam
      }
    };

    pollStatus();
    const interval = setInterval(pollStatus, 1500);
    return () => clearInterval(interval);
  }, [backend.remoteEnabled]);

  // Poll observation stats (backend feature — only when linked and active)
  useEffect(() => {
    let interval: NodeJS.Timeout | undefined;
    if (observationActive && backendLinked) {
      interval = setInterval(async () => {
        try {
          const res = await fetch(`${getApiBaseUrl()}/api/ai/observe/stats`);
          if (res.ok) {
            const data = await res.json();
            setObsStats({
              total_actions: data.total_actions || 0,
              total_patterns: data.total_patterns_learned || 0
            });
          }
        } catch { /* backend stats optional */ }
      }, 1000);
    }
    return () => { if (interval) clearInterval(interval); };
  }, [observationActive, backendLinked]);

  const toggleObservation = async () => {
    if (!backendLinked) {
      store.showToast('Passive observation requires the optional Python backend — currently unavailable.');
      return;
    }
    try {
      const endpoint = observationActive ? 'stop' : 'start';
      await fetch(`${getApiBaseUrl()}/api/ai/observe/${endpoint}`, { method: 'POST' });
      setObservationActive(!observationActive);
    } catch (e) {
      store.showToast(`Observation toggle failed: ${e instanceof Error ? e.message : String(e)}`);
    }
  };

  const startBatchSimulation = async () => {
    try {
      await runBatchSimulation(50);
      store.showToast('Batch simulation started (50 variations)');
    } catch (e) {
      store.showToast(`Batch simulation unavailable: ${e instanceof Error ? e.message : String(e)}`);
    }
  };

  const handleLearnPatterns = async () => {
    setIsLearningML(true);
    try {
      const ingested = await mlIngestData();
      const patterns = await mlLearnPatterns();
      store.showToast(`Learned ${patterns} patterns from ${ingested} datasets`);
    } catch (e) {
      store.showToast(`Pattern learning unavailable: ${e instanceof Error ? e.message : String(e)}`);
    }
    setIsLearningML(false);
  };

  const handleGenerateMachine = async () => {
    setIsGeneratingML(true);
    try {
      const machines = await mlGenerateMachine('cnc_mill', 1);
      setMlOutput(machines);
      if (machines.length === 0) store.showToast('Generator returned no machines');
    } catch (e) {
      store.showToast(`Machine generation unavailable: ${e instanceof Error ? e.message : String(e)}`);
    }
    setIsGeneratingML(false);
  };

  // The three assistance actions: real reasoning queries, gated on a real model.
  const runAssistance = async (kind: 'review' | 'optimize' | 'safety') => {
    if (!hasModelContent) {
      store.showToast('Load or build a model first — there is nothing to review yet.');
      return;
    }
    const focus =
      kind === 'review' ? 'Review the current design' :
      kind === 'optimize' ? 'Suggest mass/strength optimizations for the current design' :
      'Identify safety risks in the current design';
    const contextHint = `Model contains ${bodyParts.length + members.length} session component(s) and ${parts.length} solved part(s).`;
    setReasoningBusy(kind);
    try {
      const res = await queryReasoning(`${focus}. ${contextHint}`);
      setReasoning({
        prompt: focus,
        answer: typeof res?.answer === 'string'
          ? res.answer
          : JSON.stringify(res, null, 2).slice(0, 1200),
      });
    } catch (e) {
      store.showToast(`AI reasoning unavailable: ${e instanceof Error ? e.message : String(e)}`);
    }
    setReasoningBusy(null);
  };

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
            <div className={`w-1.5 h-1.5 rounded-full ${agentActive ? 'bg-forge-green animate-pulse' : 'bg-forge-text-muted'}`} />
            <span className="text-[9px] font-mono text-forge-text-dim">{agents.length} AGENTS {agentActive ? 'ACTIVE' : 'IDLE'}</span>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">AI BACKEND</div>
            <div className={`text-[12px] font-bold font-mono ${backendLinked ? 'text-forge-green' : 'text-forge-text-muted'}`}>
              {backendLinked ? 'LINKED' : backend.remoteEnabled ? 'DOWN' : 'OFFLINE'}
            </div>
          </div>
          <div className="text-center">
            <div className="text-[8px] font-mono text-forge-text-muted">MODEL</div>
            <div className={`text-[12px] font-bold font-mono ${hasModelContent ? 'text-forge-accent' : 'text-forge-text-muted'}`}>
              {hasModelContent ? 'LOADED' : 'NONE'}
            </div>
          </div>
        </div>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Left - Agent Status */}
        <div className="border-r border-forge-border bg-forge-panel overflow-y-auto p-3 flex-shrink-0 custom-scrollbar flex flex-col" style={{ width: leftWidth }}>
          <div className="flex items-center gap-1.5 mb-3">
            <Cpu size={11} className="text-forge-purple" />
            <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">AI AGENTS</span>
          </div>

          <div className="space-y-2 mb-4">
            {agents.map((agent, i) => (
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
                <p className="text-[8px] text-forge-text-muted mb-2 line-clamp-2">{agent.task}</p>
                <div className="flex items-center justify-between">
                  <div className="flex items-center gap-2">
                    <span className="text-[7px] font-mono text-forge-text-muted">CPU {agent.cpu.toFixed(0)}%</span>
                    {agent.confidence > 0 && (
                      <span className="text-[7px] font-mono text-forge-purple">CONF {agent.confidence}%</span>
                    )}
                  </div>
                  <ChevronRight size={10} className="text-forge-text-muted" />
                </div>
              </button>
            ))}
          </div>

          {/* Agent Activity — reflects real state: flat when idle */}
          <div className="p-3 rounded-lg border border-forge-purple/20 bg-forge-purple/5">
            <div className="flex items-center gap-1.5 mb-2">
              <Activity size={10} className="text-forge-purple" />
              <span className="text-[9px] font-semibold text-forge-text-dim">AGENT ACTIVITY</span>
              <span className="text-[7px] font-mono text-forge-text-muted ml-auto">{agentActive ? 'RUNNING' : 'IDLE'}</span>
            </div>
            <div className="h-12 flex items-end gap-px">
              {pulseVals.map((v, i) => (
                <div
                  key={i}
                  className={`flex-1 rounded-t transition-all duration-100 ${agentActive ? 'bg-forge-purple/60' : 'bg-forge-border/60'}`}
                  style={{ height: `${v}%` }}
                />
              ))}
            </div>
          </div>

          <div className="mt-auto pt-4 space-y-2">
             <div className="p-3 rounded border border-forge-border bg-forge-dark">
                <div className="flex items-center justify-between mb-2">
                  <div className="text-[9px] font-semibold text-forge-text-dim">PASSIVE OBSERVATION</div>
                  <button onClick={toggleObservation} className={`p-1 rounded ${observationActive ? 'bg-forge-green/20 text-forge-green' : 'bg-forge-surface text-forge-text-muted'}`}>
                    {observationActive ? <Eye size={12} /> : <EyeOff size={12} />}
                  </button>
                </div>
                <div className="flex justify-between text-[10px] font-mono">
                  <span className="text-forge-text-muted">ACTIONS: <span className="text-forge-accent">{obsStats.total_actions}</span></span>
                  <span className="text-forge-text-muted">PATTERNS: <span className="text-forge-purple">{obsStats.total_patterns}</span></span>
                </div>
             </div>

             <div className="p-3 rounded border border-forge-border bg-forge-dark mt-2">
                <div className="text-[9px] font-semibold text-forge-text-dim mb-1">BATCH SIMULATOR</div>
                <div className="text-[10px] text-forge-accent font-mono truncate">{batchStatus}</div>
             </div>
             <button onClick={startBatchSimulation} className="w-full py-2 bg-forge-purple text-white rounded font-semibold text-[10px] hover:bg-forge-purple/80 transition-colors">
                INITIATE SELF-TEACHING
             </button>

             <div className="p-3 rounded border border-forge-border bg-forge-dark mt-2">
                <div className="text-[9px] font-semibold text-forge-text-dim mb-2">MACHINE GENERATOR</div>
                <div className="flex gap-2 mb-2">
                  <button onClick={handleLearnPatterns} disabled={isLearningML} className="flex-1 py-1.5 bg-forge-surface text-forge-text-muted rounded text-[8px] font-mono hover:bg-forge-purple/20 hover:text-forge-purple disabled:opacity-50 transition-colors">
                    {isLearningML ? 'LEARNING...' : 'LEARN PATTERNS'}
                  </button>
                  <button onClick={handleGenerateMachine} disabled={isGeneratingML || isLearningML} className="flex-1 py-1.5 bg-forge-surface text-forge-text-muted rounded text-[8px] font-mono hover:bg-forge-green/20 hover:text-forge-green disabled:opacity-50 transition-colors">
                    {isGeneratingML ? 'GENERATING...' : 'GENERATE C++'}
                  </button>
                </div>
             </div>
          </div>
        </div>

        <Resizer onResize={(delta) => setLeftWidth(prev => Math.max(250, Math.min(500, prev + delta)))} />

        {/* Center - Insights & Optimization */}
        <div className="flex-1 overflow-y-auto p-4 space-y-4 min-w-0">
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-2 mb-3">
              <Brain size={12} className="text-forge-accent" />
              <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">SZM AI ASSISTANCE</span>
            </div>
            <p className="text-[10px] text-forge-text-dim leading-relaxed mb-3">
              Run reasoning against the model you have actually loaded. Queries use the AI backend's reasoning
              engine; without it you will get an explicit unavailable message — never a canned answer.
            </p>
            <div className="flex flex-wrap gap-2">
              <button
                onClick={() => runAssistance('review')}
                disabled={reasoningBusy !== null}
                className="px-2.5 py-1 rounded text-[8px] font-mono bg-forge-accent/10 text-forge-accent border border-forge-accent/20 hover:bg-forge-accent/20 transition-all disabled:opacity-50"
              >
                {reasoningBusy === 'review' ? 'REVIEWING…' : 'REVIEW CURRENT MODEL'}
              </button>
              <button
                onClick={() => runAssistance('optimize')}
                disabled={reasoningBusy !== null}
                className="px-2.5 py-1 rounded text-[8px] font-mono text-forge-text-muted border border-forge-border/50 hover:text-forge-text hover:bg-forge-surface transition-all disabled:opacity-50"
              >
                {reasoningBusy === 'optimize' ? 'THINKING…' : 'SUGGEST OPTIMIZATION'}
              </button>
              <button
                onClick={() => runAssistance('safety')}
                disabled={reasoningBusy !== null}
                className="px-2.5 py-1 rounded text-[8px] font-mono text-forge-text-muted border border-forge-border/50 hover:text-forge-text hover:bg-forge-surface transition-all disabled:opacity-50"
              >
                {reasoningBusy === 'safety' ? 'CHECKING…' : 'CHECK SAFETY RISKS'}
              </button>
            </div>
            {reasoning && (
              <div className="mt-3 p-3 rounded border border-forge-accent/30 bg-forge-accent/5">
                <div className="flex items-center gap-2 mb-1">
                  <span className="text-[8px] font-mono text-forge-accent tracking-widest">{reasoning.prompt.toUpperCase()}</span>
                  <button onClick={() => setReasoning(null)} className="ml-auto text-[8px] font-mono text-forge-text-muted hover:text-forge-text">DISMISS</button>
                </div>
                <pre className="text-[9px] font-mono text-forge-text-dim whitespace-pre-wrap break-words max-h-48 overflow-y-auto custom-scrollbar">{reasoning.answer}</pre>
              </div>
            )}
          </div>

          {/* Generated Machine Code Preview */}
          {mlOutput.length > 0 && (
            <div className="p-4 rounded-lg border border-forge-purple/30 bg-forge-purple/5">
              <div className="flex items-center gap-2 mb-3">
                <Brain size={12} className="text-forge-purple" />
                <span className="text-[10px] font-semibold tracking-widest text-forge-purple">GENERATED C++ MACHINE CODE</span>
                <span className="text-[8px] font-mono text-forge-text-muted ml-auto">Confidence: {(mlOutput[0].confidence * 100).toFixed(0)}%</span>
              </div>
              <div className="grid grid-cols-2 gap-4">
                <div className="flex flex-col">
                  <span className="text-[9px] font-mono text-forge-text-dim mb-1">{mlOutput[0].name}.hpp</span>
                  <pre className="p-2 rounded bg-forge-dark border border-forge-border text-[8px] font-mono text-forge-green overflow-x-auto max-h-48 custom-scrollbar">
                    {mlOutput[0].header}
                  </pre>
                </div>
                <div className="flex flex-col">
                  <span className="text-[9px] font-mono text-forge-text-dim mb-1">{mlOutput[0].name}.cpp</span>
                  <pre className="p-2 rounded bg-forge-dark border border-forge-border text-[8px] font-mono text-forge-cyan overflow-x-auto max-h-48 custom-scrollbar">
                    {mlOutput[0].source}
                  </pre>
                </div>
              </div>
            </div>
          )}

          {/* Optimization Targets — real session data only */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-2 mb-4">
              <Target size={12} className="text-forge-accent" />
              <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">OPTIMIZATION TARGETS</span>
            </div>
            {parts.length > 0 ? (
              <div className="space-y-3">
                {parts.slice(0, 6).map((p, i) => {
                  const ratio = Math.max(0, Math.min(1.5, p.stressRatio ?? 0));
                  const pct = Math.round(Math.min(1, ratio) * 100);
                  return (
                    <div key={i}>
                      <div className="flex items-center justify-between mb-1">
                        <span className="text-[10px] font-medium text-forge-text">{p.name}</span>
                        <span className="text-[9px] font-mono text-forge-text-dim">
                          {p.stress_MPa?.toFixed(1)} MPa / yield {p.yield_MPa?.toFixed(0)} MPa
                        </span>
                      </div>
                      <div className="flex items-center gap-2">
                        <div className="flex-1 h-1.5 bg-forge-dark rounded-full overflow-hidden">
                          <div
                            className={`h-full rounded-full transition-all ${ratio >= 1 ? 'bg-forge-red' : ratio >= 0.7 ? 'bg-forge-yellow' : 'bg-forge-green'}`}
                            style={{ width: `${pct}%` }}
                          />
                        </div>
                        <span className="text-[8px] font-mono text-forge-text-muted w-10 text-right">{pct}% util</span>
                      </div>
                    </div>
                  );
                })}
              </div>
            ) : (
              <p className="text-[10px] font-mono text-forge-text-muted">
                No solved parts yet. Load a preset or build members, then run SOLVE — utilization
                bars here are computed from real solver results, not sample numbers.
              </p>
            )}
          </div>

          {/* Insights — session insights + optional, clearly-labeled samples */}
          <div className="p-4 rounded-lg border border-forge-border bg-forge-panel">
            <div className="flex items-center gap-2 mb-4">
              <Lightbulb size={12} className="text-forge-yellow" />
              <span className="text-[10px] font-semibold tracking-widest text-forge-text-dim">AI INSIGHTS & ACTIONS</span>
              <span className="text-[8px] font-mono text-forge-text-muted ml-auto">
                {sessionInsights.length} from this session
              </span>
            </div>

            {sessionInsights.length === 0 && (
              <p className="text-[10px] font-mono text-forge-text-muted mb-3">
                No insights yet. Run a solve plus AI IMPROVE in the workshop, or use the reasoning
                actions above — insights shown here always come from your session.
              </p>
            )}

            <div className="space-y-2">
              {sessionInsights.map((insight, i) => {
                const tl = typeLabels[insight.type] ?? typeLabels.recommendation;
                return (
                  <div
                    key={`session-${i}`}
                    className="p-3 rounded border border-forge-border/50 bg-forge-surface/20 hover:bg-forge-surface/40 transition-all"
                  >
                    <div className="flex items-start gap-2">
                      {severityIcons.info}
                      <div className="flex-1">
                        <div className="flex items-center gap-2 mb-1">
                          <span className="text-[10px] font-semibold text-forge-text">{insight.title}</span>
                          <span className={`text-[7px] font-mono tracking-wider px-1.5 py-0.5 rounded ${tl.bg} ${tl.color}`}>
                            {tl.label}
                          </span>
                          <span className="text-[7px] font-mono tracking-wider px-1.5 py-0.5 rounded bg-forge-green/10 text-forge-green">
                            SESSION
                          </span>
                        </div>
                        <p className="text-[9px] text-forge-text-dim leading-relaxed mb-2">{insight.description}</p>
                        <span className="text-[8px] font-mono text-forge-purple">Confidence: {Math.round(insight.confidence)}%</span>
                      </div>
                    </div>
                  </div>
                );
              })}
            </div>

            <button
              onClick={() => setShowSampleInsights(v => !v)}
              className="mt-3 text-[8px] font-mono text-forge-text-muted hover:text-forge-text transition-colors"
            >
              {showSampleInsights ? 'HIDE SAMPLE INSIGHTS (DEMO)' : 'PREVIEW SAMPLE INSIGHTS (DEMO)'}
            </button>

            {showSampleInsights && (
              <div className="mt-3 space-y-2">
                <div className="p-2 rounded border border-forge-yellow/30 bg-forge-yellow/5 text-[8px] font-mono text-forge-yellow tracking-wider">
                  DEMO DATA — these sample insights describe a turbine assembly and are NOT from your model.
                </div>
                {sampleInsights.map(insight => {
                  const tl = typeLabels[insight.type];
                  return (
                    <div
                      key={insight.id}
                      className="p-3 rounded border border-dashed border-forge-yellow/20 bg-forge-surface/10 opacity-75"
                    >
                      <div className="flex items-start gap-2">
                        {severityIcons[insight.severity]}
                        <div className="flex-1">
                          <div className="flex items-center gap-2 mb-1">
                            <span className="text-[10px] font-semibold text-forge-text">{insight.title}</span>
                            <span className={`text-[7px] font-mono tracking-wider px-1.5 py-0.5 rounded ${tl.bg} ${tl.color}`}>
                              {tl.label}
                            </span>
                            <span className="text-[7px] font-mono tracking-wider px-1.5 py-0.5 rounded bg-forge-yellow/10 text-forge-yellow">
                              SAMPLE
                            </span>
                          </div>
                          <p className="text-[9px] text-forge-text-dim leading-relaxed mb-2">{insight.description}</p>
                          <div className="flex items-center gap-4">
                            <span className="text-[8px] font-mono text-forge-text-muted">{insight.timestamp}</span>
                            <span className="text-[8px] font-mono text-forge-purple">Confidence: {insight.confidence}%</span>
                          </div>
                        </div>
                      </div>
                    </div>
                  );
                })}
              </div>
            )}
          </div>

          {/* Session Counters — real values only, "—" when unavailable */}
          <div className="grid grid-cols-4 gap-3">
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">OBSERVED ACTIONS</span>
                <Activity size={14} className="text-forge-accent" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-accent">
                {observationActive ? obsStats.total_actions : '—'}
              </div>
            </div>
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">PATTERNS LEARNED</span>
                <TrendingUp size={14} className="text-forge-purple" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-purple">
                {observationActive ? obsStats.total_patterns : '—'}
              </div>
            </div>
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">MACHINES GENERATED</span>
                <Shield size={14} className="text-forge-yellow" />
              </div>
              <div className="text-[18px] font-bold font-mono text-forge-yellow">{mlOutput.length}</div>
            </div>
            <div className="p-3 rounded-lg border border-forge-border/50 bg-forge-surface/20">
              <div className="flex items-center justify-between mb-2">
                <span className="text-[8px] font-mono tracking-wider text-forge-text-muted">BATCH STATUS</span>
                <Cpu size={14} className="text-forge-green" />
              </div>
              <div className="text-[12px] font-bold font-mono text-forge-green truncate">{backend.remoteEnabled ? batchStatus : 'OFFLINE'}</div>
            </div>
          </div>
        </div>

        <Resizer onResize={(delta) => setRightWidth(prev => Math.max(250, Math.min(600, prev - delta)))} />

        {/* Right - AI Chat Bot */}
        <div className="flex-shrink-0 relative z-10 shadow-[-10px_0_30px_rgba(0,0,0,0.5)]" style={{ width: rightWidth }}>
          <AIChatBot />
        </div>
      </div>
    </div>
  );
};
