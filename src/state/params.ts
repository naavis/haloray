import { PRESETS, type CrystalPopulation } from "./populations";
export type { CrystalPopulation } from "./populations";

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
  selectedPopIndex: number;
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
  selectedPopIndex: 0,
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
