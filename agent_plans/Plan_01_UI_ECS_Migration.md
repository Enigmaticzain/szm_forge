# Implementation Plan: 01 UI ECS Migration

## Objective
Migrate all UI panels and APIs to the new ECS scene graph.

## Proposed Changes
1. **Research & Dependencies**: Identify the required open-source libraries (C++ or Python).
2. **Integration Layer**: Build the bridge via `ExternalEngineManager` or the Python `external_engine_bridge.py`.
3. **UI/UX Updates**: Update the React and ImGui frontends to expose the new functionality.

## Verification Plan
1. Compile / Run the server.
2. Execute the new workflow in the UI.
3. Validate output correctness (e.g., generated files, correct rendering, simulation accuracy).
