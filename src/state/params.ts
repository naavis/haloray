export type Projection = "0" | "1" | "2" | "3" | "4";

export type SimParams = {
  sunAlt: number;
  sunDiam: number;
  caRatio: number;
  caRatioStd: number;
  tiltGaussian: boolean;
  tiltAvg: number;
  tiltStd: number;
  rotGaussian: boolean;
  rotAvg: number;
  rotStd: number;
  camPitch: number;
  camYaw: number;
  camFov: number;
  projection: Projection;
};

export type DisplayParams = {
  brightness: number;
  showGuides: boolean;
};

export const DEFAULT_SIM: SimParams = {
  sunAlt: 30,
  sunDiam: 0.5,
  caRatio: 3.0,
  caRatioStd: 0,
  tiltGaussian: true,
  tiltAvg: 90.0,
  tiltStd: 1.5,
  rotGaussian: false,
  rotAvg: 0,
  rotStd: 0,
  camPitch: 30,
  camYaw: 15,
  camFov: 0.5,
  projection: "0",
};

export const DEFAULT_DISPLAY: DisplayParams = {
  brightness: 1,
  showGuides: false,
};

// "View" = observer's perspective + celestial geometry (camera + sun position).
// These params affect the sky background and guides, not just the halo.
const VIEW_KEYS: (keyof SimParams)[] = ["sunAlt", "camPitch", "camYaw", "camFov", "projection"];

export function didViewChange(prev: SimParams, next: SimParams): boolean {
  return VIEW_KEYS.some((k) => prev[k] !== next[k]);
}
