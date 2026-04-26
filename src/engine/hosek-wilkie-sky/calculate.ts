import { datasetsXYZ, datasetsXYZRad } from "./dataset-xyz";

function cookConfiguration(
  dataset: Float32Array,
  config: Float32Array,
  turbidity: number,
  albedo: number,
  solarElevation: number,
) {
  // dataset is a flat Float32Array of length 1080 = 2*10*6*9
  // config is an output array of length 9
  // solarElevation in [0, PI/2]
  const elevationK = Math.pow(solarElevation / (Math.PI / 2), 1.0 / 3.0);

  const intTurbidity = Math.max(1, Math.min(9, Math.floor(turbidity)));
  const turbidityRem = turbidity - intTurbidity;

  // Offsets into the flat array:
  //   [albedo][turbidity-1][bezier_cp][coeff]
  //   stride per albedo slice: 10 * 6 * 9 = 540
  //   stride per turbidity:     6 * 9 = 54
  //   stride per control point: 9
  const strideAlbedo = 540;
  const strideTurbidity = 54;
  const strideCP = 9;

  for (let i = 0; i < 9; i++) {
    // Four corners of the (turbidity, albedo) rectangle, each evaluated through the Bézier
    const aL_tL = bezier6(
      dataset,
      0 * strideAlbedo + (intTurbidity - 1) * strideTurbidity,
      strideCP,
      i,
      elevationK,
    );
    const aL_tH = bezier6(
      dataset,
      0 * strideAlbedo + intTurbidity * strideTurbidity,
      strideCP,
      i,
      elevationK,
    );
    const aH_tL = bezier6(
      dataset,
      1 * strideAlbedo + (intTurbidity - 1) * strideTurbidity,
      strideCP,
      i,
      elevationK,
    );
    const aH_tH = bezier6(
      dataset,
      1 * strideAlbedo + intTurbidity * strideTurbidity,
      strideCP,
      i,
      elevationK,
    );

    // Blend: (1 - albedo)(1 - turbidityRem) a_L_t_L + ... etc.
    const t0 = aL_tL * (1.0 - turbidityRem) + aL_tH * turbidityRem;
    const t1 = aH_tL * (1.0 - turbidityRem) + aH_tH * turbidityRem;
    config[i] = t0 * (1.0 - albedo) + t1 * albedo;
  }
}

function bezier6(
  dataset: Float32Array,
  baseOffset: number,
  strideCP: number,
  coeffIdx: number,
  t: number,
) {
  const u = 1.0 - t;
  const t2 = t * t,
    t3 = t2 * t,
    t4 = t3 * t,
    t5 = t4 * t;
  const u2 = u * u,
    u3 = u2 * u,
    u4 = u3 * u,
    u5 = u4 * u;
  return (
    dataset[baseOffset + 0 * strideCP + coeffIdx] * u5 +
    dataset[baseOffset + 1 * strideCP + coeffIdx] * 5.0 * t * u4 +
    dataset[baseOffset + 2 * strideCP + coeffIdx] * 10.0 * t2 * u3 +
    dataset[baseOffset + 3 * strideCP + coeffIdx] * 10.0 * t3 * u2 +
    dataset[baseOffset + 4 * strideCP + coeffIdx] * 5.0 * t4 * u +
    dataset[baseOffset + 5 * strideCP + coeffIdx] * t5
  );
}

function cookRadianceConfiguration(
  dataset: Float32Array,
  turbidity: number,
  albedo: number,
  solarElevation: number,
) {
  // dataset is a flat Float32Array of length 120 = 2*10*6
  //   layout: [albedo][turbidity-1][bezier_cp]
  //   stride per albedo slice: 10 * 6 = 60
  //   stride per turbidity:    6
  //   stride per control point: 1
  // Returns a single scalar (the radiance scale Z for one channel).

  const elevationK = Math.pow(solarElevation / (Math.PI / 2), 1.0 / 3.0);

  const intTurbidity = Math.max(1, Math.min(9, Math.floor(turbidity)));
  const turbidityRem = turbidity - intTurbidity;

  const strideAlbedo = 60;
  const strideTurbidity = 6;

  // Four corners of the (turbidity, albedo) rectangle, each evaluated through the Bézier
  const aL_tL = bezier6Rad(
    dataset,
    0 * strideAlbedo + (intTurbidity - 1) * strideTurbidity,
    elevationK,
  );
  const aL_tH = bezier6Rad(dataset, 0 * strideAlbedo + intTurbidity * strideTurbidity, elevationK);
  const aH_tL = bezier6Rad(
    dataset,
    1 * strideAlbedo + (intTurbidity - 1) * strideTurbidity,
    elevationK,
  );
  const aH_tH = bezier6Rad(dataset, 1 * strideAlbedo + intTurbidity * strideTurbidity, elevationK);

  // Bilinear blend in (turbidity, albedo)
  const t0 = aL_tL * (1.0 - turbidityRem) + aL_tH * turbidityRem;
  const t1 = aH_tL * (1.0 - turbidityRem) + aH_tH * turbidityRem;
  return t0 * (1.0 - albedo) + t1 * albedo;
}

function bezier6Rad(dataset: Float32Array, baseOffset: number, t: number) {
  const u = 1.0 - t;
  const t2 = t * t,
    t3 = t2 * t,
    t4 = t3 * t,
    t5 = t4 * t;
  const u2 = u * u,
    u3 = u2 * u,
    u4 = u3 * u,
    u5 = u4 * u;
  return (
    dataset[baseOffset + 0] * u5 +
    dataset[baseOffset + 1] * 5.0 * t * u4 +
    dataset[baseOffset + 2] * 10.0 * t2 * u3 +
    dataset[baseOffset + 3] * 10.0 * t3 * u2 +
    dataset[baseOffset + 4] * 5.0 * t4 * u +
    dataset[baseOffset + 5] * t5
  );
}

export type SkyState = {
  configX: Float32Array;
  configY: Float32Array;
  configZ: Float32Array;
  radianceX: number;
  radianceY: number;
  radianceZ: number;
};

export function buildSkyState(turbidity: number, albedo: number, solarElevation: number): SkyState {
  const configX = new Float32Array(9);
  const configY = new Float32Array(9);
  const configZ = new Float32Array(9);
  cookConfiguration(datasetsXYZ[0], configX, turbidity, albedo, solarElevation);
  cookConfiguration(datasetsXYZ[1], configY, turbidity, albedo, solarElevation);
  cookConfiguration(datasetsXYZ[2], configZ, turbidity, albedo, solarElevation);

  const radianceX = cookRadianceConfiguration(datasetsXYZRad[0], turbidity, albedo, solarElevation);
  const radianceY = cookRadianceConfiguration(datasetsXYZRad[1], turbidity, albedo, solarElevation);
  const radianceZ = cookRadianceConfiguration(datasetsXYZRad[2], turbidity, albedo, solarElevation);

  return { configX, configY, configZ, radianceX, radianceY, radianceZ };
}
