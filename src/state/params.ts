import { PRESETS, type CrystalPopulation } from "./populations";
export type { CrystalPopulation } from "./populations";

// Per-population fields that don't affect what the GPU computes. Inverted
// (deny-list) so newly added CrystalPopulation fields default to "sim-relevant".
const POPULATION_NON_SIM_KEYS: ReadonlySet<keyof CrystalPopulation> = new Set(["name"]);

export type Projection = "0" | "1" | "2" | "3" | "4";

export type SimParams = {
  sunAlt: number;
  sunDiam: number;
  camPitch: number;
  camYaw: number;
  camFov: number;
  projection: Projection;
  hideSubHorizon: boolean;
  populations: CrystalPopulation[];
};

export type DisplayParams = {
  brightness: number;
  showGuides: boolean;
  showSky: boolean;
};

export const DEFAULT_SIM: SimParams = {
  sunAlt: 30,
  sunDiam: 0.5,
  camPitch: 30,
  camYaw: 15,
  camFov: 75,
  projection: "0",
  hideSubHorizon: true,
  populations: [
    { ...PRESETS.column(), weight: 1 },
    { ...PRESETS.plate(), weight: 1 },
    { ...PRESETS.random(), weight: 1 },
  ],
};

export const DEFAULT_DISPLAY: DisplayParams = {
  brightness: 3,
  showGuides: false,
  showSky: true,
};

// "View" = observer's perspective + celestial geometry (camera + sun position).
// These params affect the sky background and guides, not just the halo.
const VIEW_KEYS: (keyof SimParams)[] = [
  "sunAlt",
  "sunDiam",
  "camPitch",
  "camYaw",
  "camFov",
  "projection",
];

export function didViewChange(prev: SimParams, next: SimParams): boolean {
  return VIEW_KEYS.some((k) => prev[k] !== next[k]);
}

function populationSimEqual(a: CrystalPopulation, b: CrystalPopulation): boolean {
  if (a === b) {
    return true;
  }
  for (const key of Object.keys(a) as (keyof CrystalPopulation)[]) {
    if (POPULATION_NON_SIM_KEYS.has(key)) {
      continue;
    }
    const av = a[key];
    const bv = b[key];
    if (Array.isArray(av) && Array.isArray(bv)) {
      if (av.length !== bv.length) {
        return false;
      }
      for (let i = 0; i < av.length; i++) {
        if (av[i] !== bv[i]) {
          return false;
        }
      }
    } else if (av !== bv) {
      return false;
    }
  }
  return true;
}

// True if any non-view sim-relevant field differs (populations or
// `hideSubHorizon`). Composed with `didViewChange` at the call site to
// decide whether a SimParams change should reset the halo accumulation;
// UI-only fields like population `name` don't trigger a reset.
export function didSimulationChange(prev: SimParams, next: SimParams): boolean {
  if (prev === next) {
    return false;
  }
  if (prev.hideSubHorizon !== next.hideSubHorizon) {
    return true;
  }
  if (prev.populations === next.populations) {
    return false;
  }
  if (prev.populations.length !== next.populations.length) {
    return true;
  }
  for (let i = 0; i < prev.populations.length; i++) {
    if (!populationSimEqual(prev.populations[i], next.populations[i])) {
      return true;
    }
  }
  return false;
}
