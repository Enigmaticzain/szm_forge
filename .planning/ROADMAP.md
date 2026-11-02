---
project: szm_forge
digital_twin_simulation: true
phases:
  - id: 01-visualization
    name: Interactive 3D Visualization & Isometric Environment
    goal: Operational interactive 3D environment with isometric/perspective camera, component selection, scene graph
    status: planning
    depends_on: []
  - id: 02-physics-integration
    name: Physics & Mechanical Simulation Integration
    goal: Verified physics behavior against C++20 KinematicSolver/SimulationEngine; controls; telemetry
    status: planning
    depends_on: [01-visualization]
  - id: 03-digital-twin-pipeline
    name: Digital-Twin Data Pipeline & API Integration
    goal: Working digital-twin integration using documented simulated source; real-time parameter controls
    status: planning
    depends_on: [01-visualization, 02-physics-integration]
constraints:
  - C++20 backend authoritative
  - Rapier not added without measured gap
  - All telemetry simulated/documented only
  - Preserve 58 unstaged files
  - Performance target 60 FPS (verified, not assumed)
  - No destructive git operations without approval
---
