import React, { useState, useRef, useEffect, useMemo } from 'react';
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
  "What is the lift force formula?",
  "Explain carbon fiber properties",
  "How does PID tuning work?",
  "What is drag coefficient for a drone?",
  "LiPo battery safety specs",
  "Calculate motor torque and efficiency"
];

// Knowledge Base logic now uses the Python backend GraphRAG API

export const AIChatBot: React.FC = () => {
  const [messages, setMessages] = useState<Message[]>([
    {
      id: 'msg-1',
      type: 'ai',
      text: 'SZM Forge AI Assistant initialized. Systems are online. How can I assist with your engineering tasks today?',
      timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
    }
  ]);
  const [inputValue, setInputValue] = useState('');
  const [isTyping, setIsTyping] = useState(false);
  useEffect(() => {
    // Update welcome message
    setMessages([{
      id: 'msg-1',
      type: 'ai',
      text: `SZM Forge AI Assistant initialized.\n\n📚 Knowledge Base: Connected to Neo4j GraphRAG\n🗂️ Systems Online: Causal Reasoning, Optimization, Diagnostic\n\nAsk me about high-stress joints, materials, aerodynamics, or any engineering topic to query the knowledge graph.`,
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
      let responseText = "Failed to connect to the AI Knowledge Graph backend.";
      let confidence = 0;
      let matchedTerms: string[] = [];

      try {
        const res = await fetch('http://localhost:8001/api/graphrag/query', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ query: text })
        });
        
        if (res.ok) {
          const data = await res.json();
          responseText = data.answer || "No insights found.";
          confidence = data.confidence || 0.85;
          matchedTerms = data.matchedTerms || [];
        }
      } catch (err) {
        console.error("GraphRAG fetch error:", err);
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
              FORGE AI ASSISTANT
            </div>
            <div className="text-[9px] font-mono text-forge-text-muted flex items-center gap-1.5">
              <div className={`w-1.5 h-1.5 rounded-full bg-forge-green`} />
              CONNECTED TO GRAPH RAG
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
