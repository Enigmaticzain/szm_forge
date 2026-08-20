# Plan 27: Omniverse LiveSync - Integration Guide

**Complete integration guide for real-time collaborative NVIDIA Omniverse editing**

---

## Table of Contents
1. [Quick Start](#quick-start)
2. [API Reference](#api-reference)
3. [Testing & Validation](#testing--validation)
4. [Integration Checklist](#integration-checklist)
5. [Troubleshooting](#troubleshooting)
6. [Advanced Usage](#advanced-usage)

---

## Quick Start

### Prerequisites

```bash
# Python dependencies
pip install fastapi pydantic uvicorn websockets

# C++ build (CMake 3.15+)
# Ensure ImGui headers available in include paths

# Optional: NVIDIA Omniverse SDK
pip install omniverse-client>=1.3.0
pip install omniverse-nucleus-python>=1.3.0
```

### 1. Start Python Omniverse Bridge (Port 8004)

```bash
cd /mnt/D/projects/szm_forge
python -m uvicorn ai_service.omniverse_bridge:app --host 127.0.0.1 --port 8004 --reload
```

Expected output:
```
INFO:     Uvicorn running on http://127.0.0.1:8004
```

### 2. Health Check (5 seconds)

```bash
curl http://127.0.0.1:8004/health
```

**Expected Response:**
```json
{
  "status": "healthy",
  "service": "Omniverse LiveSync Bridge",
  "version": "1.0.0",
  "active_sessions": 0,
  "total_changes": 0,
  "timestamp": "2026-08-18T..."
}
```

### 3. Create New Session (10 seconds)

```bash
curl -X POST http://127.0.0.1:8004/omniverse/connect \
  -H "Content-Type: application/json" \
  -d '{
    "nucleus_url": "http://localhost:3100",
    "username": "admin",
    "password": "password",
    "project_path": "/projects/default",
    "scene_file": "scene.usd"
  }'
```

**Expected Response:**
```json
{
  "status": "success",
  "session_id": "session_abc123def456",
  "state": "connected",
  "nucleus_url": "http://localhost:3100",
  "project_path": "/projects/default",
  "scene_file": "scene.usd"
}
```

### 4. Join Session (5 seconds)

```bash
SESSION_ID="session_abc123def456"
USER_ID="user_alice"

curl -X POST "http://127.0.0.1:8004/omniverse/join?session_id=$SESSION_ID&user_id=$USER_ID" \
  -H "Content-Type: application/json"
```

**Expected Response:**
```json
{
  "status": "success",
  "session_id": "session_abc123def456",
  "user_id": "user_alice",
  "connected_users": ["user_alice"],
  "user_count": 1
}
```

### 5. Push Scene Change (5 seconds)

```bash
curl -X POST http://127.0.0.1:8004/omniverse/push \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "session_abc123def456",
    "changes": [
      {
        "change_type": "property_changed",
        "prim_path": "/scene/cube",
        "property_name": "translate",
        "old_value": "0 0 0",
        "new_value": "1 0 0",
        "timestamp": 1692374400.5,
        "user_id": "user_alice"
      }
    ]
  }'
```

**Expected Response:**
```json
{
  "status": "success",
  "changes_accepted": 1,
  "total_changes": 1,
  "broadcasted": 0
}
```

---

## API Reference

### REST Endpoints (Port 8004)

#### Session Management

**POST /omniverse/connect** - Create new collaborative session
```bash
curl -X POST http://127.0.0.1:8004/omniverse/connect \
  -H "Content-Type: application/json" \
  -d '{
    "nucleus_url": "http://nucleus-server:3100",
    "username": "your_username",
    "password": "your_password",
    "project_path": "/projects/my_project",
    "scene_file": "main_scene.usd"
  }'
```

Response:
```json
{
  "status": "success",
  "session_id": "session_...",
  "state": "connected",
  "message": "Connected to Nucleus at ..."
}
```

**POST /omniverse/join** - User joins session
```bash
curl -X POST "http://127.0.0.1:8004/omniverse/join?session_id=session_123&user_id=user_bob"
```

**POST /omniverse/leave** - User leaves session
```bash
curl -X POST "http://127.0.0.1:8004/omniverse/leave?session_id=session_123&user_id=user_bob"
```

**GET /omniverse/session/{session_id}** - Get session info
```bash
curl http://127.0.0.1:8004/omniverse/session/session_123
```

Response:
```json
{
  "session_id": "session_123",
  "nucleus_url": "http://localhost:3100",
  "project_path": "/projects/default",
  "scene_file": "scene.usd",
  "state": "connected",
  "connected_users": ["user_alice", "user_bob"],
  "change_count": 5,
  "sync_latency_ms": 45.0,
  "created_at": "2026-08-18T10:30:00",
  "last_sync": "2026-08-18T10:30:30"
}
```

**GET /omniverse/sessions** - List all sessions
```bash
curl http://127.0.0.1:8004/omniverse/sessions
```

**DELETE /omniverse/session/{session_id}** - Close session
```bash
curl -X DELETE http://127.0.0.1:8004/omniverse/session/session_123
```

#### Change Synchronization

**POST /omniverse/push** - Push local changes to Nucleus
```bash
curl -X POST http://127.0.0.1:8004/omniverse/push \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "session_123",
    "changes": [
      {
        "change_type": "property_changed",
        "prim_path": "/Group/Cube",
        "property_name": "scale",
        "old_value": "1 1 1",
        "new_value": "2 2 2",
        "timestamp": 1692374400.5,
        "user_id": "user_alice"
      },
      {
        "change_type": "prim_created",
        "prim_path": "/Group/Sphere",
        "old_value": "null",
        "new_value": "Sphere_1",
        "timestamp": 1692374401.0,
        "user_id": "user_alice"
      }
    ]
  }'
```

Response:
```json
{
  "status": "success",
  "changes_accepted": 2,
  "total_changes": 7,
  "broadcasted": 1
}
```

**POST /omniverse/pull** - Pull remote changes
```bash
curl -X POST "http://127.0.0.1:8004/omniverse/pull?session_id=session_123"
```

Response:
```json
{
  "status": "success",
  "changes": [
    {
      "change_type": "property_changed",
      "prim_path": "/Group/Cube",
      "property_name": "translate",
      "old_value": "0 0 0",
      "new_value": "1 1 0",
      "timestamp": 1692374402.0,
      "user_id": "user_bob"
    }
  ],
  "count": 1,
  "timestamp": "2026-08-18T10:31:00"
}
```

**POST /omniverse/sync** - Force full sync
```bash
curl -X POST "http://127.0.0.1:8004/omniverse/sync?session_id=session_123"
```

#### Conflict Management

**POST /omniverse/resolve-conflict** - Resolve conflict with strategy
```bash
curl -X POST http://127.0.0.1:8004/omniverse/resolve-conflict \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "session_123",
    "conflict_id": "conflict_xyz",
    "merge_strategy": "last_write_wins"
  }'
```

Strategies:
- `last_write_wins` - Accept remote version
- `keep_local` - Keep local version
- `merge_properties` - Merge non-overlapping properties
- `manual` - Pending user resolution

**GET /omniverse/conflicts/{session_id}** - Get unresolved conflicts
```bash
curl http://127.0.0.1:8004/omniverse/conflicts/session_123
```

#### WebSocket Real-Time Sync

**WS /ws/livesync/{session_id}** - Real-time change streaming
```bash
# Using websocat or similar tool
websocat ws://127.0.0.1:8004/ws/livesync/session_123

# Send message
{"type": "property_changed", "prim_path": "/scene/cube", "property": "scale"}

# Receive broadcasts from other clients
{"type": "change_notification", "data": {...}, "timestamp": "..."}
```

---

## Testing & Validation

### Test Suite 1: Session Management

**TC-1.1: Create Session**
```bash
# Create new session
RESPONSE=$(curl -s -X POST http://127.0.0.1:8004/omniverse/connect \
  -H "Content-Type: application/json" \
  -d '{
    "nucleus_url": "http://localhost:3100",
    "username": "test_user",
    "password": "password",
    "project_path": "/projects/test",
    "scene_file": "test_scene.usd"
  }')

# Extract session ID
SESSION_ID=$(echo $RESPONSE | grep -o '"session_id":"[^"]*' | cut -d'"' -f4)

# Validation
# ✓ Response status 200
# ✓ status == "success"
# ✓ session_id matches pattern session_*
# ✓ state == "connected"
# ✓ nucleus_url preserved
```

**TC-1.2: List Sessions**
```bash
curl http://127.0.0.1:8004/omniverse/sessions

# Validation
# ✓ Returns array of sessions
# ✓ Each session has session_id, nucleus_url, state, user_count
# ✓ Count field matches actual session count
```

### Test Suite 2: User Management

**TC-2.1: Join Multiple Users**
```bash
for user in alice bob charlie; do
  curl -X POST "http://127.0.0.1:8004/omniverse/join?session_id=$SESSION_ID&user_id=user_$user" \
    -H "Content-Type: application/json"
done

# Validation
# ✓ All joins succeed with 200
# ✓ connected_users count increments
# ✓ Each join returns current user list
```

**TC-2.2: User Leaves**
```bash
curl -X POST "http://127.0.0.1:8004/omniverse/leave?session_id=$SESSION_ID&user_id=user_bob"

# Validation
# ✓ User removed from connected list
# ✓ remaining_users count decreases
```

### Test Suite 3: Change Synchronization

**TC-3.1: Push Single Change**
```bash
curl -X POST http://127.0.0.1:8004/omniverse/push \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "'$SESSION_ID'",
    "changes": [{
      "change_type": "property_changed",
      "prim_path": "/Group/Cube",
      "property_name": "scale",
      "old_value": "1 1 1",
      "new_value": "2 2 2",
      "timestamp": '$(date +%s.%N)',
      "user_id": "user_alice"
    }]
  }'

# Validation
# ✓ Response status 200
# ✓ changes_accepted == 1
# ✓ total_changes increments
```

**TC-3.2: Push Batch Changes**
```bash
# Push 10 changes at once
curl -X POST http://127.0.0.1:8004/omniverse/push \
  -H "Content-Type: application/json" \
  -d '{
    "session_id": "'$SESSION_ID'",
    "changes": [
      {"change_type": "property_changed", "prim_path": "/Group/Cube", "property_name": "translate", "old_value": "0 0 0", "new_value": "1 0 0", "timestamp": '$(date +%s.%N)', "user_id": "user_alice"},
      {"change_type": "property_changed", "prim_path": "/Group/Sphere", "property_name": "scale", "old_value": "1 1 1", "new_value": "1.5 1.5 1.5", "timestamp": '$(date +%s.%N)', "user_id": "user_bob"},
      ...
    ]
  }'

# Validation
# ✓ All changes accepted
# ✓ changes_accepted == 10
# ✓ No loss of data
```

**TC-3.3: Pull Changes**
```bash
curl -X POST "http://127.0.0.1:8004/omniverse/pull?session_id=$SESSION_ID"

# Validation
# ✓ Returns array of changes
# ✓ Changes cleared after pull
# ✓ Subsequent pull returns empty array
```

### Test Suite 4: Conflict Resolution

**TC-4.1: Detect Conflicts**
```bash
# Push conflicting changes from two users
curl -X POST http://127.0.0.1:8004/omniverse/push \
  -d '{"session_id": "'$SESSION_ID'", "changes": [
    {"change_type": "property_changed", "prim_path": "/Group/Cube", "property_name": "scale", "new_value": "2 2 2", "user_id": "user_alice"},
    {"change_type": "property_changed", "prim_path": "/Group/Cube", "property_name": "scale", "new_value": "3 3 3", "user_id": "user_bob"}
  ]}'

# Get conflicts
curl http://127.0.0.1:8004/omniverse/conflicts/$SESSION_ID

# Validation
# ✓ Conflicts detected for same prim+property
# ✓ conflict_id assigned
# ✓ local_change and remote_change stored
```

**TC-4.2: Resolve with Different Strategies**
```bash
for strategy in "last_write_wins" "keep_local" "merge_properties"; do
  curl -X POST http://127.0.0.1:8004/omniverse/resolve-conflict \
    -d '{
      "session_id": "'$SESSION_ID'",
      "conflict_id": "conflict_xyz",
      "merge_strategy": "'$strategy'"
    }'
done

# Validation
# ✓ All strategies succeed
# ✓ Conflict removed from list
# ✓ Appropriate change applied
```

### Test Suite 5: WebSocket Real-Time Streaming

**TC-5.1: WebSocket Connection**
```bash
# Connect to WebSocket
websocat ws://127.0.0.1:8004/ws/livesync/$SESSION_ID &
WS_PID=$!

# Wait for connection
sleep 1

# Send change
curl -X POST http://127.0.0.1:8004/omniverse/push \
  -d '{"session_id": "'$SESSION_ID'", "changes": [...]}'

# Validation
# ✓ WebSocket connects without error
# ✓ Message received in WebSocket (broadcast)
# ✓ Connection remains open

# Cleanup
kill $WS_PID
```

### Test Suite 6: Performance & Load

**TC-6.1: Concurrent Users**
```bash
# Simulate 5 concurrent users
for i in {1..5}; do
  (
    curl -X POST "http://127.0.0.1:8004/omniverse/join?session_id=$SESSION_ID&user_id=user_load_$i" &
    sleep 0.1
  )
done

# Validation
# ✓ All joins succeed
# ✓ No race conditions
# ✓ User count == 5
```

**TC-6.2: High-Frequency Changes**
```bash
# Send 100 changes rapidly
for i in {1..100}; do
  curl -X POST http://127.0.0.1:8004/omniverse/push \
    -d '{"session_id": "'$SESSION_ID'", "changes": [{
      "change_type": "property_changed",
      "prim_path": "/scene/cube",
      "property_name": "position_x",
      "new_value": "'$i'",
      "timestamp": '$(date +%s.%N)',
      "user_id": "user_perf"
    }]}' &
done
wait

# Validation
# ✓ All changes accepted
# ✓ No dropped messages
# ✓ Response time < 100ms per change
```

---

## Integration Checklist

### Phase 1: Python Backend (✅ Complete)
- [x] Design omniverse_bridge.py API
- [x] Implement session management endpoints
- [x] Implement change sync endpoints
- [x] Implement WebSocket streaming
- [x] Implement conflict resolution endpoints
- [x] Error handling and validation
- [x] In-memory session storage

### Phase 2: C++ Engine (✅ Complete)
- [x] Design OmniverseLiveSync orchestrator
- [x] Implement NucleusConnector wrapper
- [x] Implement LiveSyncSession state management
- [x] Implement ChangeBuffer for delta compression
- [x] Implement ConflictResolver with merge strategies
- [x] Singleton pattern implementations
- [x] Thread-safe synchronization (mutex locks)

### Phase 3: React & ImGui (✅ Complete)
- [x] Implement useOmniverseAPI React hook
- [x] Implement OmniverseLiveSyncPanel React component
- [x] Implement ImGuiOmniverseLiveSyncPanel header
- [x] Implement ImGuiOmniverseLiveSyncPanel implementation
- [x] 5 tabbed interfaces (Connect, Session, Sync, Conflicts, Settings)
- [x] WebSocket subscription management

### Phase 4: Testing & Documentation
- [ ] Run TC-1.1 through TC-6.2 test suites
- [ ] Compile C++ backend (zero errors)
- [ ] Verify React/ImGui TypeScript compilation
- [ ] End-to-end session lifecycle test
- [ ] Multi-user conflict resolution test
- [ ] Performance benchmarking
- [ ] Documentation review
- [ ] Production deployment preparation

---

## Troubleshooting

### Issue: "Connection refused" on http://127.0.0.1:8004

**Cause:** Python server not running

**Solution:**
```bash
# Start server
python -m uvicorn ai_service.omniverse_bridge:app --host 127.0.0.1 --port 8004

# Verify with health check
curl http://127.0.0.1:8004/health
```

### Issue: WebSocket connection fails

**Cause:** Server running but WebSocket endpoint not ready

**Solution:**
```bash
# Ensure server is fully initialized
sleep 2

# Test with simpler curl first
curl http://127.0.0.1:8004/omniverse/sessions

# Then try WebSocket
websocat ws://127.0.0.1:8004/ws/livesync/session_123
```

### Issue: Session not found errors

**Cause:** Using wrong session ID

**Solution:**
```bash
# Get active sessions first
curl http://127.0.0.1:8004/omniverse/sessions

# Use returned session_id from response
curl http://127.0.0.1:8004/omniverse/session/session_ABC123
```

### Issue: Changes not broadcasting to WebSocket clients

**Cause:** WebSocket clients not connected when push occurs

**Solution:**
1. Ensure WebSocket client connects BEFORE pushing changes
2. Use Pull endpoint if WebSocket unavailable
3. Check browser console for connection errors

### Issue: Conflicts not detected

**Cause:** Changes don't overlap (different properties or prims)

**Solution:**
- Conflicts only detected for simultaneous edits on SAME prim + property
- Test with changes to `/Group/Cube.scale` from multiple users

### Issue: React/ImGui components not rendering

**Cause:** Missing dependencies or API not ready

**Solution:**
```bash
# Verify Python server running on 8004
curl http://127.0.0.1:8004/health

# Check browser console for CORS errors
# Ensure useOmniverseAPI hook initialized

# For ImGui: Ensure render() called in main loop
```

---

## Advanced Usage

### Custom Change Types

The system supports any change_type string:
```bash
{
  "change_type": "custom_animation_keyframe",
  "prim_path": "/animation/character",
  "property_name": "frame_0001",
  "new_value": "position: (1,0,0)"
}
```

### Three-Way Merge

For complex merges with ancestor version:
```cpp
auto merged = conflict_resolver.ThreeWayMerge(
    ancestor_changes,
    local_changes,
    remote_changes
);
```

### Change Compression

The C++ ChangeBuffer automatically compresses changes:
```cpp
ChangeBuffer buffer;
buffer.AddChange(change1);
buffer.AddChange(change2);  // Same property
auto compressed = buffer.Compress();
// Result: only change2 (latest)
```

### Async Session Operations

All operations are designed for async/await:
```typescript
try {
  const result = await connectToNucleus(...);
  await joinSession(result.session_id, userId);
} catch (error) {
  console.error('Failed:', error);
}
```

---

## Performance Optimization

### Tuning Parameters

```python
# In omniverse_bridge.py
BATCH_SIZE = 100  # Batch changes before sending
COMPRESSION_LEVEL = 6  # 0-9, higher = smaller but slower
CACHE_SIZE = 1000  # Recent changes to keep in memory
```

### C++ Optimization

```cpp
// Enable change compression
ChangeBuffer buffer;
auto compressed = buffer.Compress();

// Use three-way merge for complex conflicts
auto merged = resolver.ThreeWayMerge(ancestor, local, remote);
```

### Network Optimization

- Use WebSocket for real-time updates (lower latency)
- Batch changes into requests (fewer HTTP calls)
- Enable gzip compression for payloads

---

## Next Steps

1. **Verify Setup**: Run all Quick Start steps (1-5)
2. **Run Tests**: Execute TC-1.1 through TC-6.2
3. **C++ Integration**: Compile and link OmniverseLiveSync.*
4. **React Integration**: Add OmniverseLiveSyncPanel to main app
5. **ImGui Integration**: Add OmniverseLiveSyncPanel to desktop window
6. **End-to-End Test**: Multi-user collaborative session
7. **Production Deployment**: Configure for production environment

---

## Files Reference

- `ai_service/omniverse_bridge.py` - Python FastAPI server (550+ LOC)
- `src/Omniverse/OmniverseLiveSync.hpp/.cpp` - Main orchestrator (550 LOC)
- `src/Omniverse/NucleusConnector.hpp/.cpp` - Nucleus integration (320 LOC)
- `src/Omniverse/LiveSyncSession.hpp/.cpp` - Session state (330 LOC)
- `src/Omniverse/ChangeBuffer.hpp/.cpp` - Change compression (170 LOC)
- `src/Omniverse/ConflictResolver.hpp/.cpp` - Conflict merging (220 LOC)
- `webui/src/hooks/useOmniverseAPI.ts` - React hook (280 LOC)
- `webui/src/components/OmniverseLiveSyncPanel.tsx` - React UI (420 LOC)
- `src/UI/panels/OmniverseLiveSyncPanel.hpp/.cpp` - ImGui panels (360 LOC)

**Total: 3,200+ LOC of production code**

---

## Support & Documentation

- Implementation Summary: `PLAN_27_IMPLEMENTATION_SUMMARY.md`
- API Docs: Auto-generated at `/docs` endpoint
- Quick Test: `/health` endpoint
- Examples: See Test Suites above

