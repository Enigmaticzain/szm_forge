import React, { useState } from 'react';
import { parseUrdf, runPybulletSimulation } from '../services/forgeApi';
import { Bot, Play, Code, AlertTriangle, CheckCircle, Save } from 'lucide-react';
import { useForgeStore } from '../store/ForgeStoreContext';

export const RoboticsWorkspace: React.FC = () => {
  const store = useForgeStore();
  const [urdfContent, setUrdfContent] = useState('<!-- Enter URDF XML here -->\n<robot name="test_robot">\n  <link name="base_link" />\n</robot>');
  const [isParsing, setIsParsing] = useState(false);
  const [isSimulating, setIsSimulating] = useState(false);
  const [parseResult, setParseResult] = useState<any>(null);

  const handleParse = async () => {
    setIsParsing(true);
    setParseResult(null);
    try {
      const res = await parseUrdf(urdfContent);
      setParseResult(res);
      store.showToast('URDF Parsed Successfully');
    } catch (e: any) {
      store.showToast(`Parse failed: ${e.message}`, 'error');
    } finally {
      setIsParsing(false);
    }
  };

  const handleSimulate = async () => {
    setIsSimulating(true);
    try {
      const res = await runPybulletSimulation(240);
      store.showToast(`Simulation complete: ${res.steps} steps executed.`);
    } catch (e: any) {
      store.showToast(`Simulation failed: ${e.message}`, 'error');
    } finally {
      setIsSimulating(false);
    }
  };

  return (
    <div className="h-full flex flex-col bg-forge-black text-forge-text overflow-hidden">
      <div className="h-[48px] border-b border-forge-border bg-forge-dark px-4 flex items-center gap-4 flex-shrink-0">
        <Bot size={16} className="text-forge-accent" />
        <span className="text-xs font-bold tracking-widest text-forge-accent">ROBOTICS & KINEMATICS</span>
      </div>

      <div className="flex-1 flex overflow-hidden">
        {/* Editor Area */}
        <div className="flex-1 flex flex-col p-4">
          <div className="flex items-center justify-between mb-2">
            <h3 className="text-xs font-mono tracking-wider flex items-center gap-2">
              <Code size={14} className="text-forge-text-muted" /> URDF EDITOR
            </h3>
            <button className="flex items-center gap-1 text-[10px] text-forge-text-muted hover:text-forge-text transition-colors">
              <Save size={12} /> Save to Project
            </button>
          </div>
          <textarea
            value={urdfContent}
            onChange={(e) => setUrdfContent(e.target.value)}
            className="flex-1 bg-forge-dark border border-forge-border rounded-lg p-4 font-mono text-xs text-forge-text outline-none focus:border-forge-accent resize-none shadow-inner"
            spellCheck={false}
          />
        </div>

        {/* Sidebar */}
        <div className="w-[320px] border-l border-forge-border bg-forge-panel flex flex-col">
          <div className="p-4 border-b border-forge-border">
            <button
              onClick={handleParse}
              disabled={isParsing}
              className="w-full py-2 bg-forge-accent/20 text-forge-accent border border-forge-accent/50 rounded flex items-center justify-center gap-2 text-xs font-mono font-bold hover:bg-forge-accent/30 disabled:opacity-50 transition-colors mb-4"
            >
              {isParsing ? <div className="w-4 h-4 border-2 border-forge-accent border-t-transparent rounded-full animate-spin" /> : <Code size={14} />}
              {isParsing ? 'PARSING...' : 'PARSE & VALIDATE URDF'}
            </button>
            
            <button
              onClick={handleSimulate}
              disabled={isSimulating}
              className="w-full py-2 bg-forge-green/20 text-forge-green border border-forge-green/50 rounded flex items-center justify-center gap-2 text-xs font-mono font-bold hover:bg-forge-green/30 disabled:opacity-50 transition-colors"
            >
              {isSimulating ? <div className="w-4 h-4 border-2 border-forge-green border-t-transparent rounded-full animate-spin" /> : <Play size={14} />}
              {isSimulating ? 'SIMULATING...' : 'LAUNCH PYBULLET (HEADLESS)'}
            </button>
          </div>

          <div className="flex-1 p-4 overflow-y-auto">
            <h3 className="text-[10px] font-semibold tracking-wider text-forge-text-muted mb-3">KINEMATIC ANALYSIS</h3>
            {parseResult ? (
              <div className="space-y-3">
                <div className="flex items-center gap-2 p-2 rounded bg-forge-green/5 border border-forge-green/20 text-forge-green text-xs font-mono">
                  <CheckCircle size={14} /> URDF Validated
                </div>
                
                <div className="bg-forge-dark p-3 rounded border border-forge-border/50">
                  <div className="text-[10px] text-forge-text-muted mb-1">ROBOT NAME</div>
                  <div className="text-sm font-mono text-forge-text">{parseResult.name || 'Unknown'}</div>
                </div>
                
                <div className="grid grid-cols-2 gap-2">
                  <div className="bg-forge-dark p-3 rounded border border-forge-border/50">
                    <div className="text-[10px] text-forge-text-muted mb-1">LINKS</div>
                    <div className="text-lg font-mono text-forge-accent">{parseResult.links_count || 0}</div>
                  </div>
                  <div className="bg-forge-dark p-3 rounded border border-forge-border/50">
                    <div className="text-[10px] text-forge-text-muted mb-1">JOINTS</div>
                    <div className="text-lg font-mono text-forge-accent">{parseResult.joints_count || 0}</div>
                  </div>
                </div>
              </div>
            ) : (
              <div className="flex flex-col items-center justify-center h-40 text-forge-text-dim opacity-50">
                <AlertTriangle size={32} className="mb-2" />
                <span className="text-[10px] font-mono">No analysis available.</span>
                <span className="text-[10px] font-mono">Parse a valid URDF.</span>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
};
