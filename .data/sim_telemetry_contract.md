# Simulated Telemetry Contract — SZM Forge Digital Twin (Phase 03)

**Status:** SIMULATED / DOCUMENTED — NOT REAL SENSOR READINGS
**Source:** Constructed dataset based on SimulationEngine output variables (temperature, stress, stressRatio, load, health derived from yield ratio)
**Purpose:** Provide clean interface for future real-hardware integration

## Fields (all simulated)

| Field | Type | Unit / Range | Source / Derivation | Label |
|---|---|---|---|---|
| timestamp | string | ISO-8601 | Simulated sequence | SIMULATED |
| machine_id | string | ID | Contract reference | SIMULATED |
| temperature | float | K (Kelvin) | SimulationEngine tick thermal formula | SIMULATED |
| stress | float | Pa | SimulationEngine: appliedForce / area | SIMULATED |
| stressRatio | float | [0,1] | min(1, stress / yieldStrength) | SIMULATED |
| load_percent | float | % | Derived from stressRatio | SIMULATED |
| health_percent | float | % | Derived from stressRatio (inverse) | SIMULATED |

## Data origin

- No physical sensor is connected.
- All values derive from existing C++ SimulationEngine formulas or documented synthetic inputs.
- For real hardware integration: replace this file with sensor bridge, then update contract.

## Limitations

- No calibrated physical measurements.
- No real-world noise model included (can be added later).
- Performance target (60 FPS) applies to visualization, not simulation accuracy claim.
