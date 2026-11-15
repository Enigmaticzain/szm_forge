/**
 * SessionEventContext — the real project event log.
 *
 * Replaces the hard-coded `timelineEvents` demo data: engineering apps must
 * show what actually happened in this session, not a fabricated asset history.
 * Events are in-memory ring-buffered (200), newest last; each entry records
 * the wall-clock time it happened.
 *
 * Producers today: STEP import, CAD generation, USD export, limit sweeps,
 * simulation runs. Consumers: TimelineBar (and future audit/undo surfaces).
 */

import React, { createContext, useCallback, useContext, useMemo, useRef, useState } from 'react';

export type SessionEventType = 'import' | 'generate' | 'export' | 'simulation' | 'test' | 'system';
export type SessionEventSeverity = 'info' | 'success' | 'warning' | 'critical';

export interface SessionEvent {
  id: string;
  type: SessionEventType;
  severity: SessionEventSeverity;
  title: string;
  /** HH:MM wall-clock time of the event. */
  time: string;
}

interface SessionEventContextValue {
  events: SessionEvent[];
  addEvent: (type: SessionEventType, title: string, severity?: SessionEventSeverity) => void;
}

const SessionEventContext = createContext<SessionEventContextValue>({
  events: [],
  addEvent: () => {},
});

const MAX_EVENTS = 200;

let seq = 0;
function nextId(): string {
  seq += 1;
  return `ev-${seq}`;
}

export const SessionEventProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [events, setEvents] = useState<SessionEvent[]>([]);
  const bufferRef = useRef<SessionEvent[]>([]);

  const addEvent = useCallback((type: SessionEventType, title: string, severity: SessionEventSeverity = 'info') => {
    const now = new Date();
    const time = `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}`;
    const ev: SessionEvent = { id: nextId(), type, severity, title, time };
    bufferRef.current = [...bufferRef.current.slice(-(MAX_EVENTS - 1)), ev];
    setEvents(bufferRef.current);
  }, []);

  const value = useMemo(() => ({ events, addEvent }), [events, addEvent]);
  return <SessionEventContext.Provider value={value}>{children}</SessionEventContext.Provider>;
};

export function useSessionEvents(): SessionEventContextValue {
  return useContext(SessionEventContext);
}
