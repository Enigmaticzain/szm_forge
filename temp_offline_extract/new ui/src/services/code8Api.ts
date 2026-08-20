/**
 * CalculiX examples catalog from codes/code 8 (backend CalculiXPipeline).
 */

import { fetchJson } from './forgeApi';

export interface Code8ToolDto {
  name: string;
  available: boolean;
}

export interface Code8ExampleDto {
  name: string;
  category: string;
  summary: string;
  directory: string;
  hasTestScript: boolean;
  inpCount: number;
}

export interface Code8CatalogDto {
  ok: boolean;
  root: string;
  exampleCount: number;
  examples: Code8ExampleDto[];
  toolchain: {
    python: Code8ToolDto;
    cgx: Code8ToolDto;
    ccx: Code8ToolDto;
    gmsh: Code8ToolDto;
    coreSolverReady: boolean;
  };
}

export async function fetchCode8Catalog(): Promise<Code8CatalogDto | null> {
  return fetchJson<Code8CatalogDto>('/api/code8/catalog', 8000);
}
