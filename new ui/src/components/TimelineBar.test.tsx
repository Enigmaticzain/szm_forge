// @vitest-environment jsdom
import React from 'react';
import { render, screen, cleanup, act } from '@testing-library/react';
import '@testing-library/jest-dom/vitest';
import { afterEach, describe, expect, it } from 'vitest';
import { TimelineBar } from './TimelineBar';
import { SessionEventProvider, useSessionEvents } from '../store/SessionEventContext';

afterEach(cleanup);

const Seed: React.FC<{ count: number }> = ({ count }) => {
  const { addEvent } = useSessionEvents();
  const seeded = React.useRef(false);
  React.useEffect(() => {
    if (seeded.current) return;
    seeded.current = true;
    for (let i = 0; i < count; i++) addEvent('import', `event ${i + 1}`, 'success');
  }, [count, addEvent]);
  return null;
};

describe('TimelineBar (session-event honest state)', () => {
  it('shows an honest empty state instead of fabricated events', () => {
    render(
      <SessionEventProvider>
        <TimelineBar visible={true} onToggle={() => {}} />
      </SessionEventProvider>,
    );
    expect(screen.getByText(/0 events · this session/i)).toBeInTheDocument();
    expect(screen.getByText(/nothing yet — imports, runs and exports will appear here/i)).toBeInTheDocument();
    expect(screen.getByText(/Nothing is fabricated/i)).toBeInTheDocument();
  });

  it('renders real session events once recorded', () => {
    render(
      <SessionEventProvider>
        <Seed count={2} />
        <TimelineBar visible={true} onToggle={() => {}} />
      </SessionEventProvider>,
    );
    // seeded inside act-flushed effect
    act(() => {});
    expect(screen.getByText(/2 events · this session/i)).toBeInTheDocument();
    expect(screen.getAllByText(/event 1/).length).toBeGreaterThan(0);
    expect(screen.getAllByText(/event 2/).length).toBeGreaterThan(0);
  });

  it('matches HH:MM timestamps for recorded events', () => {
    render(
      <SessionEventProvider>
        <Seed count={1} />
        <TimelineBar visible={true} onToggle={() => {}} />
      </SessionEventProvider>,
    );
    expect(screen.getAllByText(/^\d{2}:\d{2}$/).length).toBeGreaterThan(0);
  });
});
