import kbIndex from '@kb/index.json';
import materialsData from '@kb/materials.json';
import equationsData from '@kb/equations.json';
import manufacturingData from '@kb/manufacturing.json';
import machineElementsData from '@kb/machine_elements.json';
import machineTypesData from '@kb/machine_types.json';
import roboticsData from '@kb/robotics.json';
import controlSystemsData from '@kb/control_systems.json';
import treatmentsData from '@kb/treatments.json';
import standardsData from '@kb/standards.json';
import cadCamData from '@kb/cad_cam.json';

export interface KbDomainInfo {
  id: string;
  name: string;
  file?: string;
  entryCount?: number;
}

export interface KbSearchHit {
  domain: string;
  entry: Record<string, unknown>;
}

const DOMAIN_FILES: Record<string, { entries: Record<string, unknown>[] }> = {
  materials: materialsData as { entries: Record<string, unknown>[] },
  equations: equationsData as { entries: Record<string, unknown>[] },
  manufacturing: manufacturingData as { entries: Record<string, unknown>[] },
  machine_elements: machineElementsData as { entries: Record<string, unknown>[] },
  machine_types: machineTypesData as { entries: Record<string, unknown>[] },
  robotics: roboticsData as { entries: Record<string, unknown>[] },
  control_systems: controlSystemsData as { entries: Record<string, unknown>[] },
  treatments: treatmentsData as { entries: Record<string, unknown>[] },
  standards: standardsData as { entries: Record<string, unknown>[] },
  cad_cam: cadCamData as { entries: Record<string, unknown>[] },
};

export function listKbDomains(): KbDomainInfo[] {
  return (kbIndex as { domains: KbDomainInfo[] }).domains;
}

export function getKbDomain(domainId: string): Record<string, unknown> | null {
  const file = DOMAIN_FILES[domainId];
  if (!file) return null;
  return { domain: domainId, entries: file.entries };
}

function entrySearchText(entry: Record<string, unknown>): string {
  return [
    entry.id,
    entry.name,
    entry.category,
    entry.description,
    entry.notes,
    entry.expression,
    entry.latex,
    Array.isArray(entry.tags) ? (entry.tags as string[]).join(' ') : '',
  ]
    .filter(Boolean)
    .join(' ')
    .toLowerCase();
}

export function searchKb(query: string, domain = ''): KbSearchHit[] {
  const q = query.trim().toLowerCase();
  if (!q) return [];

  const domains = domain
    ? [domain]
    : Object.keys(DOMAIN_FILES);

  const hits: KbSearchHit[] = [];
  for (const d of domains) {
    const data = DOMAIN_FILES[d];
    if (!data) continue;
    for (const entry of data.entries) {
      if (entrySearchText(entry).includes(q)) {
        hits.push({ domain: d, entry });
      }
    }
  }
  return hits;
}
