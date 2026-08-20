/** Isometric furniture drawing + FEA stress colormap for canvas viewport */

export type IsoPoint = { x: number; y: number };

export interface IsoBox {
  /** Center in model space (meters-ish, unitless) */
  cx: number;
  cy: number;
  cz: number;
  w: number;
  d: number;
  h: number;
  stressRatio: number;
  label?: string;
}

/** Blue → cyan → green → yellow → red (utilization 0–1+) */
export function stressToRgb(ratio: number): { r: number; g: number; b: number } {
  const t = Math.min(Math.max(ratio, 0), 1.2);
  if (t < 0.25) {
    const u = t / 0.25;
    return { r: Math.round(30 + u * 20), g: Math.round(80 + u * 120), b: Math.round(200 + u * 55) };
  }
  if (t < 0.5) {
    const u = (t - 0.25) / 0.25;
    return { r: Math.round(50 + u * 30), g: Math.round(200 - u * 40), b: Math.round(255 - u * 155) };
  }
  if (t < 0.8) {
    const u = (t - 0.5) / 0.3;
    return { r: Math.round(80 + u * 175), g: Math.round(160 + u * 95), b: Math.round(100 - u * 80) };
  }
  const u = Math.min((t - 0.8) / 0.4, 1);
  return { r: Math.round(255), g: Math.round(255 - u * 200), b: Math.round(20 - u * 10) };
}

export function rgbaFromRatio(ratio: number, alpha = 1): string {
  const { r, g, b } = stressToRgb(ratio);
  return `rgba(${r},${g},${b},${alpha})`;
}

export function projectIso(
  x: number,
  y: number,
  z: number,
  originX: number,
  originY: number,
  scale: number,
  yaw: number
): IsoPoint {
  const cos = Math.cos(yaw);
  const sin = Math.sin(yaw);
  const rx = x * cos - z * sin;
  const rz = x * sin + z * cos;
  const sx = (rx - rz) * 0.866;
  const sy = y * 0.92 + (rx + rz) * 0.5;
  return { x: originX + sx * scale, y: originY + sy * scale };
}

function facePoints(
  box: IsoBox,
  originX: number,
  originY: number,
  scale: number,
  yaw: number
): { top: IsoPoint[]; left: IsoPoint[]; right: IsoPoint[] } {
  const { cx, cy, cz, w, d, h } = box;
  const x0 = cx - w / 2;
  const x1 = cx + w / 2;
  const y0 = cy;
  const y1 = cy + h;
  const z0 = cz - d / 2;
  const z1 = cz + d / 2;
  const p = (x: number, y: number, z: number) => projectIso(x, y, z, originX, originY, scale, yaw);

  const top = [p(x0, y1, z0), p(x1, y1, z0), p(x1, y1, z1), p(x0, y1, z1)];
  const left = [p(x0, y0, z1), p(x1, y0, z1), p(x1, y1, z1), p(x0, y1, z1)];
  const right = [p(x1, y0, z0), p(x1, y0, z1), p(x1, y1, z1), p(x1, y1, z0)];
  return { top, left, right };
}

function fillPoly(ctx: CanvasRenderingContext2D, pts: IsoPoint[], fill: string, stroke?: string) {
  ctx.beginPath();
  ctx.moveTo(pts[0].x, pts[0].y);
  for (let i = 1; i < pts.length; i++) ctx.lineTo(pts[i].x, pts[i].y);
  ctx.closePath();
  ctx.fillStyle = fill;
  ctx.fill();
  if (stroke) {
    ctx.strokeStyle = stroke;
    ctx.lineWidth = 1;
    ctx.stroke();
  }
}

export function drawIsoBox(
  ctx: CanvasRenderingContext2D,
  box: IsoBox,
  originX: number,
  originY: number,
  scale: number,
  yaw: number,
  useStressColors: boolean
) {
  const { top, left, right } = facePoints(box, originX, originY, scale, yaw);
  const base = stressToRgb(box.stressRatio);
  const darken = (f: number) =>
    `rgb(${Math.round(base.r * f)},${Math.round(base.g * f)},${Math.round(base.b * f)})`;

  if (useStressColors) {
    fillPoly(ctx, left, darken(0.72), 'rgba(255,255,255,0.15)');
    fillPoly(ctx, right, darken(0.85), 'rgba(255,255,255,0.15)');
    fillPoly(ctx, top, darken(1.05), 'rgba(255,255,255,0.25)');
  } else {
    const c = '#3366ff';
    fillPoly(ctx, left, c + '99', '#ffffff30');
    fillPoly(ctx, right, c + 'bb', '#ffffff30');
    fillPoly(ctx, top, c + 'dd', '#ffffff40');
  }
}

export function boxesForTable(partStress: Record<string, number>): IsoBox[] {
  const r = (n: string, f = 0.2) => partStress[n] ?? f;
  return [
    { cx: 0, cy: 0.75, cz: 0, w: 1.2, d: 0.8, h: 0.06, stressRatio: r('Tabletop'), label: 'Tabletop' },
    { cx: -0.48, cy: 0, cz: -0.32, w: 0.08, d: 0.08, h: 0.75, stressRatio: r('Leg-FL'), label: 'Leg-FL' },
    { cx: 0.48, cy: 0, cz: -0.32, w: 0.08, d: 0.08, h: 0.75, stressRatio: r('Leg-FR'), label: 'Leg-FR' },
    { cx: -0.48, cy: 0, cz: 0.32, w: 0.08, d: 0.08, h: 0.75, stressRatio: r('Leg-RL'), label: 'Leg-RL' },
    { cx: 0.48, cy: 0, cz: 0.32, w: 0.08, d: 0.08, h: 0.75, stressRatio: r('Leg-RR'), label: 'Leg-RR' },
  ];
}

export function boxesForChair(partStress: Record<string, number>): IsoBox[] {
  const r = (n: string, f = 0.2) => partStress[n] ?? f;
  return [
    { cx: 0, cy: 0.45, cz: 0, w: 0.55, d: 0.55, h: 0.05, stressRatio: r('Seat'), label: 'Seat' },
    { cx: 0, cy: 0.55, cz: -0.28, w: 0.55, d: 0.04, h: 0.42, stressRatio: r('Backrest'), label: 'Backrest' },
    { cx: -0.22, cy: 0, cz: -0.2, w: 0.06, d: 0.06, h: 0.45, stressRatio: r('Leg-FL'), label: 'Leg-FL' },
    { cx: 0.22, cy: 0, cz: -0.2, w: 0.06, d: 0.06, h: 0.45, stressRatio: r('Leg-FR'), label: 'Leg-FR' },
    { cx: -0.22, cy: 0, cz: 0.2, w: 0.06, d: 0.06, h: 0.45, stressRatio: r('Leg-RL'), label: 'Leg-RL' },
    { cx: 0.22, cy: 0, cz: 0.2, w: 0.06, d: 0.06, h: 0.45, stressRatio: r('Leg-RR'), label: 'Leg-RR' },
  ];
}

export function boxesForLadder(partStress: Record<string, number>): IsoBox[] {
  const r = (n: string, f = 0.2) => partStress[n] ?? f;
  const boxes: IsoBox[] = [
    { cx: -0.22, cy: 0, cz: 0, w: 0.05, d: 0.05, h: 1.5, stressRatio: r('Rail-Left'), label: 'Rail-L' },
    { cx: 0.22, cy: 0, cz: 0, w: 0.05, d: 0.05, h: 1.5, stressRatio: r('Rail-Right'), label: 'Rail-R' },
  ];
  for (let i = 1; i <= 5; i++) {
    const y = 0.15 + (i - 1) * 0.28;
    boxes.push({
      cx: 0,
      cy: y,
      cz: 0,
      w: 0.48,
      d: 0.04,
      h: 0.04,
      stressRatio: r(`Rung-${i}`, r('Rung-3')),
      label: `Rung-${i}`,
    });
  }
  return boxes;
}

export function drawLoadArrow(
  ctx: CanvasRenderingContext2D,
  cx: number,
  topY: number,
  loadKg: number,
  t: number
) {
  const pulse = 0.85 + Math.sin(t * 4) * 0.15;
  const arrowTop = topY - 70 * pulse;
  const arrowTip = topY - 12;

  ctx.save();
  ctx.strokeStyle = '#ffcc00';
  ctx.fillStyle = '#ffcc00';
  ctx.lineWidth = 2;
  ctx.setLineDash([4, 4]);
  ctx.beginPath();
  ctx.moveTo(cx, arrowTop);
  ctx.lineTo(cx, arrowTip);
  ctx.stroke();
  ctx.setLineDash([]);

  ctx.beginPath();
  ctx.moveTo(cx, arrowTip);
  ctx.lineTo(cx - 10, arrowTip - 14);
  ctx.lineTo(cx + 10, arrowTip - 14);
  ctx.closePath();
  ctx.fill();

  ctx.font = 'bold 11px "JetBrains Mono", monospace';
  ctx.textAlign = 'center';
  ctx.fillStyle = '#ffcc00';
  ctx.fillText(`${loadKg} kg`, cx, arrowTop - 8);
  ctx.restore();
}

export function drawStressLegend(ctx: CanvasRenderingContext2D, x: number, y: number) {
  const w = 12;
  const h = 64;
  const steps = 8;
  for (let i = 0; i < steps; i++) {
    const ratio = (steps - 1 - i) / (steps - 1);
    ctx.fillStyle = rgbaFromRatio(ratio, 1);
    ctx.fillRect(x, y + (h / steps) * i, w, h / steps + 1);
  }
  ctx.fillStyle = '#8888aa';
  ctx.font = '8px monospace';
  ctx.textAlign = 'left';
  ctx.fillText('SAFE', x + w + 4, y + 10);
  ctx.fillText('YIELD', x + w + 4, y + h);
}
