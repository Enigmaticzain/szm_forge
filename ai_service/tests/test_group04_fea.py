from simulation_integration import SimulationEngine, SimulationInputs, SimulationType, SolverType


def test_simulation_engine_init():
    engine = SimulationEngine()
    assert engine.available_solvers


def test_structural_fallback():
    engine = SimulationEngine()
    inputs = SimulationInputs(
        component_name="beam",
        geometry_file="",
        material="steel",
        material_properties={"youngs_modulus": 210e9, "yield_strength": 300e6},
        loads={"force": 1000, "area": 0.01},
        boundary_conditions={},
    )
    out = engine.run_structural_with_fallback(inputs)
    assert out.max_stress > 0
