import React, {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useState,
  ReactNode,
} from 'react';
import {
  checkRemoteBackendHealth,
  fetchSimulationStatus,
  getApiBaseUrl,
  getEngineInfo,
  isOfflineMode,
  SimulationStatusDto,
} from '../services/forgeApi';

interface BackendContextType {
  /** True when any usable engine exists (the in-browser engine always counts). */
  connected: boolean;
  /** Offline-only mode: no remote backend configured. */
  offline: boolean;
  engineName: string;
  apiBase: string;
  simulation: SimulationStatusDto | null;
  refresh: () => Promise<void>;
  lastError: string | null;
  /** The in-browser engine is always ready — reported separately from remote health. */
  localEngineReady: boolean;
  /** Remote backend explicitly enabled via VITE_ENABLE_BACKEND. */
  remoteEnabled: boolean;
  /** Remote /api/status reachable right now. null = not enabled or not checked yet. */
  remoteReachable: boolean | null;
  /** Last probe detail, e.g. "HTTP 500", "timed out", "Responding". */
  remoteDetail: string;
  /** Timestamp of the last completed probe. */
  lastCheckedAt: number | null;
}

const BackendContext = createContext<BackendContextType>({
  connected: true,
  offline: true,
  engineName: 'SZM Forge Offline Engine',
  apiBase: '',
  simulation: null,
  refresh: async () => {},
  lastError: null,
  localEngineReady: true,
  remoteEnabled: false,
  remoteReachable: null,
  remoteDetail: 'Not checked',
  lastCheckedAt: null,
});

export const BackendProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const engine = getEngineInfo();
  const [simulation, setSimulation] = useState<SimulationStatusDto | null>(null);
  const [lastError, setLastError] = useState<string | null>(null);
  const [remoteEnabled, setRemoteEnabled] = useState(false);
  const [remoteReachable, setRemoteReachable] = useState<boolean | null>(null);
  const [remoteDetail, setRemoteDetail] = useState('Not checked');
  const [lastCheckedAt, setLastCheckedAt] = useState<number | null>(null);

  const refresh = useCallback(async () => {
    // Probe the remote service honestly; never pre-set "connected".
    const health = await checkRemoteBackendHealth();
    setRemoteEnabled(health.enabled);
    setRemoteReachable(health.enabled ? health.reachable : null);
    setRemoteDetail(health.detail);
    setLastCheckedAt(Date.now());
    setLastError(health.enabled && !health.reachable ? health.detail : null);
    const sim = await fetchSimulationStatus();
    setSimulation(sim);
  }, []);

  useEffect(() => {
    refresh();
    if (!isOfflineMode()) {
      const id = window.setInterval(refresh, 5000);
      return () => window.clearInterval(id);
    }
    return undefined;
  }, [refresh]);

  // `connected` means "an engine is available for computation". The in-browser
  // engine always qualifies; remote panels must use remoteReachable instead.
  const engineAvailable = true;

  return (
    <BackendContext.Provider
      value={{
        connected: engineAvailable,
        offline: isOfflineMode(),
        engineName: engine.name,
        apiBase: getApiBaseUrl(),
        simulation,
        refresh,
        lastError,
        localEngineReady: true,
        remoteEnabled,
        remoteReachable,
        remoteDetail,
        lastCheckedAt,
      }}
    >
      {children}
    </BackendContext.Provider>
  );
};

export const useBackend = () => useContext(BackendContext);
