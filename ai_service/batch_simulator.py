import logging
import concurrent.futures
import time
from typing import Dict, Any, List
import uuid

from simulation_integration import SimulationEngine, SimulationInputs, SolverType
from knowledge_graph import KnowledgeGraph, EntityType, RelationType

logger = logging.getLogger(__name__)

class AutonomousBatchSimulator:
    """
    Headless batch simulation engine.
    Generates variations, runs them concurrently against the C++ backend,
    and feeds failures back into the Knowledge Graph to teach the AI physical limits.
    """
    def __init__(self, simulation_engine: SimulationEngine, knowledge_graph: KnowledgeGraph, max_workers: int = 4):
        self.simulation_engine = simulation_engine
        self.kg = knowledge_graph
        self.max_workers = max_workers
        self.active_batch_status = "Idle"
        self.is_running = False

    def generate_parameter_space(self, num_variations: int = 20) -> List[SimulationInputs]:
        """Generate parametric variations for a given component."""
        variations = []
        base_thickness = 0.01
        base_load = 500.0
        
        # We will generate a mesh grid of thickness and load to map out the failure envelope
        import random
        for i in range(num_variations):
            # Randomize thickness between 1cm and 20cm
            thickness = base_thickness + (random.random() * 0.19)
            # Randomize load between 100kg and 10000kg
            load = 100.0 + (random.random() * 9900.0)
            
            inputs = SimulationInputs(
                component_name=f"BatchVar_{i}",
                geometry_file="",
                material="Steel",
                material_properties={"materialId": "MAT-STEEL-STRUCT"},
                loads={"loadKg": load},
                boundary_conditions={},
                parameters={"thickness": thickness, "variation_index": i}
            )
            variations.append(inputs)
            
        return variations

    def process_feedback_loop(self, inputs: SimulationInputs, max_stress: float, margin_of_safety: float):
        """Learn from the simulation result by injecting it into the Knowledge Graph."""
        thickness = inputs.parameters.get("thickness", 0.0)
        load = inputs.loads.get("loadKg", 0.0)
        
        if margin_of_safety < 1.0:
            # The part failed! This is a valuable data point.
            failure_id = str(uuid.uuid4())
            failure_name = f"Failure: Thk {thickness:.3f}m, Load {load:.0f}kg"
            
            # 1. Create a Failure Mode entity
            self.kg.add_entity(
                name=failure_name,
                entity_type=EntityType.FAILURE_MODE,
                description="Structural failure due to insufficient thickness for applied load",
                properties={"thickness": thickness, "load_kg": load, "stress_mpa": max_stress, "safety_factor": margin_of_safety},
                source="AutonomousBatchSimulator"
            )
            logger.info(f"Learned Failure: {failure_name}")
            
        else:
            # The part succeeded!
            success_name = f"Success: Thk {thickness:.3f}m, Load {load:.0f}kg"
            self.kg.add_entity(
                name=success_name,
                entity_type=EntityType.DESIGN_PRINCIPLE,
                description="Proven safe parametric envelope",
                properties={"thickness": thickness, "load_kg": load, "stress_mpa": max_stress, "safety_factor": margin_of_safety},
                source="AutonomousBatchSimulator"
            )

    def run_headless_batch(self, num_variations: int = 20):
        """Execute the batch process in the background."""
        if self.is_running:
            return
            
        self.is_running = True
        self.active_batch_status = f"Initializing batch space ({num_variations} variations)..."
        
        variations = self.generate_parameter_space(num_variations)
        completed = 0
        failures = 0
        successes = 0
        
        self.active_batch_status = f"Simulated {completed}/{num_variations} variations..."
        
        def run_single_sim(inputs: SimulationInputs):
            # Run against C++ engine
            out = self.simulation_engine.run_structural_analysis(inputs, solver=SolverType.SZM_CPP_ENGINE)
            return inputs, out
            
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.max_workers) as executor:
            future_to_inputs = {executor.submit(run_single_sim, var): var for var in variations}
            
            for future in concurrent.futures.as_completed(future_to_inputs):
                inputs = future_to_inputs[future]
                try:
                    var_inputs, out = future.result()
                    
                    if out.success:
                        self.process_feedback_loop(var_inputs, out.max_stress, out.min_safety_factor)
                        if out.min_safety_factor < 1.0:
                            failures += 1
                        else:
                            successes += 1
                    else:
                        logger.warning(f"Variation {var_inputs.component_name} failed solver: {out.error_message}")
                        
                except Exception as exc:
                    logger.error(f"Variation generated an exception: {exc}")
                    
                completed += 1
                self.active_batch_status = f"Simulated {completed}/{num_variations} (Success: {successes}, Failures: {failures})"

        # Export the learned graph
        self.kg.export_to_json("/tmp/szm_learned_graph.json")
        self.active_batch_status = "Idle"
        self.is_running = False
        logger.info(f"Batch completed. Exported Knowledge Graph to /tmp/szm_learned_graph.json")
