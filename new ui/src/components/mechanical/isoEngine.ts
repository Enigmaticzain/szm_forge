/**
 * Isometric projection & rendering utilities for SVG mechanical drawings.
 *
 * Implements the core math from the Anatomy skill's iso-kit:
 * - Camera projection (azimuth 45°, elevation 30°)
 * - Depth ordering for painter's algorithm
 * - Four-tone shading based on face normals
 *
 * Designed to work with React's SVG rendering — no DOM mutation.
 */

// ── Projection camera ──────────────────────────────────────────

export interface IsoCamera {
  azimuth: number;   // radians
  elevation: number; // radians
  scale: number;
  originX: number;   // screen X of world origin
  originY: number;   // screen Y of world origin
  cosA: number;
  sinA: number;
  cosE: number;
  sinE: number;
}

/**
 * Create an isometric camera.
 * Default: azimuth 45°, elevation 30° (standard isometric).
 */
export function createCamera(opts: {
  width: number;
  height: number;
  scale?: number;
  azimuth?: number;
  elevation?: number;
  originX?: number;
  originY?: number;
}): IsoCamera {
  const azimuth = (opts.azimuth ?? 45) * (Math.PI / 180);
  const elevation = (opts.elevation ?? 30) * (Math.PI / 180);
  const scale = opts.scale ?? 1;
  const cosA = Math.cos(azimuth);
  const sinA = Math.sin(azimuth);
  const cosE = Math.cos(elevation);
  const sinE = Math.sin(elevation);

  return {
    azimuth,
    elevation,
    scale,
    originX: opts.originX ?? opts.width / 2,
    originY: opts.originY ?? opts.height * 0.6,
    cosA,
    sinA,
    cosE,
    sinE,
  };
}

/**
 * Project a 3D world point [x, y, z] to 2D screen coordinates [sx, sy].
 * Uses true dimetric projection (Anatomy convention):
 *   sx = (x·cosA - y·sinA) · scale
 *   sy = (-(x·sinA + y·cosA)·sinE - z·cosE) · scale
 */
export function project(x: number, y: number, z: number, cam: IsoCamera): [number, number] {
  const sx = cam.originX + (x * cam.cosA - y * cam.sinA) * cam.scale;
  const sy = cam.originY + (-(x * cam.sinA + y * cam.cosA) * cam.sinE - z * cam.cosE) * cam.scale;
  return [sx, sy];
}

/**
 * Compute depth value for painter's order sorting.
 * Higher = further from viewer = paint first.
 */
export function depthOf(x: number, y: number, z: number, cam: IsoCamera): number {
  return (x * cam.sinA + y * cam.cosA) * cam.cosE - z * cam.sinE;
}

/**
 * Compute SVG translate string for a 3D offset.
 */
export function isoTranslate(dx: number, dy: number, dz: number, cam: IsoCamera): string {
  const [sx, sy] = project(dx, dy, dz, cam);
  const [ox, oy] = project(0, 0, 0, cam);
  return `translate(${(sx - ox).toFixed(2)}, ${(sy - oy).toFixed(2)})`;
}

// ── Shade tones ────────────────────────────────────────────────

export type ShadeTone = 0 | 1 | 2 | 3;

/**
 * Determine shade tone (0–3) for a face based on its outward normal.
 * Light comes from upper-left (standard Anatomy convention).
 * 0 = brightest (lit), 3 = darkest (shadow).
 */
export function shadeTone(normalX: number, normalY: number, normalZ: number, cam: IsoCamera): ShadeTone {
  // Light direction: opposite of viewer, slightly from upper left
  const lx = -cam.sinA;
  const ly = cam.cosA;
  const lz = 0.5;
  const len = Math.sqrt(lx * lx + ly * ly + lz * lz);
  const dot = (normalX * lx + normalY * ly + normalZ * lz) / len;
  if (dot > 0.5) return 0;  // bright
  if (dot > 0.1) return 1;  // mid
  if (dot > -0.3) return 2; // dim
  return 3;                   // dark
}

// ── Gear SVG geometry ──────────────────────────────────────────

/**
 * Generate SVG path data for a gear tooth profile.
 * Returns a closed path string for a gear with the given parameters.
 */
export function gearPath(
  teeth: number,
  outerRadius: number,
  rotation: number = 0,
  cam: IsoCamera,
  cx: number = 0,
  cy: number = 0,
  z: number = 0,
): string {
  const toothAngle = (2 * Math.PI) / teeth;
  const innerR = outerRadius * 0.82;
  const tipWidth = 0.35;
  const pts: string[] = [];

  for (let i = 0; i < teeth; i++) {
    const base = rotation + i * toothAngle;
    const angles = [
      base,                                 // root left
      base + toothAngle * tipWidth,         // tip left
      base + toothAngle * (1 - tipWidth),   // tip right
      base + toothAngle,                    // root right
    ];
    const radii = [innerR, outerRadius, outerRadius, innerR];

    for (let j = 0; j < 4; j++) {
      const wx = cx + radii[j] * Math.cos(angles[j]);
      const wy = cy + radii[j] * Math.sin(angles[j]);
      const [sx, sy] = project(wx, wy, z, cam);
      pts.push(`${sx.toFixed(1)},${sy.toFixed(1)}`);
    }
  }

  return `M ${pts.join(' L ')} Z`;
}

/**
 * Generate an elliptical path for a circle projected isometrically.
 * Used for shaft cross-sections and gear hubs.
 */
export function isoCirclePath(
  cx: number,
  cy: number,
  z: number,
  radius: number,
  cam: IsoCamera,
  steps: number = 32,
): string {
  const pts: string[] = [];
  for (let i = 0; i <= steps; i++) {
    const angle = (i / steps) * 2 * Math.PI;
    const wx = cx + radius * Math.cos(angle);
    const wy = cy + radius * Math.sin(angle);
    const [sx, sy] = project(wx, wy, z, cam);
    pts.push(i === 0 ? `M ${sx.toFixed(1)},${sy.toFixed(1)}` : `L ${sx.toFixed(1)},${sy.toFixed(1)}`);
  }
  return pts.join(' ') + ' Z';
}

/**
 * Generate a cylinder (shaft) as projected side profile.
 * Returns { leftLine, rightLine, topCap } path data.
 */
export function shaftPaths(
  cx: number,
  cy: number,
  z0: number,
  z1: number,
  radius: number,
  cam: IsoCamera,
  steps: number = 32,
): { outline: string; topCap: string } {
  // Left and right silhouette lines
  const leftPts: string[] = [];
  const rightPts: string[] = [];

  // Find the two silhouette angles (where the normal is perpendicular to view)
  const viewAngle = cam.azimuth + Math.PI / 2;
  const leftAngle = viewAngle;
  const rightAngle = viewAngle + Math.PI;

  // Bottom arc (front half visible)
  const arcPts: string[] = [];
  for (let i = 0; i <= steps; i++) {
    const t = i / steps;
    const angle = rightAngle + t * Math.PI; // front half
    const wx = cx + radius * Math.cos(angle);
    const wy = cy + radius * Math.sin(angle);
    const [sx, sy] = project(wx, wy, z0, cam);
    arcPts.push(`${sx.toFixed(1)},${sy.toFixed(1)}`);
  }

  // Top arc (front half)
  const topArcPts: string[] = [];
  for (let i = 0; i <= steps; i++) {
    const t = i / steps;
    const angle = leftAngle + t * Math.PI; // front half reversed
    const wx = cx + radius * Math.cos(angle);
    const wy = cy + radius * Math.sin(angle);
    const [sx, sy] = project(wx, wy, z1, cam);
    topArcPts.push(`${sx.toFixed(1)},${sy.toFixed(1)}`);
  }

  // Left side line
  const [lbx, lby] = project(cx + radius * Math.cos(leftAngle), cy + radius * Math.sin(leftAngle), z0, cam);
  const [ltx, lty] = project(cx + radius * Math.cos(leftAngle), cy + radius * Math.sin(leftAngle), z1, cam);
  // Right side line
  const [rbx, rby] = project(cx + radius * Math.cos(rightAngle), cy + radius * Math.sin(rightAngle), z0, cam);
  const [rtx, rty] = project(cx + radius * Math.cos(rightAngle), cy + radius * Math.sin(rightAngle), z1, cam);

  const outline = `M ${arcPts.join(' L ')} L ${ltx.toFixed(1)},${lty.toFixed(1)} L ${topArcPts.join(' L ')} L ${rbx.toFixed(1)},${rby.toFixed(1)} Z`;

  const topCap = isoCirclePath(cx, cy, z1, radius, cam, steps);

  return { outline, topCap };
}

export interface CylinderFacet {
  path: string;
  tone: ShadeTone;
}

/**
 * Generate four-tone shaded facets for the visible front side of a cylinder.
 * Used for gear thickness rims and shaft collars.
 */
export function cylinderFacets(
  cx: number,
  cy: number,
  z0: number,
  z1: number,
  radius: number,
  cam: IsoCamera,
  steps: number = 16
): CylinderFacet[] {
  const facets: CylinderFacet[] = [];
  const viewAngle = cam.azimuth + Math.PI / 2;
  const startAngle = viewAngle;
  for (let i = 0; i < steps; i++) {
    const a0 = startAngle + (i / steps) * Math.PI;
    const a1 = startAngle + ((i + 1) / steps) * Math.PI;
    const midA = (a0 + a1) / 2;
    const nx = Math.cos(midA);
    const ny = Math.sin(midA);
    const tone = shadeTone(nx, ny, 0, cam);

    const [p00x, p00y] = project(cx + radius * Math.cos(a0), cy + radius * Math.sin(a0), z0, cam);
    const [p10x, p10y] = project(cx + radius * Math.cos(a1), cy + radius * Math.sin(a1), z0, cam);
    const [p11x, p11y] = project(cx + radius * Math.cos(a1), cy + radius * Math.sin(a1), z1, cam);
    const [p01x, p01y] = project(cx + radius * Math.cos(a0), cy + radius * Math.sin(a0), z1, cam);

    const path = `M ${p00x.toFixed(1)},${p00y.toFixed(1)} L ${p10x.toFixed(1)},${p10y.toFixed(1)} L ${p11x.toFixed(1)},${p11y.toFixed(1)} L ${p01x.toFixed(1)},${p01y.toFixed(1)} Z`;
    facets.push({ path, tone });
  }
  return facets;
}

// ── Color palette (matches Anatomy dark theme) ─────────────────

export const PALETTE = {
  shade: ['#e8e4df', '#c5c0b8', '#8a8580', '#5a5652'] as const,
  accent: '#f59e0b',  // matches forge-accent / amber
  accentDim: '#b45309',
  accentShade: ['#fbbf24', '#f59e0b', '#d97706', '#92400e'] as const,
  steel: '#7a7d82',
  steelDark: '#4a4d52',
  steelShade: ['#a0a5ad', '#7a7d82', '#5a5d62', '#3a3d42'] as const,
  selectedShade: ['#93c5fd', '#60a5fa', '#3b82f6', '#1d4ed8'] as const,
  shaft: '#606366',
  shaftShade: ['#888b90', '#6a6d72', '#505358', '#383a3e'] as const,
  highlight: '#fbbf24',
  bg: '#1a1a1a',
  card: '#242424',
  border: '#333',
  text: '#e8e4df',
  textDim: '#8a8580',
};
