import React, { useState } from 'react';
import {
  Bell, AlertTriangle, CheckCircle, Info, XCircle,
  X, Clock, Trash2, Eye, Settings
} from 'lucide-react';

interface Notification {
  id: string;
  type: 'info' | 'success' | 'warning' | 'critical';
  title: string;
  message: string;
  timestamp: string;
  read: boolean;
  source: string;
}

const notifications: Notification[] = [
  { id: 'n1', type: 'critical', title: 'Thermal Limit Exceeded', message: 'Blade root temperature has exceeded the design limit of 1200°C. Current: 1247°C.', timestamp: '2 min ago', read: false, source: 'Thermal Simulation' },
  { id: 'n2', type: 'warning', title: 'Safety Factor Below Threshold', message: 'Fatigue safety factor dropped to 1.82, below the 2.0 target.', timestamp: '5 min ago', read: false, source: 'FEA Solver' },
  { id: 'n3', type: 'success', title: 'Simulation Complete', message: 'Static structural FEA converged successfully in 847 iterations.', timestamp: '8 min ago', read: false, source: 'Solver Engine' },
  { id: 'n4', type: 'info', title: 'AI Recommendation Available', message: 'Topology optimization suggests 12.4% mass reduction opportunity.', timestamp: '12 min ago', read: true, source: 'AI Engine' },
  { id: 'n5', type: 'warning', title: 'Pump P-01 Vibration Alert', message: 'Bearing vibration exceeding normal operating range.', timestamp: '18 min ago', read: true, source: 'Digital Twin' },
  { id: 'n6', type: 'success', title: 'Mesh Generated', message: '8.4M elements created with 98.4% quality score.', timestamp: '25 min ago', read: true, source: 'Mesh Engine' },
  { id: 'n7', type: 'info', title: 'Material Updated', message: 'Ti-6Al-4V properties synced from material database.', timestamp: '32 min ago', read: true, source: 'Material Library' },
  { id: 'n8', type: 'critical', title: 'System Beta-01 Efficiency Drop', message: 'Operating efficiency has fallen below 90% threshold.', timestamp: '45 min ago', read: true, source: 'Fleet Monitor' },
];

const typeConfig = {
  info: { icon: <Info size={14} />, color: 'text-forge-accent', bg: 'bg-forge-accent/10', border: 'border-forge-accent/20' },
  success: { icon: <CheckCircle size={14} />, color: 'text-forge-green', bg: 'bg-forge-green/10', border: 'border-forge-green/20' },
  warning: { icon: <AlertTriangle size={14} />, color: 'text-forge-yellow', bg: 'bg-forge-yellow/10', border: 'border-forge-yellow/20' },
  critical: { icon: <XCircle size={14} />, color: 'text-forge-red', bg: 'bg-forge-red/10', border: 'border-forge-red/20' },
};

interface Props {
  isOpen: boolean;
  onClose: () => void;
}

export const NotificationCenter: React.FC<Props> = ({ isOpen, onClose }) => {
  const [filter, setFilter] = useState<'all' | 'unread' | 'critical'>('all');
  
  if (!isOpen) return null;

  const unreadCount = notifications.filter(n => !n.read).length;
  const criticalCount = notifications.filter(n => n.type === 'critical').length;

  const filtered = notifications.filter(n => {
    if (filter === 'unread') return !n.read;
    if (filter === 'critical') return n.type === 'critical';
    return true;
  });

  return (
    <div className="fixed inset-0 z-[100] flex justify-end">
      {/* Backdrop */}
      <div className="absolute inset-0 bg-black/50 backdrop-blur-sm" onClick={onClose} />
      
      {/* Panel */}
      <div className="relative w-[420px] h-full bg-forge-panel border-l border-forge-border flex flex-col animate-fade-in">
        {/* Header */}
        <div className="h-[56px] border-b border-forge-border px-4 flex items-center justify-between flex-shrink-0">
          <div className="flex items-center gap-2">
            <Bell size={16} className="text-forge-accent" />
            <span className="text-[12px] font-bold tracking-widest text-forge-text">NOTIFICATIONS</span>
            {unreadCount > 0 && (
              <span className="px-1.5 py-0.5 rounded-full text-[9px] font-mono bg-forge-accent/20 text-forge-accent">
                {unreadCount}
              </span>
            )}
          </div>
          <div className="flex items-center gap-1">
            <button className="p-1.5 rounded hover:bg-forge-surface text-forge-text-muted hover:text-forge-text-dim transition-colors">
              <Settings size={14} />
            </button>
            <button className="p-1.5 rounded hover:bg-forge-surface text-forge-text-muted hover:text-forge-text-dim transition-colors" onClick={onClose}>
              <X size={14} />
            </button>
          </div>
        </div>

        {/* Filters */}
        <div className="px-4 py-2 border-b border-forge-border/50 flex items-center gap-2">
          {[
            { id: 'all', label: 'All', count: notifications.length },
            { id: 'unread', label: 'Unread', count: unreadCount },
            { id: 'critical', label: 'Critical', count: criticalCount },
          ].map(f => (
            <button
              key={f.id}
              onClick={() => setFilter(f.id as typeof filter)}
              className={`px-2.5 py-1 rounded text-[9px] font-mono transition-all ${
                filter === f.id
                  ? 'bg-forge-accent/10 text-forge-accent border border-forge-accent/30'
                  : 'text-forge-text-muted hover:text-forge-text-dim border border-transparent'
              }`}
            >
              {f.label} ({f.count})
            </button>
          ))}
          <div className="ml-auto flex items-center gap-1">
            <button className="p-1 rounded hover:bg-forge-surface text-forge-text-muted hover:text-forge-text-dim transition-colors" title="Mark all read">
              <Eye size={12} />
            </button>
            <button className="p-1 rounded hover:bg-forge-surface text-forge-text-muted hover:text-forge-red transition-colors" title="Clear all">
              <Trash2 size={12} />
            </button>
          </div>
        </div>

        {/* Notifications List */}
        <div className="flex-1 overflow-y-auto">
          {filtered.length === 0 ? (
            <div className="flex flex-col items-center justify-center h-full text-forge-text-muted">
              <Bell size={32} className="opacity-30 mb-2" />
              <span className="text-[10px] font-mono">No notifications</span>
            </div>
          ) : (
            <div className="p-2 space-y-1">
              {filtered.map(notif => {
                const config = typeConfig[notif.type];
                return (
                  <div
                    key={notif.id}
                    className={`p-3 rounded-lg border transition-all cursor-pointer hover:border-forge-accent/30 ${
                      notif.read ? 'border-forge-border/30 bg-forge-surface/10' : `${config.border} ${config.bg}`
                    }`}
                  >
                    <div className="flex items-start gap-2.5">
                      <div className={`flex-shrink-0 ${config.color}`}>
                        {config.icon}
                      </div>
                      <div className="flex-1 min-w-0">
                        <div className="flex items-center justify-between gap-2 mb-0.5">
                          <span className={`text-[10px] font-semibold ${notif.read ? 'text-forge-text-dim' : 'text-forge-text'}`}>
                            {notif.title}
                          </span>
                          {!notif.read && (
                            <div className="w-1.5 h-1.5 rounded-full bg-forge-accent flex-shrink-0" />
                          )}
                        </div>
                        <p className="text-[9px] text-forge-text-muted leading-relaxed mb-1.5">
                          {notif.message}
                        </p>
                        <div className="flex items-center gap-3">
                          <span className="text-[8px] font-mono text-forge-text-muted flex items-center gap-1">
                            <Clock size={8} />{notif.timestamp}
                          </span>
                          <span className="text-[8px] font-mono text-forge-accent">
                            {notif.source}
                          </span>
                        </div>
                      </div>
                    </div>
                  </div>
                );
              })}
            </div>
          )}
        </div>

        {/* Footer */}
        <div className="h-[40px] border-t border-forge-border px-4 flex items-center justify-center">
          <button className="text-[9px] font-mono text-forge-accent hover:text-forge-cyan transition-colors">
            View All Notifications
          </button>
        </div>
      </div>
    </div>
  );
};
