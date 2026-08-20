# Implementation Plan 4: Pixar's USD & Omniverse Interoperability

## Objective
To enable seamless collaboration with NVIDIA Omniverse, Unreal Engine, and Maya, SZM Forge needs to speak the universal language of 3D: Pixar's Universal Scene Description (USD). This plan delegates building an export/import layer that converts our ECS Scene Graph into `.usda` format.

## Proposed Changes

### 1. Install USD Dependencies
- **Action:** (Python approach) Add `usd-core` to `ai_service/requirements.txt`.
- **Action:** Create a `/usd/export` endpoint in `external_engine_bridge.py`.

### 2. Scene Graph Traversal
- **Action:** In C++, write a serialization function that traverses the ECS `Scene` starting from root entities.
- **Action:** Package the hierarchy, `TransformComponent` (Position, Rotation, Scale), and `MaterialComponent` data into a JSON payload.
- **Action:** Send this JSON to the `/usd/export` Python endpoint.

### 3. Generate the USD Stage
- **Action:** In Python, use the `pxr.Usd` and `pxr.UsdGeom` modules to construct a USD Stage.
- **Action:** Create `UsdGeom.Xform` for each entity, maintaining parent-child relations.
- **Action:** Bind MaterialX or `UsdPreviewSurface` materials based on our MaterialComponent properties.
- **Action:** Save the file to disk (e.g., `workspace_export.usda`).

## Verification Plan
1. Build a simple assembly in SZM Forge (e.g., a Box parented to a Cylinder).
2. Trigger the "Export to Omniverse (USD)" action.
3. Open the resulting `.usda` file in NVIDIA Omniverse USD Composer or a text editor and verify the hierarchy and transforms perfectly match the SZM Forge scene.
