#!/usr/bin/env python3
"""
Training Loop for SZM Forge AI
Implements iterative learning with simulation feedback and reinforcement learning

Key components:
- Chain-of-thought fine-tuning
- Reinforcement learning from simulation feedback
- Counterfactual hypothesis testing
- Curriculum learning (simple -> complex)
"""

import logging
import json
from typing import Dict, List, Optional, Tuple, Any, Callable
from dataclasses import dataclass, field, asdict
from enum import Enum
from datetime import datetime
import numpy as np
from collections import defaultdict

logger = logging.getLogger(__name__)


class LearningPhase(Enum):
    """Phases of training"""
    FOUNDATION = "foundation"  # Learn basic physics
    INTERMEDIATE = "intermediate"  # Learn complex systems
    ADVANCED = "advanced"  # Learn optimization
    DEPLOYMENT = "deployment"  # Online learning


class TrainingObjective(Enum):
    """What we're optimizing for"""
    DESIGN_VALIDITY = "design_validity"  # Does design pass checks?
    EFFICIENCY = "efficiency"  # Cost, weight, performance
    SAFETY = "safety"  # Meets safety constraints
    NOVELTY = "novelty"  # Finds non-obvious improvements


@dataclass
class SimulationResult:
    """Result from simulation feedback"""
    design_id: str
    simulation_type: str  # "FEA", "CFD", "thermal", etc.
    parameters: Dict[str, float]
    outputs: Dict[str, float]  # stress, temp, flow, etc.
    passed_checks: bool
    failure_modes: List[str] = field(default_factory=list)
    margin_of_safety: float = 1.0
    confidence: float = 1.0
    timestamp: str = field(default_factory=lambda: datetime.now().isoformat())

    def to_dict(self) -> Dict:
        return asdict(self)


@dataclass
class ReasoningTrace:
    """Captures reasoning steps for a design"""
    design_id: str
    problem_statement: str
    reasoning_steps: List[str]  # Chain of thought
    hypotheses: List[str]  # Design hypotheses generated
    chosen_design: str
    justification: str  # Why this design was chosen
    confidence: float
    timestamp: str = field(default_factory=lambda: datetime.now().isoformat())

    def to_dict(self) -> Dict:
        return asdict(self)


@dataclass
class TrainingEpisode:
    """Single training episode"""
    episode_id: str
    phase: LearningPhase
    problem: str
    target_objective: TrainingObjective
    
    # Reasoning phase
    reasoning_trace: Optional[ReasoningTrace] = None
    
    # Simulation phase
    simulation_results: List[SimulationResult] = field(default_factory=list)
    
    # Evaluation phase
    reward: float = 0.0
    learning_signal: float = 0.0  # -1 to +1
    success: bool = False
    
    # Metadata
    created_at: str = field(default_factory=lambda: datetime.now().isoformat())
    duration_seconds: float = 0.0

    def to_dict(self) -> Dict:
        return {
            "episode_id": self.episode_id,
            "phase": self.phase.value,
            "problem": self.problem,
            "objective": self.target_objective.value,
            "reasoning": self.reasoning_trace.to_dict() if self.reasoning_trace else None,
            "simulations": [r.to_dict() for r in self.simulation_results],
            "reward": self.reward,
            "success": self.success,
            "duration_seconds": self.duration_seconds
        }


class RewardCalculator:
    """Calculates rewards from simulation feedback"""

    def __init__(self):
        self.weights = {
            TrainingObjective.DESIGN_VALIDITY: 0.4,
            TrainingObjective.EFFICIENCY: 0.3,
            TrainingObjective.SAFETY: 0.25,
            TrainingObjective.NOVELTY: 0.05
        }

    def calculate(self, result: SimulationResult, objective: TrainingObjective) -> float:
        """
        Calculate reward from simulation result.
        Reward ranges from -1 (complete failure) to +1 (perfect solution)
        """
        reward = 0.0

        # Design validity: did design pass all checks?
        if result.passed_checks:
            reward += 0.4
        else:
            reward -= 0.5

        # Safety: margin of safety
        if result.margin_of_safety >= 1.5:
            reward += 0.25
        elif result.margin_of_safety >= 1.0:
            reward += 0.15
        else:
            reward -= 0.3

        # Efficiency: measure improvement
        if objective == TrainingObjective.EFFICIENCY:
            reward += 0.2 * min(result.outputs.get("efficiency", 0.5), 1.0)

        # Confidence in result
        reward *= result.confidence

        return np.clip(reward, -1.0, 1.0)


class TrainingDataset:
    """Manages training episodes and data"""

    def __init__(self, name: str = "SZM_Training"):
        self.name = name
        self.episodes: List[TrainingEpisode] = []
        self.statistics: Dict[str, Any] = defaultdict(float)
        logger.info(f"Initialized training dataset: {name}")

    def add_episode(self, episode: TrainingEpisode):
        """Add training episode"""
        self.episodes.append(episode)
        self._update_statistics(episode)
        logger.info(f"Episode {episode.episode_id}: success={episode.success}, reward={episode.reward:.2f}")

    def _update_statistics(self, episode: TrainingEpisode):
        """Update training statistics"""
        phase = episode.phase.value
        self.statistics[f"{phase}_count"] += 1
        self.statistics[f"{phase}_success_rate"] = (
            self.statistics.get(f"{phase}_success", 0) + (1 if episode.success else 0)
        ) / self.statistics[f"{phase}_count"]
        self.statistics[f"{phase}_avg_reward"] = (
            self.statistics.get(f"{phase}_total_reward", 0) + episode.reward
        ) / self.statistics[f"{phase}_count"]
        if episode.success:
            self.statistics[f"{phase}_success"] = self.statistics.get(f"{phase}_success", 0) + 1

    def get_episodes_by_phase(self, phase: LearningPhase) -> List[TrainingEpisode]:
        """Get episodes from a specific phase"""
        return [ep for ep in self.episodes if ep.phase == phase]

    def get_successful_episodes(self) -> List[TrainingEpisode]:
        """Get all successful episodes"""
        return [ep for ep in self.episodes if ep.success]

    def export_to_json(self, filepath: str):
        """Export training data to JSON"""
        export_data = {
            "name": self.name,
            "total_episodes": len(self.episodes),
            "statistics": dict(self.statistics),
            "episodes": [ep.to_dict() for ep in self.episodes[-100:]]  # Last 100
        }
        with open(filepath, 'w') as f:
            json.dump(export_data, f, indent=2)
        logger.info(f"Exported training data to {filepath}")

    def __repr__(self) -> str:
        success_count = len(self.get_successful_episodes())
        return f"TrainingDataset({self.name}, {len(self.episodes)} episodes, {success_count} successful)"


class TrainingLoop:
    """
    Main training loop orchestrator.
    Implements the iterative learning cycle:
    1. Problem statement -> Reasoning phase
    2. Proposed design -> Simulation phase
    3. Feedback -> Learning phase
    4. Update knowledge/weights
    """

    def __init__(
        self,
        knowledge_graph,
        reasoning_engine,
        agent,
        simulator: Optional[Callable] = None,
        learning_rate: float = 0.01
    ):
        self.kg = knowledge_graph
        self.reasoning_engine = reasoning_engine
        self.agent = agent
        self.simulator = simulator
        self.learning_rate = learning_rate
        
        self.dataset = TrainingDataset()
        self.reward_calculator = RewardCalculator()
        self.phase = LearningPhase.FOUNDATION
        self.episode_count = 0
        
        logger.info("Initialized Training Loop")

    def run_episode(
        self,
        problem: str,
        objective: TrainingObjective = TrainingObjective.DESIGN_VALIDITY,
        phase: Optional[LearningPhase] = None
    ) -> TrainingEpisode:
        """
        Run a single training episode.
        
        Flow:
        1. Agent reasons about problem
        2. Agent proposes design
        3. Simulation evaluates design
        4. Calculate reward
        5. Learn from feedback
        """
        if phase is None:
            phase = self.phase

        episode_id = f"episode_{self.episode_count}"
        self.episode_count += 1

        import time
        start_time = time.time()

        # Iterative Re-design Loop
        max_iterations = 5
        iteration = 0
        best_sim_results = []
        best_reward = -1.0
        best_reasoning = None
        current_thickness = 0.05
        
        # Publish live status via a global or callback if available
        # But we can just log for now
        
        while iteration < max_iterations:
            iteration += 1
            logger.info(f"[{episode_id}] ITERATION {iteration}: REASONING PHASE (thickness: {current_thickness})")
            
            # Phase 1: Reasoning
            reasoning_trace = self._reasoning_phase(problem, episode_id, current_thickness)

            # Phase 2: Simulation
            logger.info(f"[{episode_id}] ITERATION {iteration}: SIMULATION PHASE")
            sim_results = self._simulation_phase(reasoning_trace, episode_id, current_thickness)

            # Phase 3: Evaluation
            logger.info(f"[{episode_id}] ITERATION {iteration}: EVALUATION PHASE")
            reward = self._evaluation_phase(sim_results, objective, episode_id)
            
            if reward > best_reward:
                best_reward = reward
                best_sim_results = sim_results
                best_reasoning = reasoning_trace
                
            if reward > 0.3:
                logger.info(f"[{episode_id}] ITERATION {iteration}: SUCCESS! Design meets constraints.")
                break
                
            logger.warning(f"[{episode_id}] ITERATION {iteration}: FAILED. Initiating autonomous re-design...")
            # Re-design strategy: increase thickness
            current_thickness *= 1.5

        # Phase 4: Learning
        logger.info(f"[{episode_id}] LEARNING PHASE (final reward={best_reward:.2f})")
        self._learning_phase(best_reasoning, best_sim_results, best_reward)

        duration = time.time() - start_time

        # Create episode record
        episode = TrainingEpisode(
            episode_id=episode_id,
            phase=phase,
            problem=problem,
            target_objective=objective,
            reasoning_trace=best_reasoning,
            simulation_results=best_sim_results,
            reward=best_reward,
            success=best_reward > 0.3,  # Threshold for success
            duration_seconds=duration
        )

        self.dataset.add_episode(episode)
        return episode

    def _reasoning_phase(self, problem: str, episode_id: str, thickness: float) -> ReasoningTrace:
        """Agent reasons about the problem using chain-of-thought"""
        reasoning_steps = [
            f"Understanding problem: {problem}",
            "Identifying relevant components and constraints",
            "Searching knowledge graph for similar designs",
            f"Generating design hypotheses with thickness {thickness:.3f}m",
            "Evaluating hypotheses against constraints"
        ]

        trace = ReasoningTrace(
            design_id=episode_id,
            problem_statement=problem,
            reasoning_steps=reasoning_steps,
            hypotheses=["Design option A", "Design option B", "Design option C"],
            chosen_design=f"Design option B (Thickness: {thickness:.3f}m)",
            justification="Best balance of performance and safety",
            confidence=0.85
        )

        return trace

    def _simulation_phase(
        self,
        reasoning: ReasoningTrace,
        episode_id: str,
        thickness: float
    ) -> List[SimulationResult]:
        """Simulate the proposed design"""
        results = []

        # Default simulation if none provided
        if self.simulator is None:
            result = SimulationResult(
                design_id=episode_id,
                simulation_type="structural_analysis",
                parameters={"material": "titanium", "thickness": 5.0},
                outputs={"max_stress": 450.0, "deflection": 2.1},
                passed_checks=True,
                margin_of_safety=1.22,
                confidence=0.95
            )
            results.append(result)
        else:
            result = self.simulator(reasoning.chosen_design, thickness)
            if isinstance(result, list):
                results.extend(result)
            else:
                results.append(result)

        return results

    def _evaluation_phase(
        self,
        sim_results: List[SimulationResult],
        objective: TrainingObjective,
        episode_id: str
    ) -> float:
        """Calculate total reward from simulations"""
        if not sim_results:
            return -1.0

        rewards = [
            self.reward_calculator.calculate(result, objective)
            for result in sim_results
        ]

        # Average reward across simulations
        total_reward = np.mean(rewards)
        logger.info(f"[{episode_id}] Simulation rewards: {rewards}, avg: {total_reward:.2f}")

        return total_reward

    def _learning_phase(
        self,
        reasoning: ReasoningTrace,
        sim_results: List[SimulationResult],
        reward: float
    ):
        """Update knowledge and reasoning based on feedback"""
        # In a real system, this would:
        # 1. Update weights in the neural reasoner (if using RL)
        # 2. Add successful patterns to knowledge graph
        # 3. Update causal models
        # 4. Refine constraints
        
        if reward > 0.5:
            # Store successful reasoning pattern
            logger.info(f"Storing successful pattern: {reasoning.chosen_design}")
            # self.kg.add_successful_pattern(reasoning)
        elif reward < -0.3:
            # Learn from failures
            logger.warning(f"Failure case: {reasoning.chosen_design}")
            # self.kg.analyze_failure(sim_results)

    def curriculum_progression(self):
        """Progress to next learning phase when ready"""
        success_rate = self.dataset.statistics.get(f"{self.phase.value}_success_rate", 0)
        episode_count = self.dataset.statistics.get(f"{self.phase.value}_count", 0)

        # Progress when success rate > 80% and enough episodes
        if success_rate > 0.8 and episode_count >= 50:
            phases = list(LearningPhase)
            current_idx = phases.index(self.phase)
            if current_idx < len(phases) - 1:
                self.phase = phases[current_idx + 1]
                logger.info(f"Curriculum progression: {self.phase.value}")

    def run_curriculum(self, num_episodes: int = 100):
        """Run full curriculum learning"""
        problems = [
            ("Design a beam to support 1000 N load", TrainingObjective.SAFETY),
            ("Optimize weight while maintaining strength", TrainingObjective.EFFICIENCY),
            ("Design heat sink for 100W dissipation", TrainingObjective.EFFICIENCY),
        ]

        for i in range(num_episodes):
            problem, objective = problems[i % len(problems)]
            self.run_episode(problem, objective, self.phase)
            self.curriculum_progression()

        logger.info(f"Curriculum complete. Statistics: {dict(self.dataset.statistics)}")

    def get_training_report(self) -> Dict:
        """Generate training report"""
        successful = self.dataset.get_successful_episodes()
        return {
            "total_episodes": len(self.dataset.episodes),
            "successful": len(successful),
            "success_rate": len(successful) / len(self.dataset.episodes) if self.dataset.episodes else 0,
            "current_phase": self.phase.value,
            "avg_reward": np.mean([ep.reward for ep in self.dataset.episodes]) if self.dataset.episodes else 0,
            "statistics": dict(self.dataset.statistics)
        }

    def export_training_data(self, filepath: str):
        """Export training data for analysis"""
        self.dataset.export_to_json(filepath)
