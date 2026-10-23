/**
 * Gear-train mechanical model for isometric assembly viewer.
 *
 * Pure numerical model — zero DOM or rendering dependencies.
 * Drives both the visual representation and the live readout.
 *
 * Reference: /home/szm7226/SZM/anatomy-test/model.mjs
 * Adapted for SZM Forge's Mechanical Workspace.
 */

// ── Types ──────────────────────────────────────────────────────

export interface GearDef {
  /** Label, e.g. "G1" */
  label: string;
  /** Number of teeth */
  teeth: number;
  /** Shaft index (gears on the same shaft are rigidly coupled) */
  shaft: number;
  /** Pitch circle radius in model units */
  radius: number;
  /** Whether this gear is highlighted (accent colour) */
  highlight: boolean;
  /** Vertical mounting tier along the shaft (0 = lowest stage, 1 = second stage, etc.) */
  tier?: number;
}

export interface MeshDef {
  /** Index of the driving gear in `gears` */
  from: number;
  /** Index of the driven gear in `gears` */
  to: number;
}

export interface GearTrainModel {
  gears: GearDef[];
  meshes: MeshDef[];
  /** Overall speed reduction ratio (driven/driver) */
  reduction: number;
  /** Human-readable direction relationship */
  directionNote: string;
}

export interface ShaftState {
  /** Current angle in radians */
  angle: number;
  /** Angular velocity in rad/s */
  omega: number;
}

// ── Default compound gear train (5 gears, 3 shafts, 6:1 reduction) ─

export const DEFAULT_GEAR_TRAIN: GearTrainModel = {
  gears: [
    { label: 'G1', teeth: 20, shaft: 0, radius: 40, highlight: false, tier: 0 },
    { label: 'G2', teeth: 40, shaft: 1, radius: 80, highlight: true, tier: 0 },
    { label: 'G3', teeth: 16, shaft: 1, radius: 32, highlight: false, tier: 1 },
    { label: 'G4', teeth: 48, shaft: 2, radius: 96, highlight: false, tier: 1 },
    { label: 'G5', teeth: 16, shaft: 2, radius: 32, highlight: false, tier: 2 },
  ],
  meshes: [
    { from: 0, to: 1 },  // G1 → G2  (ratio 2:1, Tier 0)
    { from: 2, to: 3 },  // G3 → G4  (ratio 3:1, Tier 1) — G2 & G3 share shaft 1
  ],
  reduction: 6,
  directionNote: 'Same direction (two external meshes)',
};

// ── Pure helpers ───────────────────────────────────────────────

/** Gear ratio between two meshing gears (driven teeth / driver teeth). */
export function gearRatio(driverTeeth: number, drivenTeeth: number): number {
  if (driverTeeth <= 0) return 0;
  return drivenTeeth / driverTeeth;
}

/** Compute the overall reduction from a gear-train model. */
export function computeReduction(model: GearTrainModel): number {
  let ratio = 1;
  for (const mesh of model.meshes) {
    const driver = model.gears[mesh.from];
    const driven = model.gears[mesh.to];
    ratio *= gearRatio(driver.teeth, driven.teeth);
  }
  return ratio;
}

/**
 * Given the input shaft angle, compute the angle for every shaft.
 * Shafts are numbered from 0. Shaft 0 is the input.
 * External meshes reverse direction at each stage.
 */
export function shaftAngles(model: GearTrainModel, inputAngle: number): number[] {
  const shaftCount = Math.max(...model.gears.map(g => g.shaft)) + 1;
  const angles = new Array<number>(shaftCount).fill(0);
  angles[0] = inputAngle;

  // Walk each mesh and propagate the rotation.
  // Assumes meshes are listed in order (shaft 0→1, 1→2, etc.)
  for (const mesh of model.meshes) {
    const driver = model.gears[mesh.from];
    const driven = model.gears[mesh.to];
    const driverShaft = driver.shaft;
    const drivenShaft = driven.shaft;
    // External mesh: direction reversal at each external mesh stage
    // ω_driven = -ω_driver × (N_driver / N_driven)
    const ratio = driver.teeth / driven.teeth;
    angles[drivenShaft] = -angles[driverShaft] * ratio;
  }

  return angles;
}

/**
 * Compute the angle for a specific gear given shaft angles.
 * Gears on the same shaft share the shaft rotation.
 * Driven gears in external meshes receive half-pitch phase offset to mesh tooth-into-gap.
 */
export function gearAngle(model: GearTrainModel, gearIndex: number, angles: number[]): number {
  const baseAngle = angles[model.gears[gearIndex].shaft];
  for (const mesh of model.meshes) {
    if (mesh.to === gearIndex) {
      return baseAngle + Math.PI / model.gears[gearIndex].teeth;
    }
  }
  return baseAngle;
}

/** RPM readout for each shaft given input RPM. */
export function shaftRPMs(model: GearTrainModel, inputRPM: number): number[] {
  const shaftCount = Math.max(...model.gears.map(g => g.shaft)) + 1;
  const rpms = new Array<number>(shaftCount).fill(0);
  rpms[0] = inputRPM;

  for (const mesh of model.meshes) {
    const driver = model.gears[mesh.from];
    const driven = model.gears[mesh.to];
    rpms[driven.shaft] = rpms[driver.shaft] * (driver.teeth / driven.teeth);
  }
  return rpms;
}

/**
 * Get gear position in 2D layout (top-down view, before isometric projection).
 * Lays gears out left-to-right with meshing distances.
 */
export function gearPositions(model: GearTrainModel): Array<{ x: number; y: number }> {
  const positions: Array<{ x: number; y: number }> = [];
  const shaftX = new Map<number, number>();

  // Place first gear at origin
  shaftX.set(model.gears[0].shaft, 0);
  positions.push({ x: 0, y: 0 });

  for (let i = 1; i < model.gears.length; i++) {
    const gear = model.gears[i];
    if (shaftX.has(gear.shaft)) {
      // Same shaft as a previously placed gear — same X
      positions.push({ x: shaftX.get(gear.shaft)!, y: 0 });
    } else {
      // Find a mesh that connects this gear (or a gear on the same shaft) to a placed shaft
      let placed = false;
      for (const mesh of model.meshes) {
        const fromGear = model.gears[mesh.from];
        const toGear = model.gears[mesh.to];
        if (toGear.shaft === gear.shaft && shaftX.has(fromGear.shaft)) {
          const fromX = shaftX.get(fromGear.shaft)!;
          const dist = fromGear.radius + toGear.radius;
          const newX = fromX + dist;
          shaftX.set(gear.shaft, newX);
          positions.push({ x: newX, y: 0 });
          placed = true;
          break;
        }
        if (fromGear.shaft === gear.shaft && shaftX.has(toGear.shaft)) {
          const toX = shaftX.get(toGear.shaft)!;
          const dist = fromGear.radius + toGear.radius;
          const newX = toX - dist;
          shaftX.set(gear.shaft, newX);
          positions.push({ x: newX, y: 0 });
          placed = true;
          break;
        }
      }
      if (!placed) {
        // Fallback: place next to the last gear
        const lastPos = positions[positions.length - 1];
        const prevGear = model.gears[i - 1];
        const newX = lastPos.x + prevGear.radius + gear.radius + 10;
        shaftX.set(gear.shaft, newX);
        positions.push({ x: newX, y: 0 });
      }
    }
  }

  return positions;
}

/** Tooth profile for rendering. Returns array of SVG polygon points. */
export function toothProfile(teeth: number, radius: number): string {
  const pts: string[] = [];
  const toothAngle = (2 * Math.PI) / teeth;
  const outerR = radius;
  const innerR = radius * 0.82;
  const tipWidth = 0.35;

  for (let i = 0; i < teeth; i++) {
    const base = i * toothAngle;
    // Root left
    pts.push(polarPoint(innerR, base));
    // Tip left
    pts.push(polarPoint(outerR, base + toothAngle * tipWidth));
    // Tip right
    pts.push(polarPoint(outerR, base + toothAngle * (1 - tipWidth)));
    // Root right
    pts.push(polarPoint(innerR, base + toothAngle));
  }

  return pts.join(' ');
}

function polarPoint(r: number, angle: number): string {
  return `${(r * Math.cos(angle)).toFixed(2)},${(r * Math.sin(angle)).toFixed(2)}`;
}
