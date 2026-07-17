"""
Causal Reasoning Engine for SZM Forge

Implements logical inference and causal reasoning:
- Causal graph traversal and inference
- Counterfactual reasoning ("what-if" scenarios)
- Failure mode analysis
- Design constraint propagation
- Hypothesis testing
"""

from typing import Dict, List, Optional, Set, Tuple, Any
from dataclasses import dataclass
from enum import Enum
import logging
from collections import defaultdict, deque
import json

logger = logging.getLogger(__name__)


class InferenceType(Enum):
    """Types of causal inference"""
    FORWARD = "forward"  # cause -> effect
    BACKWARD = "backward"  # effect -> cause
    ABDUCTIVE = "abductive"  # explain observation
    COUNTERFACTUAL = "counterfactual"  # what-if scenario


class ConstraintType(Enum):
    """Types of design constraints"""
    INEQUALITY = "inequality"  # x <= value
    EQUALITY = "equality"  # x == value
    RANGE = "range"  # min <= x <= max
    BOOLEAN = "boolean"  # true/false
    LOGICAL = "logical"  # compound logic


@dataclass
class Constraint:
    """Design constraint"""
    name: str
    constraint_type: ConstraintType
    expression: str
    value: Optional[float] = None
    min_value: Optional[float] = None
    max_value: Optional[float] = None
    severity: str = "warning"  # warning, error, critical
    reason: str = ""  # Why this constraint exists
    
    def is_satisfied(self, val: float) -> bool:
        """Check if constraint is satisfied"""
        if self.constraint_type == ConstraintType.EQUALITY:
            return val == self.value
        elif self.constraint_type == ConstraintType.INEQUALITY:
            return val <= self.value
        elif self.constraint_type == ConstraintType.RANGE:
            return self.min_value <= val <= self.max_value
        return False
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "name": self.name,
            "type": self.constraint_type.value,
            "expression": self.expression,
            "value": self.value,
            "min_value": self.min_value,
            "max_value": self.max_value,
            "severity": self.severity,
            "reason": self.reason
        }


@dataclass
class CausalModel:
    """Represents causal relationships between variables"""
    variables: Dict[str, Any]
    edges: List[Tuple[str, str, Dict[str, Any]]]  # (source, target, properties)
    constraints: List[Constraint]
    
    def get_predecessors(self, var: str) -> List[str]:
        """Get variables that cause this variable"""
        return [src for src, tgt, _ in self.edges if tgt == var]
    
    def get_successors(self, var: str) -> List[str]:
        """Get variables caused by this variable"""
        return [tgt for src, tgt, _ in self.edges if src == var]


class CausalReasoner:
    """
    Causal reasoning engine for design problems.
    
    Capabilities:
    - Causal inference (forward/backward/counterfactual)
    - Failure mode propagation
    - Constraint satisfaction checking
    - Design alternative exploration
    - Hypothesis testing
    """
    
    def __init__(self):
        self.causal_models: Dict[str, CausalModel] = {}
        self.constraints: List[Constraint] = []
        self.logger = logger
        self.inference_cache: Dict[str, Any] = {}
        
    def register_causal_model(
        self,
        model_name: str,
        variables: Dict[str, Any],
        edges: List[Tuple[str, str, Dict[str, Any]]],
        constraints: Optional[List[Constraint]] = None
    ):
        """Register a causal model for a domain"""
        model = CausalModel(
            variables=variables,
            edges=edges,
            constraints=constraints or []
        )
        self.causal_models[model_name] = model
        self.logger.info(f"Causal model registered: {model_name}")
    
    def add_constraint(self, constraint: Constraint):
        """Add a design constraint"""
        self.constraints.append(constraint)
        self.logger.info(f"Constraint added: {constraint.name}")
    
    def forward_inference(
        self,
        model_name: str,
        initial_values: Dict[str, float]
    ) -> Dict[str, float]:
        """
        Forward causal inference: Given initial values, propagate effects.
        Answers: "If we change X, what happens to Y?"
        """
        if model_name not in self.causal_models:
            return {}
        
        model = self.causal_models[model_name]
        results = initial_values.copy()
        
        # Topological sort and propagate
        computed = set(initial_values.keys())
        queue = deque(initial_values.keys())
        
        while queue:
            current = queue.popleft()
            successors = model.get_successors(current)
            
            for successor in successors:
                # Find edge properties
                edge_props = None
                for src, tgt, props in model.edges:
                    if src == current and tgt == successor:
                        edge_props = props
                        break
                
                # Simple propagation rule (would be more sophisticated)
                if edge_props and successor not in computed:
                    if "multiplier" in edge_props:
                        results[successor] = results[current] * edge_props["multiplier"]
                    else:
                        results[successor] = results[current]
                    computed.add(successor)
                    queue.append(successor)
        
        self.logger.info(f"Forward inference on {model_name}: {len(results)} variables computed")
        return results
    
    def backward_inference(
        self,
        model_name: str,
        target_variable: str,
        target_value: float
    ) -> Dict[str, float]:
        """
        Backward causal inference: Given a desired outcome, what inputs are needed?
        Answers: "To achieve X, we need Y to be..."
        """
        if model_name not in self.causal_models:
            return {}
        
        model = self.causal_models[model_name]
        results = {target_variable: target_value}
        
        # Recursive backward propagation
        queue = deque([target_variable])
        computed = {target_variable}
        
        while queue:
            current = queue.popleft()
            predecessors = model.get_predecessors(current)
            
            for predecessor in predecessors:
                if predecessor not in computed:
                    # Find edge properties
                    edge_props = None
                    for src, tgt, props in model.edges:
                        if src == predecessor and tgt == current:
                            edge_props = props
                            break
                    
                    # Inverse propagation
                    if edge_props and "multiplier" in edge_props:
                        required_value = results[current] / edge_props["multiplier"]
                    else:
                        required_value = results[current]
                    
                    results[predecessor] = required_value
                    computed.add(predecessor)
                    queue.append(predecessor)
        
        self.logger.info(f"Backward inference on {model_name}: {len(results)} variables required")
        return results
    
    def counterfactual_reasoning(
        self,
        model_name: str,
        base_values: Dict[str, float],
        intervention: Dict[str, float]
    ) -> Tuple[Dict[str, float], Dict[str, float]]:
        """
        Counterfactual reasoning: "What if we changed X?"
        Returns (original_outcome, counterfactual_outcome)
        """
        if model_name not in self.causal_models:
            return {}, {}
        
        # Original outcome
        original = self.forward_inference(model_name, base_values)
        
        # Counterfactual: apply intervention
        counterfactual_values = base_values.copy()
        counterfactual_values.update(intervention)
        counterfactual = self.forward_inference(model_name, counterfactual_values)
        
        # Compute differences
        differences = {
            var: counterfactual.get(var, 0) - original.get(var, 0)
            for var in set(original.keys()) | set(counterfactual.keys())
        }
        
        self.logger.info(f"Counterfactual analysis on {model_name}: {len(differences)} variables affected")
        
        return original, counterfactual
    
    def failure_mode_propagation(
        self,
        model_name: str,
        initial_failure: Dict[str, str]
    ) -> Dict[str, List[str]]:
        """
        Propagate failure modes through causal graph.
        Answers: "If X fails, what else fails?"
        """
        if model_name not in self.causal_models:
            return {}
        
        model = self.causal_models[model_name]
        failures: Dict[str, List[str]] = defaultdict(list)
        
        for var, mode in initial_failure.items():
            failures[var].append(mode)
            queue = deque([var])
            visited = {var}
            
            while queue:
                current = queue.popleft()
                successors = model.get_successors(current)
                
                for successor in successors:
                    if successor not in visited:
                        # Assume failure propagates
                        failures[successor].append(f"propagated_from_{current}")
                        queue.append(successor)
                        visited.add(successor)
        
        self.logger.info(f"Failure propagation: {len(failures)} variables affected")
        return dict(failures)
    
    def find_critical_paths(
        self,
        model_name: str,
        start_var: str,
        end_var: str
    ) -> List[List[str]]:
        """
        Find critical causal paths from start to end variable.
        """
        if model_name not in self.causal_models:
            return []
        
        model = self.causal_models[model_name]
        paths = []
        
        def dfs(current: str, path: List[str], visited: Set[str]):
            if current == end_var:
                paths.append(path)
                return
            
            if current in visited:
                return
            
            visited.add(current)
            successors = model.get_successors(current)
            
            for successor in successors:
                dfs(successor, path + [successor], visited.copy())
        
        dfs(start_var, [start_var], set())
        return paths
    
    def check_constraint_satisfaction(
        self,
        model_name: str,
        values: Dict[str, float]
    ) -> Tuple[bool, List[str]]:
        """
        Check if all constraints are satisfied.
        Returns (all_satisfied, violations)
        """
        violations = []
        
        model = self.causal_models.get(model_name)
        constraints = model.constraints + self.constraints if model else self.constraints
        
        for constraint in constraints:
            # Try to evaluate constraint
            try:
                # Simple variable substitution
                expr = constraint.expression
                for var, val in values.items():
                    expr = expr.replace(var, str(val))
                
                # Check constraint
                if not constraint.is_satisfied(values.get(constraint.name, float('inf'))):
                    violations.append(
                        f"{constraint.name}: {constraint.reason} "
                        f"(severity: {constraint.severity})"
                    )
            except Exception as e:
                self.logger.warning(f"Could not evaluate constraint {constraint.name}: {e}")
        
        return len(violations) == 0, violations
    
    def hypothesis_test(
        self,
        model_name: str,
        hypothesis: str,
        test_values: Dict[str, float],
        expected_outcome: str
    ) -> Dict[str, Any]:
        """
        Test a design hypothesis.
        Answers: "If we believe X, does the evidence support it?"
        """
        # Run inference with test values
        results = self.forward_inference(model_name, test_values)
        
        # Check constraints
        satisfied, violations = self.check_constraint_satisfaction(model_name, results)
        
        return {
            "hypothesis": hypothesis,
            "test_values": test_values,
            "results": results,
            "expected_outcome": expected_outcome,
            "constraints_satisfied": satisfied,
            "violations": violations,
            "verdict": "PASS" if satisfied else "FAIL"
        }
    
    def explain_outcome(
        self,
        model_name: str,
        variable: str,
        outcome: float
    ) -> str:
        """
        Generate explanation for why a variable has a certain outcome.
        Uses causal paths and constraints.
        """
        if model_name not in self.causal_models:
            return "Model not found"
        
        model = self.causal_models[model_name]
        predecessors = model.get_predecessors(variable)
        
        explanation = f"Variable '{variable}' = {outcome} because:\n"
        explanation += f"  - It is caused by: {predecessors}\n"
        
        # Add constraint reasoning
        for constraint in model.constraints:
            if constraint.name == variable:
                explanation += f"  - Constraint: {constraint.reason}\n"
        
        return explanation
    
    def export_model(self, model_name: str, filepath: str):
        """Export causal model to JSON"""
        if model_name not in self.causal_models:
            return
        
        model = self.causal_models[model_name]
        export_data = {
            "model_name": model_name,
            "variables": model.variables,
            "edges": [(s, t, p) for s, t, p in model.edges],
            "constraints": [c.to_dict() for c in model.constraints]
        }
        
        with open(filepath, 'w') as f:
            json.dump(export_data, f, indent=2)
        
        self.logger.info(f"Causal model exported to {filepath}")
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get statistics about causal models"""
        stats = {}
        
        for model_name, model in self.causal_models.items():
            stats[model_name] = {
                "variables": len(model.variables),
                "edges": len(model.edges),
                "constraints": len(model.constraints)
            }
        
        return {
            "total_models": len(self.causal_models),
            "total_constraints": len(self.constraints),
            "models": stats
        }
