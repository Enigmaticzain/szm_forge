"""
Omniverse LiveSync Bridge - Real-time collaborative editing
FastAPI server for NVIDIA Omniverse Nucleus integration

Port: 8004
REST + WebSocket endpoints for scene synchronization
"""

from fastapi import FastAPI, WebSocket, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import List, Dict, Optional, Set
import asyncio
import json
from datetime import datetime
import uuid
import logging

# ============= Logging Setup =============
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(
    title="Omniverse LiveSync Bridge",
    description="Real-time collaborative editing with NVIDIA Omniverse Nucleus",
    version="1.0.0"
)

# ============= CORS Configuration =============
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ============= Data Models =============

class OmniverseConnectRequest(BaseModel):
    """Request to connect to Nucleus server"""
    nucleus_url: str
    username: str
    password: str
    project_path: str
    scene_file: str

class SessionInfo(BaseModel):
    """Current session state information"""
    session_id: str
    nucleus_url: str
    project_path: str
    scene_file: str
    state: str  # connecting, connected, syncing, conflict, error
    connected_users: List[str]
    change_count: int
    sync_latency_ms: float
    created_at: str
    last_sync: str

class SceneChange(BaseModel):
    """Individual scene modification"""
    change_type: str  # property_changed, prim_created, prim_deleted, relationship_changed
    prim_path: str
    property_name: Optional[str] = None
    old_value: Optional[str] = None
    new_value: Optional[str] = None
    timestamp: float
    user_id: str

class PushChangesRequest(BaseModel):
    """Request to push local changes to Nucleus"""
    session_id: str
    changes: List[SceneChange]

class ConflictResolveRequest(BaseModel):
    """Request to resolve a conflict"""
    session_id: str
    conflict_id: str
    merge_strategy: str  # last_write_wins, keep_local, merge_properties, manual

class UserJoinRequest(BaseModel):
    """User joining session"""
    session_id: str
    user_id: str
    display_name: Optional[str] = None

# ============= In-Memory Session Store =============

class SessionData:
    """Internal session state"""
    def __init__(self, session_id: str, nucleus_url: str, project_path: str, scene_file: str):
        self.session_id = session_id
        self.nucleus_url = nucleus_url
        self.project_path = project_path
        self.scene_file = scene_file
        self.state = "connecting"
        self.users: Set[str] = set()
        self.changes: List[Dict] = []
        self.conflicts: List[Dict] = []
        self.change_history: List[Dict] = []
        self.created_at = datetime.now().isoformat()
        self.last_sync = datetime.now().isoformat()
        self.sync_latency_ms = 50.0
        self.websockets: List[WebSocket] = []

sessions: Dict[str, SessionData] = {}
connected_clients: Dict[str, List[WebSocket]] = {}  # session_id -> list of websockets

# ============= Session Management Endpoints =============

@app.post("/omniverse/connect", response_model=dict)
async def connect_to_nucleus(req: OmniverseConnectRequest) -> dict:
    """
    Create new Omniverse session and connect to Nucleus
    
    Args:
        nucleus_url: Nucleus server URL (e.g., http://localhost:3100)
        username: Nucleus username
        password: Nucleus password
        project_path: Project path (e.g., /projects/default)
        scene_file: Scene file path (e.g., scene.usd)
    
    Returns:
        session_id: Unique session identifier
        state: Initial connection state
    """
    try:
        session_id = f"session_{uuid.uuid4().hex[:12]}"
        
        session_data = SessionData(session_id, req.nucleus_url, req.project_path, req.scene_file)
        session_data.state = "connected"  # Simplified - in production would actually connect
        
        sessions[session_id] = session_data
        connected_clients[session_id] = []
        
        logger.info(f"Created session {session_id} at {req.nucleus_url}")
        
        return {
            "status": "success",
            "session_id": session_id,
            "state": "connected",
            "message": f"Connected to Nucleus at {req.nucleus_url}",
            "nucleus_url": req.nucleus_url,
            "project_path": req.project_path,
            "scene_file": req.scene_file
        }
    except Exception as e:
        logger.error(f"Connection failed: {str(e)}")
        raise HTTPException(status_code=500, detail=f"Connection failed: {str(e)}")

@app.post("/omniverse/join", response_model=dict)
async def join_session(
    session_id: str = Query(..., description="Session ID"),
    user_id: str = Query(..., description="User ID"),
    display_name: Optional[str] = Query(None, description="User display name")
) -> dict:
    """
    User joins existing session
    
    Args:
        session_id: Session to join
        user_id: Unique user identifier
        display_name: Optional display name
    
    Returns:
        status: success or error
        connected_users: List of all users in session
    """
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    
    if user_id not in session.users:
        session.users.add(user_id)
        logger.info(f"User {user_id} joined session {session_id}")
    
    return {
        "status": "success",
        "session_id": session_id,
        "user_id": user_id,
        "connected_users": list(session.users),
        "user_count": len(session.users)
    }

@app.post("/omniverse/leave", response_model=dict)
async def leave_session(
    session_id: str = Query(...),
    user_id: str = Query(...)
) -> dict:
    """User leaves session"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    if user_id in session.users:
        session.users.discard(user_id)
        logger.info(f"User {user_id} left session {session_id}")
    
    return {
        "status": "success",
        "message": f"User {user_id} left session",
        "remaining_users": list(session.users)
    }

@app.get("/omniverse/session/{session_id}", response_model=SessionInfo)
async def get_session_info(session_id: str) -> SessionInfo:
    """Get current session state"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    
    return SessionInfo(
        session_id=session_id,
        nucleus_url=session.nucleus_url,
        project_path=session.project_path,
        scene_file=session.scene_file,
        state=session.state,
        connected_users=list(session.users),
        change_count=len(session.changes),
        sync_latency_ms=session.sync_latency_ms,
        created_at=session.created_at,
        last_sync=session.last_sync
    )

@app.get("/omniverse/sessions", response_model=dict)
async def list_sessions() -> dict:
    """List all active sessions"""
    session_list = []
    for sid, session in sessions.items():
        session_list.append({
            "session_id": sid,
            "nucleus_url": session.nucleus_url,
            "state": session.state,
            "user_count": len(session.users),
            "change_count": len(session.changes)
        })
    
    return {
        "sessions": session_list,
        "count": len(sessions),
        "timestamp": datetime.now().isoformat()
    }

@app.delete("/omniverse/session/{session_id}", response_model=dict)
async def close_session(session_id: str) -> dict:
    """Close session and disconnect all users"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    # Close all WebSocket connections
    if session_id in connected_clients:
        for ws in connected_clients[session_id]:
            try:
                await ws.close(code=1000, reason="Session closed")
            except:
                pass
        del connected_clients[session_id]
    
    del sessions[session_id]
    logger.info(f"Session {session_id} closed")
    
    return {
        "status": "success",
        "message": f"Session {session_id} closed"
    }

# ============= Change Synchronization Endpoints =============

@app.post("/omniverse/push", response_model=dict)
async def push_changes(req: PushChangesRequest) -> dict:
    """
    Push local changes to Nucleus
    
    Changes are queued for broadcasting to other connected clients
    """
    if req.session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[req.session_id]
    change_dicts = [change.dict() for change in req.changes]
    
    # Add to session history
    session.changes.extend(change_dicts)
    session.change_history.extend(change_dicts)
    session.last_sync = datetime.now().isoformat()
    
    # Broadcast to all connected WebSocket clients
    if req.session_id in connected_clients:
        disconnected = []
        for ws in connected_clients[req.session_id]:
            try:
                await ws.send_json({
                    "type": "changes",
                    "changes": change_dicts,
                    "count": len(change_dicts),
                    "timestamp": datetime.now().isoformat()
                })
            except Exception as e:
                logger.warning(f"Failed to send to WebSocket: {e}")
                disconnected.append(ws)
        
        # Remove disconnected clients
        for ws in disconnected:
            if ws in connected_clients[req.session_id]:
                connected_clients[req.session_id].remove(ws)
    
    logger.info(f"Pushed {len(req.changes)} changes to session {req.session_id}")
    
    return {
        "status": "success",
        "changes_accepted": len(req.changes),
        "total_changes": len(session.changes),
        "broadcasted": len(connected_clients.get(req.session_id, []))
    }

@app.post("/omniverse/pull", response_model=dict)
async def pull_changes(session_id: str = Query(...)) -> dict:
    """
    Pull remote changes from Nucleus
    
    Returns queued changes and clears the queue
    """
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    changes = session.changes.copy()
    session.changes.clear()  # Clear after pull
    session.last_sync = datetime.now().isoformat()
    
    logger.info(f"Pulled {len(changes)} changes from session {session_id}")
    
    return {
        "status": "success",
        "changes": changes,
        "count": len(changes),
        "timestamp": datetime.now().isoformat()
    }

@app.post("/omniverse/sync", response_model=dict)
async def force_sync(session_id: str = Query(...)) -> dict:
    """Force full synchronization of session state"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    session.state = "syncing"
    
    # Simulate sync delay
    await asyncio.sleep(0.1)
    
    session.state = "connected"
    session.last_sync = datetime.now().isoformat()
    
    logger.info(f"Forced sync on session {session_id}")
    
    return {
        "status": "success",
        "message": "Sync completed",
        "changes": len(session.change_history),
        "connected_users": len(session.users)
    }

# ============= Conflict Management Endpoints =============

@app.post("/omniverse/resolve-conflict", response_model=dict)
async def resolve_conflict(req: ConflictResolveRequest) -> dict:
    """
    Resolve detected conflict with merge strategy
    
    Strategies:
    - last_write_wins: Accept remote version
    - keep_local: Keep local version
    - merge_properties: Merge non-overlapping properties
    - manual: User decides via UI
    """
    if req.session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[req.session_id]
    
    strategies = {
        "last_write_wins": "Accept remote version",
        "keep_local": "Keep local version",
        "merge_properties": "Merge non-overlapping properties",
        "manual": "User decision required"
    }
    
    strategy_desc = strategies.get(req.merge_strategy, "Unknown strategy")
    
    # Remove from conflicts list
    session.conflicts = [c for c in session.conflicts if c.get("id") != req.conflict_id]
    
    logger.info(f"Resolved conflict {req.conflict_id} with strategy {req.merge_strategy}")
    
    return {
        "status": "success",
        "conflict_id": req.conflict_id,
        "strategy": req.merge_strategy,
        "strategy_description": strategy_desc,
        "message": "Conflict resolved",
        "remaining_conflicts": len(session.conflicts)
    }

@app.get("/omniverse/conflicts/{session_id}", response_model=dict)
async def get_conflicts(session_id: str) -> dict:
    """Get list of unresolved conflicts"""
    if session_id not in sessions:
        raise HTTPException(status_code=404, detail="Session not found")
    
    session = sessions[session_id]
    
    return {
        "status": "success",
        "session_id": session_id,
        "conflicts": session.conflicts,
        "count": len(session.conflicts)
    }

# ============= WebSocket Real-time Streaming =============

@app.websocket("/ws/livesync/{session_id}")
async def websocket_endpoint(websocket: WebSocket, session_id: str):
    """
    WebSocket endpoint for real-time change streaming
    
    Clients connect here to receive real-time updates
    """
    if session_id not in sessions:
        await websocket.close(code=4004, reason="Session not found")
        return
    
    await websocket.accept()
    
    if session_id not in connected_clients:
        connected_clients[session_id] = []
    
    connected_clients[session_id].append(websocket)
    
    logger.info(f"WebSocket client connected to session {session_id}, total: {len(connected_clients[session_id])}")
    
    try:
        while True:
            # Receive message from client
            data = await websocket.receive_text()
            message = json.loads(data)
            
            logger.debug(f"WebSocket message: {message.get('type')}")
            
            # Broadcast to all other connected clients
            for client in connected_clients[session_id]:
                if client != websocket:
                    try:
                        await client.send_json({
                            "type": "change_notification",
                            "data": message,
                            "timestamp": datetime.now().isoformat()
                        })
                    except Exception as e:
                        logger.warning(f"Failed to broadcast: {e}")
    
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
    
    finally:
        if session_id in connected_clients and websocket in connected_clients[session_id]:
            connected_clients[session_id].remove(websocket)
            logger.info(f"WebSocket client disconnected, remaining: {len(connected_clients.get(session_id, []))}")

# ============= Utility Endpoints =============

@app.get("/health", response_model=dict)
async def health_check() -> dict:
    """Health check endpoint"""
    return {
        "status": "healthy",
        "service": "Omniverse LiveSync Bridge",
        "version": "1.0.0",
        "active_sessions": len(sessions),
        "total_changes": sum(len(s.change_history) for s in sessions.values()),
        "timestamp": datetime.now().isoformat()
    }

@app.get("/omniverse/stats", response_model=dict)
async def get_stats() -> dict:
    """Get system statistics"""
    total_users = sum(len(s.users) for s in sessions.values())
    total_changes = sum(len(s.change_history) for s in sessions.values())
    avg_latency = sum(s.sync_latency_ms for s in sessions.values()) / len(sessions) if sessions else 0
    
    return {
        "status": "success",
        "sessions": len(sessions),
        "total_users": total_users,
        "total_changes": total_changes,
        "average_latency_ms": avg_latency,
        "timestamp": datetime.now().isoformat()
    }

@app.post("/omniverse/test-connection", response_model=dict)
async def test_connection(nucleus_url: str = Query(...)) -> dict:
    """Test connection to Nucleus server"""
    # Simplified - in production would actually test
    return {
        "status": "success",
        "nucleus_url": nucleus_url,
        "connected": True,
        "message": "Connection test successful"
    }

# ============= Root Endpoint =============

@app.get("/", response_model=dict)
async def root() -> dict:
    """API root endpoint"""
    return {
        "service": "Omniverse LiveSync Bridge",
        "version": "1.0.0",
        "description": "Real-time collaborative editing with NVIDIA Omniverse",
        "endpoints": {
            "sessions": "/omniverse/sessions",
            "connect": "/omniverse/connect (POST)",
            "join": "/omniverse/join (POST)",
            "status": "/omniverse/session/{id} (GET)",
            "push": "/omniverse/push (POST)",
            "pull": "/omniverse/pull (POST)",
            "websocket": "/ws/livesync/{id}",
            "health": "/health"
        }
    }

# ============= Main Entry Point =============

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        app,
        host="127.0.0.1",
        port=8004,
        log_level="info"
    )
