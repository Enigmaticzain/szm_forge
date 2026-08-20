import React, { useState, useEffect } from 'react';
import { useTelemetryAPI, TelemetryStream, TelemetryDataPoint } from '../hooks/useTelemetryAPI';

interface TelemetryStreamManagerProps {
  onStreamSelect?: (streamId: string) => void;
}

export function TelemetryStreamManager({ onStreamSelect }: TelemetryStreamManagerProps) {
  const { streams, error, loading, createStream, getStreamStatus, exportJSON, exportCSV } = useTelemetryAPI();
  const [newStreamId, setNewStreamId] = useState('');
  const [newAssetUUID, setNewAssetUUID] = useState('');
  const [selectedStream, setSelectedStream] = useState<string | null>(null);
  const [streamDetails, setStreamDetails] = useState<any>(null);

  const handleCreateStream = async () => {
    if (!newStreamId || !newAssetUUID) return;
    await createStream(newStreamId, newAssetUUID);
    setNewStreamId('');
    setNewAssetUUID('');
  };

  const handleSelectStream = async (streamId: string) => {
    setSelectedStream(streamId);
    const details = await getStreamStatus(streamId);
    setStreamDetails(details);
    onStreamSelect?.(streamId);
  };

  const handleExportJSON = async (streamId: string) => {
    const data = await exportJSON(streamId);
    if (data) {
      const blob = new Blob([data], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `${streamId}_telemetry.json`;
      a.click();
      URL.revokeObjectURL(url);
    }
  };

  const handleExportCSV = async (streamId: string) => {
    const data = await exportCSV(streamId);
    if (data) {
      const blob = new Blob([data], { type: 'text/csv' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `${streamId}_telemetry.csv`;
      a.click();
      URL.revokeObjectURL(url);
    }
  };

  return (
    <div className="h-full flex flex-col bg-forge-surface">
      {/* Header */}
      <div className="px-4 py-3 border-b border-forge-border">
        <h2 className="text-sm font-bold text-forge-text tracking-wider">TELEMETRY STREAMS</h2>
        <p className="text-xs text-forge-text-muted mt-1">Digital Twin Live Data Sync</p>
      </div>

      {/* Create Stream Form */}
      <div className="px-4 py-3 border-b border-forge-border space-y-2">
        <div>
          <label className="text-xs text-forge-text-dim">Stream ID</label>
          <input
            type="text"
            value={newStreamId}
            onChange={(e) => setNewStreamId(e.target.value)}
            placeholder="e.g., machine-01"
            className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text placeholder-forge-text-muted"
          />
        </div>
        <div>
          <label className="text-xs text-forge-text-dim">Asset UUID</label>
          <input
            type="text"
            value={newAssetUUID}
            onChange={(e) => setNewAssetUUID(e.target.value)}
            placeholder="e.g., uuid-12345..."
            className="w-full px-2 py-1 mt-1 bg-forge-input border border-forge-border rounded text-xs text-forge-text placeholder-forge-text-muted"
          />
        </div>
        <button
          onClick={handleCreateStream}
          disabled={!newStreamId || !newAssetUUID || loading}
          className="w-full px-3 py-2 bg-forge-accent text-forge-text text-xs font-bold rounded hover:bg-forge-accent-hover disabled:opacity-50"
        >
          {loading ? 'Creating...' : 'Create Stream'}
        </button>
      </div>

      {/* Error Message */}
      {error && (
        <div className="px-4 py-2 bg-red-500/10 border-b border-red-500/20">
          <p className="text-xs text-red-400">{error}</p>
        </div>
      )}

      {/* Streams List */}
      <div className="flex-1 overflow-y-auto px-2 py-2">
        {loading && !streams.length ? (
          <div className="text-xs text-forge-text-muted p-4">Loading streams...</div>
        ) : streams.length === 0 ? (
          <div className="text-xs text-forge-text-muted p-4">No streams created yet</div>
        ) : (
          <div className="space-y-2">
            {streams.map((stream) => (
              <div
                key={stream.stream_id}
                className={`p-3 rounded border cursor-pointer transition-all ${
                  selectedStream === stream.stream_id
                    ? 'bg-forge-accent/20 border-forge-accent'
                    : 'bg-forge-panel border-forge-border hover:border-forge-accent'
                }`}
                onClick={() => handleSelectStream(stream.stream_id)}
              >
                <div className="flex items-center justify-between mb-2">
                  <div className="flex items-center gap-2">
                    <div className="w-2 h-2 rounded-full bg-forge-green animate-pulse"></div>
                    <span className="text-xs font-bold text-forge-text">{stream.stream_id}</span>
                  </div>
                  <span className="text-[10px] text-forge-text-muted font-mono">{stream.data_points} pts</span>
                </div>
                <p className="text-[10px] text-forge-text-muted truncate mb-2">UUID: {stream.asset_uuid}</p>
                <div className="flex gap-1">
                  <button
                    onClick={(e) => {
                      e.stopPropagation();
                      handleExportJSON(stream.stream_id);
                    }}
                    className="flex-1 px-2 py-1 bg-forge-border text-forge-text-muted text-[9px] rounded hover:bg-forge-accent/20"
                  >
                    JSON
                  </button>
                  <button
                    onClick={(e) => {
                      e.stopPropagation();
                      handleExportCSV(stream.stream_id);
                    }}
                    className="flex-1 px-2 py-1 bg-forge-border text-forge-text-muted text-[9px] rounded hover:bg-forge-accent/20"
                  >
                    CSV
                  </button>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>

      {/* Stream Details */}
      {selectedStream && streamDetails && (
        <div className="px-4 py-3 border-t border-forge-border bg-forge-panel text-xs space-y-1">
          <div className="flex justify-between">
            <span className="text-forge-text-dim">Created:</span>
            <span className="text-forge-text-muted font-mono">
              {new Date(streamDetails.created_at * 1000).toLocaleTimeString()}
            </span>
          </div>
          <div className="flex justify-between">
            <span className="text-forge-text-dim">Latest Update:</span>
            <span className="text-forge-text-muted font-mono">
              {streamDetails.latest_update ? new Date(streamDetails.latest_update * 1000).toLocaleTimeString() : 'N/A'}
            </span>
          </div>
        </div>
      )}
    </div>
  );
}

export default TelemetryStreamManager;
