import React, { useState } from 'react';
import { Lock, User } from 'lucide-react';

interface Props {
  onLoginSuccess: (token: string) => void;
}

export const AuthScreen: React.FC<Props> = ({ onLoginSuccess }) => {
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);
    setError('');

    try {
      const res = await fetch('http://localhost:8000/api/auth/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ username, password })
      });
      const data = await res.json();
      
      if (data.access_token) {
        onLoginSuccess(data.access_token);
      } else {
        setError(data.error || 'Login failed');
      }
    } catch (err) {
      setError('Connection error to Authentication server.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="w-full h-full flex items-center justify-center bg-forge-black text-forge-text relative z-[9999]">
      <div className="bg-forge-surface border border-forge-border p-8 rounded shadow-lg w-full max-w-sm">
        <h1 className="text-xl font-bold mb-6 text-center tracking-widest text-forge-accent">SZM FORGE</h1>
        <p className="text-[10px] text-forge-text-muted text-center mb-6 font-mono">AUTHORIZED PERSONNEL ONLY</p>
        
        {error && (
          <div className="mb-4 p-2 text-[11px] text-forge-red bg-forge-red/10 border border-forge-red/30 rounded text-center">
            {error}
          </div>
        )}

        <form onSubmit={handleSubmit} className="flex flex-col gap-4">
          <div>
            <label className="text-[10px] text-forge-text-muted font-mono mb-1 block">USERNAME</label>
            <div className="relative">
              <User size={14} className="absolute left-2.5 top-2.5 text-forge-text-muted" />
              <input 
                type="text" 
                value={username}
                onChange={e => setUsername(e.target.value)}
                className="w-full bg-forge-dark border border-forge-border rounded py-2 pl-8 pr-3 text-[12px] font-mono text-forge-text focus:border-forge-accent outline-none transition-colors"
                placeholder="admin"
                required
              />
            </div>
          </div>
          
          <div>
            <label className="text-[10px] text-forge-text-muted font-mono mb-1 block">PASSWORD</label>
            <div className="relative">
              <Lock size={14} className="absolute left-2.5 top-2.5 text-forge-text-muted" />
              <input 
                type="password" 
                value={password}
                onChange={e => setPassword(e.target.value)}
                className="w-full bg-forge-dark border border-forge-border rounded py-2 pl-8 pr-3 text-[12px] font-mono text-forge-text focus:border-forge-accent outline-none transition-colors"
                placeholder="forge2026"
                required
              />
            </div>
          </div>

          <button 
            type="submit" 
            disabled={loading}
            className="mt-4 bg-forge-accent text-forge-black font-bold font-mono text-[12px] py-2 rounded hover:bg-forge-accent-dim disabled:opacity-50 transition-colors"
          >
            {loading ? 'AUTHENTICATING...' : 'ACCESS SYSTEM'}
          </button>
        </form>
      </div>
    </div>
  );
};
