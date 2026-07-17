import React, {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useState,
  ReactNode,
} from 'react';
import {
  pingBackend,
  fetchSimulationStatus,
  getApiBaseUrl,
  SimulationStatusDto,
} from '../services/forgeApi';

interface BackendContextType {
  connected: boolean;
  apiBase: string;
  simulation: SimulationStatusDto | null;
  refresh: () => Promise<void>;
  lastError: string | null;
}

const BackendContext = createContext<BackendContextType>({
  connected: false,
  apiBase: getApiBaseUrl(),
  simulation: null,
  refresh: async () => {},
  lastError: null,
});

export const BackendProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [connected, setConnected] = useState(false);
  const [simulation, setSimulation] = useState<SimulationStatusDto | null>(null);
  const [lastError, setLastError] = useState<string | null>(null);

  const refresh = useCallback(async () => {
    const ok = await pingBackend();
    setConnected(ok);
    if (!ok) {
      setLastError('C++ backend offline — start SZM_Forge desktop build');
      setSimulation(null);
      return;
    }
    setLastError(null);
    const sim = await fetchSimulationStatus();
    setSimulation(sim);
  }, []);

  useEffect(() => {
    refresh();
    const id = window.setInterval(refresh, 2000);
    return () => window.clearInterval(id);
  }, [refresh]);

  return (
    <BackendContext.Provider
      value={{
        connected,
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
