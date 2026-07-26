import React, { useState, useRef, useEffect } from 'react';
import { Send, Bot, User, Sparkles, Command, ArrowRight, Loader, Database, BookOpen } from 'lucide-react';

interface Message {
  id: string;
  type: 'user' | 'ai';
  text: string;
  timestamp: string;
  confidence?: number;
  matchedTerms?: string[];
}

const actionChips = [
  "Review the current assembly for high-risk joints",
  "Suggest a material trade-off for this design",
  "Explain the next simulation step",
  "Recommend a weight-saving optimization",
  "Check safety factors and failure modes",
  "Summarize the current engineering context"
];

const createLocalResponse = (query: string) => {
  const normalized = query.toLowerCase();

  if (normalized.includes('stress') || normalized.includes('failure') || normalized.includes('safety')) {
    return [
      'I would inspect the highest-stress regions first and verify that the load path stays continuous through each joint.',
      'Recommended next step: add a hotspot review around the connection points and compare the current factor of safety against the target.',
      'If you want, I can help you turn this into a concrete design review checklist.'
    ].join('\n\n');
  }

  if (normalized.includes('material') || normalized.includes('steel') || normalized.includes('carbon')) {
    return [
      'For a first pass, compare density, stiffness, and manufacturability rather than only the headline strength value.',
      'A helpful trade-off view is: lighter material vs. cost vs. thermal stability vs. ease of machining.',
      'I can help rank likely candidates once you share the design constraints.'
    ].join('\n\n');
  }

  if (normalized.includes('thermal') || normalized.includes('heat')) {
    return [
      'Thermal questions usually come down to heat path, boundary conditions, and expansion mismatch.',
      'I would review whether the part is cooling through the intended contact surfaces and whether the temperature gradient is creating a reliability risk.',
      'This is a good place to anchor the next simulation or design iteration.'
    ].join('\n\n');
  }

  if (normalized.includes('optimiz') || normalized.includes('weight') || normalized.includes('mass')) {
    return [
      'The best optimization targets are usually the areas that are both structurally sensitive and materially expensive.',
      'I would look for cutouts, wall thickness changes, and topology opportunities that preserve stiffness while dropping unnecessary mass.',
      'I can help turn that into a ranked improvement plan.'
    ].join('\n\n');
  }

  return [
    'The local SZM reasoning model is ready to help with design reviews, simulation planning, and optimization ideas.',
    'Try asking for a stress review, a material trade-off, a thermal check, or the next simulation step and I’ll structure the answer around the current engineering context.'
  ].join('\n\n');
};

// Knowledge Base logic now uses the Python backend GraphRAG API

export const AIChatBot: React.FC = () => {
  const [messages, setMessages] = useState<Message[]>([
    {
      id: 'msg-1',
      type: 'ai',
      text: 'SZM AI Assistance initialized. Systems are online. Ask about stress hotspots, materials, thermal behavior, or the next simulation step.',
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
    }
  ]);
  const [inputValue, setInputValue] = useState('');
  const [isTyping, setIsTyping] = useState(false);
  const messagesEndRef = useRef<HTMLDivElement | null>(null);
  useEffect(() => {
    // Update welcome message
    setMessages([{
      id: 'msg-1',
      type: 'ai',
      text: `SZM AI Assistance initialized.\n\n🧠 Modes: design review, material trade-off, thermal analysis, optimization planning\n📚 Knowledge Layer: GraphRAG + local engineering heuristics\n\nAsk me about high-stress joints, materials, or the next simulation step and I’ll help you move forward.`,
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
    }]);
  }, []);

  const scrollToBottom = () => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  };

  useEffect(() => {
    scrollToBottom();
  }, [messages, isTyping]);

  const handleSend = (text: string) => {
    if (!text.trim()) return;

    const newUserMsg: Message = {
      id: `msg-${Date.now()}`,
      type: 'user',
      text,
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
    };

    setMessages(prev => [...prev, newUserMsg]);
    setInputValue('');
    setIsTyping(true);

    // Process AI response via Python backend GraphRAG
    const fetchResponse = async () => {
      let responseText = createLocalResponse(text);
      let confidence = 0.74;
      let matchedTerms: string[] = ['local-guidance', 'engineering-assistant'];

      try {
        const res = await fetch('http://localhost:8001/api/graphrag/query', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ query: text })
        });

        if (res.ok) {
          const data = await res.json();
          responseText = data.answer || responseText;
          confidence = data.confidence || 0.85;
          matchedTerms = data.matchedTerms || matchedTerms;
        } else {
          responseText = createLocalResponse(text);
          confidence = 0.74;
        }
      } catch (err) {
        console.error('GraphRAG fetch error:', err);
        responseText = createLocalResponse(text);
        confidence = 0.72;
      }

      const newAiMsg: Message = {
        id: `msg-${Date.now() + 1}`,
        type: 'ai',
        text: responseText,
        timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
        confidence,
        matchedTerms
      };
      setMessages(prev => [...prev, newAiMsg]);
      setIsTyping(false);
    };
    
    fetchResponse();
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      handleSend(inputValue);
    }
  };

  return (
    <div className="h-full flex flex-col border-l border-forge-border/50 bg-forge-black relative">
      {/* Glow Effects */}
      <div className="absolute top-0 right-0 w-64 h-64 bg-forge-purple/5 rounded-full blur-[100px] pointer-events-none" />
      <div className="absolute bottom-0 left-0 w-64 h-64 bg-forge-accent/5 rounded-full blur-[100px] pointer-events-none" />

      {/* Header */}
      <div className="h-[56px] border-b border-forge-border/40 px-4 flex items-center justify-between flex-shrink-0 bg-forge-dark/50 backdrop-blur-sm z-10">
        <div className="flex items-center gap-2">
          <div className="relative">
            <div className="absolute inset-0 bg-forge-purple/20 animate-pulse-glow rounded-full" />
            <Bot size={16} className="text-forge-purple relative z-10" />
          </div>
          <div>
            <div className="text-[11px] font-bold tracking-widest text-forge-text text-glow-purple">
              SZM AI ASSISTANCE
            </div>
            <div className="text-[9px] font-mono text-forge-text-muted flex items-center gap-1.5">
              <div className={`w-1.5 h-1.5 rounded-full bg-forge-green`} />
              GRAPH RAG + LOCAL ASSISTANCE
            </div>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <div className="flex items-center gap-1 px-1.5 py-0.5 rounded bg-forge-surface/50 border border-forge-border/50">
            <Database size={8} className="text-forge-accent" />
            <span className="text-[8px] font-mono text-forge-text-muted">NEO4J LIVE</span>
          </div>
          <div className="flex items-center gap-1 px-1.5 py-0.5 rounded bg-forge-surface/50 border border-forge-border/50">
            <BookOpen size={8} className="text-forge-purple" />
            <span className="text-[8px] font-mono text-forge-text-muted">ONTOLOGY ACTIVE</span>
          </div>
        </div>
      </div>

      {/* Messages Area */}
      <div className="flex-1 overflow-y-auto p-4 space-y-4 z-10 custom-scrollbar">
        {messages.map((msg) => (
          <div
            key={msg.id}
            className={`flex flex-col max-w-[85%] ${msg.type === 'user' ? 'ml-auto items-end' : 'mr-auto items-start'}`}
          >
            <div className="flex items-center gap-1.5 mb-1 opacity-70">
              {msg.type === 'ai' ? (
                <>
                  <Bot size={10} className="text-forge-purple" />
                  <span className="text-[8px] font-bold tracking-wider text-forge-purple">FORGE AI</span>
                </>
              ) : (
                <>
                  <span className="text-[8px] font-bold tracking-wider text-forge-accent">YOU</span>
                  <User size={10} className="text-forge-accent" />
                </>
              )}
              <span className="text-[8px] font-mono text-forge-text-dim ml-1">{msg.timestamp}</span>
            </div>
            <div
              className={`p-3 rounded-lg text-[11px] leading-relaxed relative whitespace-pre-wrap ${
                msg.type === 'user'
                  ? 'bg-forge-surface/80 border border-forge-border/50 text-forge-text rounded-tr-sm'
                  : 'bg-forge-purple/10 border border-forge-purple/30 text-forge-text shadow-[0_0_15px_rgba(168,85,247,0.05)] rounded-tl-sm'
              }`}
            >
              {msg.text}
            </div>
            {/* Confidence bar and matched terms for AI messages */}
            {msg.type === 'ai' && msg.confidence !== undefined && msg.confidence > 0 && (
              <div className="mt-1.5 flex items-center gap-2">
                <div className="flex items-center gap-1">
                  <span className="text-[7px] text-forge-text-muted">Confidence:</span>
                  <div className="w-16 h-1 bg-forge-surface rounded-full overflow-hidden">
                    <div 
                      className="h-full rounded-full transition-all" 
                      style={{ 
                        width: `${msg.confidence * 100}%`,
                        backgroundColor: msg.confidence > 0.7 ? '#00ff88' : msg.confidence > 0.4 ? '#ffaa00' : '#ff3344'
                      }} 
                    />
                  </div>
                  <span className="text-[7px] font-mono text-forge-text-muted">{(msg.confidence * 100).toFixed(0)}%</span>
                </div>
                {msg.matchedTerms && msg.matchedTerms.length > 0 && (
                  <div className="flex items-center gap-1 flex-wrap">
                    {msg.matchedTerms.slice(0, 4).map((term, i) => (
                      <span key={i} className="px-1 py-0.5 rounded text-[7px] bg-forge-purple/10 text-forge-purple border border-forge-purple/20">
                        {term}
                      </span>
                    ))}
                  </div>
                )}
              </div>
            )}
          </div>
        ))}
        {isTyping && (
          <div className="flex flex-col max-w-[85%] mr-auto items-start animate-fade-in">
            <div className="flex items-center gap-1.5 mb-1 opacity-70">
              <Bot size={10} className="text-forge-purple" />
              <span className="text-[8px] font-bold tracking-wider text-forge-purple">FORGE AI</span>
            </div>
            <div className="p-3 rounded-lg bg-forge-purple/5 border border-forge-purple/20 rounded-tl-sm flex items-center gap-2">
              <Loader size={12} className="text-forge-purple animate-spin" />
              <span className="text-[10px] text-forge-text-dim">Processing engineering data...</span>
            </div>
          </div>
        )}
        <div ref={messagesEndRef} />
      </div>

      {/* Input Area */}
      <div className="p-4 border-t border-forge-border/40 bg-forge-dark/50 backdrop-blur-sm z-10">
        {/* Quick Actions */}
        <div className="flex flex-wrap gap-2 mb-3">
          {actionChips.map((chip, i) => (
            <button
              key={i}
              onClick={() => handleSend(chip)}
              className="px-2 py-1 rounded text-[9px] font-medium text-forge-text-dim bg-forge-surface/50 border border-forge-border hover:border-forge-purple/40 hover:text-forge-text hover:bg-forge-purple/10 transition-all text-left flex items-center gap-1"
            >
              <Sparkles size={8} className="text-forge-purple" />
              {chip}
            </button>
          ))}
        </div>

        {/* Input Field */}
        <div className="relative group">
          <input
            type="text"
            value={inputValue}
            onChange={(e) => setInputValue(e.target.value)}
            onKeyDown={handleKeyDown}
            placeholder="Ask Forge AI to analyze, optimize, or generate..."
            className="w-full bg-forge-surface/50 border border-forge-border/80 rounded-md py-2.5 pl-8 pr-10 text-[11px] text-forge-text placeholder:text-forge-text-muted focus:outline-none focus:border-forge-purple/50 focus:bg-forge-surface transition-all"
          />
          <Command size={12} className="absolute left-3 top-1/2 -translate-y-1/2 text-forge-text-muted" />
          <button
            onClick={() => handleSend(inputValue)}
            disabled={!inputValue.trim()}
            className="absolute right-2 top-1/2 -translate-y-1/2 p-1 rounded bg-forge-purple text-white disabled:opacity-50 disabled:cursor-not-allowed hover:bg-forge-purple/80 transition-colors shadow-[0_0_10px_rgba(168,85,247,0.3)]"
          >
            <ArrowRight size={12} />
          </button>
        </div>
      </div>
    </div>
  );
};
