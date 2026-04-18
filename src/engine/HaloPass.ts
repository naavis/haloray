import raytraceCode from "../shaders/raytrace.wgsl?raw";
import accumulateCode from "../shaders/accumulate.wgsl?raw";
import { PARAMS_SIZE, encodeSimParams } from "../state/encodeParams";
import type { SimParams } from "../state/params";

const WORKGROUP_SIZE = 64;
const RAYS_PER_STEP = 500_000;
const NUM_WORKGROUPS = Math.ceil(RAYS_PER_STEP / WORKGROUP_SIZE);
const ACTUAL_RAYS = NUM_WORKGROUPS * WORKGROUP_SIZE;
const RAY_RESULT_STRIDE = 20;
const MAX_TOTAL_RAYS = 1_000_000_000;

export class HaloPass {
  private device: GPUDevice;

  private raytracePipeline: GPUComputePipeline;
  private accumulatePipeline: GPUComputePipeline;

  private paramsBuffer: GPUBuffer;
  private rayBuffer: GPUBuffer;
  private paramsBuf = new ArrayBuffer(PARAMS_SIZE);

  private raytraceBindGroup: GPUBindGroup | null = null;
  private accumulateBindGroup: GPUBindGroup | null = null;
  private accBuffer: GPUBuffer | null = null;

  private _totalRays = 0;
  private rngSeed = 0;
  private resetRequested = false;

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

    this.paramsBuffer = device.createBuffer({
      size: PARAMS_SIZE,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    this.rayBuffer = device.createBuffer({
      size: ACTUAL_RAYS * RAY_RESULT_STRIDE,
      usage: GPUBufferUsage.STORAGE,
    });
  }

  get totalRays(): number {
    return this._totalRays;
  }

  get maxRaysReached(): boolean {
    return this._totalRays >= MAX_TOTAL_RAYS;
  }

  createBindGroups(accBuffer: GPUBuffer, displayParamsBuffer: GPUBuffer): void {
    this.accBuffer = accBuffer;
    this.raytraceBindGroup = this.device.createBindGroup({
      layout: this.raytracePipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.paramsBuffer } },
        { binding: 1, resource: { buffer: this.rayBuffer } },
      ],
    });
    this.accumulateBindGroup = this.device.createBindGroup({
      layout: this.accumulatePipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.rayBuffer } },
        { binding: 1, resource: { buffer: accBuffer } },
        { binding: 2, resource: { buffer: displayParamsBuffer } },
      ],
    });
  }

  resetAccumulation(): void {
    this.resetRequested = true;
  }

  /** Encodes raytrace + accumulate commands. Returns true if work was dispatched. */
  encode(
    encoder: GPUCommandEncoder,
    simParams: SimParams,
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

    if (this.maxRaysReached || !this.raytraceBindGroup || !this.accumulateBindGroup) {
      return false;
    }

    this.rngSeed++;
    encodeSimParams(this.paramsBuf, simParams, canvasWidth, canvasHeight, this.rngSeed);
    this.device.queue.writeBuffer(this.paramsBuffer, 0, this.paramsBuf);
    this._totalRays += ACTUAL_RAYS;

    const rp = encoder.beginComputePass();
    rp.setPipeline(this.raytracePipeline);
    rp.setBindGroup(0, this.raytraceBindGroup);
    rp.dispatchWorkgroups(NUM_WORKGROUPS);
    rp.end();

    const ap = encoder.beginComputePass();
    ap.setPipeline(this.accumulatePipeline);
    ap.setBindGroup(0, this.accumulateBindGroup);
    ap.dispatchWorkgroups(NUM_WORKGROUPS);
    ap.end();

    return true;
  }
}
