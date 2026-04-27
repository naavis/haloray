import type { DisplayParams, SimParams } from "./params";
import type { SkyState } from "../engine/hosek-wilkie-sky/calculate";

// Byte sizes of the WGSL uniform structs. Each must match its corresponding
// shader struct: `Params` in raytrace.wgsl, `DisplayParams` in display.wgsl,
// `AccParams` in accumulate.wgsl, `SkyParams` in sky.wgsl, `GuidesParams` in
// guides.wgsl.
// 128 base bytes + 16 (atmosphere_enabled u32 + vec4 padding)
// + 128 (sun_spectrum: array<vec4f, 8>, 31 of 32 lanes used) = 272 bytes.
export const PARAMS_SIZE = 272;
export const DISPLAY_PARAMS_SIZE = 32;
export const ACC_PARAMS_SIZE = 16;
export const SKY_PARAMS_SIZE = 192;
export const GUIDES_PARAMS_SIZE = 32;

const degToRad = (d: number) => (d * Math.PI) / 180;

// Mirrors desktop Camera::getFocalLength (camera.cpp:8-26). The shaders
// (raytrace, sky, guides) take a focal length, but the UI tracks FOV in
// degrees to match the desktop UX, so we convert at encode time.
export function fovDegToFocalLength(fovDeg: number, projection: string): number {
  const fovRad = degToRad(fovDeg);
  switch (projection) {
    case "0": // Stereographic
      return 1 / (4 * Math.tan(fovRad / 4));
    case "1": // Rectilinear
      return 1 / (2 * Math.tan(fovRad / 2));
    case "2": // Equidistant
      return 1 / fovRad;
    case "3": // Equal area
      return 1 / (4 * Math.sin(fovRad / 4));
    case "4": // Orthographic
      return 1 / (2 * Math.sin(fovRad / 2));
    default:
      return 1 / fovRad;
  }
}

// Mirrors desktop Camera::getMaximumFov (camera.cpp:28-41). Some projections
// can't represent a full hemisphere/sphere, so the slider needs a tighter cap.
export function getMaxFovDeg(projection: string): number {
  switch (projection) {
    case "0": // Stereographic
      return 350;
    case "1": // Rectilinear
      return 160;
    case "4": // Orthographic
      return 180;
    default:
      return 360;
  }
}

// Serializes SimParams into the 128-byte layout expected by raytrace.wgsl's
// `Params` uniform. Each slot below is 4 bytes; u32[i] and f32[i] alias the
// same bytes, so we pick the view that matches the shader field's type.
// Angles are stored in radians (the UI works in degrees).
export function encodeSimParams(
  outBuf: ArrayBuffer,
  sim: SimParams,
  canvasWidth: number,
  canvasHeight: number,
  rngSeed: number,
  sunSpectrum: Float32Array | null,
): void {
  const u32 = new Uint32Array(outBuf);
  const f32 = new Float32Array(outBuf);

  u32[0] = rngSeed;
  f32[1] = degToRad(sim.sunAlt);
  f32[2] = degToRad(sim.sunDiam);
  f32[3] = sim.caRatio;
  f32[4] = sim.caRatioStd;
  u32[5] = sim.tiltGaussian ? 1 : 0;
  f32[6] = degToRad(sim.tiltAvg);
  f32[7] = degToRad(sim.tiltStd);
  u32[8] = sim.rotGaussian ? 1 : 0;
  f32[9] = degToRad(sim.rotAvg);
  f32[10] = degToRad(sim.rotStd);
  f32[11] = degToRad(sim.camPitch);
  f32[12] = degToRad(sim.camYaw);
  f32[13] = fovDegToFocalLength(sim.camFov, sim.projection);
  u32[14] = parseInt(sim.projection, 10);
  u32[15] = sim.hideSubHorizon ? 1 : 0;
  u32[16] = canvasWidth;
  u32[17] = canvasHeight;
  // Upper and lower pyramidal apex caps (angle + height avg/std). Zero angle
  // disables the cap in the shader, leaving a plain hexagonal prism.
  // Not yet exposed in the UI.
  f32[18] = 0;
  f32[19] = 0;
  f32[20] = 0;
  f32[21] = 0;
  f32[22] = 0;
  f32[23] = 0;
  // Six prism face distances (`prism_distances` in the shader, declared as
  // array<vec4f, 2>). Equal values produce a regular hexagonal cross-section.
  // Not yet exposed in the UI.
  f32[24] = 1;
  f32[25] = 1;
  f32[26] = 1;
  f32[27] = 1;
  f32[28] = 1;
  f32[29] = 1;
  // Unused tail of the array<vec4f, 2> (8 floats total, only 6 are meaningful).
  f32[30] = 0;
  f32[31] = 0;

  // atmosphere_enabled (u32) + 12 bytes padding to vec4 alignment.
  u32[32] = sunSpectrum ? 1 : 0;
  u32[33] = 0;
  u32[34] = 0;
  u32[35] = 0;

  // sun_spectrum: array<vec4f, 8> starting at f32[36]. 31 samples written
  // into the first 31 lanes; lane 31 (f32[67]) stays zero.
  if (sunSpectrum) {
    for (let i = 0; i < 31; i++) {
      f32[36 + i] = sunSpectrum[i];
    }
    f32[67] = 0;
  } else {
    for (let i = 36; i < 68; i++) {
      f32[i] = 0;
    }
  }
}

// Serializes sky params into the 192-byte layout expected by sky.wgsl's
// `SkyParams` uniform: a small view header (resolution, projection, camera,
// sun altitude), the per-channel Hosek-Wilkie radiance scales, and the 27
// configuration coefficients packed as `array<vec4<f32>, 9>` (one vec4 per
// coefficient index, holding the X, Y, Z values for that index plus a
// padding lane).
export function encodeSkyParams(
  outBuf: ArrayBuffer,
  sim: SimParams,
  sky: SkyState,
  turbidity: number,
  canvasWidth: number,
  canvasHeight: number,
): void {
  const u32 = new Uint32Array(outBuf);
  const f32 = new Float32Array(outBuf);
  u32[0] = canvasWidth;
  u32[1] = canvasHeight;
  u32[2] = parseInt(sim.projection, 10);
  f32[3] = degToRad(sim.sunAlt);
  f32[4] = degToRad(sim.camPitch);
  f32[5] = degToRad(sim.camYaw);
  f32[6] = fovDegToFocalLength(sim.camFov, sim.projection);
  f32[7] = turbidity;
  f32[8] = sky.radianceX;
  f32[9] = sky.radianceY;
  f32[10] = sky.radianceZ;
  // f32[11] padding to vec4 boundary
  for (let i = 0; i < 9; i++) {
    const base = 12 + i * 4;
    f32[base + 0] = sky.configX[i];
    f32[base + 1] = sky.configY[i];
    f32[base + 2] = sky.configZ[i];
    // f32[base + 3] padding to vec4 boundary
  }
}

// Serializes guides params into the 32-byte layout expected by guides.wgsl's
// `GuidesParams` uniform.
export function encodeGuidesParams(
  outBuf: ArrayBuffer,
  sim: SimParams,
  canvasWidth: number,
  canvasHeight: number,
): void {
  const u32 = new Uint32Array(outBuf);
  const f32 = new Float32Array(outBuf);
  u32[0] = canvasWidth;
  u32[1] = canvasHeight;
  f32[2] = degToRad(sim.camPitch);
  f32[3] = degToRad(sim.camYaw);
  f32[4] = fovDegToFocalLength(sim.camFov, sim.projection);
  u32[5] = parseInt(sim.projection, 10);
  f32[6] = degToRad(sim.sunAlt);
}

// Serializes display params into the layout expected by display.wgsl's
// `DisplayParams` uniform.
export function encodeDisplayParams(
  outBuf: ArrayBuffer,
  display: DisplayParams,
  totalRays: number,
  canvasWidth: number,
  canvasHeight: number,
  fovDeg: number,
): void {
  const f32 = new Float32Array(outBuf);
  const u32 = new Uint32Array(outBuf);
  f32[0] = totalRays;
  f32[1] = canvasWidth;
  f32[2] = canvasHeight;
  f32[3] = display.brightness;
  u32[4] = display.showGuides ? 1 : 0;
  u32[5] = display.showSky ? 1 : 0;
  f32[6] = fovDeg;
}

// Serializes accumulate params into the layout expected by accumulate.wgsl's
// `AccParams` uniform. Only canvas width is needed (the shader maps pixel
// coordinates into the accumulation buffer). The uniform buffer is 16 bytes
// to satisfy WebGPU alignment even though only 4 are used.
export function encodeAccParams(outBuf: ArrayBuffer, canvasWidth: number): void {
  const u32 = new Uint32Array(outBuf);
  u32[0] = canvasWidth;
}
