import { SensorData } from '../hooks/useSimulationData';

interface TelemetryPanelProps {
  sensors: SensorData[];
}

export default function TelemetryPanel({ sensors }: TelemetryPanelProps) {
  return (
    <div className="h-full flex flex-col bg-forge-surface overflow-hidden">
      <div className="px-3 py-2 border-b border-forge-border flex items-center justify-between">
        <div className="flex items-center gap-2">
          <div className="w-1.5 h-1.5 rounded-full bg-forge-green animate-pulse-glow" />
          <span className="text-[10px] font-bold text-forge-text tracking-wider">LIVE TELEMETRY</span>
        </div>
        <span className="text-[8px] text-forge-text-muted font-mono">8 CHANNELS ACTIVE</span>
      </div>
      
      <div className="flex-1 overflow-y-auto p-2 space-y-1.5">
        {sensors.map(sensor => (
          <SensorCard key={sensor.id} sensor={sensor} />
        ))}
      </div>
    </div>
  );
}

function SensorCard({ sensor }: { sensor: SensorData }) {
  const ratio = sensor.value / sensor.max;
  const sparklinePoints = sensor.history.map((p, i) => {
    const x = (i / (sensor.history.length - 1)) * 100;
    const y = 100 - ((p.value - sensor.min) / (sensor.max - sensor.min)) * 100;
    return `${x},${y}`;
  }).join(' ');

  const statusColor = sensor.status === 'critical' ? '#ff3344' : 
                       sensor.status === 'warning' ? '#ffaa00' : '#00ff88';

  return (
    <div className={`glass-panel rounded-lg p-2.5 transition-all duration-300 ${
      sensor.status === 'critical' ? 'glow-danger' :
      sensor.status === 'warning' ? 'border-forge-warning/20' : ''
    }`}>
      <div className="flex items-center justify-between mb-1.5">
        <div className="flex items-center gap-1.5">
          <div className="w-1.5 h-1.5 rounded-full" style={{ backgroundColor: statusColor }} />
          <span className="text-[9px] font-medium text-forge-text-dim uppercase tracking-wider">
            {sensor.name}
          </span>
        </div>
        <span className={`text-[8px] font-bold tracking-wider ${
          sensor.status === 'critical' ? 'text-forge-danger' :
          sensor.status === 'warning' ? 'text-forge-warning' : 'text-forge-green'
        }`}>
          {sensor.status.toUpperCase()}
        </span>
      </div>

      <div className="flex items-end justify-between mb-2">
        <div className="flex items-baseline gap-1">
          <span className="text-lg font-bold text-forge-text font-mono tabular-nums">
            {sensor.value.toFixed(1)}
          </span>
          <span className="text-[9px] text-forge-text-muted">{sensor.unit}</span>
        </div>
        <div className="text-[8px] text-forge-text-muted text-right">
          <div>MAX: {sensor.max} {sensor.unit}</div>
        </div>
      </div>

      {/* Progress bar */}
      <div className="h-1 bg-forge-panel rounded-full overflow-hidden mb-2">
        <div 
          className="h-full rounded-full transition-all duration-500"
          style={{ 
            width: `${ratio * 100}%`,
            background: sensor.status === 'critical' 
              ? 'linear-gradient(90deg, #ff3344, #ff6644)'
              : sensor.status === 'warning'
              ? 'linear-gradient(90deg, #ffaa00, #ffcc44)'
              : 'linear-gradient(90deg, #00ff88, #00cc66)',
          }}
        />
      </div>

      {/* Sparkline */}
      <div className="h-8 w-full">
        <svg viewBox="0 0 100 100" preserveAspectRatio="none" className="w-full h-full">
          <defs>
            <linearGradient id={`grad-${sensor.id}`} x1="0" y1="0" x2="0" y2="1">
              <stop offset="0%" stopColor={statusColor} stopOpacity="0.3" />
              <stop offset="100%" stopColor={statusColor} stopOpacity="0" />
            </linearGradient>
          </defs>
          {/* Area fill */}
          <polygon
            points={`0,100 ${sparklinePoints} 100,100`}
            fill={`url(#grad-${sensor.id})`}
          />
          {/* Line */}
          <polyline
            points={sparklinePoints}
            fill="none"
            stroke={statusColor}
            strokeWidth="1.5"
            vectorEffect="non-scaling-stroke"
          />
        </svg>
      </div>
    </div>
  );
}
