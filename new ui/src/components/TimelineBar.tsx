import React from 'react';
import { useSessionEvents, type SessionEventType, type SessionEventSeverity } from '../store/SessionEventContext';
import {
  Clock, Cpu, Brain, AlertTriangle, Wrench,
  Activity, ChevronUp,
  FileDown, FileUp, TestTube2
} from 'lucide-react';

interface Props {
  visible: boolean;
  onToggle: () => void;
}

const eventIcons: Record<SessionEventType, React.ReactNode> = {
  simulation: <Cpu size={9} className="text-forge-cyan" />,
  test: <TestTube2 size={9} className="text-forge-purple" />,
  import: <FileUp size={9} className="text-forge-teal" />,
  export: <FileDown size={9} className="text-forge-accent" />,
  generate: <Brain size={9} className="text-forge-magenta" />,
  system: <Activity size={9} className="text-forge-text-muted" />,
};

const severityDots: Record<SessionEventSeverity, string> = {
  info: 'bg-forge-accent',
  success: 'bg-forge-green',
  warning: 'bg-forge-yellow',
  critical: 'bg-forge-red',
};

export const TimelineBar: React.FC<Props> = ({ visible, onToggle }) => {
  const { events } = useSessionEvents();
  const recent = events.slice(-12);
  const start = events.length > 0 ? events[0].time : '—';
  const end = events.length > 0 ? events[events.length - 1].time : '—';

  if (!visible) {
    return (
      <button
        onClick={onToggle}
        className="h-6 w-full bg-forge-dark border-t border-forge-border flex items-center justify-center gap-1 text-[8px] font-mono text-forge-text-muted hover:text-forge-text-dim transition-colors"
      >
        <ChevronUp size={10} />
        SHOW TIMELINE
      </button>
    );
  }

  return (
    <div className="h-[130px] glass-panel border-t border-forge-border/40 flex flex-col z-20 relative">
      {/* Timeline Header */}
      <div className="h-[36px] flex items-center px-4 border-b border-forge-border/40 justify-between bg-forge-dark/30">
        <div className="flex items-center gap-2">
          <Clock size={13} className="text-forge-accent" />
          <span className="text-[11px] font-semibold tracking-widest text-forge-text-dim">
            SESSION EVENTS
          </span>
          <span className="text-[10px] font-mono text-forge-text-muted ml-3">
            {events.length} events · this session
          </span>
        </div>
        <button
          onClick={onToggle}
          className="p-1 hover:bg-forge-surface rounded text-forge-text-muted hover:text-forge-text-dim transition-colors"
        >
          <ChevronUp size={10} className="rotate-180" />
        </button>
      </div>

      {/* Timeline Progress Bar */}
      <div className="h-[24px] px-3 flex items-center gap-2 border-b border-forge-border/30">
        <span className="text-[8px] font-mono text-forge-text-muted">{start}</span>
        <div className="flex-1 relative h-2 bg-forge-surface rounded-full overflow-hidden">
          {events.length === 0 ? (
            <div className="absolute inset-0 flex items-center justify-center text-[7px] font-mono text-forge-text-muted">
              nothing yet — imports, runs and exports will appear here
            </div>
          ) : (
            <>
              <div
                className="absolute inset-y-0 left-0 bg-gradient-to-r from-forge-accent/30 to-forge-accent/60 rounded-full"
                style={{ width: `${Math.min(100, (events.length / 12) * 100)}%` }}
              />
              {recent.map((ev, i) => (
                <div
                  key={ev.id}
                  className={`absolute top-1/2 -translate-y-1/2 w-1.5 h-1.5 rounded-full ${severityDots[ev.severity]} cursor-pointer hover:scale-150 transition-transform`}
                  style={{ left: `${6 + (i / Math.max(recent.length - 1, 1)) * 88}%` }}
                  title={ev.title}
                />
              ))}
            </>
          )}
        </div>
        <span className="text-[8px] font-mono text-forge-accent">{end}</span>
      </div>

      {/* Event Stream — most recent first */}
      <div className="flex-1 overflow-x-auto overflow-y-hidden">
        {recent.length === 0 ? (
          <div className="flex items-center justify-center h-full px-6">
            <div className="text-center">
              <Wrench size={14} className="mx-auto text-forge-text-muted mb-1.5" />
              <p className="text-[9px] font-mono text-forge-text-muted leading-relaxed max-w-sm">
                This log records real events of your session — STEP imports, geometry generation,
                exports and analyses — as they happen. Nothing is fabricated.
              </p>
            </div>
          </div>
        ) : (
          <div className="flex gap-1.5 p-2 h-full">
            {[...recent].reverse().map((event, i) => (
              <div
                key={event.id}
                className={`
                  flex-shrink-0 w-[180px] p-2 rounded border transition-all cursor-default
                  hover:border-forge-accent/30 hover:bg-forge-surface/50
                  ${i === 0 ? 'border-forge-accent/20 bg-forge-accent/5' : 'border-forge-border/50 bg-forge-surface/20'}
                `}
              >
                <div className="flex items-center gap-2 mb-1">
                  {eventIcons[event.type] ?? <AlertTriangle size={9} />}
                  <span className="text-[10px] font-mono text-forge-text-muted">{event.time}</span>
                  <div className={`w-1.5 h-1.5 rounded-full ml-auto ${severityDots[event.severity]}`} />
                </div>
                <p className="text-[11px] text-forge-text-dim leading-tight line-clamp-2">
                  {event.title}
                </p>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
};
