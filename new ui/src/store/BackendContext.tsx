import React, {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useState,
  ReactNode,
} from 'react';
import {
  fetchSimulationStatus,
  getApiBaseUrl,
  getEngineInfo,
  isOfflineMode,
  SimulationStatusDto,
} from '../services/forgeApi';

interface BackendContextType {
  connected: boolean;
  offline: boolean;
  engineName: string;
  apiBase: string;
  simulation: SimulationStatusDto | null;
  refresh: () => Promise<void>;
  lastError: string | null;
}

const BackendContext = createContext<BackendContextType>({
  connected: true,
  offline: true,
  engineName: 'SZM Forge Offline Engine',
  apiBase: '',
  simulation: null,
  refresh: async () => {},
  lastError: null,
});

export const BackendProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const engine = getEngineInfo();
  const [connected, setConnected] = useState(true);
  const [simulation, setSimulation] = useState<SimulationStatusDto | null>(null);
  const [lastError, setLastError] = useState<string | null>(null);

  const refresh = useCallback(async () => {
    setConnected(true);
    setLastError(null);
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

  return (
    <BackendContext.Provider
      value={{
        connected,
        offline: isOfflineMode(),
        engineName: engine.name,
        apiBase: getApiBaseUrl(),
        simulation,
        refresh,
        lastError,
      }}
    >
      {children}
    </BackendContext.Provider>
  );
};

export const useBackend = () => useContext(BackendContext);
