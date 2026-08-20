# Implementation Plan 2: OpenCASCADE / CAD Geometry Integration

## Objective
SZM Forge currently relies on placeholder geometry logic (basic area/thickness). To compete with SolidWorks and Fusion 360, we must integrate a true Boundary Representation (BRep) kernel. This plan delegates the integration of OpenCASCADE (via PythonOCC or CadQuery) to handle real 3D geometry and boolean operations.

## Proposed Changes

### 1. Update the Python Microservice (`ai_service/external_engine_bridge.py`)
- **Action:** Install `cadquery` or `pythonocc-core` in the `ai_service` environment.
- **Action:** Implement the `/cad/import` endpoint to load STEP/IGES files.
- **Action:** Implement a `/cad/boolean` endpoint to perform union/difference on two shapes.
- **Action:** Have these endpoints extract the tessellated mesh data (vertices/normals/indices) from the BRep shape and return it as JSON.

### 2. Connect the C++ Backend (`src/CAD/CADIntegration.cpp`)
- **Action:** Create a new module that sends HTTP POST requests to `http://127.0.0.1:8003/cad/import`.
- **Action:** Parse the returned JSON mesh data.
- **Action:** Populate the ECS `MeshComponent` of an entity with the real geometry vertices.

### 3. Update Graphics Rendering
- **Action:** Ensure `Viewport.cpp` (or the React 3D viewer) renders the dynamic vertices fetched from the CAD microservice instead of hardcoded cubes.

## Verification Plan
1. Start the Python microservice.
2. Send a request from the SZM Forge UI to "Import STEP file".
3. Verify that the C++ backend calls the Python bridge, receives the tessellated mesh, and renders the complex CAD part accurately in the 3D viewport.
