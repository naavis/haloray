export type Projection = "0" | "1" | "2" | "3" | "4"

export type SimParams = {
  sunAlt: number
  sunDiam: number
  caRatio: number
  caRatioStd: number
  tiltGaussian: boolean
  tiltAvg: number
  tiltStd: number
  rotGaussian: boolean
  rotAvg: number
  rotStd: number
  camPitch: number
  camYaw: number
  camFov: number
  projection: Projection
}

export type DisplayParams = {
  brightness: number
}

export const DEFAULT_SIM: SimParams = {
  sunAlt: 15,
  sunDiam: 0.5,
  caRatio: 0.5,
  caRatioStd: 0,
  tiltGaussian: false,
  tiltAvg: 0,
  tiltStd: 0,
  rotGaussian: false,
  rotAvg: 0,
  rotStd: 0,
  camPitch: 0,
  camYaw: 0,
  camFov: 1,
  projection: "0",
}

export const DEFAULT_DISPLAY: DisplayParams = {
  brightness: 1,
}
