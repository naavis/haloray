import displayCode from "../shaders/display.wgsl?raw";
import { DISPLAY_PARAMS_SIZE, encodeDisplayParams } from "../state/encodeParams";
import type { DisplayParams } from "../state/params";

export class DisplayPass {
  private device: GPUDevice;
  private context: GPUCanvasContext;

  private pipeline: GPURenderPipeline;
  private uniformBuffer: GPUBuffer;
  private paramsBuf = new ArrayBuffer(DISPLAY_PARAMS_SIZE);
  private bindGroup: GPUBindGroup | null = null;
  private dirty = true;

  constructor(device: GPUDevice, context: GPUCanvasContext, format: GPUTextureFormat) {
    this.device = device;
    this.context = context;

    const module = device.createShaderModule({ code: displayCode });
    this.pipeline = device.createRenderPipeline({
      layout: "auto",
      vertex: { module, entryPoint: "vertex_shader" },
      fragment: { module, entryPoint: "fragment_shader", targets: [{ format }] },
    });

    this.uniformBuffer = device.createBuffer({
      size: DISPLAY_PARAMS_SIZE,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
  }

  createBindGroups(accBuffer: GPUBuffer, skyBuffer: GPUBuffer, guidesBuffer: GPUBuffer): void {
    this.bindGroup = this.device.createBindGroup({
      layout: this.pipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: accBuffer } },
        { binding: 1, resource: { buffer: this.uniformBuffer } },
        { binding: 2, resource: { buffer: skyBuffer } },
        { binding: 3, resource: { buffer: guidesBuffer } },
      ],
    });
  }

  markDirty(): void {
    this.dirty = true;
  }

  /** Encodes the display render pass if dirty. Returns true if work was dispatched. */
  encode(
    encoder: GPUCommandEncoder,
    display: DisplayParams,
    totalRays: number,
    canvasWidth: number,
    canvasHeight: number,
  ): boolean {
    if (!this.dirty || !this.bindGroup) {
      return false;
    }
    this.dirty = false;

    encodeDisplayParams(this.paramsBuf, display, totalRays, canvasWidth, canvasHeight);
    this.device.queue.writeBuffer(this.uniformBuffer, 0, this.paramsBuf);

    const rp = encoder.beginRenderPass({
      colorAttachments: [
        {
          view: this.context.getCurrentTexture().createView(),
          loadOp: "clear",
          storeOp: "store",
          clearValue: { r: 0, g: 0, b: 0, a: 1 },
        },
      ],
    });
    rp.setPipeline(this.pipeline);
    rp.setBindGroup(0, this.bindGroup);
    rp.draw(3);
    rp.end();

    return true;
  }
}
