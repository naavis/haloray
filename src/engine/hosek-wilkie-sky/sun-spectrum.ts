// Hosek-Wilkie spectral solar-radiance evaluation. Mirrors the relevant parts
// of desktop's ArHosekSkyModel.c (arhosekskymodel_solar_radiance_plain +
// arhosekskymodel_sr_internal) and the spectrum normalization used in
// SkyModel::Create (skyModel.cpp). The result is the per-wavelength weight
// fed into the raytracer's spectral->sRGB integration.
import { solarDatasets } from "./dataset-solar";

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
