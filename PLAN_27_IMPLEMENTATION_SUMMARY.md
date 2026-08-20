# Plan 27: Omniverse LiveSync - Implementation Summary

**Real-time collaborative editing with NVIDIA Omniverse Nucleus Server**

---

## Table of Contents
1. [Architecture Overview](#architecture-overview)
2. [Technical Stack](#technical-stack)
3. [System Components](#system-components)
4. [API Specification](#api-specification)
5. [File Structure](#file-structure)
6. [Implementation Phases](#implementation-phases)
7. [Testing Strategy](#testing-strategy)

---

## Architecture Overview

### Three-Layer Architecture

```
┌─────────────────────────────────────────────────────┐
│  User Interface Layer                               │
├─────────────────────────────────────────────────────┤
│  • React: OmniverseLiveSyncPanel.tsx                │
│  • Hook: useOmniverseAPI.ts                         │
│  • ImGui: OmniverseLiveSyncPanel.hpp/cpp            │
│  • WebSocket Manager: LiveSyncManager.tsx           │
└──────────────────┬──────────────────────────────────┘
                   │ WebSocket + REST API
┌──────────────────▼──────────────────────────────────┐
│  Python API Bridge Layer (Port 8004)                │
├─────────────────────────────────────────────────────┤
│  • FastAPI server: omniverse_bridge.py              │
│  • WebSocket endpoints: /ws/livesync/{session_id}   │
│  • REST endpoints: /omniverse/...                   │
│  • Nucleus client: NucleusClient wrapper            │
│  • Session manager: OmniverseSessionManager         │
│  • Change delta tracker: ChangeTracker              │
└──────────────────┬──────────────────────────────────┘
                   │ Nucleus Connector (C++)
┌──────────────────▼──────────────────────────────────┐
│  C++ LiveSync Engine Layer                          │
├─────────────────────────────────────────────────────┤
│  • OmniverseLiveSync.hpp/cpp - Main orchestrator    │
│  • NucleusConnector.hpp/cpp - Nucleus API wrapper   │
│  • LiveSyncSession.hpp/cpp - Session management     │
│  • ChangeBuffer.hpp/cpp - Delta compression         │
│  • ConflictResolver.hpp/cpp - Merge strategies      │
│  • ExternalEngineManager integration                │
└──────────────────┬──────────────────────────────────┘
                   │ gRPC/HTTP
┌──────────────────▼──────────────────────────────────┐
│  NVIDIA Omniverse Nucleus Server                    │
├─────────────────────────────────────────────────────┤
│  • Remote scene: project:/model/scene.usd           │
│  • Layers: Layers, Properties, References           │
│  • Real-time updates: WebSocket subscriptions       │
└─────────────────────────────────────────────────────┘
```

### Session Workflow

```
User Creates Session
        ↓
   Connect to Nucleus
        ↓
   Open Scene (USD)
        ↓
   Subscribe to Changes
        ↓
   Load Remote State
        ↓
   Local Edits ← → Broadcast to Others
        ↓
   Conflict Detection
        ↓
   Merge/Resolve
        ↓
   Persist to Nucleus
        ↓
   Sync Complete State
```

---

## Technical Stack

### Languages & Frameworks
- **C++17** - Core LiveSync engine, session management
- **Python 3.8+** - FastAPI bridge, Nucleus client
- **TypeScript/React 18** - Real-time UI updates
- **WebSockets** - Bidirectional sync
- **ImGui** - Desktop panel

### External Dependencies
- **NVIDIA Omniverse Python SDK v1.3+**
  ```bash
  pip install omniverse-client>=1.3.0
  pip install omniverse-nucleus-python>=1.3.0
  ```
- **USD Core** (already in project)
- **WebSocket Library** - Python `websockets` + C++ `websocketpp`
- **gRPC** (optional) - For high-performance Nucleus communication

### Key Libraries
```cpp
// C++ Libraries
#include <vector>
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>

// USD Libraries
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/sdf/data.h>
```

---

## System Components

### 1. C++ LiveSync Engine (750 LOC)

#### OmniverseLiveSync.hpp (180 lines)
```cpp
namespace SZM::Omniverse {

// Session states
enum class SessionState {
    DISCONNECTED,     // 0
    CONNECTING,       // 1
    CONNECTED,        // 2
    SYNCING,          // 3
    CONFLICT,         // 4
    ERROR             // 5
};

// Change types
enum class ChangeType {
    PROPERTY_CHANGED,
    PRIM_CREATED,
    PRIM_DELETED,
    RELATIONSHIP_CHANGED,
    METADATA_CHANGED,
    LAYER_MUTED,
    LAYER_UNMUTED
};

// Conflict resolution strategies
enum class MergeStrategy {
    LAST_WRITE_WINS,  // Accept remote version
    KEEP_LOCAL,       // Reject remote, keep local
    MERGE_PROPERTIES, // Merge non-overlapping properties
    MANUAL            // User decides via UI
};

// Represents a single change in the scene
struct SceneChange {
    ChangeType type;
    std::string prim_path;
    std::string property_name;
    std::string old_value;
    std::string new_value;
    double timestamp;
    std::string user_id;
    std::string session_id;
};

// Session metadata
struct OmniverseSessionInfo {
    std::string session_id;
    std::string nucleus_url;
    std::string project_path;
    std::string scene_file;
    SessionState state;
    std::vector<std::string> connected_users;
    int change_count;
    double sync_latency_ms;
    std::string local_user_id;
};

// Main orchestrator class
class OmniverseLiveSync {
public:
    // Singleton
    static OmniverseLiveSync& GetInstance();
    
    // Session management
    void CreateSession(const std::string& nucleus_url,
                       const std::string& project_path,
                       const std::string& scene_file);
    void JoinSession(const std::string& session_id,
                     const std::string& user_id);
    void LeaveSession(const std::string& session_id);
    void CloseSession(const std::string& session_id);
    
    // Scene operations
    void OpenRemoteScene(const std::string& session_id);
    void SaveLocalChanges(const std::string& session_id);
    void PushChanges(const std::string& session_id,
                     const std::vector<SceneChange>& changes);
    std::vector<SceneChange> PullChanges(const std::string& session_id);
    
    // Conflict handling
    void ResolveConflict(const std::string& session_id,
                         const SceneChange& conflict,
                         MergeStrategy strategy);
    
    // Subscriptions
    void SubscribeToChanges(const std::string& session_id,
                           std::function<void(const SceneChange&)> callback);
    void UnsubscribeFromChanges(const std::string& session_id);
    
    // State queries
    SessionState GetSessionState(const std::string& session_id);
    OmniverseSessionInfo GetSessionInfo(const std::string& session_id);
    std::vector<std::string> ListActiveSessions();
    
    // Utilities
    void SetMergeStrategy(MergeStrategy strategy);
    void EnableAutoSync(bool enabled);
    void SetSyncInterval(double seconds);

private:
    struct SessionData {
        OmniverseSessionInfo info;
        std::vector<SceneChange> pending_changes;
        std::vector<SceneChange> change_history;
        std::queue<SceneChange> incoming_changes;
        std::function<void(const SceneChange&)> change_callback;
        std::thread sync_thread;
        bool auto_sync;
    };
    
    std::map<std::string, SessionData> sessions_;
    MergeStrategy default_merge_strategy_;
    std::mutex sessions_mutex_;
    
    // Private implementation methods
    void SyncThread(const std::string& session_id);
    void ProcessIncomingChanges(const std::string& session_id);
    void DetectConflicts(const std::string& session_id);
    void ApplyMergeStrategy(SceneChange& conflict);
};

} // namespace SZM::Omniverse
```

#### OmniverseLiveSync.cpp (370 lines)
- `CreateSession()` - Initialize new collaborative session
- `JoinSession()` - Connect existing user to session
- `OpenRemoteScene()` - Load USD scene from Nucleus
- `PushChanges()` - Broadcast local modifications
- `PullChanges()` - Fetch remote modifications
- `ResolveConflict()` - Apply merge strategy
- `SyncThread()` - Background synchronization loop
- `ProcessIncomingChanges()` - Queue and apply remote changes
- `DetectConflicts()` - Identify simultaneous edits

#### NucleusConnector.hpp (120 lines)
```cpp
class NucleusConnector {
public:
    static NucleusConnector& GetInstance();
    
    // Connection
    bool Connect(const std::string& nucleus_url,
                 const std::string& api_key = "");
    bool Disconnect();
    bool IsConnected() const;
    
    // Authentication
    void SetCredentials(const std::string& username,
                        const std::string& password);
    bool Authenticate();
    
    // Scene operations
    std::shared_ptr<pxr::UsdStage> OpenScene(const std::string& project_path,
                                              const std::string& scene_file);
    bool SaveScene(std::shared_ptr<pxr::UsdStage> stage);
    bool DeleteScene(const std::string& project_path);
    
    // Layer management
    std::vector<std::string> GetLayers(const std::string& project_path);
    std::shared_ptr<pxr::SdfLayer> GetLayer(const std::string& layer_path);
    bool MuteLayer(const std::string& layer_path);
    bool UnmuteLayer(const std::string& layer_path);
    
    // Versioning
    std::vector<std::string> GetVersions(const std::string& scene_path);
    std::string CreateCheckpoint(const std::string& scene_path,
                                  const std::string& comment);
    bool RevertToCheckpoint(const std::string& checkpoint_id);
    
    // Utilities
    std::string GetProjectMetadata(const std::string& project_path);
    bool SetProjectMetadata(const std::string& project_path,
                            const std::string& metadata);
    
private:
    void* nucleus_client_;  // Opaque pointer to Python Nucleus client
    bool is_connected_;
};
```

#### LiveSyncSession.hpp (150 lines)
```cpp
class LiveSyncSession {
public:
    explicit LiveSyncSession(const std::string& session_id);
    
    // Session lifecycle
    void Start();
    void Stop();
    bool IsActive() const;
    
    // User management
    void AddUser(const std::string& user_id);
    void RemoveUser(const std::string& user_id);
    std::vector<std::string> GetConnectedUsers() const;
    
    // Change tracking
    void RecordChange(const SceneChange& change);
    std::vector<SceneChange> GetPendingChanges();
    void ClearPendingChanges();
    
    // State synchronization
    void SyncState();
    void ApplyRemoteChanges(const std::vector<SceneChange>& changes);
    
    // Conflict detection
    bool HasConflicts() const;
    std::vector<SceneChange> GetConflicts() const;
    void ResolveConflict(const SceneChange& change, MergeStrategy strategy);
    
    // Getters
    std::string GetSessionId() const;
    double GetLatency() const;
    int GetChangeCount() const;
    SessionState GetState() const;

private:
    std::string session_id_;
    std::vector<std::string> connected_users_;
    std::vector<SceneChange> pending_changes_;
    std::vector<SceneChange> conflicts_;
    std::vector<SceneChange> change_history_;
    double latency_ms_;
    int change_count_;
    SessionState state_;
    std::mutex mutex_;
};
```

#### ChangeBuffer.hpp (80 lines)
```cpp
class ChangeBuffer {
public:
    // Add change to buffer
    void AddChange(const SceneChange& change);
    
    // Compress consecutive changes on same property
    std::vector<SceneChange> Compress();
    
    // Get deltas (only changed properties)
    std::vector<SceneChange> GetDeltas() const;
    
    // Clear buffer
    void Clear();
    
    // Capacity management
    size_t Size() const;
    bool IsEmpty() const;

private:
    std::vector<SceneChange> buffer_;
    std::map<std::string, SceneChange> latest_;  // Latest change per property
};
```

#### ConflictResolver.hpp (100 lines)
```cpp
class ConflictResolver {
public:
    // Detect conflicts between two change sets
    std::vector<SceneChange> DetectConflicts(
        const std::vector<SceneChange>& local,
        const std::vector<SceneChange>& remote);
    
    // Apply merge strategy
    SceneChange Resolve(const SceneChange& local,
                        const SceneChange& remote,
                        MergeStrategy strategy);
    
    // Merge non-conflicting properties
    std::vector<SceneChange> MergeProperties(
        const std::vector<SceneChange>& local,
        const std::vector<SceneChange>& remote);

private:
    bool IsConflict(const SceneChange& a, const SceneChange& b) const;
};
```

### 2. Python Omniverse Bridge (550 LOC)

#### omniverse_bridge.py
```python
from fastapi import FastAPI, WebSocket, HTTPException
from pydantic import BaseModel
from typing import List, Dict, Optional
import asyncio
import json
from datetime import datetime

app = FastAPI(title="Omniverse LiveSync Bridge")

# ============= Data Models =============

class OmniverseConnectRequest(BaseModel):
    nucleus_url: str
    username: str
    password: str
    project_path: str
    scene_file: str

class SessionInfo(BaseModel):
    session_id: str
    nucleus_url: str
    project_path: str
    scene_file: str
    state: str
    connected_users: List[str]
    change_count: int
    sync_latency_ms: float

class SceneChange(BaseModel):
    change_type: str  # property_changed, prim_created, etc.
    prim_path: str
    property_name: Optional[str] = None
    old_value: Optional[str] = None
    new_value: Optional[str] = None
    timestamp: float
    user_id: str

class PushChangesRequest(BaseModel):
    session_id: str
    changes: List[SceneChange]

class ConflictResolveRequest(BaseModel):
    session_id: str
    conflict_id: str
    merge_strategy: str  # "last_write_wins", "keep_local", etc.

# ============= Session Management =============

sessions = {}  # In-memory session store
connected_users = {}  # Track connected WebSocket clients

@app.post("/omniverse/connect")
async def connect_to_nucleus(req: OmniverseConnectRequest):
    """Create new Omniverse session"""
    session_id = f"session_{len(sessions) + 1}"
    
    sessions[session_id] = {
        "nucleus_url": req.nucleus_url,
        "project_path": req.project_path,
        "scene_file": req.scene_file,
        "state": "connecting",
        "users": [],
        "changes": [],
        "conflicts": []
    }
    
    return {
        "status": "success",
        "session_id": session_id,
        "message": f"Connected to Nucleus at {req.nucleus_url}"
    }

@app.post("/omniverse/join")
async def join_session(session_id: str, user_id: str):
    """User joins existing session"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    if user_id not in sessions[session_id]["users"]:
        sessions[session_id]["users"].append(user_id)
    
    return {
        "status": "success",
        "session_id": session_id,
        "user_id": user_id,
        "connected_users": sessions[session_id]["users"]
    }

@app.post("/omniverse/leave")
async def leave_session(session_id: str, user_id: str):
    """User leaves session"""
    if session_id in sessions and user_id in sessions[session_id]["users"]:
        sessions[session_id]["users"].remove(user_id)
    
    return {
        "status": "success",
        "message": f"User {user_id} left session"
    }

@app.get("/omniverse/session/{session_id}")
async def get_session_info(session_id: str):
    """Get current session state"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    return SessionInfo(
        session_id=session_id,
        nucleus_url=session["nucleus_url"],
        project_path=session["project_path"],
        scene_file=session["scene_file"],
        state=session["state"],
        connected_users=session["users"],
        change_count=len(session["changes"]),
        sync_latency_ms=50.0
    )

@app.get("/omniverse/sessions")
async def list_sessions():
    """List all active sessions"""
    return {
        "sessions": list(sessions.keys()),
        "count": len(sessions)
    }

# ============= Change Sync =============

@app.post("/omniverse/push")
async def push_changes(req: PushChangesRequest):
    """Push local changes to Nucleus"""
    if req.session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[req.session_id]
    session["changes"].extend([change.dict() for change in req.changes])
    
    return {
        "status": "success",
        "changes_accepted": len(req.changes),
        "total_changes": len(session["changes"])
    }

@app.post("/omniverse/pull")
async def pull_changes(session_id: str):
    """Pull remote changes from Nucleus"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    changes = session["changes"]
    session["changes"] = []  # Clear after pull
    
    return {
        "status": "success",
        "changes": changes,
        "count": len(changes)
    }

# ============= Conflict Management =============

@app.post("/omniverse/resolve-conflict")
async def resolve_conflict(req: ConflictResolveRequest):
    """Resolve detected conflict with merge strategy"""
    if req.session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    strategies = {
        "last_write_wins": "Accept remote version",
        "keep_local": "Keep local version",
        "merge_properties": "Merge non-overlapping properties",
        "manual": "User decision required"
    }
    
    strategy_desc = strategies.get(req.merge_strategy, "Unknown")
    
    return {
        "status": "success",
        "conflict_id": req.conflict_id,
        "strategy": req.merge_strategy,
        "description": strategy_desc,
        "message": "Conflict resolved"
    }

@app.get("/omniverse/conflicts/{session_id}")
async def get_conflicts(session_id: str):
    """Get list of unresolved conflicts"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    return {
        "session_id": session_id,
        "conflicts": sessions[session_id]["conflicts"],
        "count": len(sessions[session_id]["conflicts"])
    }

# ============= WebSocket (Real-time Sync) =============

@app.websocket("/ws/livesync/{session_id}")
async def websocket_endpoint(websocket: WebSocket, session_id: str):
    """WebSocket endpoint for real-time change streaming"""
    await websocket.accept()
    
    if session_id not in sessions:
        await websocket.close(code=4004, reason="Session not found")
        return
    
    connected_users[session_id] = connected_users.get(session_id, [])
    connected_users[session_id].append(websocket)
    
    try:
        while True:
            data = await websocket.receive_text()
            message = json.loads(data)
            
            # Broadcast to all connected clients
            for client in connected_users[session_id]:
                if client != websocket:
                    await client.send_json({
                        "type": "change",
                        "data": message
                    })
    except Exception as e:
        print(f"WebSocket error: {e}")
    finally:
        connected_users[session_id].remove(websocket)

# ============= Utility Endpoints =============

@app.post("/omniverse/sync")
async def force_sync(session_id: str):
    """Force full synchronization"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    return {
        "status": "success",
        "message": "Sync initiated",
        "session_id": session_id
    }

@app.delete("/omniverse/session/{session_id}")
async def close_session(session_id: str):
    """Close session and disconnect all users"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    del sessions[session_id]
    if session_id in connected_users:
        del connected_users[session_id]
    
    return {
        "status": "success",
        "message": f"Session {session_id} closed"
    }

@app.get("/health")
async def health_check():
    """Health check endpoint"""
    return {
        "status": "healthy",
        "active_sessions": len(sessions),
        "timestamp": datetime.now().isoformat()
    }

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8004)
```

### 3. React Frontend (380 LOC)

#### useOmniverseAPI.ts (120 lines)
```typescript
import { useState, useCallback } from 'react';

const API_BASE = 'http://127.0.0.1:8004';

export interface OmniverseSession {
  session_id: string;
  nucleus_url: string;
  project_path: string;
  scene_file: string;
  state: string;
  connected_users: string[];
  change_count: number;
  sync_latency_ms: number;
}

export interface SceneChange {
  change_type: string;
  prim_path: string;
  property_name?: string;
  old_value?: string;
  new_value?: string;
  timestamp: number;
  user_id: string;
}

export function useOmniverseAPI() {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [sessionInfo, setSessionInfo] = useState<OmniverseSession | null>(null);
  const [changes, setChanges] = useState<SceneChange[]>([]);

  const connectToNucleus = useCallback(async (
    nucleusUrl: string,
    username: string,
    password: string,
    projectPath: string,
    sceneFile: string
  ) => {
    setLoading(true);
    setError(null);
    try {
      const res = await fetch(`${API_BASE}/omniverse/connect`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          nucleus_url: nucleusUrl,
          username,
          password,
          project_path: projectPath,
          scene_file: sceneFile
        })
      });
      
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Unknown error';
      setError(msg);
      throw err;
    } finally {
      setLoading(false);
    }
  }, []);

  const joinSession = useCallback(async (sessionId: string, userId: string) => {
    setLoading(true);
    try {
      const res = await fetch(`${API_BASE}/omniverse/join?session_id=${sessionId}&user_id=${userId}`, {
        method: 'POST'
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Join failed');
      throw err;
    } finally {
      setLoading(false);
    }
  }, []);

  const getSessionInfo = useCallback(async (sessionId: string) => {
    setLoading(true);
    try {
      const res = await fetch(`${API_BASE}/omniverse/session/${sessionId}`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();
      setSessionInfo(data);
      return data;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Fetch failed');
      throw err;
    } finally {
      setLoading(false);
    }
  }, []);

  const pushChanges = useCallback(async (sessionId: string, changeList: SceneChange[]) => {
    try {
      const res = await fetch(`${API_BASE}/omniverse/push`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          session_id: sessionId,
          changes: changeList
        })
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Push failed');
      throw err;
    }
  }, []);

  const pullChanges = useCallback(async (sessionId: string) => {
    try {
      const res = await fetch(`${API_BASE}/omniverse/pull`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ session_id: sessionId })
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();
      setChanges(data.changes);
      return data;
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Pull failed');
      throw err;
    }
  }, []);

  return {
    loading,
    error,
    sessionInfo,
    changes,
    connectToNucleus,
    joinSession,
    getSessionInfo,
    pushChanges,
    pullChanges
  };
}
```

#### OmniverseLiveSyncPanel.tsx (260 lines)
```typescript
import React, { useState, useEffect } from 'react';
import { useOmniverseAPI } from '../hooks/useOmniverseAPI';

export const OmniverseLiveSyncPanel: React.FC = () => {
  const {
    loading,
    error,
    sessionInfo,
    changes,
    connectToNucleus,
    joinSession,
    getSessionInfo,
    pushChanges,
    pullChanges
  } = useOmniverseAPI();

  const [activeTab, setActiveTab] = useState<'connect' | 'session' | 'sync' | 'conflicts'>('connect');
  
  // Connect tab state
  const [nucleusUrl, setNucleusUrl] = useState('http://localhost:3100');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [projectPath, setProjectPath] = useState('');
  const [sceneFile, setSceneFile] = useState('scene.usd');

  // Session tab state
  const [sessionId, setSessionId] = useState('');
  const [userId, setUserId] = useState('');

  const handleConnect = async () => {
    try {
      const result = await connectToNucleus(nucleusUrl, username, password, projectPath, sceneFile);
      setSessionId(result.session_id);
    } catch (e) {
      console.error('Connection failed:', e);
    }
  };

  const handleJoin = async () => {
    if (sessionId && userId) {
      await joinSession(sessionId, userId);
      await getSessionInfo(sessionId);
    }
  };

  const handlePush = async () => {
    if (sessionId && changes.length > 0) {
      await pushChanges(sessionId, changes);
    }
  };

  const handlePull = async () => {
    if (sessionId) {
      await pullChanges(sessionId);
    }
  };

  return (
    <div className="omniverse-sync-panel">
      <h2>Omniverse LiveSync</h2>
      
      {error && <div className="error-message">{error}</div>}

      {/* Tab Navigation */}
      <div className="tabs">
        <button 
          className={activeTab === 'connect' ? 'active' : ''}
          onClick={() => setActiveTab('connect')}
        >
          Connect
        </button>
        <button 
          className={activeTab === 'session' ? 'active' : ''}
          onClick={() => setActiveTab('session')}
        >
          Session
        </button>
        <button 
          className={activeTab === 'sync' ? 'active' : ''}
          onClick={() => setActiveTab('sync')}
        >
          Sync
        </button>
        <button 
          className={activeTab === 'conflicts' ? 'active' : ''}
          onClick={() => setActiveTab('conflicts')}
        >
          Conflicts
        </button>
      </div>

      {/* Connect Tab */}
      {activeTab === 'connect' && (
        <div className="tab-content">
          <h3>Connect to Nucleus</h3>
          <input
            type="text"
            placeholder="Nucleus URL"
            value={nucleusUrl}
            onChange={(e) => setNucleusUrl(e.target.value)}
          />
          <input
            type="text"
            placeholder="Username"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
          />
          <input
            type="password"
            placeholder="Password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
          />
          <input
            type="text"
            placeholder="Project Path"
            value={projectPath}
            onChange={(e) => setProjectPath(e.target.value)}
          />
          <input
            type="text"
            placeholder="Scene File"
            value={sceneFile}
            onChange={(e) => setSceneFile(e.target.value)}
          />
          <button onClick={handleConnect} disabled={loading}>
            {loading ? 'Connecting...' : 'Connect'}
          </button>
          {sessionId && <p>✓ Session ID: {sessionId}</p>}
        </div>
      )}

      {/* Session Tab */}
      {activeTab === 'session' && (
        <div className="tab-content">
          <h3>Session Management</h3>
          <input
            type="text"
            placeholder="Session ID"
            value={sessionId}
            onChange={(e) => setSessionId(e.target.value)}
          />
          <input
            type="text"
            placeholder="User ID"
            value={userId}
            onChange={(e) => setUserId(e.target.value)}
          />
          <button onClick={handleJoin} disabled={loading}>
            {loading ? 'Joining...' : 'Join Session'}
          </button>
          
          {sessionInfo && (
            <div className="session-info">
              <p><strong>State:</strong> {sessionInfo.state}</p>
              <p><strong>Users:</strong> {sessionInfo.connected_users.join(', ')}</p>
              <p><strong>Changes:</strong> {sessionInfo.change_count}</p>
              <p><strong>Latency:</strong> {sessionInfo.sync_latency_ms}ms</p>
            </div>
          )}
        </div>
      )}

      {/* Sync Tab */}
      {activeTab === 'sync' && (
        <div className="tab-content">
          <h3>Synchronization</h3>
          <button onClick={handlePush} disabled={loading}>
            Push Changes ({changes.length})
          </button>
          <button onClick={handlePull} disabled={loading}>
            Pull Changes
          </button>
          
          <div className="changes-list">
            <h4>Recent Changes</h4>
            {changes.length === 0 ? (
              <p>No changes</p>
            ) : (
              <ul>
                {changes.map((change, idx) => (
                  <li key={idx}>
                    {change.change_type}: {change.prim_path}
                  </li>
                ))}
              </ul>
            )}
          </div>
        </div>
      )}

      {/* Conflicts Tab */}
      {activeTab === 'conflicts' && (
        <div className="tab-content">
          <h3>Conflict Resolution</h3>
          <p>No conflicts detected</p>
        </div>
      )}
    </div>
  );
};
```

### 4. ImGui Desktop Panel (420 LOC)

#### OmniverseLiveSyncPanel.hpp (90 lines)
```cpp
class OmniverseLiveSyncPanel {
public:
    void Render();
    void SetOmniverseInstance(SZM::Omniverse::OmniverseLiveSync* livesync);

private:
    SZM::Omniverse::OmniverseLiveSync* livesync_;
    bool show_connect_dialog_;
    bool show_session_manager_;
    
    char nucleus_url_[256];
    char username_[128];
    char password_[128];
    char project_path_[256];
    char scene_file_[128];
    char session_id_[64];
    char user_id_[64];
    
    float sync_interval_;
    int merge_strategy_; // 0=Last Write, 1=Keep Local, etc.
    
    void RenderConnectTab();
    void RenderSessionTab();
    void RenderSyncTab();
    void RenderConflictTab();
    void RenderSettingsTab();
};
```

#### OmniverseLiveSyncPanel.cpp (330 lines)
- Main `Render()` method with 5 tabs
- Connect to Nucleus form
- Session management interface
- Real-time change monitoring
- Conflict resolution UI
- Settings and preferences

---

## API Specification

### REST Endpoints (Port 8004)

#### Session Management
```
POST   /omniverse/connect              - Create new session
POST   /omniverse/join?session_id=...  - Join existing session
POST   /omniverse/leave?session_id=... - Leave session
GET    /omniverse/session/{id}         - Get session info
GET    /omniverse/sessions             - List all sessions
DELETE /omniverse/session/{id}         - Close session
```

#### Change Synchronization
```
POST   /omniverse/push                 - Push local changes
POST   /omniverse/pull                 - Pull remote changes
GET    /omniverse/conflicts/{id}       - Get conflicts
POST   /omniverse/resolve-conflict     - Resolve conflict
POST   /omniverse/sync                 - Force sync
```

#### WebSocket
```
WS     /ws/livesync/{session_id}       - Real-time change streaming
```

#### Utilities
```
GET    /health                         - Health check
```

### Request/Response Examples

See PLAN_27_INTEGRATION_GUIDE.md for detailed examples

---

## File Structure

```
/mnt/D/projects/szm_forge/
├── src/Omniverse/
│   ├── OmniverseLiveSync.hpp (180 LOC)
│   ├── OmniverseLiveSync.cpp (370 LOC)
│   ├── NucleusConnector.hpp (120 LOC)
│   ├── NucleusConnector.cpp (200 LOC)
│   ├── LiveSyncSession.hpp (150 LOC)
│   ├── LiveSyncSession.cpp (180 LOC)
│   ├── ChangeBuffer.hpp (80 LOC)
│   ├── ChangeBuffer.cpp (90 LOC)
│   ├── ConflictResolver.hpp (100 LOC)
│   └── ConflictResolver.cpp (120 LOC)
├── src/UI/panels/
│   ├── OmniverseLiveSyncPanel.hpp (90 LOC)
│   └── OmniverseLiveSyncPanel.cpp (330 LOC)
├── ai_service/
│   └── omniverse_bridge.py (550 LOC)
├── webui/src/
│   ├── hooks/
│   │   └── useOmniverseAPI.ts (120 LOC)
│   └── components/
│       └── OmniverseLiveSyncPanel.tsx (260 LOC)
└── Documentation/
    ├── PLAN_27_IMPLEMENTATION_SUMMARY.md
    └── PLAN_27_INTEGRATION_GUIDE.md
```

---

## Implementation Phases

### Phase 1: Python Omniverse Bridge (2-3 days)
- [x] Design omniverse_bridge.py FastAPI server
- [ ] Implement session management endpoints
- [ ] Implement change sync endpoints
- [ ] Implement WebSocket real-time streaming
- [ ] Test all endpoints with curl

### Phase 2: C++ LiveSync Engine (3-4 days)
- [ ] Implement OmniverseLiveSync orchestrator
- [ ] Implement NucleusConnector for Nucleus API
- [ ] Implement LiveSyncSession state management
- [ ] Implement ChangeBuffer for delta compression
- [ ] Implement ConflictResolver with merge strategies
- [ ] Verify C++ compilation (zero errors)

### Phase 3: React UI & ImGui (2-3 days)
- [ ] Implement useOmniverseAPI hook
- [ ] Implement OmniverseLiveSyncPanel React component
- [ ] Implement OmniverseLiveSyncPanel ImGui panel
- [ ] Verify React/ImGui compilation (zero errors)
- [ ] Basic UI testing

### Phase 4: Testing & Documentation (2 days)
- [ ] Test all REST endpoints
- [ ] Test WebSocket real-time sync
- [ ] Test conflict resolution
- [ ] End-to-end integration testing
- [ ] Create comprehensive integration guide
- [ ] Performance benchmarking

---

## Testing Strategy

### Unit Tests
- Change detection and tracking
- Merge strategy application
- Conflict detection
- Buffer compression

### Integration Tests
- Session creation and management
- User join/leave operations
- Change push/pull cycles
- Real-time WebSocket streaming
- Conflict resolution workflows

### End-to-End Tests
- Full collaborative editing session
- Multiple concurrent users
- Network latency simulation
- Nucleus server integration

---

## Performance Targets

- Session creation: <500ms
- Join session: <1s
- Change sync: <100ms latency
- Conflict resolution: <200ms
- WebSocket messages: <50ms

---

## Next Steps

1. Start Phase 1: Create omniverse_bridge.py with all endpoints
2. Test Python server with curl examples
3. Proceed with Phase 2: C++ implementation
4. Integrate React and ImGui components
5. Run comprehensive test suite
6. Deploy to production

---

## Dependencies

```bash
# Python dependencies
pip install fastapi uvicorn websockets pydantic

# C++ dependencies (header-only or included)
# - websocketpp (header-only)
# - nlohmann/json (header-only)

# NVIDIA Omniverse SDK (requires manual installation)
pip install omniverse-client>=1.3.0
pip install omniverse-nucleus-python>=1.3.0
```

---

## References

- NVIDIA Omniverse Documentation: https://docs.omniverse.nvidia.com
- Nucleus API Guide: https://docs.omniverse.nvidia.com/nucleus
- USD Documentation: https://graphics.pixar.com/usd/
- WebSocket Protocol: https://tools.ietf.org/html/rfc6455
