interface OptimizationChartProps {
  generations: { gen: number; fitness: number; diversity: number }[];
}

export default function OptimizationChart({ generations }: OptimizationChartProps) {
  if (generations.length === 0) return null;

  const latest = generations[generations.length - 1];
  const maxGen = Math.max(...generations.map(g => g.gen));
  const minGen = Math.min(...generations.map(g => g.gen));

  const fitnessPoints = generations.map((g) => {
    const x = ((g.gen - minGen) / Math.max(1, maxGen - minGen)) * 280 + 10;
    const y = (1 - g.fitness) * 70 + 10;
    return `${x},${y}`;
  }).join(' ');

  const diversityPoints = generations.map((g) => {
    const x = ((g.gen - minGen) / Math.max(1, maxGen - minGen)) * 280 + 10;
    const y = (1 - g.diversity) * 70 + 10;
    return `${x},${y}`;
  }).join(' ');

  return (
    <div className="glass-panel rounded-lg p-3 h-full flex flex-col">
      <div className="flex items-center justify-between mb-2">
        <div className="flex items-center gap-2">
          <div className="w-1.5 h-1.5 rounded-full bg-forge-purple animate-pulse-glow" />
          <span className="text-[9px] font-bold text-forge-text tracking-wider">AI OPTIMIZATION EVOLUTION</span>
        </div>
        <div className="flex items-center gap-3">
          <div className="flex items-center gap-1">
            <div className="w-3 h-0.5 bg-forge-green rounded" />
            <span className="text-[7px] text-forge-text-muted">Fitness</span>
          </div>
          <div className="flex items-center gap-1">
            <div className="w-3 h-0.5 bg-forge-purple rounded" />
            <span className="text-[7px] text-forge-text-muted">Diversity</span>
          </div>
        </div>
      </div>

      <div className="flex-1 relative">
        <svg viewBox="0 0 300 90" className="w-full h-full" preserveAspectRatio="none">
          <defs>
            <linearGradient id="fitnessGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="0%" stopColor="#00ff88" stopOpacity="0.3" />
              <stop offset="100%" stopColor="#00ff88" stopOpacity="0" />
            </linearGradient>
            <linearGradient id="diversityGrad" x1="0" y1="0" x2="0" y2="1">
              <stop offset="0%" stopColor="#8855ff" stopOpacity="0.2" />
              <stop offset="100%" stopColor="#8855ff" stopOpacity="0" />
            </linearGradient>
          </defs>

          {/* Grid lines */}
          {[0.25, 0.5, 0.75, 1.0].map(v => (
            <g key={v}>
              <line x1="10" y1={(1 - v) * 70 + 10} x2="290" y2={(1 - v) * 70 + 10}
                stroke="rgba(30,34,48,0.4)" strokeWidth="0.5" strokeDasharray="2,4" />
              <text x="5" y={(1 - v) * 70 + 13} fill="rgba(122,130,153,0.5)" fontSize="4" textAnchor="end">
                {(v * 100).toFixed(0)}%
              </text>
            </g>
          ))}

          {/* Fitness area */}
          <polygon
            points={`10,80 ${fitnessPoints} 290,80`}
            fill="url(#fitnessGrad)"
          />
          <polyline points={fitnessPoints} fill="none" stroke="#00ff88" strokeWidth="1.5" />

          {/* Diversity line */}
          <polyline points={diversityPoints} fill="none" stroke="#8855ff" strokeWidth="1" strokeDasharray="3,2" />
        </svg>
      </div>

      <div className="flex items-center justify-between mt-2 pt-2 border-t border-forge-border">
        <div className="text-center">
          <div className="text-[7px] text-forge-text-muted tracking-wider">GENERATION</div>
          <div className="text-sm font-bold text-forge-text font-mono">{latest.gen}</div>
        </div>
        <div className="text-center">
          <div className="text-[7px] text-forge-text-muted tracking-wider">FITNESS</div>
          <div className="text-sm font-bold text-forge-green font-mono">{(latest.fitness * 100).toFixed(1)}%</div>
        </div>
        <div className="text-center">
          <div className="text-[7px] text-forge-text-muted tracking-wider">DIVERSITY</div>
          <div className="text-sm font-bold text-forge-purple font-mono">{(latest.diversity * 100).toFixed(1)}%</div>
        </div>
        <div className="text-center">
          <div className="text-[7px] text-forge-text-muted tracking-wider">CONVERGENCE</div>
          <div className="text-sm font-bold text-forge-accent font-mono">
            {latest.fitness > 0.95 ? 'NEAR' : latest.fitness > 0.85 ? 'PROGRESSING' : 'EVOLVING'}
          </div>
        </div>
      </div>
    </div>
  );
}
