import React from 'react';
import { timelineEvents } from '../data/mockData';
import {
  Clock, Cpu, Brain, AlertTriangle, Wrench,
  Activity, ChevronUp, SkipBack, SkipForward,
  Play, Repeat
} from 'lucide-react';

interface Props {
  visible: boolean;
  onToggle: () => void;
}

const eventIcons = {
  simulation: <Cpu size={9} className="text-forge-cyan" />,
  optimization: <Activity size={9} className="text-forge-purple" />,
  maintenance: <Wrench size={9} className="text-forge-yellow" />,
  alert: <AlertTriangle size={9} className="text-forge-orange" />,
  'ai-decision': <Brain size={9} className="text-forge-magenta" />,
  lifecycle: <Clock size={9} className="text-forge-teal" />,
};

const severityDots = {
  info: 'bg-forge-accent',
  success: 'bg-forge-green',
  warning: 'bg-forge-yellow',
  critical: 'bg-forge-red',
};

export const TimelineBar: React.FC<Props> = ({ visible, onToggle }) => {
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
    <div className="h-[130px] bg-forge-dark border-t border-forge-border flex flex-col">
      {/* Timeline Header */}
      <div className="h-[28px] flex items-center px-3 border-b border-forge-border/50 justify-between">
        <div className="flex items-center gap-2">
          <Clock size={11} className="text-forge-accent" />
          <span className="text-[9px] font-semibold tracking-widest text-forge-text-dim">
            ENGINEERING TIMELINE
          </span>
          <span className="text-[8px] font-mono text-forge-text-muted ml-2">
            {timelineEvents.length} events
          </span>
        </div>
        <div className="flex items-center gap-1">
          <button className="p-1 hover:bg-forge-surface rounded text-forge-text-muted hover:text-forge-text-dim transition-colors">
            <SkipBack size={10} />
          </button>
          <button className="p-1 hover:bg-forge-surface rounded text-forge-accent transition-colors">
            <Play size={10} />
          </button>
          <button className="p-1 hover:bg-forge-surface rounded text-forge-text-muted hover:text-forge-text-dim transition-colors">
            <SkipForward size={10} />
          </button>
          <button className="p-1 hover:bg-forge-surface rounded text-forge-text-muted hover:text-forge-text-dim transition-colors">
            <Repeat size={10} />
          </button>
          <div className="w-px h-3 bg-forge-border mx-1" />
          <button
            onClick={onToggle}
            className="p-1 hover:bg-forge-surface rounded text-forge-text-muted hover:text-forge-text-dim transition-colors rotate-180"
          >
            <ChevronUp size={10} />
          </button>
        </div>
      </div>

      {/* Timeline Progress Bar */}
      <div className="h-[24px] px-3 flex items-center gap-2 border-b border-forge-border/30">
        <span className="text-[8px] font-mono text-forge-text-muted">14:05</span>
        <div className="flex-1 relative h-2 bg-forge-surface rounded-full overflow-hidden">
          <div className="absolute inset-y-0 left-0 bg-gradient-to-r from-forge-accent/30 to-forge-accent/60 rounded-full" style={{ width: '75%' }} />
          {/* Event markers */}
          {timelineEvents.map((ev, i) => (
            <div
              key={ev.id}
              className={`absolute top-1/2 -translate-y-1/2 w-1.5 h-1.5 rounded-full ${severityDots[ev.severity]} cursor-pointer hover:scale-150 transition-transform`}
              style={{ left: `${10 + i * 8}%` }}
              title={ev.title}
            />
          ))}
          {/* Playhead */}
          <div className="absolute top-0 bottom-0 w-0.5 bg-forge-accent" style={{ left: '75%' }}>
            <div className="absolute -top-0.5 -translate-x-1/2 w-2 h-2 bg-forge-accent rounded-full" />
          </div>
        </div>
        <span className="text-[8px] font-mono text-forge-accent">14:32</span>
      </div>

      {/* Event Stream */}
      <div className="flex-1 overflow-x-auto overflow-y-hidden">
        <div className="flex gap-1.5 p-2 h-full">
          {timelineEvents.map((event, i) => (
            <div
              key={event.id}
              className={`
                flex-shrink-0 w-[180px] p-2 rounded border transition-all cursor-pointer
                hover:border-forge-accent/30 hover:bg-forge-surface/50
                ${i === 0 ? 'border-forge-accent/20 bg-forge-accent/5' : 'border-forge-border/50 bg-forge-surface/20'}
              `}
            >
              <div className="flex items-center gap-1.5 mb-1">
                {eventIcons[event.type]}
                <span className="text-[8px] font-mono text-forge-text-muted">{event.timestamp}</span>
                <div className={`w-1 h-1 rounded-full ml-auto ${severityDots[event.severity]}`} />
              </div>
              <p className="text-[9px] text-forge-text-dim leading-tight line-clamp-2">
                {event.title}
              </p>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};
