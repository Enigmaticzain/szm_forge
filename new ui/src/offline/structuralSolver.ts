import type { StructuralMember } from '../workshop/structuralAssembly';
import type { FurniturePart } from '../workshop/furnitureEngine';
import { getMaterialLabel, getYieldMPa, getYoungsGPa } from './materials';

const G = 9.81;

interface CrossSection {
  area: number;
  I: number;
  c: number;
}

function crossSection(m: StructuralMember): CrossSection {
  const L = Math.max(m.length, 1e-4);
  const W = Math.max(m.width, 1e-4);
  const H = Math.max(m.height, 1e-4);

  if (m.kind === 'rod') {
    const d = W;
    const r = d / 2;
    const area = Math.PI * r * r;
    const I = (Math.PI * d ** 4) / 64;
    return { area, I, c: d / 2 };
  }
  if (m.kind === 'plate') {
    return { area: W * H, I: (W * H ** 3) / 12, c: H / 2 };
  }
  const area = W * H;
  let I = (W * H ** 3) / 12;
  if (I <= 0) I = (area * L * L) / 12;
  return { area, I, c: H / 2 };
}

function vonMises(axial: number, bending: number): number {
  return Math.sqrt(axial * axial + bending * bending);
}

function statusFromRatio(ratio: number): string {
  if (ratio >= 1) return 'FAIL';
  if (ratio >= 0.8) return 'WARN';
  return 'SAFE';
}

function loadShare(member: StructuralMember, members: StructuralMember[]): number {
  let totalW = 0;
  const weights: number[] = [];
  for (const m of members) {
    const cs = crossSection(m);
    const massProxy = cs.area * m.length;
    weights.push(massProxy);
    totalW += massProxy;
  }

  const idx = members.findIndex(m => m.id === member.id);
  const base = totalW > 0 ? weights[idx] / totalW : 1 / members.length;

  let maxY = 0;
  for (const m of members) {
    maxY = Math.max(maxY, m.cy + m.length);
  }
  const topness = (member.cy + member.length) / (maxY > 0 ? maxY : 1);
  const blend = base * 0.4 + topness * 0.6;

  let sum = 0;
  for (const m of members) {
    const cs = crossSection(m);
    const my = Math.max(maxY, 1e-6);
    const t = (m.cy + m.length) / my;
    const w = base * 0.4 + t * 0.6;
    sum += w * cs.area * m.length;
  }
  const csM = crossSection(member);
  return sum > 0 ? (blend * csM.area * member.length) / sum : 1 / members.length;
}

export function solveStructural(
  members: StructuralMember[],
  materialId: string,
  loadKg: number,
): { ok: boolean; parts: FurniturePart[]; solver: string; error?: string } {
  if (!members.length) {
    return { ok: false, parts: [], solver: 'SZM Offline Structural', error: 'No members' };
  }

  const parts: FurniturePart[] = [];

  for (const m of members) {
    const matId = m.materialId || materialId;
    const yieldMPa = getYieldMPa(matId);
    const E_GPa = getYoungsGPa(matId);

    const cs = crossSection(m);
    const share = loadShare(m, members);
    const F = loadKg * G * share;
    const L = m.length;

    const sigmaAxial = Math.abs(F / Math.max(cs.area, 1e-9)) / 1e6;
    const M = (F * L) / 4;
    const sigmaBend = cs.I > 0 ? Math.abs(((M * cs.c) / cs.I) / 1e6) : 0;

    let sigma = vonMises(sigmaAxial, sigmaBend);

    if (m.kind === 'rod') {
      const E = E_GPa * 1e9;
      const r = Math.max(m.width * 0.5, 1e-4);
      const slenderness = L / r;
      if (slenderness > 80) {
        const Pcr = (Math.PI ** 2 * E * cs.I) / (4 * L * L);
        if (F > Pcr * 0.5) {
          sigma *= 1 + (F / Math.max(Pcr, 1)) * 0.35;
        }
      }
    }

    const ratio = yieldMPa > 0 ? sigma / yieldMPa : 0;

    parts.push({
      name: m.name,
      stress_MPa: sigma,
      yield_MPa: yieldMPa,
      stressRatio: ratio,
      status: statusFromRatio(ratio),
      force_N: F,
      material: getMaterialLabel(matId),
    });
  }

  return {
    ok: true,
    parts,
    solver: 'SZM Offline Structural (bundled MaterialDatabase + beam/rod mechanics)',
  };
}
