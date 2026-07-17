import time
import logging
from typing import Dict, Any

logger = logging.getLogger(__name__)

class AIBenchmarker:
    """Measures the performance and speed of the AI reasoning loop."""
    
    @staticmethod
    def run_benchmark(training_loop, num_runs: int = 3) -> Dict[str, Any]:
        logger.info(f"Starting AI Benchmark ({num_runs} runs)...")
        
        total_time = 0.0
        successes = 0
        
        for i in range(num_runs):
            start = time.time()
            # Run a dummy episode to measure speed
            episode = training_loop.run_episode(problem="Benchmark structural load case")
            duration = time.time() - start
            
            total_time += duration
            if episode.success:
                successes += 1
                
        avg_time = total_time / num_runs
        score = (successes / num_runs) * 1000.0 / max(avg_time, 0.1)
        
        return {
            "runs": num_runs,
            "success_rate": successes / num_runs,
            "average_time_seconds": avg_time,
            "benchmark_score": score
        }
