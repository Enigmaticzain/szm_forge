import React, { useState, useEffect } from 'react';
import { useOmniverseAPI } from '../hooks/useOmniverseAPI';

type TabType = 'connect' | 'session' | 'sync' | 'conflicts';

export const OmniverseLiveSyncPanel: React.FC = () => {
  const {
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
  } = useOmniverseAPI();

  const [activeTab, setActiveTab] = useState<TabType>('connect');

  // Connect tab state
  const [nucleusUrl, setNucleusUrl] = useState('http://localhost:3100');
  const [username, setUsername] = useState('admin');
  const [password, setPassword] = useState('');
  const [projectPath, setProjectPath] = useState('/projects/default');
  const [sceneFile, setSceneFile] = useState('scene.usd');

  // Session tab state
  const [sessionId, setSessionId] = useState('');
  const [userId, setUserId] = useState('user_' + Math.random().toString(36).substr(2, 9));
  const [displayName, setDisplayName] = useState('');

  // Sync tab state
  const [pendingChanges, setPendingChanges] = useState<any[]>([]);
  const [autoSync, setAutoSync] = useState(false);

  // Conflict resolution
  const [conflicts, setConflicts] = useState<any[]>([]);
  const [selectedConflict, setSelectedConflict] = useState<string | null>(null);
  const [mergeStrategy, setMergeStrategy] = useState('last_write_wins');

  // Load sessions on mount
  useEffect(() => {
    listSessions().catch(console.error);
  }, [listSessions]);

  const handleConnect = async () => {
    try {
      const result = await connectToNucleus(nucleusUrl, username, password, projectPath, sceneFile);
      setSessionId(result.session_id);
      setActiveTab('session');
    } catch (e) {
      console.error('Connection failed:', e);
    }
  };

  const handleJoin = async () => {
    if (sessionId && userId) {
      try {
        await joinSession(sessionId, userId, displayName);
        await getSessionInfo(sessionId);
      } catch (e) {
        console.error('Join failed:', e);
      }
    }
  };

  const handleLeave = async () => {
    if (sessionId && userId) {
      try {
        await leaveSession(sessionId, userId);
      } catch (e) {
        console.error('Leave failed:', e);
      }
    }
  };

  const handlePush = async () => {
    if (sessionId && pendingChanges.length > 0) {
      try {
        await pushChanges(sessionId, pendingChanges);
        setPendingChanges([]);
      } catch (e) {
        console.error('Push failed:', e);
      }
    }
  };

  const handlePull = async () => {
    if (sessionId) {
      try {
        await pullChanges(sessionId);
      } catch (e) {
        console.error('Pull failed:', e);
      }
    }
  };

  const handleForceSync = async () => {
    if (sessionId) {
      try {
        await forceSync(sessionId);
      } catch (e) {
        console.error('Sync failed:', e);
      }
    }
  };

  const handleLoadConflicts = async () => {
    if (sessionId) {
      try {
        const result = await getConflicts(sessionId);
        setConflicts(result.conflicts || []);
      } catch (e) {
        console.error('Load conflicts failed:', e);
      }
    }
  };

  const handleResolveConflict = async () => {
    if (sessionId && selectedConflict) {
      try {
        await resolveConflict(sessionId, selectedConflict, mergeStrategy);
        setSelectedConflict(null);
        await handleLoadConflicts();
      } catch (e) {
        console.error('Resolve failed:', e);
      }
    }
  };

  const handleCloseSession = async () => {
    if (sessionId) {
      try {
        await closeSession(sessionId);
        setSessionId('');
        setActiveTab('connect');
      } catch (e) {
        console.error('Close failed:', e);
      }
    }
  };

  const handleAddChange = () => {
    const newChange = {
      change_type: 'property_changed',
      prim_path: '/scene/cube',
      property_name: 'translate',
      old_value: '0 0 0',
      new_value: '1 0 0',
      timestamp: Date.now() / 1000,
      user_id: userId
    };
    setPendingChanges([...pendingChanges, newChange]);
  };

  return (
    <div className="omniverse-sync-panel" style={styles.container}>
      <h2 style={styles.title}>🌌 Omniverse LiveSync</h2>

      {error && (
        <div style={styles.errorBox}>
          <strong>Error:</strong> {error}
        </div>
      )}

      {/* Tab Navigation */}
      <div style={styles.tabs}>
        <button
          style={{...styles.tabButton, ...(activeTab === 'connect' ? styles.tabActive : {})}}
          onClick={() => setActiveTab('connect')}
        >
          Connect
        </button>
        <button
          style={{...styles.tabButton, ...(activeTab === 'session' ? styles.tabActive : {})}}
          onClick={() => setActiveTab('session')}
        >
          Session
        </button>
        <button
          style={{...styles.tabButton, ...(activeTab === 'sync' ? styles.tabActive : {})}}
          onClick={() => setActiveTab('sync')}
        >
          Sync
        </button>
        <button
          style={{...styles.tabButton, ...(activeTab === 'conflicts' ? styles.tabActive : {})}}
          onClick={() => setActiveTab('conflicts')}
        >
          Conflicts
        </button>
      </div>

      {/* Connect Tab */}
      {activeTab === 'connect' && (
        <div style={styles.tabContent}>
          <h3>Connect to Nucleus</h3>
          <div style={styles.formGroup}>
            <label>Nucleus URL:</label>
            <input
              type="text"
              value={nucleusUrl}
              onChange={(e) => setNucleusUrl(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.formGroup}>
            <label>Username:</label>
            <input
              type="text"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.formGroup}>
            <label>Password:</label>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.formGroup}>
            <label>Project Path:</label>
            <input
              type="text"
              value={projectPath}
              onChange={(e) => setProjectPath(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.formGroup}>
            <label>Scene File:</label>
            <input
              type="text"
              value={sceneFile}
              onChange={(e) => setSceneFile(e.target.value)}
              style={styles.input}
            />
          </div>
          <button
            onClick={handleConnect}
            disabled={loading}
            style={{...styles.button, ...styles.primaryButton}}
          >
            {loading ? 'Connecting...' : 'Connect to Nucleus'}
          </button>
          {sessionId && <p style={styles.success}>✓ Session ID: {sessionId}</p>}
        </div>
      )}

      {/* Session Tab */}
      {activeTab === 'session' && (
        <div style={styles.tabContent}>
          <h3>Session Management</h3>
          <div style={styles.formGroup}>
            <label>Session ID:</label>
            <input
              type="text"
              value={sessionId}
              onChange={(e) => setSessionId(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.formGroup}>
            <label>User ID:</label>
            <input
              type="text"
              value={userId}
              onChange={(e) => setUserId(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.formGroup}>
            <label>Display Name (optional):</label>
            <input
              type="text"
              value={displayName}
              onChange={(e) => setDisplayName(e.target.value)}
              style={styles.input}
            />
          </div>
          <div style={styles.buttonGroup}>
            <button
              onClick={handleJoin}
              disabled={loading || !sessionId || !userId}
              style={{...styles.button, ...styles.primaryButton}}
            >
              {loading ? 'Joining...' : 'Join Session'}
            </button>
            <button
              onClick={handleLeave}
              disabled={loading || !sessionId || !userId}
              style={{...styles.button, ...styles.secondaryButton}}
            >
              Leave Session
            </button>
            <button
              onClick={handleCloseSession}
              disabled={loading || !sessionId}
              style={{...styles.button, ...styles.dangerButton}}
            >
              Close Session
            </button>
          </div>

          {sessionInfo && (
            <div style={styles.infoBox}>
              <h4>Session Status</h4>
              <p><strong>State:</strong> <span style={styles.status}>{sessionInfo.state}</span></p>
              <p><strong>Users Connected:</strong> {sessionInfo.connected_users.length}</p>
              <p><strong>Users:</strong> {sessionInfo.connected_users.join(', ') || 'None'}</p>
              <p><strong>Pending Changes:</strong> {sessionInfo.change_count}</p>
              <p><strong>Latency:</strong> {sessionInfo.sync_latency_ms}ms</p>
              <p><strong>Created:</strong> {new Date(sessionInfo.created_at).toLocaleString()}</p>
              <p><strong>Last Sync:</strong> {new Date(sessionInfo.last_sync).toLocaleString()}</p>
            </div>
          )}
        </div>
      )}

      {/* Sync Tab */}
      {activeTab === 'sync' && (
        <div style={styles.tabContent}>
          <h3>Synchronization Control</h3>
          <div style={styles.formGroup}>
            <label>
              <input
                type="checkbox"
                checked={autoSync}
                onChange={(e) => setAutoSync(e.target.checked)}
              />
              {' '}Enable Auto Sync
            </label>
          </div>
          <div style={styles.buttonGroup}>
            <button
              onClick={handlePull}
              disabled={loading || !sessionId}
              style={{...styles.button, ...styles.primaryButton}}
            >
              Pull Remote Changes ({changes.length})
            </button>
            <button
              onClick={handlePush}
              disabled={loading || !sessionId || pendingChanges.length === 0}
              style={{...styles.button, ...styles.primaryButton}}
            >
              Push Local Changes ({pendingChanges.length})
            </button>
            <button
              onClick={handleForceSync}
              disabled={loading || !sessionId}
              style={{...styles.button, ...styles.secondaryButton}}
            >
              Force Sync
            </button>
            <button
              onClick={handleAddChange}
              disabled={!sessionId}
              style={{...styles.button, ...styles.secondaryButton}}
            >
              Add Test Change
            </button>
          </div>

          <div style={styles.changesContainer}>
            <div style={styles.changesColumn}>
              <h4>Remote Changes ({changes.length})</h4>
              {changes.length === 0 ? (
                <p style={styles.emptyText}>No remote changes</p>
              ) : (
                <ul style={styles.changesList}>
                  {changes.slice(-10).map((change, idx) => (
                    <li key={idx} style={styles.changeItem}>
                      <strong>{change.change_type}</strong><br />
                      {change.prim_path}
                      {change.property_name && <> - {change.property_name}</>}
                      <br /><small>by {change.user_id}</small>
                    </li>
                  ))}
                </ul>
              )}
            </div>
            <div style={styles.changesColumn}>
              <h4>Pending Changes ({pendingChanges.length})</h4>
              {pendingChanges.length === 0 ? (
                <p style={styles.emptyText}>No pending changes</p>
              ) : (
                <ul style={styles.changesList}>
                  {pendingChanges.map((change, idx) => (
                    <li key={idx} style={styles.changeItem}>
                      <strong>{change.change_type}</strong><br />
                      {change.prim_path} - {change.property_name}
                      <br /><small>pending</small>
                    </li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        </div>
      )}

      {/* Conflicts Tab */}
      {activeTab === 'conflicts' && (
        <div style={styles.tabContent}>
          <h3>Conflict Resolution</h3>
          <button
            onClick={handleLoadConflicts}
            disabled={loading || !sessionId}
            style={{...styles.button, ...styles.primaryButton}}
          >
            Load Conflicts
          </button>

          {conflicts.length === 0 ? (
            <p style={styles.success}>✓ No conflicts detected</p>
          ) : (
            <div style={styles.conflictsList}>
              {conflicts.map((conflict, idx) => (
                <div
                  key={idx}
                  style={{...styles.conflictItem, ...(selectedConflict === conflict.conflict_id ? styles.selected : {})}}
                  onClick={() => setSelectedConflict(conflict.conflict_id)}
                >
                  <strong>Conflict at {conflict.prim_path}</strong>
                  <p>Local: {conflict.local_change?.new_value}</p>
                  <p>Remote: {conflict.remote_change?.new_value}</p>
                </div>
              ))}
            </div>
          )}

          {selectedConflict && (
            <div style={styles.resolutionBox}>
              <h4>Resolve Selected Conflict</h4>
              <div style={styles.formGroup}>
                <label>Merge Strategy:</label>
                <select
                  value={mergeStrategy}
                  onChange={(e) => setMergeStrategy(e.target.value)}
                  style={styles.input}
                >
                  <option value="last_write_wins">Last Write Wins (Use Remote)</option>
                  <option value="keep_local">Keep Local</option>
                  <option value="merge_properties">Merge Properties</option>
                  <option value="manual">Manual (Pending)</option>
                </select>
              </div>
              <button
                onClick={handleResolveConflict}
                disabled={loading}
                style={{...styles.button, ...styles.primaryButton}}
              >
                {loading ? 'Resolving...' : 'Resolve Conflict'}
              </button>
            </div>
          )}
        </div>
      )}
    </div>
  );
};

// ============= Inline Styles =============

const styles: { [key: string]: React.CSSProperties } = {
  container: {
    padding: '20px',
    fontFamily: 'system-ui, -apple-system, sans-serif',
    backgroundColor: '#f5f5f5',
    borderRadius: '8px',
    boxShadow: '0 2px 8px rgba(0,0,0,0.1)'
  },
  title: {
    fontSize: '24px',
    fontWeight: 'bold',
    marginBottom: '16px',
    color: '#333'
  },
  errorBox: {
    padding: '12px',
    backgroundColor: '#fee',
    color: '#c33',
    borderRadius: '4px',
    marginBottom: '16px',
    border: '1px solid #fcc'
  },
  tabs: {
    display: 'flex',
    gap: '8px',
    marginBottom: '16px',
    borderBottom: '2px solid #ddd'
  },
  tabButton: {
    padding: '10px 16px',
    backgroundColor: '#fff',
    border: 'none',
    borderBottom: '2px solid transparent',
    cursor: 'pointer',
    fontSize: '14px',
    fontWeight: '500',
    transition: 'all 0.2s'
  },
  tabActive: {
    borderBottomColor: '#007bff',
    color: '#007bff'
  },
  tabContent: {
    backgroundColor: '#fff',
    padding: '20px',
    borderRadius: '4px'
  },
  formGroup: {
    marginBottom: '16px',
    display: 'flex',
    flexDirection: 'column'
  },
  input: {
    padding: '8px 12px',
    border: '1px solid #ccc',
    borderRadius: '4px',
    fontSize: '14px',
    marginTop: '4px'
  },
  button: {
    padding: '10px 16px',
    border: 'none',
    borderRadius: '4px',
    cursor: 'pointer',
    fontSize: '14px',
    fontWeight: '500',
    transition: 'all 0.2s'
  },
  primaryButton: {
    backgroundColor: '#007bff',
    color: '#fff'
  },
  secondaryButton: {
    backgroundColor: '#6c757d',
    color: '#fff'
  },
  dangerButton: {
    backgroundColor: '#dc3545',
    color: '#fff'
  },
  buttonGroup: {
    display: 'flex',
    gap: '8px',
    flexWrap: 'wrap',
    marginBottom: '16px'
  },
  success: {
    color: '#28a745',
    fontWeight: '500',
    marginTop: '12px'
  },
  infoBox: {
    backgroundColor: '#f0f7ff',
    padding: '16px',
    borderRadius: '4px',
    marginTop: '16px',
    border: '1px solid #b3d9ff'
  },
  status: {
    backgroundColor: '#e3f2fd',
    padding: '2px 8px',
    borderRadius: '3px',
    fontFamily: 'monospace'
  },
  changesContainer: {
    display: 'flex',
    gap: '16px',
    marginTop: '16px'
  },
  changesColumn: {
    flex: 1,
    backgroundColor: '#f9f9f9',
    padding: '12px',
    borderRadius: '4px',
    border: '1px solid #e0e0e0'
  },
  changesList: {
    listStyle: 'none',
    padding: 0,
    margin: 0
  },
  changeItem: {
    padding: '8px',
    marginBottom: '8px',
    backgroundColor: '#fff',
    borderLeft: '3px solid #007bff',
    paddingLeft: '12px',
    borderRadius: '2px',
    fontSize: '13px'
  },
  emptyText: {
    color: '#999',
    fontStyle: 'italic'
  },
  conflictsList: {
    marginTop: '16px',
    display: 'flex',
    flexDirection: 'column',
    gap: '8px'
  },
  conflictItem: {
    padding: '12px',
    backgroundColor: '#fff3cd',
    borderLeft: '4px solid #ff9800',
    cursor: 'pointer',
    borderRadius: '4px',
    transition: 'all 0.2s'
  },
  selected: {
    backgroundColor: '#ffe0b2',
    borderLeftColor: '#f57c00'
  },
  resolutionBox: {
    marginTop: '16px',
    padding: '16px',
    backgroundColor: '#e8f5e9',
    borderRadius: '4px',
    border: '1px solid #c8e6c9'
  }
};
