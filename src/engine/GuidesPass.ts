import guidesCode from "../shaders/guides.wgsl?raw";
import { GUIDES_PARAMS_SIZE, encodeGuidesParams } from "../state/encodeParams";
import type { SimParams } from "../state/params";

export class GuidesPass {
  private device: GPUDevice;

  private pipeline: GPUComputePipeline;
  private uniformBuffer: GPUBuffer;
  private paramsBuf = new ArrayBuffer(GUIDES_PARAMS_SIZE);
  private bindGroup: GPUBindGroup | null = null;
  private dirty = true;

  constructor(device: GPUDevice) {
    this.device = device;

    this.pipeline = device.createComputePipeline({
      layout: "auto",
      compute: {
        module: device.createShaderModule({ code: guidesCode }),
        entryPoint: "main",
      },
    });

    this.uniformBuffer = device.createBuffer({
      size: GUIDES_PARAMS_SIZE,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
  }

  createBindGroups(guidesBuffer: GPUBuffer): void {
    this.bindGroup = this.device.createBindGroup({
      layout: this.pipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.uniformBuffer } },
        { binding: 1, resource: { buffer: guidesBuffer } },
      ],
    });
  }

  markDirty(): void {
    this.dirty = true;
  }

  /** Encodes guides compute dispatch if dirty. Returns true if work was dispatched. */
  encode(
    encoder: GPUCommandEncoder,
    sim: SimParams,
    canvasWidth: number,
    canvasHeight: number,
  ): boolean {
    if (!this.dirty || !this.bindGroup) {
      return false;
    }
    this.dirty = false;

    encodeGuidesParams(this.paramsBuf, sim, canvasWidth, canvasHeight);
    this.device.queue.writeBuffer(this.uniformBuffer, 0, this.paramsBuf);

    const pass = encoder.beginComputePass();
    pass.setPipeline(this.pipeline);
    pass.setBindGroup(0, this.bindGroup);
    pass.dispatchWorkgroups(Math.ceil(canvasWidth / 8), Math.ceil(canvasHeight / 8));
    pass.end();

    return true;
  }
}
