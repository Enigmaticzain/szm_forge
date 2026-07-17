/**
 * Structural kit: beams, rods, plates, blocks — attach + FEA-style stress (local solver).
 * Axial + simplified Euler–Bernoulli bending; von Mises combine for rectangular sections.
 */

export type MemberKind = 'beam' | 'rod' | 'plate' | 'block';

export type AttachEnd = 'start' | 'end' | 'top' | 'side';

export interface StructuralMember {
  id: string;
  name: string;
  kind: MemberKind;
  length: number;
  width: number;
  height: number;
  cx: number;
  cy: number;
  cz: number;
  rotY: number;
  materialId: string;
  attachTo: string | null;
  attachEnd: AttachEnd;
}

export interface MemberStress {
  name: string;
  stress_MPa: number;
  yield_MPa: number;
  stressRatio: number;
  status: string;
  force_N: number;
  material: string;
  kind: MemberKind;
}

const MATERIALS: Record<string, { label: string; yieldMPa: number; E_GPa: number }> = {
  'MAT-STEEL-STRUCT': { label: 'Structural Steel', yieldMPa: 250, E_GPa: 200 },
  'MAT-AL-6061-T6': { label: '6061-T6 Aluminum', yieldMPa: 275, E_GPa: 69 },
  'MAT-TI-6AL4V': { label: 'Ti-6Al-4V', yieldMPa: 880, E_GPa: 114 },
  'MAT-CU-C110': { label: 'C110 Copper', yieldMPa: 70, E_GPa: 117 },
};

const G = 9.81;

export function materialCatalog() {
  return Object.entries(MATERIALS).map(([id, m]) => ({ id, label: m.label }));
}

function crossSection(member: StructuralMember): { area: number; I: number; c: number } {
  const { kind, length, width, height } = member;
  const L = Math.max(length, 1e-4);
  const W = Math.max(width, 1e-4);
  const H = Math.max(height, 1e-4);

  if (kind === 'rod') {
    const d = W;
    const r = d / 2;
    const area = Math.PI * r * r;
    const I = (Math.PI * d ** 4) / 64;
    return { area, I, c: d / 2 };
  }
  if (kind === 'plate') {
    return { area: W * H, I: (W * H ** 3) / 12, c: H / 2 };
  }
  // beam / block — rectangular along length (Y)
  const area = W * H;
  const I = (W * H ** 3) / 12;
  return { area, I: I > 0 ? I : area * L * L / 12, c: H / 2 };
}

// Fallback math logic (vonMises, solveAssembly) removed — we strictly use the C++ backend now.

export function attachPosition(
  parent: StructuralMember,
  child: StructuralMember,
  end: AttachEnd
): Pick<StructuralMember, 'cx' | 'cy' | 'cz' | 'rotY'> {
  const px = parent.cx;
  const py = parent.cy;
  const pz = parent.cz;
  const L = parent.length;

  switch (end) {
    case 'end':
      return { cx: px, cy: py + L, cz: pz, rotY: parent.rotY };
    case 'start':
      return { cx: px, cy: py - child.length, cz: pz, rotY: parent.rotY };
    case 'top':
      return { cx: px, cy: py + L, cz: pz, rotY: parent.rotY };
    case 'side':
      return { cx: px + parent.width / 2 + child.width / 2, cy: py, cz: pz, rotY: parent.rotY };
    default:
      return { cx: px, cy: py + L, cz: pz, rotY: parent.rotY };
  }
}

export function createMember(
  kind: MemberKind,
  name: string,
  dims: { length: number; width: number; height: number },
  materialId: string,
  attachTo: string | null,
  attachEnd: AttachEnd,
  members: StructuralMember[]
): StructuralMember {
  const id = `m-${Date.now()}-${Math.random().toString(36).slice(2, 6)}`;
  let cx = 0;
  let cy = 0;
  let cz = 0;
  let rotY = 0;

  if (attachTo) {
    const parent = members.find(m => m.id === attachTo);
    if (parent) {
      const pos = attachPosition(parent, { id, name, kind, ...dims, cx: 0, cy: 0, cz: 0, rotY: 0, materialId, attachTo, attachEnd }, attachEnd);
      cx = pos.cx;
      cy = pos.cy;
      cz = pos.cz;
      rotY = pos.rotY;
    }
  }

  return {
    id,
    name,
    kind,
    length: dims.length,
    width: dims.width,
    height: dims.height,
    cx,
    cy,
    cz,
    rotY,
    materialId,
    attachTo,
    attachEnd,
  };
}

/** Preset kits → member lists */
export function presetTable(materialId: string): StructuralMember[] {
  const top = createMember('plate', 'Tabletop', { length: 0.05, width: 1.2, height: 0.8 }, materialId, null, 'end', []);
  const legs: StructuralMember[] = [];
  const corners = [
    [-0.5, -0.35],
    [0.5, -0.35],
    [-0.5, 0.35],
    [0.5, 0.35],
  ];
  corners.forEach(([x, z], i) => {
    const leg = createMember('beam', `Leg-${i + 1}`, { length: 0.75, width: 0.08, height: 0.08 }, materialId, top.id, 'end', [top, ...legs]);
    leg.cx = x;
    leg.cz = z;
    leg.cy = 0;
    legs.push(leg);
  });
  top.cy = 0.75;
  return [top, ...legs];
}

export function presetChair(materialId: string): StructuralMember[] {
  const seat = createMember('plate', 'Seat', { length: 0.05, width: 0.55, height: 0.55 }, materialId, null, 'end', []);
  seat.cy = 0.45;
  const back = createMember('plate', 'Backrest', { length: 0.42, width: 0.55, height: 0.04 }, materialId, seat.id, 'end', [seat]);
  back.cy = 0.5;
  back.cz = -0.28;
  const legs: StructuralMember[] = [];
  [[-0.22, -0.2], [0.22, -0.2], [-0.22, 0.2], [0.22, 0.2]].forEach(([x, z], i) => {
    const leg = createMember('beam', `Leg-${i + 1}`, { length: 0.45, width: 0.06, height: 0.06 }, materialId, seat.id, 'start', [seat, back, ...legs]);
    leg.cx = x;
    leg.cz = z;
    leg.cy = 0;
    legs.push(leg);
  });
  return [seat, back, ...legs];
}

export function presetLadder(materialId: string): StructuralMember[] {
  const railL = createMember('beam', 'Rail-Left', { length: 1.5, width: 0.05, height: 0.05 }, materialId, null, 'end', []);
  railL.cx = -0.22;
  const railR = createMember('beam', 'Rail-Right', { length: 1.5, width: 0.05, height: 0.05 }, materialId, null, 'end', []);
  railR.cx = 0.22;
  const rungs: StructuralMember[] = [];
  for (let i = 0; i < 5; i++) {
    const rung = createMember('rod', `Rung-${i + 1}`, { length: 0.44, width: 0.04, height: 0.04 }, materialId, railL.id, 'side', [railL, railR, ...rungs]);
    rung.cy = 0.15 + i * 0.28;
    rung.cx = 0;
    rungs.push(rung);
  }
  return [railL, railR, ...rungs];
}

export function membersToViewportBoxes(
  members: StructuralMember[],
  partStress: Record<string, number>
): import('../utils/viewportFurniture3d').IsoBox[] {
  return members.map(m => {
    const ratio = partStress[m.name] ?? 0.2;
    const w = m.kind === 'rod' ? m.width : m.width;
    const h = m.kind === 'plate' ? m.length : m.height;
    const d = m.kind === 'beam' || m.kind === 'block' ? m.width : m.width;
    const len = m.kind === 'plate' ? m.height : m.length;
    return {
      cx: m.cx,
      cy: m.cy,
      cz: m.cz,
      w: m.kind === 'rod' ? m.width * 2 : w,
      d: m.kind === 'plate' ? m.height : d,
      h: len,
      stressRatio: ratio,
      label: m.name,
    };
  });
}
