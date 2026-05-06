import raytraceCode from "../shaders/raytrace.wgsl?raw";
import accumulateCode from "../shaders/accumulate.wgsl?raw";
import {
  ACC_PARAMS_SIZE,
  PARAMS_SIZE,
  encodeAccParams,
  encodeSimParams,
} from "../state/encodeParams";
import type { SimParams } from "../state/params";
import type { CrystalPopulation } from "../state/populations";

const WORKGROUP_SIZE = 64;
const RAYS_PER_STEP = 500_000;
const NUM_WORKGROUPS = Math.ceil(RAYS_PER_STEP / WORKGROUP_SIZE);
const ACTUAL_RAYS = NUM_WORKGROUPS * WORKGROUP_SIZE;
const RAY_RESULT_STRIDE = 20;
export const MAX_TOTAL_RAYS = 250_000_000;
// One params buffer + bind group per slot so per-population queue.writeBuffer
// calls target distinct GPU memory — all writeBuffers execute before the
// command encoder's submit, so a shared buffer would have every raytrace pass
// see only the last population's params.
const MAX_POPULATIONS = 8;

export class HaloPass {
  private device: GPUDevice;

  private raytracePipeline: GPUComputePipeline;
  private accumulatePipeline: GPUComputePipeline;

  // One buffer + staging ArrayBuffer + bind group per population slot.
  private paramsBuffers: GPUBuffer[];
  private paramsBufs: ArrayBuffer[];
  private rayBuffer: GPUBuffer;
  private accParamsBuffer: GPUBuffer;
  private accParamsBuf = new ArrayBuffer(ACC_PARAMS_SIZE);

  private raytraceBindGroups: (GPUBindGroup | null)[];
  private accumulateBindGroup: GPUBindGroup | null = null;
  private accBuffer: GPUBuffer | null = null;

  private _totalRays = 0;
  private rngSeed = 0;
  private resetRequested = false;
  private sunSpectrum: Float32Array | null = null;

  constructor(device: GPUDevice) {
    this.device = device;

    this.raytracePipeline = device.createComputePipeline({
      layout: "auto",
      compute: {
        module: device.createShaderModule({ code: raytraceCode }),
        entryPoint: "main",
      },
    });

    this.accumulatePipeline = device.createComputePipeline({
      layout: "auto",
      compute: {
        module: device.createShaderModule({ code: accumulateCode }),
        entryPoint: "main",
      },
    });

    this.paramsBuffers = Array.from({ length: MAX_POPULATIONS }, () =>
      device.createBuffer({
        size: PARAMS_SIZE,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
      }),
    );
    this.paramsBufs = Array.from({ length: MAX_POPULATIONS }, () => new ArrayBuffer(PARAMS_SIZE));
    this.raytraceBindGroups = Array<GPUBindGroup | null>(MAX_POPULATIONS).fill(null);

    this.rayBuffer = device.createBuffer({
      size: ACTUAL_RAYS * RAY_RESULT_STRIDE,
      usage: GPUBufferUsage.STORAGE,
    });

    this.accParamsBuffer = device.createBuffer({
      size: ACC_PARAMS_SIZE,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
  }

  get totalRays(): number {
    return this._totalRays;
  }

  get maxRaysReached(): boolean {
    return this._totalRays >= MAX_TOTAL_RAYS;
  }

  createBindGroups(accBuffer: GPUBuffer): void {
    this.accBuffer = accBuffer;
    for (let i = 0; i < MAX_POPULATIONS; i++) {
      this.raytraceBindGroups[i] = this.device.createBindGroup({
        layout: this.raytracePipeline.getBindGroupLayout(0),
        entries: [
          { binding: 0, resource: { buffer: this.paramsBuffers[i] } },
          { binding: 1, resource: { buffer: this.rayBuffer } },
        ],
      });
    }
    this.accumulateBindGroup = this.device.createBindGroup({
      layout: this.accumulatePipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.rayBuffer } },
        { binding: 1, resource: { buffer: accBuffer } },
        { binding: 2, resource: { buffer: this.accParamsBuffer } },
      ],
    });
  }

  writeAccParams(canvasWidth: number): void {
    encodeAccParams(this.accParamsBuf, canvasWidth);
    this.device.queue.writeBuffer(this.accParamsBuffer, 0, this.accParamsBuf);
  }

  resetAccumulation(): void {
    this.resetRequested = true;
  }

  setSunSpectrum(spectrum: Float32Array | null): void {
    this.sunSpectrum = spectrum;
  }

  /** Encodes raytrace + accumulate commands. Returns true if work was dispatched. */
  encode(
    encoder: GPUCommandEncoder,
    simParams: SimParams,
    populations: CrystalPopulation[],
    canvasWidth: number,
    canvasHeight: number,
  ): boolean {
    if (this.resetRequested) {
      this.resetRequested = false;
      this._totalRays = 0;
      if (this.accBuffer) {
        encoder.clearBuffer(this.accBuffer);
      }
    }

    if (this.maxRaysReached || !this.accumulateBindGroup) {
      return false;
    }

    const enabled = populations.filter((p) => p.enabled && p.weight > 0);
    if (enabled.length === 0) {
      return false;
    }

    const weightSum = enabled.reduce((s, p) => s + p.weight, 0);

    // Allocate workgroups proportionally, distributing the rounding remainder
    // to the highest-weight populations so the total always equals NUM_WORKGROUPS.
    const wgs = enabled.map((p) => Math.floor((p.weight / weightSum) * NUM_WORKGROUPS));
    let remainder = NUM_WORKGROUPS - wgs.reduce((s, w) => s + w, 0);
    const order = enabled.map((_, i) => i).sort((a, b) => enabled[b].weight - enabled[a].weight);
    for (let k = 0; k < remainder; k++) {
      wgs[order[k % order.length]]++;
    }
    remainder = 0; // consumed

    let dispatched = false;
    for (let i = 0; i < enabled.length; i++) {
      const wg = wgs[i];
      if (wg === 0) {
        continue;
      }

      const slot = i % MAX_POPULATIONS;
      const bindGroup = this.raytraceBindGroups[slot];
      if (!bindGroup) {
        continue;
      }

      this.rngSeed++;
      encodeSimParams(
        this.paramsBufs[slot],
        simParams,
        enabled[i],
        canvasWidth,
        canvasHeight,
        this.rngSeed,
        this.sunSpectrum,
      );
      this.device.queue.writeBuffer(this.paramsBuffers[slot], 0, this.paramsBufs[slot]);
      this._totalRays += wg * WORKGROUP_SIZE;

      const rp = encoder.beginComputePass();
      rp.setPipeline(this.raytracePipeline);
      rp.setBindGroup(0, bindGroup);
      rp.dispatchWorkgroups(wg);
      rp.end();

      const ap = encoder.beginComputePass();
      ap.setPipeline(this.accumulatePipeline);
      ap.setBindGroup(0, this.accumulateBindGroup);
      ap.dispatchWorkgroups(wg);
      ap.end();

      dispatched = true;
    }

    return dispatched;
  }
}
