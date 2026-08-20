import { useState, useCallback, useRef, useEffect } from 'react';

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
  created_at: string;
  last_sync: string;
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

export interface UseOmniverseAPIResult {
  loading: boolean;
  error: string | null;
  sessionInfo: OmniverseSession | null;
  changes: SceneChange[];
  connectedSessions: string[];
  connectToNucleus: (nucleusUrl: string, username: string, password: string, projectPath: string, sceneFile: string) => Promise<any>;
  joinSession: (sessionId: string, userId: string, displayName?: string) => Promise<any>;
  leaveSession: (sessionId: string, userId: string) => Promise<any>;
  getSessionInfo: (sessionId: string) => Promise<OmniverseSession>;
  listSessions: () => Promise<any>;
  pushChanges: (sessionId: string, changes: SceneChange[]) => Promise<any>;
  pullChanges: (sessionId: string) => Promise<any>;
  resolveConflict: (sessionId: string, conflictId: string, strategy: string) => Promise<any>;
  getConflicts: (sessionId: string) => Promise<any>;
  forceSync: (sessionId: string) => Promise<any>;
  closeSession: (sessionId: string) => Promise<any>;
}

export function useOmniverseAPI(): UseOmniverseAPIResult {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [sessionInfo, setSessionInfo] = useState<OmniverseSession | null>(null);
  const [changes, setChanges] = useState<SceneChange[]>([]);
  const [connectedSessions, setConnectedSessions] = useState<string[]>([]);
  const wsRef = useRef<WebSocket | null>(null);

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
      const data = await res.json();
      
      // Auto-subscribe to WebSocket
      if (data.session_id) {
        subscribeToSession(data.session_id);
      }
      
      return data;
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Connection failed';
      setError(msg);
      throw err;
    } finally {
      setLoading(false);
    }
  }, []);

  const joinSession = useCallback(async (sessionId: string, userId: string, displayName?: string) => {
    setLoading(true);
    setError(null);
    try {
      const params = new URLSearchParams();
      params.append('session_id', sessionId);
      params.append('user_id', userId);
      if (displayName) params.append('display_name', displayName);

      const res = await fetch(`${API_BASE}/omniverse/join?${params}`, {
        method: 'POST'
      });

      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();

      // Subscribe to WebSocket for this session
      subscribeToSession(sessionId);

      return data;
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Join failed';
      setError(msg);
      throw err;
    } finally {
      setLoading(false);
    }
  }, []);

  const leaveSession = useCallback(async (sessionId: string, userId: string) => {
    try {
      const params = new URLSearchParams();
      params.append('session_id', sessionId);
      params.append('user_id', userId);

      const res = await fetch(`${API_BASE}/omniverse/leave?${params}`, {
        method: 'POST'
      });

      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      
      // Close WebSocket if this is the session
      if (sessionInfo?.session_id === sessionId) {
        if (wsRef.current) {
          wsRef.current.close();
          wsRef.current = null;
        }
      }

      return await res.json();
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Leave failed';
      setError(msg);
      throw err;
    }
  }, [sessionInfo?.session_id]);

  const getSessionInfo = useCallback(async (sessionId: string) => {
    setLoading(true);
    try {
      const res = await fetch(`${API_BASE}/omniverse/session/${sessionId}`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();
      setSessionInfo(data);
      return data;
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Fetch failed';
      setError(msg);
      throw err;
    } finally {
      setLoading(false);
    }
  }, []);

  const listSessions = useCallback(async () => {
    try {
      const res = await fetch(`${API_BASE}/omniverse/sessions`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();
      setConnectedSessions(data.sessions.map((s: any) => s.session_id));
      return data;
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'List failed';
      setError(msg);
      throw err;
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
      const msg = err instanceof Error ? err.message : 'Push failed';
      setError(msg);
      throw err;
    }
  }, []);

  const pullChanges = useCallback(async (sessionId: string) => {
    try {
      const params = new URLSearchParams();
      params.append('session_id', sessionId);

      const res = await fetch(`${API_BASE}/omniverse/pull?${params}`, {
        method: 'POST'
      });

      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const data = await res.json();
      setChanges(data.changes || []);
      return data;
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Pull failed';
      setError(msg);
      throw err;
    }
  }, []);

  const resolveConflict = useCallback(async (sessionId: string, conflictId: string, strategy: string) => {
    try {
      const res = await fetch(`${API_BASE}/omniverse/resolve-conflict`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          session_id: sessionId,
          conflict_id: conflictId,
          merge_strategy: strategy
        })
      });

      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Resolve failed';
      setError(msg);
      throw err;
    }
  }, []);

  const getConflicts = useCallback(async (sessionId: string) => {
    try {
      const res = await fetch(`${API_BASE}/omniverse/conflicts/${sessionId}`);
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Get conflicts failed';
      setError(msg);
      throw err;
    }
  }, []);

  const forceSync = useCallback(async (sessionId: string) => {
    try {
      const params = new URLSearchParams();
      params.append('session_id', sessionId);

      const res = await fetch(`${API_BASE}/omniverse/sync?${params}`, {
        method: 'POST'
      });

      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Sync failed';
      setError(msg);
      throw err;
    }
  }, []);

  const closeSession = useCallback(async (sessionId: string) => {
    try {
      const res = await fetch(`${API_BASE}/omniverse/session/${sessionId}`, {
        method: 'DELETE'
      });

      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      
      if (sessionInfo?.session_id === sessionId) {
        setSessionInfo(null);
        if (wsRef.current) {
          wsRef.current.close();
          wsRef.current = null;
        }
      }

      return await res.json();
    } catch (err) {
      const msg = err instanceof Error ? err.message : 'Close failed';
      setError(msg);
      throw err;
    }
  }, [sessionInfo?.session_id]);

  const subscribeToSession = useCallback((sessionId: string) => {
    // Close existing connection if any
    if (wsRef.current) {
      wsRef.current.close();
    }

    // Connect to WebSocket
    const wsUrl = `ws://127.0.0.1:8004/ws/livesync/${sessionId}`;
    
    try {
      wsRef.current = new WebSocket(wsUrl);

      wsRef.current.onmessage = (event) => {
        try {
          const message = JSON.parse(event.data);
          if (message.type === 'changes') {
            setChanges(prev => [...prev, ...message.changes]);
          }
        } catch (e) {
          console.error('Failed to parse WebSocket message:', e);
        }
      };

      wsRef.current.onerror = (event) => {
        console.error('WebSocket error:', event);
        setError('WebSocket connection error');
      };

      wsRef.current.onclose = () => {
        wsRef.current = null;
      };
    } catch (err) {
      console.error('Failed to create WebSocket:', err);
    }
  }, []);

  // Cleanup on unmount
  useEffect(() => {
    return () => {
      if (wsRef.current) {
        wsRef.current.close();
        wsRef.current = null;
      }
    };
  }, []);

  return {
    loading,
    error,
    sessionInfo,
    changes,
    connectedSessions,
    connectToNucleus,
    joinSession,
    leaveSession,
    getSessionInfo,
    listSessions,
    pushChanges,
    pullChanges,
    resolveConflict,
    getConflicts,
    forceSync,
    closeSession
  };
}
