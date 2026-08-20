import React, { useState } from 'react';
import { Hexagon, LogIn, UserPlus, Eye, EyeOff } from 'lucide-react';
import { useAuth } from '../store/AuthContext';

export const LoginScreen: React.FC = () => {
  const { login, register } = useAuth();
  const [mode, setMode] = useState<'login' | 'register'>('login');
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [showPw, setShowPw] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState(false);

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError(null);
    setLoading(true);
    const err = mode === 'login'
      ? await login(username, password)
      : await register(username, password);
    setLoading(false);
    if (err) setError(err);
  };

  return (
    <div className="w-full h-full flex items-center justify-center bg-forge-black">
      <div className="w-[360px] bg-forge-panel border border-forge-border rounded-xl shadow-2xl overflow-hidden">
        {/* Header */}
        <div className="p-6 border-b border-forge-border/50 flex flex-col items-center gap-3">
          <div className="relative">
            <Hexagon size={40} className="text-forge-accent" strokeWidth={2} />
            <div className="absolute inset-0 flex items-center justify-center">
              <div className="w-2.5 h-2.5 bg-forge-accent rounded-full" />
            </div>
          </div>
          <div className="text-center">
            <div className="text-[14px] font-bold tracking-[0.3em] text-forge-accent">SZM FORGE</div>
            <div className="text-[9px] tracking-[0.2em] text-forge-text-muted font-mono mt-0.5">INDUSTRIAL INTELLIGENCE</div>
          </div>
        </div>

        {/* Tab switcher */}
        <div className="flex border-b border-forge-border/50">
          {(['login', 'register'] as const).map(m => (
            <button
              key={m}
              onClick={() => { setMode(m); setError(null); }}
              className={`flex-1 py-2.5 text-[10px] font-semibold tracking-widest transition-all ${
                mode === m
                  ? 'text-forge-accent border-b-2 border-forge-accent bg-forge-accent/5'
                  : 'text-forge-text-muted hover:text-forge-text-dim'
              }`}
            >
              {m === 'login' ? 'SIGN IN' : 'REGISTER'}
            </button>
          ))}
        </div>

        {/* Form */}
        <form onSubmit={handleSubmit} className="p-6 space-y-4">
          <div>
            <label className="text-[9px] font-semibold tracking-widest text-forge-text-dim block mb-1.5">USERNAME</label>
            <input
              value={username}
              onChange={e => setUsername(e.target.value)}
              autoFocus
              required
              minLength={3}
              className="w-full px-3 py-2 rounded bg-forge-surface border border-forge-border text-[12px] text-forge-text placeholder-forge-text-muted focus:outline-none focus:border-forge-accent/50 transition-colors"
              placeholder="your_username"
            />
          </div>

          <div>
            <label className="text-[9px] font-semibold tracking-widest text-forge-text-dim block mb-1.5">PASSWORD</label>
            <div className="relative">
              <input
                type={showPw ? 'text' : 'password'}
                value={password}
                onChange={e => setPassword(e.target.value)}
                required
                minLength={6}
                className="w-full px-3 py-2 pr-9 rounded bg-forge-surface border border-forge-border text-[12px] text-forge-text placeholder-forge-text-muted focus:outline-none focus:border-forge-accent/50 transition-colors"
                placeholder="••••••••"
              />
              <button
                type="button"
                onClick={() => setShowPw(v => !v)}
                className="absolute right-2.5 top-1/2 -translate-y-1/2 text-forge-text-muted hover:text-forge-text-dim"
              >
                {showPw ? <EyeOff size={13} /> : <Eye size={13} />}
              </button>
            </div>
            {mode === 'register' && (
              <div className="text-[8px] text-forge-text-muted mt-1">Minimum 6 characters</div>
            )}
          </div>

          {error && (
            <div className="px-3 py-2 rounded bg-forge-red/10 border border-forge-red/30 text-[10px] text-forge-red">
              {error}
            </div>
          )}

          <button
            type="submit"
            disabled={loading}
            className="w-full py-2.5 rounded bg-forge-accent/15 text-forge-accent border border-forge-accent/30 text-[11px] font-bold tracking-widest hover:bg-forge-accent/25 transition-all disabled:opacity-50 flex items-center justify-center gap-2"
          >
            {loading ? (
              <span className="animate-pulse">PROCESSING...</span>
            ) : mode === 'login' ? (
              <><LogIn size={13} /> SIGN IN</>
            ) : (
              <><UserPlus size={13} /> CREATE ACCOUNT</>
            )}
          </button>
        </form>

        <div className="px-6 pb-4 text-center">
          <span className="text-[8px] text-forge-text-muted font-mono">
            {mode === 'login' ? 'No account? ' : 'Already registered? '}
          </span>
          <button
            onClick={() => { setMode(mode === 'login' ? 'register' : 'login'); setError(null); }}
            className="text-[8px] text-forge-accent font-mono hover:underline"
          >
            {mode === 'login' ? 'Register here' : 'Sign in'}
          </button>
        </div>
      </div>
    </div>
  );
};
