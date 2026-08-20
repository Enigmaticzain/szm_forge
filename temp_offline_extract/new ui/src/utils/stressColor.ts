/** FEA stress colormap (matches codes/ viewport + ExportToThreeJS). */
export function stressRatioToHex(ratio: number): number {
  const t = Math.min(Math.max(ratio, 0), 1.2);
  if (t < 0.25) return 0x1e50c8;
  if (t < 0.5) return 0x00c8ff;
  if (t < 0.8) return 0xffaa33;
  return 0xff2200;
}

export function stressRatioToCss(ratio: number): string {
  const hex = stressRatioToHex(ratio);
  const r = (hex >> 16) & 0xff;
  const g = (hex >> 8) & 0xff;
  const b = hex & 0xff;
  return `rgb(${r},${g},${b})`;
}
