"""
Agentic Reasoning System for SZM Forge

Implements the agentic AI layer that:
- Decomposes design problems into sub-tasks
- Manages context and memory (short-term and long-term)
- Orchestrates tool calls (simulators, databases, CAD)
- Plans actions and refines strategies
"""

from typing import Dict, List, Optional, Any, Callable, Tuple
from dataclasses import dataclass, field
from enum import Enum
import logging
from datetime import datetime
import uuid
import json

logger = logging.getLogger(__name__)


class AgentState(Enum):
    """Agent lifecycle states"""
    IDLE = "idle"
    THINKING = "thinking"
    PLANNING = "planning"
    EXECUTING = "executing"
    WAITING_FOR_FEEDBACK = "waiting_for_feedback"
    REFINING = "refining"
    COMPLETE = "complete"
    ERROR = "error"


class ToolType(Enum):
    """Types of tools the agent can invoke"""
    SIMULATOR = "simulator"
    DATABASE = "database"
    CAD = "cad"
    OPTIMIZER = "optimizer"
    VALIDATOR = "validator"
    RETRIEVER = "retriever"
    COMPUTATION = "computation"


@dataclass
class Tool:
    """Tool definition that agent can call"""
    name: str
    tool_type: ToolType
    description: str
    input_schema: Dict[str, Any]
    output_schema: Dict[str, Any]
    callable: Optional[Callable] = None
    parameters: Dict[str, Any] = field(default_factory=dict)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "type": self.tool_type.value,
            "description": self.description,
            "input_schema": self.input_schema,
            "output_schema": self.output_schema,
            "parameters": self.parameters
        }


@dataclass
class ToolCall:
    """Record of a tool invocation"""
    id: str
    tool_name: str
    input_args: Dict[str, Any]
    output: Optional[Any] = None
    status: str = "pending"  # pending, executing, completed, failed
    reasoning: str = ""
    created_at: datetime = field(default_factory=datetime.now)
    completed_at: Optional[datetime] = None
    error: Optional[str] = None
    duration_ms: float = 0.0
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "tool": self.tool_name,
            "input": self.input_args,
            "output": self.output,
            "status": self.status,
            "reasoning": self.reasoning,
            "error": self.error,
            "duration_ms": self.duration_ms
        }


@dataclass
class Memory:
    """Agent memory module"""
    short_term: List[Dict[str, Any]] = field(default_factory=list)
    long_term: Dict[str, Any] = field(default_factory=dict)
    episodic: List[Dict[str, Any]] = field(default_factory=list)
    max_short_term: int = 50
    
    def add_short_term(self, item: Dict[str, Any]):
        """Add to short-term memory (current context)"""
        self.short_term.append({
            **item,
            "timestamp": datetime.now().isoformat()
        })
        # Keep only recent items
        if len(self.short_term) > self.max_short_term:
            self.short_term = self.short_term[-self.max_short_term:]
    
    def add_long_term(self, key: str, value: Any):
        """Add to long-term memory (persistent knowledge)"""
        self.long_term[key] = {
            "value": value,
            "timestamp": datetime.now().isoformat()
        }
    
    def add_episode(self, episode: Dict[str, Any]):
        """Record episodic memory (past experiences)"""
        self.episodic.append({
            **episode,
            "timestamp": datetime.now().isoformat()
        })
    
    def recall_long_term(self, key: str) -> Optional[Any]:
        """Retrieve from long-term memory"""
        if key in self.long_term:
            return self.long_term[key]["value"]
        return None
    
    def get_context_window(self, size: int = 20) -> List[Dict[str, Any]]:
        """Get recent context (last N items from short-term)"""
        return self.short_term[-size:]


@dataclass
class Plan:
    """Action plan for agent"""
    id: str
    goal: str
    steps: List[Dict[str, Any]] = field(default_factory=list)
    constraints: List[str] = field(default_factory=list)
    success_criteria: List[str] = field(default_factory=list)
    current_step: int = 0
    status: str = "active"  # active, paused, completed, failed
    reasoning: str = ""
    created_at: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "goal": self.goal,
            "steps": self.steps,
            "constraints": self.constraints,
            "current_step": self.current_step,
            "status": self.status
        }


class AgentOrchestrator:
    """
    Orchestrates the agentic reasoning system.
    
    Responsibilities:
    - Problem decomposition (chain-of-thought planning)
    - Tool management and orchestration
    - Memory management
    - Plan execution and refinement
    - Feedback integration
    """
    
    def __init__(self, name: str = "SZM_Agent"):
        self.name = name
        self.state = AgentState.IDLE
        self.tools: Dict[str, Tool] = {}
        self.memory = Memory()
        self.current_plan: Optional[Plan] = None
        self.tool_calls: List[ToolCall] = []
        self.reasoning_trace: List[str] = []
        self.logger = logger
        
    def register_tool(self, tool: Tool):
        """Register a tool that agent can invoke"""
        self.tools[tool.name] = tool
        self.logger.info(f"Tool registered: {tool.name}")
    
    def decompose_problem(
        self,
        problem_statement: str,
        context: Optional[Dict[str, Any]] = None
    ) -> Plan:
        """
        Decompose a design problem into sub-tasks (chain-of-thought).
        
        This is where the agent "thinks through" the problem step-by-step.
        """
        self.state = AgentState.THINKING
        plan_id = str(uuid.uuid4())
        
        # Log reasoning trace
        reasoning_step = f"[DECOMPOSE] Problem: {problem_statement[:100]}..."
        self.reasoning_trace.append(reasoning_step)
        
        # Basic decomposition logic - would be enhanced with LLM
        steps = [
            {
                "step": 1,
                "action": "analyze_requirements",
                "description": "Analyze design requirements and constraints",
                "tools_needed": ["retriever", "validator"]
            },
            {
                "step": 2,
                "action": "search_knowledge_graph",
                "description": "Search knowledge graph for similar designs",
                "tools_needed": ["database"]
            },
            {
                "step": 3,
                "action": "generate_candidates",
                "description": "Generate design candidates",
                "tools_needed": ["optimizer", "cad"]
            },
            {
                "step": 4,
                "action": "simulate_and_evaluate",
                "description": "Run simulations to evaluate designs",
                "tools_needed": ["simulator", "validator"]
            },
            {
                "step": 5,
                "action": "refine_design",
                "description": "Refine best design based on results",
                "tools_needed": ["optimizer", "simulator"]
            }
        ]
        
        plan = Plan(
            id=plan_id,
            goal=problem_statement,
            steps=steps,
            constraints=context.get("constraints", []) if context else [],
            success_criteria=context.get("success_criteria", []) if context else [],
            reasoning="Decomposed problem into sequential design steps"
        )
        
        self.current_plan = plan
        self.memory.add_short_term({
            "type": "plan",
            "plan": plan.to_dict(),
            "reasoning": reasoning_step
        })
        
        self.state = AgentState.PLANNING
        self.logger.info(f"Problem decomposed into {len(steps)} steps")
        
        return plan
    
    def plan_next_action(
        self,
        current_context: Dict[str, Any]
    ) -> Optional[Dict[str, Any]]:
        """
        Plan the next action based on current state and context.
        Uses chain-of-thought to select appropriate tools and parameters.
        """
        if not self.current_plan or self.current_plan.status != "active":
            return None
        
        step_idx = self.current_plan.current_step
        if step_idx >= len(self.current_plan.steps):
            self.current_plan.status = "completed"
            return None
        
        current_step = self.current_plan.steps[step_idx]
        
        # Log reasoning
        reasoning = f"[ACTION_PLAN] Executing step {step_idx + 1}: {current_step['action']}"
        self.reasoning_trace.append(reasoning)
        
        action_plan = {
            "step_index": step_idx,
            "action": current_step["action"],
            "description": current_step["description"],
            "tools_needed": current_step["tools_needed"],
            "reasoning": reasoning
        }
        
        self.memory.add_short_term({
            "type": "action_plan",
            "plan": action_plan
        })
        
        return action_plan
    
    def execute_tool(
        self,
        tool_name: str,
        input_args: Dict[str, Any],
        reasoning: str = ""
    ) -> Tuple[bool, Optional[Any], Optional[str]]:
        """
        Execute a tool and record the invocation.
        Returns (success, output, error_message)
        """
        self.state = AgentState.EXECUTING
        
        if tool_name not in self.tools:
            error_msg = f"Tool not found: {tool_name}"
            self.logger.error(error_msg)
            self.state = AgentState.ERROR
            return False, None, error_msg
        
        tool = self.tools[tool_name]
        tool_call = ToolCall(
            id=str(uuid.uuid4()),
            tool_name=tool_name,
            input_args=input_args,
            reasoning=reasoning,
            status="executing"
        )
        
        self.tool_calls.append(tool_call)
        
        try:
            import time
            start_time = time.time()
            
            if tool.callable:
                output = tool.callable(**input_args)
            else:
                output = {"status": "tool_not_implemented"}
            
            duration_ms = (time.time() - start_time) * 1000
            
            tool_call.output = output
            tool_call.status = "completed"
            tool_call.duration_ms = duration_ms
            tool_call.completed_at = datetime.now()
            
            # Record in memory
            self.memory.add_short_term({
                "type": "tool_call",
                "tool_call": tool_call.to_dict()
            })
            
            self.logger.info(f"Tool executed: {tool_name} ({duration_ms:.2f}ms)")
            return True, output, None
            
        except Exception as e:
            error_msg = str(e)
            tool_call.status = "failed"
            tool_call.error = error_msg
            tool_call.completed_at = datetime.now()
            self.logger.error(f"Tool execution failed: {error_msg}")
            self.state = AgentState.ERROR
            return False, None, error_msg
    
    def refine_plan(
        self,
        feedback: Dict[str, Any]
    ) -> bool:
        """
        Refine current plan based on feedback from tool execution.
        """
        self.state = AgentState.REFINING
        
        if not self.current_plan:
            return False
        
        # Log feedback
        reasoning = f"[REFINE] Feedback received: {feedback.get('type', 'unknown')}"
        self.reasoning_trace.append(reasoning)
        
        self.memory.add_short_term({
            "type": "feedback",
            "feedback": feedback,
            "reasoning": reasoning
        })
        
        # Move to next step
        self.current_plan.current_step += 1
        
        # Check if plan is complete
        if self.current_plan.current_step >= len(self.current_plan.steps):
            self.current_plan.status = "completed"
            self.state = AgentState.COMPLETE
        else:
            self.state = AgentState.PLANNING
        
        return True
    
    def get_reasoning_trace(self) -> List[str]:
        """Get full trace of agent's reasoning"""
        return self.reasoning_trace
    
    def get_tool_call_history(self) -> List[Dict[str, Any]]:
        """Get history of tool calls"""
        return [tc.to_dict() for tc in self.tool_calls]
    
    def get_context_window(self, size: int = 20) -> List[Dict[str, Any]]:
        """Get current context window from memory"""
        return self.memory.get_context_window(size)
    
    def reset(self):
        """Reset agent state"""
        self.state = AgentState.IDLE
        self.current_plan = None
        self.tool_calls = []
        self.reasoning_trace = []
        self.logger.info("Agent reset")
    
    def export_execution_log(self, filepath: str):
        """Export execution log to JSON"""
        log_data = {
            "agent_name": self.name,
            "final_state": self.state.value,
            "reasoning_trace": self.reasoning_trace,
            "tool_calls": self.get_tool_call_history(),
            "context_window": self.get_context_window(),
            "exported_at": datetime.now().isoformat()
        }
        
        with open(filepath, 'w') as f:
            json.dump(log_data, f, indent=2)
        
        self.logger.info(f"Execution log exported to {filepath}")
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get execution statistics"""
        completed_tools = sum(1 for tc in self.tool_calls if tc.status == "completed")
        failed_tools = sum(1 for tc in self.tool_calls if tc.status == "failed")
        total_duration = sum(tc.duration_ms for tc in self.tool_calls)
        
        return {
            "total_tool_calls": len(self.tool_calls),
            "completed_tools": completed_tools,
            "failed_tools": failed_tools,
            "total_duration_ms": total_duration,
            "average_tool_duration_ms": total_duration / max(completed_tools, 1),
            "reasoning_steps": len(self.reasoning_trace),
            "current_state": self.state.value
        }
