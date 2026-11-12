# SZM Forge Design Workflow

The workflow is SZM Forge's native implementation of a prompt-driven hardware-design path, inspired by the public Zoo homepage's Design, Analyze, and Manufacture product flow. No Zoo source code or assets are used.

## API

The AI service exposes:

- `POST /api/workflow/design-to-manufacture`
- `GET /api/workflow/design/{design_id}/step`

Example request:

```json
{
  "prompt": "Aluminum box 100 mm long, 50 mm wide, 20 mm tall",
  "include_toolpath_preview": false
}
```

The response contains the generated primitive and preview geometry, nominal primitive volume and bounding box, analysis status, manufacturing review status, and an optional STEP download URL. The workflow ID is a UUID; generated STEP artifacts are served only through that ID and are stored under the operating system's temporary directory.

The active Text-to-CAD panel calls the AI service at `http://127.0.0.1:8001` by default. Override this with the frontend build variable `VITE_AI_WORKFLOW_URL` when the service runs elsewhere.

## Current scope and limitations

- Prompt dimensions are inferred, not confirmed measurements. The current CAD parser supports box, cylinder, sphere, cone, and torus primitives.
- The analysis reports ideal-primitive dimensions and volume only. Structural FEA is not run; it requires reviewed loads, restraints, material properties, and a validated mesh.
- DFM is reported as `not_assessed`; the workflow does not claim geometry-aware manufacturability or machine collision clearance.
- Toolpath preview is opt-in and limited to box primitives with X/Y dimensions up to 500 mm and depth up to 100 mm. It uses a fixed 10 mm tool and produces an illustrative rectangular 2.5D profile only.
- Generated G-code is not production-ready and must not be sent directly to a machine. Verify the CAD, stock, tooling, fixtures, post-processor, and machine setup independently.
- STEP export is available only when a local CAD backend successfully creates a file; the parametric preview remains available without it.
