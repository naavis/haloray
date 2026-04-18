import skyCode from "../shaders/sky.wgsl?raw";
import { SKY_PARAMS_SIZE, encodeSkyParams } from "../state/encodeParams";

export class SkyPass {
  private device: GPUDevice;

  private pipeline: GPUComputePipeline;
  private uniformBuffer: GPUBuffer;
  private paramsBuf = new ArrayBuffer(SKY_PARAMS_SIZE);
  private bindGroup: GPUBindGroup | null = null;
  private dirty = true;

  constructor(device: GPUDevice) {
    this.device = device;

    this.pipeline = device.createComputePipeline({
      layout: "auto",
      compute: {
        module: device.createShaderModule({ code: skyCode }),
        entryPoint: "main",
      },
    });

    this.uniformBuffer = device.createBuffer({
      size: SKY_PARAMS_SIZE,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
  }

  createBindGroups(skyBuffer: GPUBuffer): void {
    this.bindGroup = this.device.createBindGroup({
      layout: this.pipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.uniformBuffer } },
        { binding: 1, resource: { buffer: skyBuffer } },
      ],
    });
  }

  markDirty(): void {
    this.dirty = true;
  }

  /** Encodes sky compute dispatch if dirty. Returns true if work was dispatched. */
  // TODO: accept SimParams (sun altitude, camera, projection) once the shader
  // computes real sky colors.
  encode(
    encoder: GPUCommandEncoder,
    canvasWidth: number,
    canvasHeight: number,
  ): boolean {
    if (!this.dirty || !this.bindGroup) return false;
    this.dirty = false;

    encodeSkyParams(this.paramsBuf, canvasWidth, canvasHeight);
    this.device.queue.writeBuffer(this.uniformBuffer, 0, this.paramsBuf);

    const pass = encoder.beginComputePass();
    pass.setPipeline(this.pipeline);
    pass.setBindGroup(0, this.bindGroup);
    pass.dispatchWorkgroups(
      Math.ceil(canvasWidth / 8),
      Math.ceil(canvasHeight / 8),
    );
    pass.end();

    return true;
  }
}
