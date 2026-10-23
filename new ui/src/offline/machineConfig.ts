import type { AxisLimit, MachineConfigDto } from '../services/forgeApi';
import { loadJson, saveJson } from './storage';

const CONFIG_KEY = 'machine_config';

const DEFAULT_LINEAR: AxisLimit = { travel_mm: 500, feed_mm_min: 5000, accel_mm_s2: 500 };
const DEFAULT_ROTARY: AxisLimit = { travel_mm: 360, feed_mm_min: 3000, accel_mm_s2: 200 };

export function defaultMachineConfig(): MachineConfigDto {
  return {
    id: 'default',
    name: 'CNC Machine',
    is_5axis: false,
    linear: [{ ...DEFAULT_LINEAR }, { ...DEFAULT_LINEAR }, { ...DEFAULT_LINEAR }],
    rotary: [{ ...DEFAULT_ROTARY }, { ...DEFAULT_ROTARY }],
  };
}

export function loadMachineConfig(id = 'default'): MachineConfigDto {
  const all = loadJson<Record<string, MachineConfigDto>>(CONFIG_KEY, {});
  return all[id] ?? { ...defaultMachineConfig(), id };
}

export function saveMachineConfigLocal(cfg: MachineConfigDto): boolean {
  const all = loadJson<Record<string, MachineConfigDto>>(CONFIG_KEY, {});
  all[cfg.id] = cfg;
  saveJson(CONFIG_KEY, all);
  return true;
}

export interface ValidationResult {
  valid: boolean;
  errors: string[];
  warnings: string[];
  axis_count: number;
}

export function validateMachineConfig(cfg: MachineConfigDto): ValidationResult {
  const errors: string[] = [];
  const warnings: string[] = [];

  const expectedLinear = 3;
  if (cfg.linear.length < expectedLinear) {
    errors.push(`Expected ${expectedLinear} linear axes, got ${cfg.linear.length}`);
  }

  cfg.linear.forEach((ax, i) => {
    if (ax.travel_mm <= 0) errors.push(`Axis ${i}: travel must be positive`);
    if (ax.feed_mm_min <= 0) errors.push(`Axis ${i}: feed must be positive`);
    if (ax.accel_mm_s2 <= 0) errors.push(`Axis ${i}: acceleration must be positive`);
    if (ax.feed_mm_min > 30000) warnings.push(`Axis ${i}: feed > 30000 mm/min — verify spindle capability`);
  });

  if (cfg.is_5axis && cfg.rotary.length < 2) {
    warnings.push('5-axis mode enabled but fewer than 2 rotary axes configured');
  }

  return {
    valid: errors.length === 0,
    errors,
    warnings,
    axis_count: cfg.linear.length + (cfg.is_5axis ? cfg.rotary.length : 0),
  };
}
