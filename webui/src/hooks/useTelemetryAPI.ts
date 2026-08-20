import { useState, useEffect, useCallback } from 'react';

export interface TelemetryStream {
  stream_id: string;
  asset_uuid: string;
  data_points: number;
  latest_update: number;
}

export interface TelemetryDataPoint {
  timestamp: number;
  sensor_type: string;
  value: number;
}

export interface TelemetryStatistics {
  count: number;
  min: number;
  max: number;
  avg: number;
  stdev: number;
}

const TELEMETRY_API_BASE = 'http://127.0.0.1:8003';

/**
 * Hook for interacting with the live Digital Twin telemetry API
 */
export function useTelemetryAPI() {
  const [streams, setStreams] = useState<TelemetryStream[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);

  // Fetch all active telemetry streams
  const fetchStreams = useCallback(async () => {
    try {
      setLoading(true);
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/streams`);
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const data = await response.json();
      if (data.status === 'success') {
        setStreams(data.streams || []);
        setError(null);
      } else {
        setError(data.error || 'Unknown error');
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to fetch streams');
    } finally {
      setLoading(false);
    }
  }, []);

  // Create a new telemetry stream
  const createStream = useCallback(async (streamId: string, assetUUID: string) => {
    try {
      setLoading(true);
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/stream/create`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ stream_id: streamId, asset_uuid: assetUUID }),
      });
      const data = await response.json();
      if (data.status === 'success') {
        await fetchStreams();
        setError(null);
        return true;
      } else {
        setError(data.error || 'Failed to create stream');
        return false;
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Failed to create stream');
      return false;
    } finally {
      setLoading(false);
    }
  }, [fetchStreams]);

  // Ingest telemetry data
  const ingestData = useCallback(async (
    streamId: string,
    sensorType: string,
    value: number,
    timestamp?: number
  ) => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/ingest`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          stream_id: streamId,
          sensor_type: sensorType,
          value,
          timestamp: timestamp || Date.now() / 1000,
        }),
      });
      const data = await response.json();
      return data.status === 'success';
    } catch (err) {
      console.error('Failed to ingest telemetry data:', err);
      return false;
    }
  }, []);

  // Query telemetry data
  const queryData = useCallback(async (
    streamId: string,
    options?: {
      sensorType?: string;
      startTime?: number;
      endTime?: number;
      timeWindowSeconds?: number;
    }
  ): Promise<{ dataPoints: TelemetryDataPoint[]; statistics?: TelemetryStatistics } | null> => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/query`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          stream_id: streamId,
          sensor_type: options?.sensorType,
          start_time: options?.startTime,
          end_time: options?.endTime,
          time_window_seconds: options?.timeWindowSeconds,
        }),
      });
      const data = await response.json();
      if (data.status === 'success') {
        return {
          dataPoints: data.data_points || [],
          statistics: data.statistics,
        };
      }
      return null;
    } catch (err) {
      console.error('Failed to query telemetry data:', err);
      return null;
    }
  }, []);

  // Get stream status
  const getStreamStatus = useCallback(async (streamId: string) => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/stream/${streamId}`);
      const data = await response.json();
      if (data.status === 'success') {
        return data.stream;
      }
      return null;
    } catch (err) {
      console.error('Failed to get stream status:', err);
      return null;
    }
  }, []);

  // Export telemetry as JSON
  const exportJSON = useCallback(async (streamId: string): Promise<string | null> => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/export/json/${streamId}`, {
        method: 'POST',
      });
      const data = await response.json();
      if (data.status === 'success') {
        return data.json_data;
      }
      return null;
    } catch (err) {
      console.error('Failed to export telemetry as JSON:', err);
      return null;
    }
  }, []);

  // Export telemetry as CSV
  const exportCSV = useCallback(async (streamId: string): Promise<string | null> => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/export/csv/${streamId}`, {
        method: 'POST',
      });
      const data = await response.json();
      if (data.status === 'success') {
        return data.csv_data;
      }
      return null;
    } catch (err) {
      console.error('Failed to export telemetry as CSV:', err);
      return null;
    }
  }, []);

  // Set alert threshold
  const setAlertThreshold = useCallback(async (
    streamId: string,
    sensorType: string,
    options: {
      warningLow?: number;
      warningHigh?: number;
      criticalLow?: number;
      criticalHigh?: number;
    }
  ) => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/alert/set`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          stream_id: streamId,
          sensor_type: sensorType,
          warning_low: options.warningLow,
          warning_high: options.warningHigh,
          critical_low: options.criticalLow,
          critical_high: options.criticalHigh,
        }),
      });
      const data = await response.json();
      return data.status === 'success';
    } catch (err) {
      console.error('Failed to set alert threshold:', err);
      return false;
    }
  }, []);

  // Get system health
  const getHealth = useCallback(async () => {
    try {
      const response = await fetch(`${TELEMETRY_API_BASE}/telemetry/health`);
      const data = await response.json();
      return data.status === 'healthy' ? data : null;
    } catch (err) {
      console.error('Failed to get telemetry health:', err);
      return null;
    }
  }, []);

  // Auto-fetch streams on mount
  useEffect(() => {
    fetchStreams();
    const interval = setInterval(fetchStreams, 5000); // Refresh every 5 seconds
    return () => clearInterval(interval);
  }, [fetchStreams]);

  return {
    streams,
    error,
    loading,
    fetchStreams,
    createStream,
    ingestData,
    queryData,
    getStreamStatus,
    exportJSON,
    exportCSV,
    setAlertThreshold,
    getHealth,
  };
}
