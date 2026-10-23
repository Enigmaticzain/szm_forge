# SZM Forge External Integration Guide

This document outlines the workflow for integrating third-party open-source engines (e.g., FreeCAD, OpenCASCADE, CalculiX, OpenFOAM, USD) into SZM Forge.

## Architectural Approach

SZM Forge follows an **Integration-First Architecture**. Instead of reinventing complex CAD operations or physics solvers, we utilize existing robust open-source solutions.

The integration happens at two levels:
1. **C++ Native Integration (`src/Core/ExternalEngineManager.hpp`)**: For performance-critical engines or libraries with direct C++ APIs (e.g., OpenCASCADE, USD).
2. **Python Microservice Integration (`ai_service/external_engine_bridge.py`)**: For tools that are easier to interact with via Python (e.g., FreeCAD headless, PythonOCC, OpenFOAM CLI wrappers).

## How to Add a New C++ Engine
1. **Add Dependency**: Use CMake's `FetchContent` or Git submodules in the `external/` directory to pull the dependency.
2. **Create Wrapper**: Inherit from `SZM::Core::Integration::IExternalEngine` (or `ICADEngine` / `ISimulationEngine`).
3. **Register**: Register your new engine class in `Application.cpp` using `ExternalEngineManager::GetInstance().RegisterEngine(...)`.
4. **Expose**: Ensure `FastAPIBridge` can route frontend requests to your engine.

## How to Add a New Python-Based Engine
1. **Install Package**: Add the required pip package (e.g., `cadquery`, `pythonocc-core`) to `ai_service/requirements.txt`.
2. **Add Endpoint**: Open `ai_service/external_engine_bridge.py` and create a new FastAPI endpoint (e.g., `/simulation/openfoam/run`).
3. **Call from C++**: If the C++ core needs to trigger it, use HTTP requests to `127.0.0.1:8003`.

## Philosophy
- **Never Write Complex Solvers From Scratch**: Always search GitHub for an existing C++ or Python solver.
- **Maintain UI Responsiveness**: External engine calls can take minutes to hours (e.g., CFD simulation). Always run them asynchronously and report progress back via the `FastAPIBridge`.
