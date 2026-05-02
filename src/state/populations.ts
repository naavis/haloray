export type PresetKey = "random" | "plate" | "column" | "parry" | "lowitz" | "pyramid";

export type CrystalPopulation = {
  name: string;
  preset: PresetKey;
  enabled: boolean;
  weight: number;

  caRatio: number;
  caRatioStd: number;
  tiltGaussian: boolean;
  tiltAvg: number;
  tiltStd: number;
  rotGaussian: boolean;
  rotAvg: number;
  rotStd: number;

  // Not yet exposed in UI; kept here so the encoder reads them from
  // the population rather than from hardcoded literals.
  upperApexAngle: number;
  upperApexHeightAvg: number;
  upperApexHeightStd: number;
  lowerApexAngle: number;
  lowerApexHeightAvg: number;
  lowerApexHeightStd: number;
  prismDistances: [number, number, number, number, number, number];
};

// Shared apex angle for all presets (56.142°), matching desktop crystalPopulation.cpp.
const APEX_ANGLE = 56.142;
const UNIT_PRISM: [number, number, number, number, number, number] = [1, 1, 1, 1, 1, 1];

function base(
  preset: PresetKey,
  name: string,
  caRatio: number,
  caRatioStd: number,
  tiltGaussian: boolean,
  tiltAvg: number,
  tiltStd: number,
  rotGaussian: boolean,
  rotAvg: number,
  rotStd: number,
  upperApexHeightAvg = 0,
  upperApexHeightStd = 0,
  lowerApexHeightAvg = 0,
  lowerApexHeightStd = 0,
): CrystalPopulation {
  return {
    name,
    preset,
    enabled: true,
    weight: 1,
    caRatio,
    caRatioStd,
    tiltGaussian,
    tiltAvg,
    tiltStd,
    rotGaussian,
    rotAvg,
    rotStd,
    upperApexAngle: APEX_ANGLE,
    upperApexHeightAvg,
    upperApexHeightStd,
    lowerApexAngle: APEX_ANGLE,
    lowerApexHeightAvg,
    lowerApexHeightStd,
    prismDistances: UNIT_PRISM,
  };
}

export const PRESETS: Record<PresetKey, () => CrystalPopulation> = {
  random: () => base("random", "Random", 1.0, 0.1, false, 0, 0, false, 0, 0),
  plate: () => base("plate", "Plate", 0.3, 0.1, true, 0, 1.0, false, 0, 0),
  column: () => base("column", "Column", 2.0, 1.0, true, 90.0, 0.5, false, 0, 0),
  parry: () => base("parry", "Parry", 1.0, 0.1, true, 90.0, 0.5, true, 0, 0.5),
  lowitz: () => base("lowitz", "Lowitz", 1.0, 0.1, false, 0, 0, true, 0, 1.0),
  pyramid: () => base("pyramid", "Pyramid", 0.1, 0.1, false, 0, 0, false, 0, 0, 0.5, 0.1, 0.5, 0.1),
};
