# Implementation Plan 10: Digital Twin Telemetry - Integration Guide

## Overview

This document provides a comprehensive guide to integrating and testing the Digital Twin Telemetry system implemented in Plan 10.

## Architecture

### Components Implemented

1. **C++ Telemetry Engine** (`src/DigitalTwin/SensorTelemetry.hpp/cpp`)
   - Enhanced with data persistence, statistics, and export capabilities
   - Supports alert thresholds and data callbacks
   - Manages multiple telemetry streams with configurable history limits

2. **Python REST API** (`ai_service/external_engine_bridge.py`)
   - FastAPI-based telemetry ingestion layer
   - 9 new endpoints for stream management, data ingestion, querying, and export
   - In-memory data storage (production should use database)

3. **React Frontend** (`webui/src/components/` and `webui/src/hooks/`)
   - `useTelemetryAPI.ts` - Custom hook for API integration
   - Enhanced `TelemetryPanel.tsx` - Supports both demo and live data
   - `TelemetryStreamManager.tsx` - Stream creation and management UI

4. **ImGui Desktop UI** (`src/UI/panels/TelemetryPanel.hpp/cpp`)
   - Tabbed interface for stream management
   - Data visualization with time-series plots
   - Export functionality (JSON/CSV)

## Integration Steps

### Step 1: Build the C++ Components

```bash
cd /mnt/D/projects/szm_forge
cmake . -B build
cmake --build build --config Release
```

### Step 2: Start the Python Telemetry Service

```bash
cd /mnt/D/projects/szm_forge/ai_service
python -m pip install fastapi uvicorn pydantic
python external_engine_bridge.py
# Server should start on http://127.0.0.1:8003
```

### Step 3: Verify API Endpoints

All telemetry endpoints are available at `http://127.0.0.1:8003`:

#### Stream Management
- `POST /telemetry/stream/create` - Create a new stream
- `GET /telemetry/streams` - List all streams
- `GET /telemetry/stream/{stream_id}` - Get stream status

#### Data Operations
- `POST /telemetry/ingest` - Ingest sensor data
- `POST /telemetry/query` - Query historical data
- `POST /telemetry/export/json/{stream_id}` - Export as JSON
- `POST /telemetry/export/csv/{stream_id}` - Export as CSV

#### System Management
- `POST /telemetry/alert/set` - Set alert thresholds
- `GET /telemetry/health` - Get system health status

## Testing

### Test 1: Create and Ingest Data

```bash
# Create a stream
curl -X POST http://127.0.0.1:8003/telemetry/stream/create \
  -H "Content-Type: application/json" \
  -d '{"stream_id": "test-machine-01", "asset_uuid": "asset-12345"}'

# Ingest data points
for i in {1..10}; do
  curl -X POST http://127.0.0.1:8003/telemetry/ingest \
    -H "Content-Type: application/json" \
    -d "{\"stream_id\": \"test-machine-01\", \"sensor_type\": \"temperature\", \"value\": $((70 + RANDOM % 30))}"
  sleep 1
done
```

### Test 2: Query Data

```bash
# Query last 5 minutes of data
curl -X POST http://127.0.0.1:8003/telemetry/query \
  -H "Content-Type: application/json" \
  -d '{"stream_id": "test-machine-01", "sensor_type": "temperature", "time_window_seconds": 300}'
```

### Test 3: Export Data

```bash
# Export as JSON
curl -X POST http://127.0.0.1:8003/telemetry/export/json/test-machine-01 \
  -o telemetry_export.json

# Export as CSV
curl -X POST http://127.0.0.1:8003/telemetry/export/csv/test-machine-01 \
  -o telemetry_export.csv
```

### Test 4: React Frontend Integration

1. Start the React development server:
   ```bash
   cd /mnt/D/projects/szm_forge/webui
   npm start
   ```

2. In your React component, use the telemetry hook:
   ```typescript
   import { useTelemetryAPI } from './hooks/useTelemetryAPI';
   
   function MyComponent() {
     const { streams, createStream, ingestData } = useTelemetryAPI();
     
     // Use the API methods
   }
   ```

3. The enhanced `TelemetryPanel` will automatically connect to live data if a `streamId` prop is provided:
   ```typescript
   <TelemetryPanel sensors={mockSensors} streamId="test-machine-01" />
   ```

### Test 5: ImGui Desktop Panel

1. Compile with the telemetry panel enabled:
   ```bash
   cd /mnt/D/projects/szm_forge/build
   cmake --build . --config Release
   ```

2. Launch the application and look for the "Telemetry Manager" panel
   - Create streams through the ImGui interface
   - View statistics and export data
   - Visualize time-series data

## API Reference

### POST /telemetry/stream/create

Create a new telemetry stream for a digital twin asset.

**Request:**
```json
{
  "stream_id": "machine-01",
  "asset_uuid": "asset-uuid-12345"
}
```

**Response:**
```json
{
  "status": "success",
  "stream_id": "machine-01",
  "message": "Telemetry stream created"
}
```

### POST /telemetry/ingest

Ingest sensor data into a stream.

**Request:**
```json
{
  "stream_id": "machine-01",
  "sensor_type": "temperature",
  "value": 75.5,
  "timestamp": 1692374400.0
}
```

**Response:**
```json
{
  "status": "success",
  "stream_id": "machine-01",
  "data_point_count": 42
}
```

### POST /telemetry/query

Query historical telemetry data.

**Request:**
```json
{
  "stream_id": "machine-01",
  "sensor_type": "temperature",
  "start_time": 1692374000,
  "end_time": 1692375000,
  "time_window_seconds": 300
}
```

**Response:**
```json
{
  "status": "success",
  "stream_id": "machine-01",
  "data_point_count": 25,
  "data_points": [
    {"timestamp": 1692374400, "sensor_type": "temperature", "value": 75.5},
    ...
  ],
  "statistics": {
    "count": 25,
    "min": 72.1,
    "max": 78.9,
    "avg": 75.3,
    "stdev": 1.8
  }
}
```

### GET /telemetry/streams

List all active telemetry streams.

**Response:**
```json
{
  "status": "success",
  "stream_count": 2,
  "streams": [
    {
      "stream_id": "machine-01",
      "asset_uuid": "asset-uuid-12345",
      "data_points": 1250,
      "latest_update": 1692375400.5
    }
  ]
}
```

### POST /telemetry/alert/set

Configure alert thresholds for a sensor.

**Request:**
```json
{
  "stream_id": "machine-01",
  "sensor_type": "temperature",
  "warning_low": 50,
  "warning_high": 90,
  "critical_low": 30,
  "critical_high": 110
}
```

### GET /telemetry/health

Get telemetry system health status.

**Response:**
```json
{
  "status": "healthy",
  "total_streams": 5,
  "total_data_points": 12450,
  "active_alert_configs": 3,
  "system_uptime": 86400.5
}
```

## Features Implemented

### Core Telemetry Ingestion
- ✅ Stream creation and management
- ✅ Multi-sensor type support
- ✅ Timestamp-based data points
- ✅ Configurable history limits
- ✅ Automatic data pruning

### Data Analysis
- ✅ Statistical calculations (min, max, avg, stddev)
- ✅ Time-window queries
- ✅ Sensor type filtering
- ✅ Real-time data aggregation

### Data Management
- ✅ JSON export with full metadata
- ✅ CSV export for analysis
- ✅ Persistent storage support
- ✅ Alert threshold configuration

### Frontend Integration
- ✅ React hooks for API access
- ✅ Real-time data streaming
- ✅ Interactive stream management
- ✅ ImGui desktop visualization

### System Features
- ✅ WebSocket-ready callback system
- ✅ Memory-efficient data storage
- ✅ Comprehensive error handling
- ✅ Logging and diagnostics

## Performance Considerations

1. **Data Storage**: Each stream keeps up to 10,000 data points in memory
2. **Query Speed**: O(n) for time-range queries; optimize with indexing in production
3. **Export Size**: Large datasets may take time to generate; consider pagination
4. **API Rate**: No built-in rate limiting; implement in reverse proxy for production

## Production Deployment

For production use, consider:

1. **Database Integration**
   - Replace in-memory storage with PostgreSQL or similar
   - Implement connection pooling
   - Add data retention policies

2. **Message Queue**
   - Add Kafka/RabbitMQ for high-throughput ingestion
   - Implement consumer groups for processing

3. **Caching Layer**
   - Add Redis for frequently accessed data
   - Implement cache invalidation strategy

4. **Security**
   - Add authentication (JWT tokens)
   - Implement role-based access control
   - Add rate limiting

5. **Monitoring**
   - Integrate with Prometheus for metrics
   - Add distributed tracing (Jaeger)
   - Set up alerts for anomalies

## Troubleshooting

### API Connection Issues
- Verify server is running: `curl http://127.0.0.1:8003/telemetry/health`
- Check firewall settings
- Review application logs

### No Data in Streams
- Verify stream was created: `GET /telemetry/streams`
- Check timestamp format (should be Unix seconds)
- Verify sensor_type spelling matches

### Export File Issues
- Check file permissions on export directory
- Ensure disk space is available
- Verify directory exists

## Next Steps

1. **Database Integration**: Migrate to persistent storage
2. **Real-time Sync**: Implement WebSocket support for live streaming
3. **Advanced Analytics**: Add anomaly detection and forecasting
4. **Dashboard**: Create comprehensive visualization dashboard
5. **Alerts**: Implement notification system (email, Slack)

## Support

For issues or questions:
- Check application logs in `./logs/` directory
- Review API response error messages
- Consult the architecture documentation
- Check codebase comments and examples

---

**Last Updated**: 2026-08-17
**Implementation Status**: ✅ Complete (Plan 10)
