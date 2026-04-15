import type { DisplayParams, SimParams } from "./params"

export const PARAMS_SIZE = 128
export const DISPLAY_PARAMS_SIZE = 16

const degToRad = (d: number) => (d * Math.PI) / 180

export function encodeSimParams(
  buf: ArrayBuffer,
  sim: SimParams,
  canvasWidth: number,
  canvasHeight: number,
  rngSeed: number,
): void {
  const u32 = new Uint32Array(buf)
  const f32 = new Float32Array(buf)

  u32[0] = rngSeed
  f32[1] = degToRad(sim.sunAlt)
  f32[2] = degToRad(sim.sunDiam)
  f32[3] = sim.caRatio
  f32[4] = sim.caRatioStd
  u32[5] = sim.tiltGaussian ? 1 : 0
  f32[6] = degToRad(sim.tiltAvg)
  f32[7] = degToRad(sim.tiltStd)
  u32[8] = sim.rotGaussian ? 1 : 0
  f32[9] = degToRad(sim.rotAvg)
  f32[10] = degToRad(sim.rotStd)
  f32[11] = degToRad(sim.camPitch)
  f32[12] = degToRad(sim.camYaw)
  f32[13] = sim.camFov
  u32[14] = parseInt(sim.projection, 10)
  u32[15] = 0
  u32[16] = canvasWidth
  u32[17] = canvasHeight
  f32[18] = 0
  f32[19] = 0
  f32[20] = 0
  f32[21] = 0
  f32[22] = 0
  f32[23] = 0
  f32[24] = 1
  f32[25] = 1
  f32[26] = 1
  f32[27] = 1
  f32[28] = 1
  f32[29] = 1
  f32[30] = 0
  f32[31] = 0
}

export function encodeDisplayParams(
  buf: ArrayBuffer,
  display: DisplayParams,
  totalRays: number,
  canvasWidth: number,
  canvasHeight: number,
): void {
  const f32 = new Float32Array(buf)
  f32[0] = totalRays
  f32[1] = canvasWidth
  f32[2] = canvasHeight
  f32[3] = display.brightness
}
