# Plan 27: Omniverse LiveSync - Analysis Report

**Analysis Date:** 2026-08-18  
**Status:** Specification identified, NO IMPLEMENTATION

---

## 1. Plan 27 Overview

### Name & Description
**Plan 27: Omniverse LiveSync**

Integrate NVIDIA Omniverse Nucleus Connector for **real-time collaborative editing and LiveSync** capabilities between SZM Forge and NVIDIA Omniverse ecosystem.

### Purpose/Domain
- **System:** NVIDIA Omniverse Platform Integration
- **Technology Stack:** 
  - Omniverse Nucleus (collaborative cloud backend)
  - Omniverse Python SDK
  - Universal Scene Description (USD) format
  - WebSocket for real-time sync
  - Pixar USD Core library

### Relationship to Other Plans
- **Depends on Plan 4** (USD & Omniverse Interoperability) - foundational USD export
- **Builds on Plan 26** (USD MaterialX) - material representation
- **Uses Plan 28** (WebGL/ThreeJS Parity) - similar multi-engine sync patterns

---

## 2. Implementation Status

### Current Implementation: ❌ **NOT IMPLEMENTED**

**What exists:**
- ✅ Plan 4: USD export foundation (basic .usda generation)
- ✅ Plan 26: USD/MaterialX export endpoint (`/export/usd`)
- ✅ ExternalEngineManager infrastructure (C++ plugin pattern)

**What's missing for Plan 27 - LiveSync:**
- ❌ Omniverse Nucleus Connector
- ❌ LiveSync protocol implementation
- ❌ WebSocket sync manager
- ❌ Real-time collaborative editing
- ❌ Session management for Omniverse
- ❌ Bidirectional scene graph sync

---

## 3. Required Files & Architecture

### Should Exist After Implementation

#### C++ Backend (src/)
```
src/
├── Omniverse/
│   ├── OmniverseLiveSync.hpp        (Main LiveSync manager)
│   ├── OmniverseLiveSync.cpp
│   ├── NucleusConnector.hpp         (Nucleus server bridge)
│   ├── NucleusConnector.cpp
│   ├── LiveSyncSession.hpp          (Collaborative session)
│   └── LiveSyncSession.cpp
└── Core/
    └── ExternalEngineManager.hpp    (Already exists - can extend)
```

#### Python REST API (ai_service/)
```
ai_service/
├── external_engine_bridge.py        (Add new endpoints)
│   ├── POST /omniverse/connect       (Connect to Nucleus)
│   ├── POST /omniverse/sync/start    (Start LiveSync)
│   ├── POST /omniverse/sync/stop     (Stop LiveSync)
│   ├── POST /omniverse/session/new   (Create session)
│   ├── GET  /omniverse/status        (Get sync status)
│   └── POST /omniverse/export        (Export to Nucleus)
└── omniverse_bridge.py              (NEW - Omniverse SDK wrapper)
    ├── OmniverseClient class
    ├── NucleusSessionManager class
    └── LiveSyncManager class
```

#### React/TypeScript (webui/src/)
```
webui/src/
├── hooks/
│   └── useOmniverseAPI.ts           (API communication hook)
├── components/
│   └── OmniverseLiveSyncPanel.tsx   (UI panel)
└── types/
    └── omniverse.ts                 (TypeScript interfaces)
```

#### ImGui Desktop (src/UI/)
```
src/UI/
├── OmniversePanels.hpp              (ImGui panels)
├── OmniversePanels.cpp
└── LiveSyncStatusWindow.cpp
```

---

## 4. Technical Requirements

### Core Dependencies
```
Python packages (requirements.txt):
- omniverse-sdk >= 1.3.0
- websockets >= 10.0
- pydantic (already present)
- fastapi (already present)

C++ Dependencies:
- USD libraries (usd-core)
- Boost ASIO (for WebSocket)
- nlohmann/json (for serialization)
```

### Infrastructure Requirements
1. **Nucleus Server**: NVIDIA Omniverse Nucleus instance (cloud or self-hosted)
2. **User Authentication**: NVIDIA Omniverse account/credentials
3. **Network**: Persistent internet connection for cloud sync
4. **Protocol Support**: WebSocket for bidirectional real-time updates

### Data Flow
```
SZM Forge ←→ ExternalEngineManager ←→ OmniverseLiveSync
    ↓                                        ↓
  ECS Scene                          USD Stage
  (C++ memory)                        (USD files)
    ↓                                        ↓
  Python REST API ←→ Omniverse SDK ←→ Nucleus Server
    (FastAPI)        (omniverse_bridge)    (Cloud)
    ↓
  React UI / ImGui Desktop
  (Real-time sync status)
```

---

## 5. Implementation Approach

### Recommended Strategy Based on Plans 10, 17, 19

#### Phase 1: Python Bridge Layer (2-3 days)
1. **Create `omniverse_bridge.py`**
   - Wrap NVIDIA Omniverse Python SDK
   - Implement NucleusSessionManager
   - Add authentication handling

2. **Add REST endpoints to `external_engine_bridge.py`**
   - POST `/omniverse/connect` - authenticate and connect
   - POST `/omniverse/sync/start` - initialize LiveSync
   - POST `/omniverse/sync/stop` - stop syncing
   - GET `/omniverse/status` - poll sync status

3. **JSON Request/Response Models (Pydantic)**
   ```python
   class OmniverseConnectRequest:
       nucleus_url: str
       username: str
       password: str
       
   class LiveSyncStartRequest:
       project_name: str
       layer_url: str
       component_id: int
       
   class LiveSyncStatus:
       is_connected: bool
       is_syncing: bool
       last_sync: float
       pending_changes: int
   ```

#### Phase 2: C++ Backend (3-4 days)
1. **OmniverseLiveSync.hpp/cpp**
   - Inherit from IExternalEngine pattern
   - Manage Nucleus connections
   - Handle scene graph sync

2. **NucleusConnector.hpp/cpp**
   - Low-level Nucleus protocol
   - USD stage creation/updates
   - Change notification handling

3. **LiveSyncSession.hpp/cpp**
   - Multi-user session management
   - Conflict resolution
   - Undo/redo integration (Plan 7)

#### Phase 3: Frontend UI (2-3 days)
1. **React Hook: `useOmniverseAPI.ts`**
   - Connect/disconnect
   - Sync control
   - Status polling

2. **React Component: `OmniverseLiveSyncPanel.tsx`**
   - Connection form
   - Sync controls
   - Status indicators
   - User list (collaborative)

3. **ImGui Panels**
   - Quick sync toggle
   - Nucleus URL display
   - Active users list

#### Phase 4: Testing & Integration (2 days)
1. Unit tests for OmniverseLiveSync
2. Integration tests with mock Nucleus
3. E2E tests with real Omniverse account (if available)

---

## 6. Implementation Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                     SZM Forge Application                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────┐              ┌──────────────────┐        │
│  │   React UI       │              │   ImGui Panel    │        │
│  │ LiveSyncPanel    │──────────────│ LiveSync Window  │        │
│  │   .tsx           │   REST JSON  │      .cpp        │        │
│  └────────┬─────────┘              └────────┬─────────┘        │
│           │                                  │                   │
│           │        ┌─────────────────────────┘                  │
│           │        │ C++ API Calls                              │
│           │        ▼                                            │
│           └─────► http://127.0.0.1:8003/omniverse/sync/start    │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                    FastAPI Bridge Layer                         │
│  File: ai_service/external_engine_bridge.py                    │
│                                                                 │
│  @app.post("/omniverse/connect")                               │
│  @app.post("/omniverse/sync/start")                            │
│  @app.post("/omniverse/sync/stop")                             │
│  @app.get("/omniverse/status")                                 │
│                                                                 │
│  Called by: omniverse_bridge.py (NEW)                          │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                   C++ Backend Layer                             │
│  File: src/Omniverse/OmniverseLiveSync.cpp                     │
│                                                                 │
│  OmniverseLiveSync::RunSync()                                  │
│  ├─ Initialize NucleusConnector                               │
│  ├─ Create/load USD Stage                                      │
│  ├─ Traverse ECS scene graph                                   │
│  ├─ Sync transforms & materials to USD                         │
│  └─ Listen for remote changes                                  │
│                                                                 │
│  ExternalEngineManager::RegisterEngine()                       │
│  └─ Register OmniverseLiveSync                                 │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│                  NVIDIA Omniverse SDK                           │
│  (ai_service/omniverse_bridge.py)                              │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ OmniverseClient (wraps SDK)                             │  │
│  │  • Authenticate to Nucleus                              │  │
│  │  • Create/open USD layers                               │  │
│  │  • Listen for changes                                   │  │
│  │  • Push updates                                         │  │
│  └──────────────────────────────────────────────────────────┘  │
│                           │                                     │
│                           ▼                                     │
├─────────────────────────────────────────────────────────────────┤
│           NVIDIA Omniverse Nucleus Server (Cloud)              │
│                                                                 │
│  ┌─ Project                                                    │
│  │  ├─ Layer (USD document)                                    │
│  │  │  ├─ Geometry (Xforms, Meshes)                            │
│  │  │  ├─ Materials (PreviewSurface)                           │
│  │  │  └─ Metadata (author, timestamp)                         │
│  │  └─ Metadata (project_name, created_at)                     │
│  └─ Users (alice, bob, charlie) - collaborative               │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 7. Key Integration Points

### 1. Scene Graph Serialization
Reuse existing traversal from Plan 4:
```cpp
// Traverse ECS entities
auto& scene = Scene::GetInstance();
for (auto entity : scene.GetEntities()) {
    auto transform = scene.GetComponent<TransformComponent>(entity);
    auto material = scene.GetComponent<MaterialComponent>(entity);
    // Serialize to USD Stage
    usdStage.CreateXform(entity_name, transform);
}
```

### 2. Material Export
Leverage existing MaterialX code from Plan 26:
```python
# In omniverse_bridge.py
def serialize_materials(components):
    mtlx_doc = UsdMtlx.OpenMaterialX()
    for mat in components:
        # Generate MaterialX document
    return mtlx_doc.ExportToString()
```

### 3. WebSocket Live Updates
```cpp
// In OmniverseLiveSync::OnRemoteChange()
void OnRemoteChange(const USD::Change& change) {
    // Update local ECS from remote USD changes
    Scene::GetInstance().UpdateComponentFromUSD(change);
}
```

### 4. REST API Integration
Follow Plan 19 pattern:
```python
@app.post("/omniverse/sync/start")
async def start_livesync(request: LiveSyncStartRequest):
    client = OmniverseClient(...)
    session = client.CreateSession(request.project_name)
    return {"status": "syncing", "session_id": session.id}
```

---

## 8. Testing Strategy

### Unit Tests
- OmniverseLiveSync initialization
- NucleusConnector connection/disconnection
- Scene graph serialization
- USD stage updates

### Integration Tests
- End-to-end sync with mock Nucleus
- Multi-user conflict resolution
- Undo/redo with LiveSync
- Material and geometry sync

### E2E Tests (requires real Omniverse account)
- Connect to actual Nucleus server
- Create real project
- Verify sync with USD Composer or Omniverse Create

---

## 9. Success Criteria

### Phase 1 (Python): COMPLETE when:
- [ ] `omniverse_bridge.py` successfully authenticates
- [ ] Nucleus connection endpoint works
- [ ] LiveSync can start/stop
- [ ] Status API returns valid responses

### Phase 2 (C++): COMPLETE when:
- [ ] OmniverseLiveSync::Initialize() succeeds
- [ ] Scene graph converts to USD correctly
- [ ] Can read/write USD stages
- [ ] All unit tests pass

### Phase 3 (UI): COMPLETE when:
- [ ] React panel renders without errors
- [ ] Can trigger sync from UI
- [ ] Status updates in real-time
- [ ] ImGui panel displays sync info

### Phase 4 (Testing): COMPLETE when:
- [ ] All unit tests pass
- [ ] Integration tests pass
- [ ] E2E test with Nucleus succeeds (if available)
- [ ] Documentation complete

---

## 10. Estimated Effort

| Phase | Component | LOC | Effort |
|-------|-----------|-----|--------|
| 1     | omniverse_bridge.py | 300 | 2 days |
| 1     | REST endpoints | 200 | 1 day |
| 2     | OmniverseLiveSync.hpp/cpp | 400 | 2 days |
| 2     | NucleusConnector.hpp/cpp | 350 | 2 days |
| 3     | useOmniverseAPI.ts | 150 | 1 day |
| 3     | OmniverseLiveSyncPanel.tsx | 250 | 1 day |
| 3     | ImGui panels | 200 | 1 day |
| 4     | Tests | 400 | 2 days |
| **Total** | | **2,250** | **12-13 days** |

---

## 11. Dependencies & Prerequisites

### External Dependencies
```
pip install omniverse-sdk>=1.3.0
pip install websockets>=10.0
apt-get install nvidia-omniverse-nucleus  (optional, if self-hosting)
```

### Internal Dependencies
- ExternalEngineManager (already exists)
- Scene/ECS system (already exists)
- USD export foundation (Plan 4, 26)
- REST API framework (fastapi - already exists)

### Prerequisite Plans
- ✅ Plan 4: USD & Omniverse Interoperability
- ✅ Plan 26: USD MaterialX
- ✅ Plan 7: Undo/Redo System (for undo/redo sync)

---

## 12. Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|-----------|
| Omniverse SDK compatibility | Medium | High | Pin SDK version, test early |
| Network latency in LiveSync | High | Medium | Implement batching, local cache |
| USD schema version mismatch | Medium | High | Validate USD versions, use adapters |
| Multi-user conflict race conditions | Medium | High | Implement OT/CRDT, version tracking |
| Performance with large scenes | Medium | High | Lazy load, delta updates only |

---

## 13. Next Steps

1. **Immediate:**
   - [x] Analyze Plan 27 specification ✓
   - [ ] Install and test Omniverse SDK locally
   - [ ] Create `omniverse_bridge.py` stub

2. **Short Term:**
   - [ ] Implement Phase 1 (Python REST endpoints)
   - [ ] Test Nucleus connection
   - [ ] Create test harness

3. **Medium Term:**
   - [ ] Implement Phase 2 (C++ backend)
   - [ ] Integrate with ExternalEngineManager
   - [ ] Write unit tests

4. **Long Term:**
   - [ ] Complete UI (React + ImGui)
   - [ ] E2E testing
   - [ ] Documentation & examples

---

**Report Generated:** 2026-08-18  
**Analyst:** GitHub Copilot  
**Status:** Analysis Complete - Ready for Implementation Planning
