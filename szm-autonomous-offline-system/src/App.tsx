import { useState, useEffect, useCallback } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import {
  Brain,
  Cpu,
  Database,
  Layers,
  GitBranch,
  Zap,
  Shield,
  Activity,
  Box,
  Cog,
  Search,
  RefreshCw,
  CheckCircle,
  AlertCircle,
  TrendingUp,
  Network,
  Microscope,
  Factory,
  Settings,
  Menu,
  X,
  ChevronRight,
  Lightbulb,
  Target,
  BarChart3,
  WifiOff,
  Download,
  Upload,
  Lock,
  Plus,
  ArrowRight,
  ArrowDown
} from 'lucide-react';
import {
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  AreaChart,
  Area,
  RadarChart,
  Radar,
  PolarGrid,
  PolarAngleAxis,
  PolarRadiusAxis,
  Legend
} from 'recharts';

// Types
interface Agent {
  id: string;
  name: string;
  icon: React.ElementType;
  status: 'active' | 'idle' | 'processing' | 'error';
  progress: number;
  tasksCompleted: number;
  description: string;
  color: string;
}

interface SystemLayer {
  id: string;
  name: string;
  description: string;
  status: 'online' | 'offline' | 'initializing';
  health: number;
  components: string[];
}

interface MemoryEntry {
  id: string;
  type: 'session' | 'project' | 'technical' | 'evolution' | 'decision';
  content: string;
  timestamp: Date;
  priority: 'high' | 'medium' | 'low';
}

interface SimulationTask {
  id: string;
  name: string;
  type: 'mechanical' | 'thermal' | 'electrical' | 'fluid' | 'structural';
  status: 'pending' | 'running' | 'complete' | 'failed';
  progress: number;
  result?: string;
}

// Mock Data
const initialAgents: Agent[] = [
  {
    id: 'research',
    name: 'Research Agent',
    icon: Microscope,
    status: 'active',
    progress: 75,
    tasksCompleted: 1247,
    description: 'Reads documentation, learns architectures, extracts engineering knowledge',
    color: '#8b5cf6'
  },
  {
    id: 'architect',
    name: 'Code Architect',
    icon: Layers,
    status: 'processing',
    progress: 45,
    tasksCompleted: 892,
    description: 'Designs scalable architecture, refactors systems, maintains modularity',
    color: '#3b82f6'
  },
  {
    id: 'simulation',
    name: 'Simulation Agent',
    icon: Box,
    status: 'active',
    progress: 60,
    tasksCompleted: 2341,
    description: 'Handles physics reasoning, mechanical/thermal/electrical systems',
    color: '#10b981'
  },
  {
    id: 'systems',
    name: 'Systems Engineering',
    icon: Factory,
    status: 'idle',
    progress: 0,
    tasksCompleted: 567,
    description: 'Industrial machines, CAD workflows, robotics, control systems',
    color: '#f59e0b'
  },
  {
    id: 'memory',
    name: 'Memory Agent',
    icon: Database,
    status: 'active',
    progress: 90,
    tasksCompleted: 15678,
    description: 'Stores long-term learning, compresses insights, maintains indexes',
    color: '#ec4899'
  },
  {
    id: 'testing',
    name: 'Testing Agent',
    icon: CheckCircle,
    status: 'processing',
    progress: 35,
    tasksCompleted: 4521,
    description: 'Runs automated tests, detects instability, finds regressions',
    color: '#ef4444'
  },
  {
    id: 'optimization',
    name: 'Optimization Agent',
    icon: TrendingUp,
    status: 'active',
    progress: 55,
    tasksCompleted: 789,
    description: 'Improves speed, reduces RAM usage, optimizes GPU/VRAM',
    color: '#06b6d4'
  }
];

const systemLayers: SystemLayer[] = [
  {
    id: 'layer1',
    name: 'Foundation LLM',
    description: 'Base model providing general language understanding and knowledge',
    status: 'online',
    health: 98,
    components: ['Llama-4', 'Qwen 3.5', 'Mistral Large 2', 'Nemotron']
  },
  {
    id: 'layer2',
    name: 'Agentic Reasoning System',
    description: 'Orchestrates reasoning steps, planning, tool selection, and data access',
    status: 'online',
    health: 95,
    components: ['Planner', 'Memory Modules', 'Tool APIs', 'RAG System']
  },
  {
    id: 'layer3',
    name: 'Simulation & Physics Integration',
    description: 'CFD, FEA, kinematics, thermal, electrical simulation engines',
    status: 'online',
    health: 92,
    components: ['OpenFOAM', 'Code_Aster', 'FreeCAD', 'SU2', 'Elmer']
  },
  {
    id: 'layer4',
    name: 'World Model & Knowledge Graph',
    description: 'Ontology-driven knowledge graph encoding components, materials, constraints',
    status: 'online',
    health: 97,
    components: ['Neo4j', 'Ontology Engine', 'Causal Engine', 'Traceability']
  },
  {
    id: 'layer5',
    name: 'Autonomous Self-Improvement',
    description: 'Iterative training loops, meta-learning, reinforcement from simulation',
    status: 'initializing',
    health: 85,
    components: ['RL Loop', 'Meta-Learning', 'Benchmark System', 'Rollback']
  }
];

const performanceData = [
  { time: '00:00', accuracy: 85, speed: 72, stability: 90 },
  { time: '04:00', accuracy: 87, speed: 75, stability: 91 },
  { time: '08:00', accuracy: 89, speed: 78, stability: 92 },
  { time: '12:00', accuracy: 91, speed: 82, stability: 93 },
  { time: '16:00', accuracy: 93, speed: 85, stability: 94 },
  { time: '20:00', accuracy: 94, speed: 87, stability: 95 },
  { time: '24:00', accuracy: 95, speed: 89, stability: 96 }
];

const resourceData = [
  { name: 'RAM Usage', value: 45, total: 64, unit: 'GB' },
  { name: 'VRAM Usage', value: 18, total: 24, unit: 'GB' },
  { name: 'CPU Load', value: 62, total: 100, unit: '%' },
  { name: 'GPU Load', value: 78, total: 100, unit: '%' },
  { name: 'Storage', value: 512, total: 2048, unit: 'GB' }
];

const capabilityRadar = [
  { subject: 'Mechanical', A: 92, fullMark: 100 },
  { subject: 'Electrical', A: 88, fullMark: 100 },
  { subject: 'Thermal', A: 85, fullMark: 100 },
  { subject: 'Fluid Dynamics', A: 82, fullMark: 100 },
  { subject: 'Control Systems', A: 90, fullMark: 100 },
  { subject: 'Material Science', A: 87, fullMark: 100 }
];

// Components
const StatusIndicator: React.FC<{ status: string; size?: 'sm' | 'md' | 'lg' }> = ({ status, size = 'md' }) => {
  const sizeClasses = {
    sm: 'w-2 h-2',
    md: 'w-3 h-3',
    lg: 'w-4 h-4'
  };

  const statusColors: Record<string, string> = {
    active: 'bg-emerald-500',
    online: 'bg-emerald-500',
    processing: 'bg-amber-500 animate-pulse',
    initializing: 'bg-blue-500 animate-pulse',
    idle: 'bg-gray-400',
    offline: 'bg-red-500',
    error: 'bg-red-500',
    pending: 'bg-gray-300',
    running: 'bg-blue-500 animate-pulse',
    complete: 'bg-emerald-500',
    failed: 'bg-red-500'
  };

  return (
    <span className={`${sizeClasses[size]} ${statusColors[status] || 'bg-gray-400'} rounded-full inline-block`} />
  );
};

const Card: React.FC<{
  children: React.ReactNode;
  className?: string;
  title?: string;
  icon?: React.ElementType;
  action?: React.ReactNode;
}> = ({ children, className = '', title, icon: Icon, action }) => (
  <motion.div
    initial={{ opacity: 0, y: 20 }}
    animate={{ opacity: 1, y: 0 }}
    className={`bg-white rounded-xl border border-slate-200 shadow-sm overflow-hidden ${className}`}
  >
    {(title || Icon) && (
      <div className="px-6 py-4 border-b border-slate-100 flex items-center justify-between">
        <div className="flex items-center gap-3">
          {Icon && <Icon className="w-5 h-5 text-slate-500" />}
          {title && <h3 className="font-semibold text-slate-800">{title}</h3>}
        </div>
        {action}
      </div>
    )}
    <div className="p-6">{children}</div>
  </motion.div>
);

const AgentCard: React.FC<{ agent: Agent }> = ({ agent }) => {
  const Icon = agent.icon;
  
  return (
    <motion.div
      whileHover={{ scale: 1.02 }}
      className="bg-gradient-to-br from-white to-slate-50 rounded-xl border border-slate-200 p-5 shadow-sm hover:shadow-md transition-shadow"
    >
      <div className="flex items-start justify-between mb-4">
        <div className="flex items-center gap-3">
          <div
            className="w-12 h-12 rounded-xl flex items-center justify-center"
            style={{ backgroundColor: `${agent.color}15` }}
          >
            <Icon className="w-6 h-6" style={{ color: agent.color }} />
          </div>
          <div>
            <h4 className="font-semibold text-slate-800">{agent.name}</h4>
            <div className="flex items-center gap-2">
              <StatusIndicator status={agent.status} size="sm" />
              <span className="text-xs text-slate-500 capitalize">{agent.status}</span>
            </div>
          </div>
        </div>
        <div className="text-right">
          <div className="text-2xl font-bold text-slate-800">{agent.tasksCompleted.toLocaleString()}</div>
          <div className="text-xs text-slate-500">tasks completed</div>
        </div>
      </div>
      
      <p className="text-sm text-slate-600 mb-4">{agent.description}</p>
      
      <div className="space-y-2">
        <div className="flex justify-between text-xs">
          <span className="text-slate-500">Current Progress</span>
          <span className="font-medium text-slate-700">{agent.progress}%</span>
        </div>
        <div className="h-2 bg-slate-200 rounded-full overflow-hidden">
          <motion.div
            initial={{ width: 0 }}
            animate={{ width: `${agent.progress}%` }}
            transition={{ duration: 1, ease: 'easeOut' }}
            className="h-full rounded-full"
            style={{ backgroundColor: agent.color }}
          />
        </div>
      </div>
    </motion.div>
  );
};

const LayerCard: React.FC<{ layer: SystemLayer; index: number }> = ({ layer, index }) => (
  <motion.div
    initial={{ opacity: 0, x: -20 }}
    animate={{ opacity: 1, x: 0 }}
    transition={{ delay: index * 0.1 }}
    className="relative"
  >
    {index < systemLayers.length - 1 && (
      <div className="absolute left-6 top-full w-0.5 h-8 bg-gradient-to-b from-slate-300 to-transparent" />
    )}
    <div className="bg-white rounded-xl border border-slate-200 p-5 shadow-sm ml-4">
      <div className="flex items-center gap-4 mb-3">
        <div className="w-12 h-12 rounded-xl bg-gradient-to-br from-violet-500 to-indigo-600 flex items-center justify-center text-white font-bold">
          {layer.id.replace('layer', 'L')}
        </div>
        <div className="flex-1">
          <div className="flex items-center gap-2">
            <h4 className="font-semibold text-slate-800">{layer.name}</h4>
            <StatusIndicator status={layer.status} size="sm" />
          </div>
          <p className="text-sm text-slate-600">{layer.description}</p>
        </div>
        <div className="text-right">
          <div className="text-lg font-bold" style={{ color: layer.health >= 90 ? '#10b981' : layer.health >= 70 ? '#f59e0b' : '#ef4444' }}>
            {layer.health}%
          </div>
          <div className="text-xs text-slate-500">health</div>
        </div>
      </div>
      <div className="flex flex-wrap gap-2">
        {layer.components.map((comp, i) => (
          <span
            key={i}
            className="px-2 py-1 bg-slate-100 rounded-md text-xs font-medium text-slate-600"
          >
            {comp}
          </span>
        ))}
      </div>
    </div>
  </motion.div>
);

const MemoryPanel: React.FC = () => {
  const [memories, _setMemories] = useState<MemoryEntry[]>([
    { id: '1', type: 'session', content: 'Active project: SZM Forge Architecture v2.1', timestamp: new Date(), priority: 'high' },
    { id: '2', type: 'project', content: 'Repository analysis complete: 847 files, 23 modules', timestamp: new Date(), priority: 'medium' },
    { id: '3', type: 'technical', content: 'Learned: ASML lithography alignment tolerances ±2nm', timestamp: new Date(), priority: 'high' },
    { id: '4', type: 'evolution', content: 'Self-improvement cycle #47: +12% inference speed', timestamp: new Date(), priority: 'medium' },
    { id: '5', type: 'decision', content: 'Selected OpenFOAM over SU2 for CFD simulations', timestamp: new Date(), priority: 'low' }
  ]);

  const typeColors: Record<string, string> = {
    session: 'bg-blue-100 text-blue-700',
    project: 'bg-purple-100 text-purple-700',
    technical: 'bg-emerald-100 text-emerald-700',
    evolution: 'bg-amber-100 text-amber-700',
    decision: 'bg-rose-100 text-rose-700'
  };

  return (
    <Card title="Memory System" icon={Database} className="h-full">
      <div className="space-y-3 max-h-96 overflow-y-auto">
        {memories.map((memory) => (
          <motion.div
            key={memory.id}
            initial={{ opacity: 0, x: -10 }}
            animate={{ opacity: 1, x: 0 }}
            className="flex items-start gap-3 p-3 rounded-lg bg-slate-50 hover:bg-slate-100 transition-colors"
          >
            <span className={`px-2 py-0.5 rounded text-xs font-medium ${typeColors[memory.type]}`}>
              {memory.type}
            </span>
            <div className="flex-1">
              <p className="text-sm text-slate-700">{memory.content}</p>
              <p className="text-xs text-slate-400 mt-1">
                {memory.timestamp.toLocaleTimeString()}
              </p>
            </div>
            <StatusIndicator status={memory.priority === 'high' ? 'active' : 'idle'} size="sm" />
          </motion.div>
        ))}
      </div>
      <div className="mt-4 pt-4 border-t border-slate-100 flex justify-between items-center">
        <div className="text-sm text-slate-500">
          <span className="font-semibold text-slate-700">15,678</span> entries indexed
        </div>
        <button className="text-sm text-violet-600 hover:text-violet-700 font-medium flex items-center gap-1">
          <Search className="w-4 h-4" />
          Search Memory
        </button>
      </div>
    </Card>
  );
};

const SimulationPanel: React.FC = () => {
  const [simulations, _setSimulations] = useState<SimulationTask[]>([
    { id: '1', name: 'Thermal Analysis - Engine Block', type: 'thermal', status: 'running', progress: 67 },
    { id: '2', name: 'Stress Test - Landing Gear', type: 'structural', status: 'complete', progress: 100, result: 'Pass: Safety factor 2.4' },
    { id: '3', name: 'CFD - Aerodynamic Profile', type: 'fluid', status: 'pending', progress: 0 },
    { id: '4', name: 'Vibration Analysis - Rotor', type: 'mechanical', status: 'running', progress: 34 },
    { id: '5', name: 'Circuit Simulation - Power Module', type: 'electrical', status: 'complete', progress: 100, result: 'Pass: All tolerances met' }
  ]);

  const typeIcons: Record<string, React.ElementType> = {
    thermal: Zap,
    structural: Box,
    fluid: Activity,
    mechanical: Cog,
    electrical: Cpu
  };

  const typeColors: Record<string, string> = {
    thermal: 'text-orange-500',
    structural: 'text-blue-500',
    fluid: 'text-cyan-500',
    mechanical: 'text-slate-500',
    electrical: 'text-amber-500'
  };

  return (
    <Card title="Simulation Queue" icon={Microscope} className="h-full">
      <div className="space-y-4">
        {simulations.map((sim) => {
          const Icon = typeIcons[sim.type];
          return (
            <div key={sim.id} className="p-3 rounded-lg border border-slate-200 hover:border-slate-300 transition-colors">
              <div className="flex items-center justify-between mb-2">
                <div className="flex items-center gap-2">
                  <Icon className={`w-4 h-4 ${typeColors[sim.type]}`} />
                  <span className="font-medium text-slate-700 text-sm">{sim.name}</span>
                </div>
                <StatusIndicator status={sim.status} size="sm" />
              </div>
              <div className="flex items-center gap-2">
                <div className="flex-1 h-1.5 bg-slate-200 rounded-full overflow-hidden">
                  <motion.div
                    initial={{ width: 0 }}
                    animate={{ width: `${sim.progress}%` }}
                    className={`h-full rounded-full ${
                      sim.status === 'complete' ? 'bg-emerald-500' :
                      sim.status === 'failed' ? 'bg-red-500' :
                      'bg-blue-500'
                    }`}
                  />
                </div>
                <span className="text-xs text-slate-500 w-10">{sim.progress}%</span>
              </div>
              {sim.result && (
                <p className="text-xs text-emerald-600 mt-1 flex items-center gap-1">
                  <CheckCircle className="w-3 h-3" />
                  {sim.result}
                </p>
              )}
            </div>
          );
        })}
      </div>
      <button className="mt-4 w-full py-2 bg-gradient-to-r from-violet-500 to-indigo-600 text-white rounded-lg font-medium hover:opacity-90 transition-opacity flex items-center justify-center gap-2">
        <Plus className="w-4 h-4" />
        Add Simulation Task
      </button>
    </Card>
  );
};

const ResourceMonitor: React.FC = () => (
  <Card title="Resource Monitor" icon={Activity}>
    <div className="grid grid-cols-2 gap-4">
      {resourceData.map((resource, index) => (
        <motion.div
          key={resource.name}
          initial={{ opacity: 0, scale: 0.9 }}
          animate={{ opacity: 1, scale: 1 }}
          transition={{ delay: index * 0.1 }}
          className="p-4 rounded-lg bg-slate-50"
        >
          <div className="flex justify-between items-center mb-2">
            <span className="text-sm font-medium text-slate-600">{resource.name}</span>
            <span className="text-lg font-bold text-slate-800">
              {resource.value}/{resource.total}
            </span>
          </div>
          <div className="h-2 bg-slate-200 rounded-full overflow-hidden">
            <motion.div
              initial={{ width: 0 }}
              animate={{ width: `${(resource.value / resource.total) * 100}%` }}
              transition={{ duration: 1, delay: index * 0.1 }}
              className={`h-full rounded-full ${
                (resource.value / resource.total) > 0.8 ? 'bg-red-500' :
                (resource.value / resource.total) > 0.6 ? 'bg-amber-500' :
                'bg-emerald-500'
              }`}
            />
          </div>
        </motion.div>
      ))}
    </div>
    <div className="mt-4 pt-4 border-t border-slate-100">
      <div className="flex items-center justify-between">
        <div className="flex items-center gap-2">
          <WifiOff className="w-4 h-4 text-emerald-500" />
          <span className="text-sm text-slate-600">Offline Mode</span>
        </div>
        <div className="flex items-center gap-2">
          <Lock className="w-4 h-4 text-emerald-500" />
          <span className="text-sm text-slate-600">Security Active</span>
        </div>
      </div>
    </div>
  </Card>
);

const PerformanceChart: React.FC = () => (
  <Card title="Performance Metrics" icon={BarChart3}>
    <div className="h-64">
      <ResponsiveContainer width="100%" height="100%">
        <AreaChart data={performanceData}>
          <defs>
            <linearGradient id="colorAccuracy" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#8b5cf6" stopOpacity={0.3} />
              <stop offset="95%" stopColor="#8b5cf6" stopOpacity={0} />
            </linearGradient>
            <linearGradient id="colorSpeed" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.3} />
              <stop offset="95%" stopColor="#3b82f6" stopOpacity={0} />
            </linearGradient>
            <linearGradient id="colorStability" x1="0" y1="0" x2="0" y2="1">
              <stop offset="5%" stopColor="#10b981" stopOpacity={0.3} />
              <stop offset="95%" stopColor="#10b981" stopOpacity={0} />
            </linearGradient>
          </defs>
          <CartesianGrid strokeDasharray="3 3" stroke="#e2e8f0" />
          <XAxis dataKey="time" stroke="#64748b" fontSize={12} />
          <YAxis stroke="#64748b" fontSize={12} domain={[0, 100]} />
          <Tooltip
            contentStyle={{
              backgroundColor: '#fff',
              border: '1px solid #e2e8f0',
              borderRadius: '8px',
              boxShadow: '0 4px 6px -1px rgb(0 0 0 / 0.1)'
            }}
          />
          <Legend />
          <Area
            type="monotone"
            dataKey="accuracy"
            stroke="#8b5cf6"
            fillOpacity={1}
            fill="url(#colorAccuracy)"
            name="Accuracy %"
          />
          <Area
            type="monotone"
            dataKey="speed"
            stroke="#3b82f6"
            fillOpacity={1}
            fill="url(#colorSpeed)"
            name="Speed %"
          />
          <Area
            type="monotone"
            dataKey="stability"
            stroke="#10b981"
            fillOpacity={1}
            fill="url(#colorStability)"
            name="Stability %"
          />
        </AreaChart>
      </ResponsiveContainer>
    </div>
  </Card>
);

const CapabilityRadar: React.FC = () => (
  <Card title="Engineering Capabilities" icon={Target}>
    <div className="h-64">
      <ResponsiveContainer width="100%" height="100%">
        <RadarChart cx="50%" cy="50%" outerRadius="80%" data={capabilityRadar}>
          <PolarGrid stroke="#e2e8f0" />
          <PolarAngleAxis dataKey="subject" tick={{ fill: '#64748b', fontSize: 12 }} />
          <PolarRadiusAxis angle={90} domain={[0, 100]} tick={{ fill: '#64748b', fontSize: 10 }} />
          <Radar
            name="Capability Score"
            dataKey="A"
            stroke="#8b5cf6"
            fill="#8b5cf6"
            fillOpacity={0.3}
          />
        </RadarChart>
      </ResponsiveContainer>
    </div>
  </Card>
);

const SelfImprovementPanel: React.FC = () => {
  const [isEvolving, setIsEvolving] = useState(false);
  const [evolutionProgress, setEvolutionProgress] = useState(0);

  const startEvolution = useCallback(() => {
    setIsEvolving(true);
    setEvolutionProgress(0);
    
    const interval = setInterval(() => {
      setEvolutionProgress(prev => {
        if (prev >= 100) {
          clearInterval(interval);
          setIsEvolving(false);
          return 100;
        }
        return prev + 2;
      });
    }, 100);
  }, []);

  const improvements = [
    { metric: 'Inference Speed', before: 85, after: 97, unit: 'tokens/s' },
    { metric: 'Memory Efficiency', before: 72, after: 89, unit: '%' },
    { metric: 'Accuracy', before: 91, after: 95, unit: '%' },
    { metric: 'Tool Selection', before: 78, after: 92, unit: '%' }
  ];

  return (
    <Card
      title="Self-Improvement System"
      icon={RefreshCw}
      action={
        <button
          onClick={startEvolution}
          disabled={isEvolving}
          className={`px-4 py-2 rounded-lg font-medium text-sm flex items-center gap-2 transition-all ${
            isEvolving
              ? 'bg-slate-100 text-slate-400 cursor-not-allowed'
              : 'bg-gradient-to-r from-violet-500 to-indigo-600 text-white hover:opacity-90'
          }`}
        >
          {isEvolving ? (
            <>
              <RefreshCw className="w-4 h-4 animate-spin" />
              Evolving...
            </>
          ) : (
            <>
              <Lightbulb className="w-4 h-4" />
              Start Evolution
            </>
          )}
        </button>
      }
    >
      {isEvolving && (
        <motion.div
          initial={{ opacity: 0, height: 0 }}
          animate={{ opacity: 1, height: 'auto' }}
          className="mb-4 p-4 bg-violet-50 rounded-lg border border-violet-200"
        >
          <div className="flex items-center justify-between mb-2">
            <span className="text-sm font-medium text-violet-700">Evolution in Progress</span>
            <span className="text-sm font-bold text-violet-700">{evolutionProgress}%</span>
          </div>
          <div className="h-2 bg-violet-200 rounded-full overflow-hidden">
            <motion.div
              animate={{ width: `${evolutionProgress}%` }}
              className="h-full bg-gradient-to-r from-violet-500 to-indigo-600 rounded-full"
            />
          </div>
          <div className="mt-2 text-xs text-violet-600">
            Running benchmarks • Comparing metrics • Validating stability
          </div>
        </motion.div>
      )}

      <div className="space-y-3">
        {improvements.map((imp, index) => (
          <motion.div
            key={imp.metric}
            initial={{ opacity: 0, x: -10 }}
            animate={{ opacity: 1, x: 0 }}
            transition={{ delay: index * 0.1 }}
            className="flex items-center justify-between p-3 rounded-lg bg-slate-50"
          >
            <span className="text-sm font-medium text-slate-700">{imp.metric}</span>
            <div className="flex items-center gap-3">
              <span className="text-sm text-slate-500 line-through">{imp.before} {imp.unit}</span>
              <ArrowRight className="w-4 h-4 text-slate-400" />
              <span className="text-sm font-bold text-emerald-600">{imp.after} {imp.unit}</span>
              <span className="text-xs px-2 py-0.5 bg-emerald-100 text-emerald-700 rounded-full font-medium">
                +{imp.after - imp.before}
              </span>
            </div>
          </motion.div>
        ))}
      </div>

      <div className="mt-4 pt-4 border-t border-slate-100">
        <div className="flex items-center justify-between text-sm">
          <span className="text-slate-500">Last Evolution</span>
          <span className="font-medium text-slate-700">2 hours ago</span>
        </div>
        <div className="flex items-center justify-between text-sm mt-2">
          <span className="text-slate-500">Total Cycles</span>
          <span className="font-medium text-slate-700">47</span>
        </div>
      </div>
    </Card>
  );
};

const ProjectAnalyzer: React.FC = () => {
  const [isAnalyzing, setIsAnalyzing] = useState(false);
  const [analysisComplete, setAnalysisComplete] = useState(false);

  const startAnalysis = useCallback(() => {
    setIsAnalyzing(true);
    setAnalysisComplete(false);
    
    setTimeout(() => {
      setIsAnalyzing(false);
      setAnalysisComplete(true);
    }, 3000);
  }, []);

  return (
    <Card title="Project Intelligence" icon={Search}>
      {!analysisComplete ? (
        <div className="text-center py-8">
          {isAnalyzing ? (
            <motion.div
              initial={{ opacity: 0 }}
              animate={{ opacity: 1 }}
              className="space-y-4"
            >
              <RefreshCw className="w-12 h-12 text-violet-500 mx-auto animate-spin" />
              <div>
                <p className="font-medium text-slate-700">Analyzing Project Structure</p>
                <p className="text-sm text-slate-500">Building architecture maps and dependency graphs</p>
              </div>
            </motion.div>
          ) : (
            <div className="space-y-4">
              <div className="w-16 h-16 rounded-2xl bg-gradient-to-br from-violet-500 to-indigo-600 flex items-center justify-center mx-auto">
                <Search className="w-8 h-8 text-white" />
              </div>
              <div>
                <p className="font-medium text-slate-700">Ready to Analyze</p>
                <p className="text-sm text-slate-500">Scan project directories for architecture understanding</p>
              </div>
              <button
                onClick={startAnalysis}
                className="px-6 py-2 bg-gradient-to-r from-violet-500 to-indigo-600 text-white rounded-lg font-medium hover:opacity-90 transition-opacity"
              >
                Start Analysis
              </button>
            </div>
          )}
        </div>
      ) : (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          className="space-y-4"
        >
          <div className="grid grid-cols-3 gap-4">
            <div className="p-3 rounded-lg bg-slate-50 text-center">
              <div className="text-2xl font-bold text-slate-800">847</div>
              <div className="text-xs text-slate-500">Files</div>
            </div>
            <div className="p-3 rounded-lg bg-slate-50 text-center">
              <div className="text-2xl font-bold text-slate-800">23</div>
              <div className="text-xs text-slate-500">Modules</div>
            </div>
            <div className="p-3 rounded-lg bg-slate-50 text-center">
              <div className="text-2xl font-bold text-slate-800">156</div>
              <div className="text-xs text-slate-500">Dependencies</div>
            </div>
          </div>

          <div>
            <h4 className="font-medium text-slate-700 mb-2 flex items-center gap-2">
              <AlertCircle className="w-4 h-4 text-amber-500" />
              Detected Weaknesses
            </h4>
            <ul className="space-y-1">
              {['Circular dependency in utils module', 'Missing error handling in API layer', 'No unit tests for core services'].map((weakness, i) => (
                <li key={i} className="text-sm text-slate-600 flex items-center gap-2">
                  <span className="w-1.5 h-1.5 rounded-full bg-amber-500" />
                  {weakness}
                </li>
              ))}
            </ul>
          </div>

          <div>
            <h4 className="font-medium text-slate-700 mb-2 flex items-center gap-2">
              <Lightbulb className="w-4 h-4 text-violet-500" />
              Suggestions
            </h4>
            <ul className="space-y-1">
              {['Refactor utils into separate packages', 'Add try-catch blocks to API handlers', 'Implement Jest test suite'].map((suggestion, i) => (
                <li key={i} className="text-sm text-slate-600 flex items-center gap-2">
                  <span className="w-1.5 h-1.5 rounded-full bg-violet-500" />
                  {suggestion}
                </li>
              ))}
            </ul>
          </div>

          <button
            onClick={() => setAnalysisComplete(false)}
            className="w-full py-2 border border-slate-200 rounded-lg font-medium text-slate-600 hover:bg-slate-50 transition-colors"
          >
            Analyze Another Project
          </button>
        </motion.div>
      )}
    </Card>
  );
};

const ArchitectureDiagram: React.FC = () => (
  <Card title="System Architecture" icon={Layers}>
    <div className="relative py-4">
      {systemLayers.map((layer, index) => (
        <LayerCard key={layer.id} layer={layer} index={index} />
      ))}
    </div>
  </Card>
);

// Main App Component
export default function App() {
  const [activeTab, setActiveTab] = useState<'dashboard' | 'agents' | 'architecture' | 'memory' | 'settings'>('dashboard');
  const [sidebarOpen, setSidebarOpen] = useState(true);
  const [agents, setAgents] = useState<Agent[]>(initialAgents);
  const [systemTime, setSystemTime] = useState(new Date());

  // Update system time
  useEffect(() => {
    const interval = setInterval(() => setSystemTime(new Date()), 1000);
    return () => clearInterval(interval);
  }, []);

  // Simulate agent activity
  useEffect(() => {
    const interval = setInterval(() => {
      setAgents(prev => prev.map(agent => ({
        ...agent,
        progress: agent.status === 'active' || agent.status === 'processing'
          ? Math.min(100, agent.progress + Math.random() * 5)
          : agent.progress,
        tasksCompleted: agent.status === 'active'
          ? agent.tasksCompleted + Math.floor(Math.random() * 3)
          : agent.tasksCompleted
      })));
    }, 2000);
    return () => clearInterval(interval);
  }, []);

  const navItems = [
    { id: 'dashboard', label: 'Dashboard', icon: Activity },
    { id: 'agents', label: 'Agent Orchestrator', icon: Brain },
    { id: 'architecture', label: 'Architecture', icon: Layers },
    { id: 'memory', label: 'Memory System', icon: Database },
    { id: 'settings', label: 'System Settings', icon: Settings }
  ] as const;

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-50 via-white to-slate-100">
      {/* Header */}
      <header className="fixed top-0 left-0 right-0 h-16 bg-white border-b border-slate-200 z-50 flex items-center justify-between px-6">
        <div className="flex items-center gap-4">
          <button
            onClick={() => setSidebarOpen(!sidebarOpen)}
            className="p-2 hover:bg-slate-100 rounded-lg transition-colors"
          >
            {sidebarOpen ? <X className="w-5 h-5 text-slate-500" /> : <Menu className="w-5 h-5 text-slate-500" />}
          </button>
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-gradient-to-br from-violet-500 to-indigo-600 flex items-center justify-center">
              <Brain className="w-6 h-6 text-white" />
            </div>
            <div>
              <h1 className="font-bold text-slate-800">SZM Forge</h1>
              <p className="text-xs text-slate-500">Autonomous Engineering Intelligence</p>
            </div>
          </div>
        </div>

        <div className="flex items-center gap-6">
          <div className="flex items-center gap-2 px-3 py-1.5 bg-emerald-50 rounded-full">
            <WifiOff className="w-4 h-4 text-emerald-500" />
            <span className="text-sm font-medium text-emerald-700">Offline Mode</span>
          </div>
          <div className="text-right">
            <div className="text-sm font-medium text-slate-700">
              {systemTime.toLocaleTimeString()}
            </div>
            <div className="text-xs text-slate-500">
              {systemTime.toLocaleDateString()}
            </div>
          </div>
          <div className="w-10 h-10 rounded-full bg-gradient-to-br from-slate-200 to-slate-300 flex items-center justify-center">
            <Cpu className="w-5 h-5 text-slate-600" />
          </div>
        </div>
      </header>

      {/* Sidebar */}
      <AnimatePresence>
        {sidebarOpen && (
          <motion.aside
            initial={{ width: 0, opacity: 0 }}
            animate={{ width: 280, opacity: 1 }}
            exit={{ width: 0, opacity: 0 }}
            className="fixed left-0 top-16 bottom-0 bg-white border-r border-slate-200 z-40 overflow-hidden"
          >
            <nav className="p-4 space-y-1">
              {navItems.map((item) => {
                const Icon = item.icon;
                const isActive = activeTab === item.id;
                return (
                  <button
                    key={item.id}
                    onClick={() => setActiveTab(item.id)}
                    className={`w-full flex items-center gap-3 px-4 py-3 rounded-lg transition-all ${
                      isActive
                        ? 'bg-gradient-to-r from-violet-500 to-indigo-600 text-white shadow-lg shadow-violet-200'
                        : 'text-slate-600 hover:bg-slate-100'
                    }`}
                  >
                    <Icon className="w-5 h-5" />
                    <span className="font-medium">{item.label}</span>
                    {isActive && <ChevronRight className="w-4 h-4 ml-auto" />}
                  </button>
                );
              })}
            </nav>

            <div className="absolute bottom-0 left-0 right-0 p-4 border-t border-slate-100">
              <div className="p-4 rounded-xl bg-gradient-to-br from-violet-50 to-indigo-50 border border-violet-100">
                <div className="flex items-center gap-2 mb-2">
                  <Shield className="w-4 h-4 text-violet-600" />
                  <span className="text-sm font-semibold text-violet-700">Security Status</span>
                </div>
                <div className="space-y-1">
                  <div className="flex items-center gap-2 text-xs text-violet-600">
                    <Lock className="w-3 h-3" />
                    <span>All systems secured</span>
                  </div>
                  <div className="flex items-center gap-2 text-xs text-violet-600">
                    <CheckCircle className="w-3 h-3" />
                    <span>Sandbox active</span>
                  </div>
                </div>
              </div>
            </div>
          </motion.aside>
        )}
      </AnimatePresence>

      {/* Main Content */}
      <main
        className={`pt-20 pb-8 px-6 transition-all duration-300 ${
          sidebarOpen ? 'ml-[280px]' : 'ml-0'
        }`}
      >
        <AnimatePresence mode="wait">
          {activeTab === 'dashboard' && (
            <motion.div
              key="dashboard"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              className="space-y-6"
            >
              {/* Hero Stats */}
              <div className="grid grid-cols-4 gap-6">
                {[
                  { label: 'Active Agents', value: '7', icon: Brain, color: 'from-violet-500 to-purple-600' },
                  { label: 'Tasks Completed', value: '26,035', icon: CheckCircle, color: 'from-emerald-500 to-teal-600' },
                  { label: 'System Health', value: '94%', icon: Activity, color: 'from-blue-500 to-cyan-600' },
                  { label: 'Memory Entries', value: '15,678', icon: Database, color: 'from-amber-500 to-orange-600' }
                ].map((stat, index) => {
                  const Icon = stat.icon;
                  return (
                    <motion.div
                      key={stat.label}
                      initial={{ opacity: 0, y: 20 }}
                      animate={{ opacity: 1, y: 0 }}
                      transition={{ delay: index * 0.1 }}
                      className="bg-white rounded-xl border border-slate-200 p-6 shadow-sm"
                    >
                      <div className="flex items-center justify-between mb-4">
                        <div className={`w-12 h-12 rounded-xl bg-gradient-to-br ${stat.color} flex items-center justify-center`}>
                          <Icon className="w-6 h-6 text-white" />
                        </div>
                        <StatusIndicator status="active" />
                      </div>
                      <div className="text-3xl font-bold text-slate-800 mb-1">{stat.value}</div>
                      <div className="text-sm text-slate-500">{stat.label}</div>
                    </motion.div>
                  );
                })}
              </div>

              {/* Main Grid */}
              <div className="grid grid-cols-3 gap-6">
                <div className="col-span-2 space-y-6">
                  <PerformanceChart />
                  <div className="grid grid-cols-2 gap-6">
                    <ResourceMonitor />
                    <CapabilityRadar />
                  </div>
                </div>
                <div className="space-y-6">
                  <MemoryPanel />
                  <SimulationPanel />
                </div>
              </div>

              {/* Bottom Row */}
              <div className="grid grid-cols-2 gap-6">
                <SelfImprovementPanel />
                <ProjectAnalyzer />
              </div>
            </motion.div>
          )}

          {activeTab === 'agents' && (
            <motion.div
              key="agents"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              className="space-y-6"
            >
              <div className="flex items-center justify-between">
                <div>
                  <h2 className="text-2xl font-bold text-slate-800">Agent Orchestrator</h2>
                  <p className="text-slate-500">Multi-agent intelligence system coordination</p>
                </div>
                <button className="px-4 py-2 bg-gradient-to-r from-violet-500 to-indigo-600 text-white rounded-lg font-medium hover:opacity-90 transition-opacity flex items-center gap-2">
                  <RefreshCw className="w-4 h-4" />
                  Refresh Status
                </button>
              </div>

              <div className="grid grid-cols-2 gap-6">
                {agents.map((agent) => (
                  <AgentCard key={agent.id} agent={agent} />
                ))}
              </div>

              <Card title="Agent Communication Network" icon={Network}>
                <div className="h-64 flex items-center justify-center">
                  <div className="relative w-full h-full">
                    {/* Simplified network visualization */}
                    <svg className="w-full h-full" viewBox="0 0 800 400">
                      {/* Connection lines */}
                      <line x1="400" y1="200" x2="200" y2="100" stroke="#e2e8f0" strokeWidth="2" />
                      <line x1="400" y1="200" x2="600" y2="100" stroke="#e2e8f0" strokeWidth="2" />
                      <line x1="400" y1="200" x2="150" y2="250" stroke="#e2e8f0" strokeWidth="2" />
                      <line x1="400" y1="200" x2="650" y2="250" stroke="#e2e8f0" strokeWidth="2" />
                      <line x1="400" y1="200" x2="250" y2="350" stroke="#e2e8f0" strokeWidth="2" />
                      <line x1="400" y1="200" x2="550" y2="350" stroke="#e2e8f0" strokeWidth="2" />
                      
                      {/* Center node - Orchestrator */}
                      <circle cx="400" cy="200" r="40" fill="url(#centerGradient)" />
                      <text x="400" y="205" textAnchor="middle" fill="white" fontSize="12" fontWeight="bold">Core</text>
                      
                      {/* Agent nodes */}
                      {agents.slice(0, 6).map((agent, i) => {
                        const angle = (i / 6) * 2 * Math.PI - Math.PI / 2;
                        const x = 400 + Math.cos(angle) * 180;
                        const y = 200 + Math.sin(angle) * 180;
                        return (
                          <g key={agent.id}>
                            <circle cx={x} cy={y} r="30" fill={agent.color} opacity="0.2" />
                            <circle cx={x} cy={y} r="25" fill={agent.color} />
                            <text x={x} y={y + 5} textAnchor="middle" fill="white" fontSize="10">
                              {agent.name.split(' ')[0]}
                            </text>
                          </g>
                        );
                      })}
                      
                      <defs>
                        <linearGradient id="centerGradient" x1="0%" y1="0%" x2="100%" y2="100%">
                          <stop offset="0%" stopColor="#8b5cf6" />
                          <stop offset="100%" stopColor="#4f46e5" />
                        </linearGradient>
                      </defs>
                    </svg>
                  </div>
                </div>
              </Card>
            </motion.div>
          )}

          {activeTab === 'architecture' && (
            <motion.div
              key="architecture"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              className="space-y-6"
            >
              <div>
                <h2 className="text-2xl font-bold text-slate-800">System Architecture</h2>
                <p className="text-slate-500">Multi-layer cognitive architecture stack</p>
              </div>

              <div className="grid grid-cols-2 gap-6">
                <ArchitectureDiagram />
                <Card title="Layer Health Metrics" icon={Activity}>
                  <div className="space-y-4">
                    {systemLayers.map((layer, index) => (
                      <motion.div
                        key={layer.id}
                        initial={{ opacity: 0, x: -10 }}
                        animate={{ opacity: 1, x: 0 }}
                        transition={{ delay: index * 0.1 }}
                        className="p-4 rounded-lg bg-slate-50"
                      >
                        <div className="flex items-center justify-between mb-2">
                          <span className="font-medium text-slate-700">{layer.name}</span>
                          <StatusIndicator status={layer.status} />
                        </div>
                        <div className="flex items-center gap-2">
                          <div className="flex-1 h-2 bg-slate-200 rounded-full overflow-hidden">
                            <motion.div
                              initial={{ width: 0 }}
                              animate={{ width: `${layer.health}%` }}
                              transition={{ duration: 1, delay: index * 0.1 }}
                              className={`h-full rounded-full ${
                                layer.health >= 90 ? 'bg-emerald-500' :
                                layer.health >= 70 ? 'bg-amber-500' :
                                'bg-red-500'
                              }`}
                            />
                          </div>
                          <span className="text-sm font-medium text-slate-600 w-12">{layer.health}%</span>
                        </div>
                      </motion.div>
                    ))}
                  </div>
                </Card>
              </div>

              <Card title="Component Dependencies" icon={GitBranch}>
                <div className="grid grid-cols-5 gap-4">
                  {['Foundation LLM', 'Agentic System', 'Simulation', 'Knowledge Graph', 'Self-Improvement'].map((comp, i) => (
                    <motion.div
                      key={comp}
                      initial={{ opacity: 0, y: 20 }}
                      animate={{ opacity: 1, y: 0 }}
                      transition={{ delay: i * 0.1 }}
                      className="p-4 rounded-xl bg-gradient-to-br from-slate-50 to-slate-100 border border-slate-200 text-center"
                    >
                      <div className="w-12 h-12 rounded-xl bg-gradient-to-br from-violet-500 to-indigo-600 flex items-center justify-center mx-auto mb-3">
                        <Layers className="w-6 h-6 text-white" />
                      </div>
                      <div className="text-sm font-medium text-slate-700">{comp}</div>
                      {i < 4 && (
                        <ArrowDown className="w-4 h-4 text-slate-400 mx-auto mt-3" />
                      )}
                    </motion.div>
                  ))}
                </div>
              </Card>
            </motion.div>
          )}

          {activeTab === 'memory' && (
            <motion.div
              key="memory"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              className="space-y-6"
            >
              <div className="flex items-center justify-between">
                <div>
                  <h2 className="text-2xl font-bold text-slate-800">Memory System</h2>
                  <p className="text-slate-500">Long-term knowledge storage and retrieval</p>
                </div>
                <div className="flex items-center gap-3">
                  <button className="px-4 py-2 border border-slate-200 rounded-lg font-medium text-slate-600 hover:bg-slate-50 transition-colors flex items-center gap-2">
                    <Download className="w-4 h-4" />
                    Export
                  </button>
                  <button className="px-4 py-2 border border-slate-200 rounded-lg font-medium text-slate-600 hover:bg-slate-50 transition-colors flex items-center gap-2">
                    <Upload className="w-4 h-4" />
                    Import
                  </button>
                </div>
              </div>

              <div className="grid grid-cols-5 gap-6">
                {[
                  { type: 'Session', count: 156, color: 'from-blue-500 to-cyan-600' },
                  { type: 'Project', count: 23, color: 'from-purple-500 to-pink-600' },
                  { type: 'Technical', count: 8945, color: 'from-emerald-500 to-teal-600' },
                  { type: 'Evolution', count: 47, color: 'from-amber-500 to-orange-600' },
                  { type: 'Decision', count: 6507, color: 'from-rose-500 to-red-600' }
                ].map((mem, index) => (
                  <motion.div
                    key={mem.type}
                    initial={{ opacity: 0, y: 20 }}
                    animate={{ opacity: 1, y: 0 }}
                    transition={{ delay: index * 0.1 }}
                    className="bg-white rounded-xl border border-slate-200 p-6 shadow-sm"
                  >
                    <div className={`w-12 h-12 rounded-xl bg-gradient-to-br ${mem.color} flex items-center justify-center mb-4`}>
                      <Database className="w-6 h-6 text-white" />
                    </div>
                    <div className="text-3xl font-bold text-slate-800 mb-1">{mem.count.toLocaleString()}</div>
                    <div className="text-sm text-slate-500">{mem.type} Memory</div>
                  </motion.div>
                ))}
              </div>

              <MemoryPanel />
            </motion.div>
          )}

          {activeTab === 'settings' && (
            <motion.div
              key="settings"
              initial={{ opacity: 0, y: 20 }}
              animate={{ opacity: 1, y: 0 }}
              exit={{ opacity: 0, y: -20 }}
              className="space-y-6"
            >
              <div>
                <h2 className="text-2xl font-bold text-slate-800">System Settings</h2>
                <p className="text-slate-500">Configure SZM Forge parameters</p>
              </div>

              <div className="grid grid-cols-2 gap-6">
                <Card title="Resource Allocation" icon={Cpu}>
                  <div className="space-y-4">
                    {[
                      { label: 'Max RAM Usage', value: '48 GB', max: '64 GB' },
                      { label: 'Max VRAM Usage', value: '20 GB', max: '24 GB' },
                      { label: 'CPU Threads', value: '12', max: '16' },
                      { label: 'GPU Acceleration', value: 'Enabled', max: '' }
                    ].map((setting) => (
                      <div key={setting.label} className="flex items-center justify-between p-3 rounded-lg bg-slate-50">
                        <span className="text-sm font-medium text-slate-700">{setting.label}</span>
                        <span className="text-sm text-slate-600">{setting.value}</span>
                      </div>
                    ))}
                  </div>
                </Card>

                <Card title="Safety Systems" icon={Shield}>
                  <div className="space-y-4">
                    {[
                      { label: 'Sandbox Mode', enabled: true },
                      { label: 'Rollback Protection', enabled: true },
                      { label: 'Action Logging', enabled: true },
                      { label: 'Human Approval Mode', enabled: false },
                      { label: 'Auto-evolution', enabled: true }
                    ].map((setting) => (
                      <div key={setting.label} className="flex items-center justify-between p-3 rounded-lg bg-slate-50">
                        <span className="text-sm font-medium text-slate-700">{setting.label}</span>
                        <button
                          className={`w-12 h-6 rounded-full transition-colors ${
                            setting.enabled ? 'bg-emerald-500' : 'bg-slate-300'
                          }`}
                        >
                          <div
                            className={`w-5 h-5 rounded-full bg-white shadow-sm transition-transform ${
                              setting.enabled ? 'translate-x-6' : 'translate-x-0.5'
                            }`}
                          />
                        </button>
                      </div>
                    ))}
                  </div>
                </Card>

                <Card title="Model Configuration" icon={Brain}>
                  <div className="space-y-4">
                    <div className="p-3 rounded-lg bg-slate-50">
                      <div className="text-sm font-medium text-slate-700 mb-1">Base Model</div>
                      <select className="w-full p-2 border border-slate-200 rounded-lg text-sm">
                        <option>Llama-4 Scout (109B)</option>
                        <option>Qwen 3.5 (122B)</option>
                        <option>Mistral Large 2 (123B)</option>
                        <option>Nemotron Ultra</option>
                      </select>
                    </div>
                    <div className="p-3 rounded-lg bg-slate-50">
                      <div className="text-sm font-medium text-slate-700 mb-1">Context Window</div>
                      <select className="w-full p-2 border border-slate-200 rounded-lg text-sm">
                        <option>32K tokens</option>
                        <option>128K tokens</option>
                        <option>256K tokens</option>
                        <option>1M tokens</option>
                      </select>
                    </div>
                    <div className="p-3 rounded-lg bg-slate-50">
                      <div className="text-sm font-medium text-slate-700 mb-1">Quantization</div>
                      <select className="w-full p-2 border border-slate-200 rounded-lg text-sm">
                        <option>FP16 (Full Precision)</option>
                        <option>INT8 (8-bit)</option>
                        <option>INT4 (4-bit)</option>
                      </select>
                    </div>
                  </div>
                </Card>

                <Card title="Simulation Backends" icon={Microscope}>
                  <div className="space-y-3">
                    {[
                      { name: 'OpenFOAM', status: 'connected', type: 'CFD' },
                      { name: 'Code_Aster', status: 'connected', type: 'FEA' },
                      { name: 'FreeCAD', status: 'connected', type: 'CAD' },
                      { name: 'SU2', status: 'offline', type: 'Aero' },
                      { name: 'Elmer', status: 'connected', type: 'Multiphysics' }
                    ].map((backend) => (
                      <div key={backend.name} className="flex items-center justify-between p-3 rounded-lg bg-slate-50">
                        <div>
                          <div className="text-sm font-medium text-slate-700">{backend.name}</div>
                          <div className="text-xs text-slate-500">{backend.type}</div>
                        </div>
                        <StatusIndicator status={backend.status === 'connected' ? 'active' : 'offline'} />
                      </div>
                    ))}
                  </div>
                </Card>
              </div>
            </motion.div>
          )}
        </AnimatePresence>
      </main>
    </div>
  );
}
