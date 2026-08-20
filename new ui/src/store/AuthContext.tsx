import React, { createContext, useCallback, useContext, useEffect, useState } from 'react';

const AI_BASE = 'http://localhost:8000/api';
const TOKEN_KEY = 'szm-auth-token';
const USER_KEY = 'szm-auth-user';

export interface AuthUser {
  username: string;
  projects: string[];
}

interface AuthContextType {
  user: AuthUser | null;
  token: string | null;
  login: (username: string, password: string) => Promise<string | null>;
  register: (username: string, password: string) => Promise<string | null>;
  logout: () => void;
  addProject: (name: string) => Promise<void>;
  removeProject: (name: string) => Promise<void>;
  authFetch: (input: RequestInfo, init?: RequestInit) => Promise<Response>;
}

const AuthContext = createContext<AuthContextType | null>(null);

export const AuthProvider: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const [token, setToken] = useState<string | null>(() => localStorage.getItem(TOKEN_KEY));
  const [user, setUser] = useState<AuthUser | null>(() => {
    try { return JSON.parse(localStorage.getItem(USER_KEY) ?? 'null'); } catch { return null; }
  });

  const persist = (t: string, u: AuthUser) => {
    localStorage.setItem(TOKEN_KEY, t);
    localStorage.setItem(USER_KEY, JSON.stringify(u));
    setToken(t);
    setUser(u);
  };

  const logout = useCallback(() => {
    localStorage.removeItem(TOKEN_KEY);
    localStorage.removeItem(USER_KEY);
    setToken(null);
    setUser(null);
  }, []);

  // Refresh user info on mount if token exists
  useEffect(() => {
    if (!token) return;
    fetch(`${AI_BASE}/auth/me`, { headers: { Authorization: `Bearer ${token}` } })
      .then(r => r.ok ? r.json() : Promise.reject())
      .then((data: AuthUser) => setUser(data))
      .catch(logout);
  }, []); // eslint-disable-line react-hooks/exhaustive-deps

  const login = useCallback(async (username: string, password: string): Promise<string | null> => {
    try {
      const r = await fetch(`${AI_BASE}/auth/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password }),
      });
      if (!r.ok) { const e = await r.json(); return e.detail ?? 'Login failed'; }
      const data = await r.json();
      persist(data.access_token, { username: data.username, projects: [] });
      // Fetch full profile
      const me = await fetch(`${AI_BASE}/auth/me`, { headers: { Authorization: `Bearer ${data.access_token}` } });
      if (me.ok) setUser(await me.json());
      return null;
    } catch { return 'Network error'; }
  }, []);

  const register = useCallback(async (username: string, password: string): Promise<string | null> => {
    try {
      const r = await fetch(`${AI_BASE}/auth/register`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password }),
      });
      if (!r.ok) { const e = await r.json(); return e.detail ?? 'Registration failed'; }
      const data = await r.json();
      persist(data.access_token, { username: data.username, projects: [] });
      return null;
    } catch { return 'Network error'; }
  }, []);

  const authFetch = useCallback((input: RequestInfo, init: RequestInit = {}): Promise<Response> => {
    return fetch(input, {
      ...init,
      headers: { ...(init.headers ?? {}), ...(token ? { Authorization: `Bearer ${token}` } : {}) },
    });
  }, [token]);

  const refreshProjects = async (t: string) => {
    const r = await fetch(`${AI_BASE}/auth/projects`, { headers: { Authorization: `Bearer ${t}` } });
    if (r.ok) { const d = await r.json(); setUser(u => u ? { ...u, projects: d.projects } : u); }
  };

  const addProject = useCallback(async (name: string) => {
    if (!token) return;
    await fetch(`${AI_BASE}/auth/projects`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', Authorization: `Bearer ${token}` },
      body: JSON.stringify({ project_name: name }),
    });
    await refreshProjects(token);
  }, [token]);

  const removeProject = useCallback(async (name: string) => {
    if (!token) return;
    await fetch(`${AI_BASE}/auth/projects/${encodeURIComponent(name)}`, {
      method: 'DELETE',
      headers: { Authorization: `Bearer ${token}` },
    });
    await refreshProjects(token);
  }, [token]);

  return (
    <AuthContext.Provider value={{ user, token, login, register, logout, addProject, removeProject, authFetch }}>
      {children}
    </AuthContext.Provider>
  );
};

export function useAuth(): AuthContextType {
  const ctx = useContext(AuthContext);
  if (!ctx) throw new Error('useAuth must be used within AuthProvider');
  return ctx;
}
