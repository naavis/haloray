// Hosek-Wilkie spectral solar-radiance evaluation. Mirrors the relevant parts
// of desktop's ArHosekSkyModel.c (arhosekskymodel_solar_radiance_plain +
// arhosekskymodel_sr_internal) and the spectrum normalization used in
// SkyModel::Create (skyModel.cpp). The result is the per-wavelength weight
// fed into the raytracer's spectral->sRGB integration.
import { solarDatasets } from "./dataset-solar";
import { LIMB_DARKENING_DATASETS } from "./dataset-solar-limb-darkening";

const PIECES = 45;
const ORDER = 4;
// Empirical normalization from desktop SkyModel::Create — keeps the halo
// brightness compatible with the sky dome's radiance scale.
const SPECTRUM_NORMALIZER = 30663.7;

// 31 CIE wavelengths (400..700 nm @ 10 nm) used for spectral->XYZ integration
// in the raytrace shader.
export const CIE_WAVELENGTHS = Float32Array.from({ length: 31 }, (_, i) => 400 + i * 10);

function srInternal(turbidity: number, wlIndex: number, elevation: number): number {
  let pos = Math.floor(Math.pow((2 * elevation) / Math.PI, 1 / 3) * PIECES);
  if (pos > PIECES - 1) {
    pos = PIECES - 1;
  }

  const breakX = Math.pow(pos / PIECES, 3) * (Math.PI * 0.5);
  const dataset = solarDatasets[wlIndex];
  // C original walks coefs backwards starting at `order * pieces * turbidity
  // + order * (pos+1) - 1`, accumulating res += x_exp * *coefs--; x_exp *= x.
  const baseEnd = ORDER * PIECES * turbidity + ORDER * (pos + 1) - 1;
  const x = elevation - breakX;
  let res = 0;
  let xExp = 1;
  for (let i = 0; i < ORDER; i++) {
    res += xExp * dataset[baseEnd - i];
    xExp *= x;
  }
  return res;
}

function solarRadiancePlain(turbidity: number, elevation: number, wavelength: number): number {
  let turbLow = Math.floor(turbidity) - 1;
  let turbFrac = turbidity - (turbLow + 1);
  if (turbLow === 9) {
    turbLow = 8;
    turbFrac = 1;
  }

  let wlLow = Math.floor((wavelength - 320) / 40);
  let wlFrac = ((wavelength - 320) % 40) / 40;
  if (wlLow === 10) {
    wlLow = 9;
    wlFrac = 1;
  }

  const a = srInternal(turbLow, wlLow, elevation);
  const b = srInternal(turbLow, wlLow + 1, elevation);
  const c = srInternal(turbLow + 1, wlLow, elevation);
  const d = srInternal(turbLow + 1, wlLow + 1, elevation);

  return (
    (1 - turbFrac) * ((1 - wlFrac) * a + wlFrac * b) + turbFrac * ((1 - wlFrac) * c + wlFrac * d)
  );
}

// CIE 1931 2° standard observer color matching functions, sampled at the
// 31 wavelengths in CIE_WAVELENGTHS (400-700nm @ 10nm). Values copied from
// desktop colorUtilities.h. The analytic Gaussian approximations used in
// raytrace.wgsl are not accurate enough here — they smooth over the bimodal
// X CMF and noticeably under-weight red contributions at low sun elevation,
// which makes the sun look yellow instead of orange near the horizon.
const CIE_X = [
  0.01431, 0.04351, 0.13438, 0.2839, 0.34828, 0.3362, 0.2908, 0.19536, 0.09564, 0.03201, 0.0049,
  0.0093, 0.06327, 0.1655, 0.2904, 0.4334499, 0.5945, 0.7621, 0.9163, 1.0263, 1.0622, 1.0026,
  0.8544499, 0.6424, 0.4479, 0.2835, 0.1649, 0.0874, 0.04677, 0.0227, 0.01135916,
];
const CIE_Y = [
  0.000396, 0.00121, 0.004, 0.0116, 0.023, 0.038, 0.06, 0.09098, 0.13902, 0.20802, 0.323, 0.503,
  0.71, 0.862, 0.954, 0.9949501, 0.995, 0.952, 0.87, 0.757, 0.631, 0.503, 0.381, 0.265, 0.175,
  0.107, 0.061, 0.032, 0.017, 0.00821, 0.004102,
];
const CIE_Z = [
  0.06785001, 0.2074, 0.6456, 1.3856, 1.74706, 1.77211, 1.6692, 1.28764, 0.8129501, 0.46518, 0.272,
  0.1582, 0.07824999, 0.04216, 0.0203, 0.00874999, 0.0039, 0.0021, 0.00165001, 0.0011, 0.0008,
  0.00034, 0.00019, 0.00005, 0.00002, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
];

// Interpolated limb-darkening factor at disk edge (sampleCosine = 0).
// At sampleCosine=0 the 5th-degree polynomial collapses to its constant term.
function limbDarkeningEdgeFactor(wavelength: number): number {
  let wlLow = Math.floor((wavelength - 320) / 40);
  let wlFrac = ((wavelength - 320) % 40) / 40;
  if (wlLow >= 10) {
    wlLow = 9;
    wlFrac = 1;
  }
  return (
    (1 - wlFrac) * LIMB_DARKENING_DATASETS[wlLow][0] +
    wlFrac * LIMB_DARKENING_DATASETS[wlLow + 1][0]
  );
}

export type SunDiskState = {
  sunTopXYZ: [number, number, number];
  sunBottomXYZ: [number, number, number];
  limbDarkeningScaler: [number, number, number];
  solarRadius: number;
};

// Computes the CIE XYZ values for the top and bottom of the solar disk and the
// per-channel limb-darkening scaler. Mirrors SkyModel::Create in skyModel.cpp.
// solarElevation may be negative (used for partial below-horizon occlusion);
// spectral lookups are clamped to >= 0 internally.
export function buildSunDiskState(
  turbidity: number,
  solarElevation: number,
  solarRadius: number,
): SunDiskState {
  const topElevation = Math.max(solarElevation + solarRadius, 0);
  const bottomElevation = Math.max(solarElevation - solarRadius, 0);

  const topXYZ: [number, number, number] = [0, 0, 0];
  const bottomXYZ: [number, number, number] = [0, 0, 0];
  const topLDXYZ: [number, number, number] = [0, 0, 0];

  for (let i = 0; i < 31; i++) {
    const wl = CIE_WAVELENGTHS[i];
    const topR = solarRadiancePlain(turbidity, topElevation, wl);
    const bottomR = solarRadiancePlain(turbidity, bottomElevation, wl);
    const ld = limbDarkeningEdgeFactor(wl);
    const cx = CIE_X[i],
      cy = CIE_Y[i],
      cz = CIE_Z[i];

    topXYZ[0] += topR * cx;
    topXYZ[1] += topR * cy;
    topXYZ[2] += topR * cz;
    bottomXYZ[0] += bottomR * cx;
    bottomXYZ[1] += bottomR * cy;
    bottomXYZ[2] += bottomR * cz;
    topLDXYZ[0] += topR * ld * cx;
    topLDXYZ[1] += topR * ld * cy;
    topLDXYZ[2] += topR * ld * cz;
  }

  const limbDarkeningScaler: [number, number, number] = [
    topXYZ[0] > 0 ? topLDXYZ[0] / topXYZ[0] : 1,
    topXYZ[1] > 0 ? topLDXYZ[1] / topXYZ[1] : 1,
    topXYZ[2] > 0 ? topLDXYZ[2] / topXYZ[2] : 1,
  ];

  return { sunTopXYZ: topXYZ, sunBottomXYZ: bottomXYZ, limbDarkeningScaler, solarRadius };
}

// Builds the 31-sample sun spectrum (400..700 nm) used by the raytracer when
// the atmosphere is enabled. Matches desktop SkyModel::Create:
//   sunSpectrum[i] = solarRadiancePlain(turbidity, max(elevation, 0), wl) / 30663.7
// A linear fade is applied when the sun is below the horizon, matching the
// sky shader's fade from full at 0° to black at -10° (MIN_SUN_ELEVATION).
const MIN_SUN_ELEVATION = -(10 * Math.PI) / 180;
export function buildSunSpectrum(turbidity: number, solarElevation: number): Float32Array {
  const fade =
    solarElevation >= 0
      ? 1
      : solarElevation <= MIN_SUN_ELEVATION
        ? 0
        : (solarElevation - MIN_SUN_ELEVATION) / -MIN_SUN_ELEVATION;
  const e = Math.max(solarElevation, 0);
  const out = new Float32Array(31);
  for (let i = 0; i < 31; i++) {
    out[i] = (fade * solarRadiancePlain(turbidity, e, CIE_WAVELENGTHS[i])) / SPECTRUM_NORMALIZER;
  }
  return out;
}
