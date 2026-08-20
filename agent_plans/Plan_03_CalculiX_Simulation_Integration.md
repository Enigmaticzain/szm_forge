# Implementation Plan 3: CalculiX & OpenFOAM Simulation Integration

## Objective
SZM Forge's current physics engine computes stress and temperature using simplified, hardcoded formulas (`stress = force/area`). To be a true CAE tool (like Ansys), we must run actual FEA (Finite Element Analysis) and CFD (Computational Fluid Dynamics). This plan delegates the integration of open-source solvers like CalculiX or OpenFOAM.

## Proposed Changes

### 1. Extend the Python Microservice (`ai_service/external_engine_bridge.py`)
- **Action:** Implement `/simulation/fea/run` which takes a mesh, material properties, and boundary conditions (forces/fixed faces).
- **Action:** The endpoint should write an `.inp` (CalculiX input) file, spawn a subprocess `ccx_2.17 -i job`, parse the `.dat` or `.frd` output file, and extract maximum stress/displacement.
- **Action:** Return the stress distribution array as JSON.

### 2. Connect C++ Simulation Engine (`src/Simulation/SimulationEngine.cpp`)
- **Action:** Intercept the `Tick` loop. If a component has complex geometry, freeze the tick and dispatch an async HTTP request to the Python microservice.
- **Action:** When the async HTTP request completes, update the `PhysicsStateComponent` with the highly accurate stress/temperature values.

### 3. Heatmap Visualization
- **Action:** Modify the 3D rendering pipeline to color vertices based on the stress array returned by the solver, creating a true FEA heatmap (blue for low stress, red for high stress).

## Verification Plan
1. Apply a 10,000N force to a cantilever beam entity in the UI.
2. Verify the C++ backend dispatches the job to the Python bridge.
3. Verify CalculiX executes in the background.
4. Verify the 3D UI updates to show a red heatmap at the root of the beam (high stress) and updates the UI panels with the correct Von Mises stress.
