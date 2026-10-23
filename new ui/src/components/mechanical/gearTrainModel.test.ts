import { describe, it, expect } from 'vitest';
import {
  DEFAULT_GEAR_TRAIN,
  gearRatio,
  computeReduction,
  shaftAngles,
  shaftRPMs,
  gearPositions,
  gearAngle,
  toothProfile,
} from './gearTrainModel';
import { getGearZ } from './IsometricGearViewer';

describe('gearTrainModel kinematic calculations', () => {
  it('calculates correct reduction ratio for default 5-gear train', () => {
    // 20T -> 40T is 2:1, 16T -> 48T is 3:1. Total = 6:1
    const reduction = computeReduction(DEFAULT_GEAR_TRAIN);
    expect(reduction).toBe(6);
    expect(DEFAULT_GEAR_TRAIN.reduction).toBe(6);
  });

  it('calculates individual gear ratios correctly', () => {
    expect(gearRatio(20, 40)).toBe(2);
    expect(gearRatio(16, 48)).toBe(3);
    expect(gearRatio(0, 40)).toBe(0);
  });

  it('propagates angular rotation across shafts with true external mesh direction reversal', () => {
    const inputAngle = Math.PI; // 180 degrees
    const angles = shaftAngles(DEFAULT_GEAR_TRAIN, inputAngle);

    // Shaft 0 (Input) = +PI
    expect(angles[0]).toBeCloseTo(Math.PI);
    // Shaft 1 (Driven by external mesh from Shaft 0): reverses direction -> -PI * (20/40) = -PI / 2
    expect(angles[1]).toBeCloseTo(-Math.PI * 0.5);
    // Shaft 2 (Driven by external mesh from Shaft 1): reverses direction again -> -(-PI / 2) * (16/48) = +PI / 6
    expect(angles[2]).toBeCloseTo(Math.PI / 6);
  });

  it('applies half-pitch phase offset to driven gears for tooth-into-gap meshing', () => {
    const angles = [0, 0, 0];
    // Driver G1 (no phase offset)
    expect(gearAngle(DEFAULT_GEAR_TRAIN, 0, angles)).toBe(0);
    // Driven G2 (meshed from G1, 40T): phase offset = PI / 40
    expect(gearAngle(DEFAULT_GEAR_TRAIN, 1, angles)).toBeCloseTo(Math.PI / 40);
    // G3 (rigidly on shaft 1 with G2): base angle 0
    expect(gearAngle(DEFAULT_GEAR_TRAIN, 2, angles)).toBe(0);
    // Driven G4 (meshed from G3, 48T): phase offset = PI / 48
    expect(gearAngle(DEFAULT_GEAR_TRAIN, 3, angles)).toBeCloseTo(Math.PI / 48);
  });

  it('calculates shaft RPMs accurately based on 120 input RPM', () => {
    const inputRPM = 120;
    const rpms = shaftRPMs(DEFAULT_GEAR_TRAIN, inputRPM);

    expect(rpms[0]).toBe(120);
    expect(rpms[1]).toBe(60);
    expect(rpms[2]).toBe(20); // 120 / 6 = 20 RPM
  });

  it('lays out gears in 2D space without zero-distance overlaps between meshing gears', () => {
    const positions = gearPositions(DEFAULT_GEAR_TRAIN);
    expect(positions.length).toBe(DEFAULT_GEAR_TRAIN.gears.length);

    // Gear 0 on shaft 0 is at (0, 0)
    expect(positions[0].x).toBe(0);
    // Gear 1 on shaft 1 should be at x = r0 + r1 = 40 + 80 = 120
    expect(positions[1].x).toBe(120);
    // Gear 2 is also on shaft 1, so same x
    expect(positions[2].x).toBe(120);
    // Gear 3 on shaft 2 should be at x = 120 + r2 + r3 = 120 + 32 + 96 = 248
    expect(positions[3].x).toBe(248);
    // Gear 4 is also on shaft 2, so same x
    expect(positions[4].x).toBe(248);
  });

  it('prevents physical collision between G2 and G4 via multi-tier vertical mounting', () => {
    const g2 = DEFAULT_GEAR_TRAIN.gears[1];
    const g4 = DEFAULT_GEAR_TRAIN.gears[3];

    // G2 is at Tier 0, G4 is at Tier 1
    expect(g2.tier).toBe(0);
    expect(g4.tier).toBe(1);

    const g2Z = getGearZ(g2, false);
    const g4Z = getGearZ(g4, false);

    // Vertical gap between bottom of Tier 1 (g4Z) and top of Tier 0 (g2Z + 11) must be positive (>= 7 units)
    const verticalClearance = g4Z - (g2Z + 11);
    expect(verticalClearance).toBeGreaterThanOrEqual(7);
  });

  it('generates SVG tooth profile polygon points', () => {
    const points = toothProfile(20, 40);
    expect(points.length).toBeGreaterThan(0);
    const coords = points.split(' ');
    // 20 teeth * 4 points per tooth = 80 points
    expect(coords.length).toBe(80);
  });
});
