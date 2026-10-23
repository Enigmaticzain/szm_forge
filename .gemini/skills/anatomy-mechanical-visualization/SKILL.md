---
name: Anatomy Mechanical Visualization
description: Apply Anatomy isometric drawing, projection, shading, and geometry patterns to SZM Forge mechanical components.
---

# Anatomy Mechanical Visualization

## When to use
When a SZM Forge component requires an interactive isometric mechanical drawing (compound gear train, shaft, assembly, or mechanism viewer) rather than a static wireframe. Use the existing viewer in `new ui/src/components/mechanical/IsometricGearViewer.tsx` and the model in `gearTrainModel.ts`.

## Load principles (reference only)
Reference assets (not absolute imports in production):
- `/home/szm7226/SZM/anatomy-test/model.mjs` — mechanical model (5 gears, 3 meshes, 6:1, same direction)
- `new ui/src/components/mechanical/isoEngine.ts` — isometric projection, depth, four-tone shading (`PALETTE`, `createCamera`)
- `new ui/src/components/mechanical/gearTrainModel.ts` — pure numerical model; `computeReduction`, `shaftAngles`, `shaftRPMs`, `gearPositions`
- Skill source (`/home/szm7226/.config/goose/skills/anatomy/`) — `SKILL.md`, `references/craft.md`, `kit/lathe.mjs`, `kit/tube.mjs`

License: MIT (skill copy includes MIT terms). Treat standalone files as reference; keep repo-local files self-contained.

## Apply to SZM Forge components
1. Use `IsometricGearViewer` inside `MechanicalWorkspace` (already integrated; see diff in workspace file).
2. Pass an optional `model` prop to override `DEFAULT_GEAR_TRAIN` (e.g. for other compound ratios).
3. For custom parts, extend `gearTrainModel.ts` with new `GearDef` / `MeshDef` entries; keep module consistent (pitch radius = 2*radius/teeth) to match `isoEngine` projection scale.
4. Keep SVG isometric, no WebGL dependency for basic viewer; use `PALETTE` four-tone shading and `depthOf` for painter's order.

## Run relevant checks
- Focused mechanical test: `npm test -- --run src/components/MechanicalWorkspace.test.tsx`
- Production build: `npm run build`
- Audit geometry manually (reference `kit/audit.mjs` from skill): verify mesh ratios match shaft distances; no overlapping solids; depth ordering preserved.
- Line ends check (reference `scripts/lines.mjs`): verify texture lines end on edges/strokes/dots.

## Integration notes
The skill is registered via repository-local `.claude/skills/anatomy-mechanical-visualization/skill.md` and `.gemini/skills/anatomy-mechanical-visualization/SKILL.md`. Antigravity should load it from these workspace paths; if not detected, verify by opening `.claude/skills/` in the Antigravity workspace settings and confirming the folder is indexed.
