import type { DisplayParams, SimParams } from "./params";

// Byte sizes of the WGSL uniform structs. Must match the `Params` struct in
// raytrace.wgsl and the `DisplayParams`/`AccParams` structs in display.wgsl /
// accumulate.wgsl (those two shaders bind the same buffer, so their struct
// layouts must be identical).
export const PARAMS_SIZE = 128;
export const DISPLAY_PARAMS_SIZE = 32;
export const SKY_PARAMS_SIZE = 8;
export const GUIDES_PARAMS_SIZE = 32;

const degToRad = (d: number) => (d * Math.PI) / 180;

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
  f32[13] = sim.camFov;
  u32[14] = parseInt(sim.projection, 10);
  // camera_hide_sub_horizon: sub-horizon ray filter, not yet exposed in the UI.
  u32[15] = 0;
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
}

// Serializes sky params into the 8-byte layout expected by sky.wgsl's
// `SkyParams` uniform.
// TODO: add sun altitude, camera pitch/yaw/fov, and projection once the sky
// shader computes actual sky colors instead of writing black.
export function encodeSkyParams(
  outBuf: ArrayBuffer,
  canvasWidth: number,
  canvasHeight: number,
): void {
  const u32 = new Uint32Array(outBuf);
  u32[0] = canvasWidth;
  u32[1] = canvasHeight;
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
  f32[4] = sim.camFov;
  u32[5] = parseInt(sim.projection, 10);
  f32[6] = degToRad(sim.sunAlt);
}

// Serializes display/accumulate params into the 16-byte layout shared by
// DisplayParams (display.wgsl) and AccParams (accumulate.wgsl).  Both shaders
// bind the same GPU buffer, so this single write feeds both passes.
export function encodeDisplayParams(
  outBuf: ArrayBuffer,
  display: DisplayParams,
  totalRays: number,
  canvasWidth: number,
  canvasHeight: number,
): void {
  const f32 = new Float32Array(outBuf);
  const u32 = new Uint32Array(outBuf);
  f32[0] = totalRays;
  f32[1] = canvasWidth;
  f32[2] = canvasHeight;
  f32[3] = display.brightness;
  u32[4] = display.showGuides ? 1 : 0;
}
