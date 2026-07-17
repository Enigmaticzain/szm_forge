import logging
import random
from typing import Dict, Any

from benchmarking import AIBenchmarker

logger = logging.getLogger(__name__)

class EvolutionSandbox:
    """Isolated environment where the AI mutates its own hyperparameters and benchmarks the result."""
    
    def __init__(self, training_loop):
        self.training_loop = training_loop
        self.current_params = {
            "learning_rate": 0.010,
            "max_iterations": 5,
            "confidence_threshold": 0.85
        }
        self.best_score = 0.0

    def mutate_parameters(self) -> Dict[str, Any]:
        """Randomly mutate a hyperparameter, benchmark, and adopt if successful."""
        logger.info("Entering Evolution Sandbox...")
        
        # Baseline Benchmark if first time
        if self.best_score == 0.0:
            base_result = AIBenchmarker.run_benchmark(self.training_loop, num_runs=1)
            self.best_score = base_result["benchmark_score"]
            logger.info(f"Established baseline score: {self.best_score:.2f}")

        # Store backup
        backup_params = self.current_params.copy()
        
        # Mutate one parameter
        mutation_key = random.choice(list(self.current_params.keys()))
        mutation_factor = random.choice([0.8, 0.9, 1.1, 1.2]) # +/- 20%
        
        # Apply mutation
        if isinstance(self.current_params[mutation_key], int):
            self.current_params[mutation_key] = max(1, int(self.current_params[mutation_key] * mutation_factor))
        else:
            self.current_params[mutation_key] = self.current_params[mutation_key] * mutation_factor
            
        logger.info(f"Testing mutation: {mutation_key} -> {self.current_params[mutation_key]}")
        
        # Benchmark new parameters
        new_result = AIBenchmarker.run_benchmark(self.training_loop, num_runs=2)
        new_score = new_result["benchmark_score"]
        
        adopted = False
        if new_score > self.best_score:
            logger.info(f"Sandbox test PASSED! Score improved {self.best_score:.2f} -> {new_score:.2f}")
            self.best_score = new_score
            adopted = True
        else:
            logger.info(f"Sandbox test FAILED. Rolling back {mutation_key}.")
            self.current_params = backup_params
            
        return {
            "mutation_tested": mutation_key,
            "new_value": self.current_params[mutation_key] if adopted else backup_params[mutation_key],
            "adopted": adopted,
            "score": new_score,
            "current_params": self.current_params
        }
