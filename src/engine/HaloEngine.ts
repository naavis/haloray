import raytraceCode from "../shaders/raytrace.wgsl?raw";
import accumulateCode from "../shaders/accumulate.wgsl?raw";
import displayCode from "../shaders/display.wgsl?raw";
import {
  DISPLAY_PARAMS_SIZE,
  PARAMS_SIZE,
  encodeDisplayParams,
  encodeSimParams,
} from "../state/encodeParams";
import type { SimParams, DisplayParams } from "../state/params";

const WORKGROUP_SIZE = 64;
const RAYS_PER_STEP = 500_000;
const NUM_WORKGROUPS = Math.ceil(RAYS_PER_STEP / WORKGROUP_SIZE);
const ACTUAL_RAYS = NUM_WORKGROUPS * WORKGROUP_SIZE;
const RAY_RESULT_STRIDE = 20;
const MAX_TOTAL_RAYS = 1_000_000_000;

export class HaloEngine {
  private device: GPUDevice;
  private context: GPUCanvasContext;
  private canvas: HTMLCanvasElement;

  private raytracePipeline: GPUComputePipeline;
  private accumulatePipeline: GPUComputePipeline;
  private displayPipeline: GPURenderPipeline;

  private paramsBuffer: GPUBuffer;
  private rayBuffer: GPUBuffer;
  private displayParamsBuffer: GPUBuffer;
  private accBuffer: GPUBuffer | null = null;

  private raytraceBindGroup: GPUBindGroup | null = null;
  private accumulateBindGroup: GPUBindGroup | null = null;
  private displayBindGroup: GPUBindGroup | null = null;

  private paramsBuf = new ArrayBuffer(PARAMS_SIZE);
  private displayBuf = new ArrayBuffer(DISPLAY_PARAMS_SIZE);

  private canvasWidth = 0;
  private canvasHeight = 0;
  private totalRays = 0;
  private rngSeed = 0;

  private simParams: SimParams;
  private displayParams: DisplayParams;
  private displayDirty = true;
  private resetRequested = false;

  private animFrameId = 0;
  private running = false;

  private constructor(
    device: GPUDevice,
    context: GPUCanvasContext,
    format: GPUTextureFormat,
    canvas: HTMLCanvasElement,
    simParams: SimParams,
    displayParams: DisplayParams,
  ) {
    this.device = device;
    this.context = context;
    this.canvas = canvas;
    this.simParams = simParams;
    this.displayParams = displayParams;

    const raytraceModule = device.createShaderModule({ code: raytraceCode });
    const accumulateModule = device.createShaderModule({
      code: accumulateCode,
    });
    const displayModule = device.createShaderModule({ code: displayCode });

    this.raytracePipeline = device.createComputePipeline({
      layout: "auto",
      compute: { module: raytraceModule, entryPoint: "main" },
    });
    this.accumulatePipeline = device.createComputePipeline({
      layout: "auto",
      compute: { module: accumulateModule, entryPoint: "main" },
    });
    this.displayPipeline = device.createRenderPipeline({
      layout: "auto",
      vertex: { module: displayModule, entryPoint: "vs" },
      fragment: {
        module: displayModule,
        entryPoint: "fs",
        targets: [{ format }],
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
    this.displayParamsBuffer = device.createBuffer({
      size: DISPLAY_PARAMS_SIZE,
      usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });
  }

  static async create(
    canvas: HTMLCanvasElement,
    simParams: SimParams,
    displayParams: DisplayParams,
  ): Promise<HaloEngine> {
    if (!navigator.gpu) {
      throw new Error("WebGPU is not supported in this browser");
    }
    const adapter = await navigator.gpu.requestAdapter();
    if (!adapter) {
      throw new Error("No WebGPU adapter available");
    }
    const device = await adapter.requestDevice();

    const context = canvas.getContext("webgpu");
    if (!context) {
      device.destroy();
      throw new Error("Could not get a WebGPU context from the canvas");
    }
    const format = navigator.gpu.getPreferredCanvasFormat();
    context.configure({ device, format, alphaMode: "opaque" });

    return new HaloEngine(
      device,
      context,
      format,
      canvas,
      simParams,
      displayParams,
    );
  }

  setSimParams(params: SimParams): void {
    this.simParams = params;
  }

  setDisplayParams(params: DisplayParams): void {
    this.displayParams = params;
    this.displayDirty = true;
  }

  resetAccumulation(): void {
    this.resetRequested = true;
    this.displayDirty = true;
  }

  resize(containerWidth: number, containerHeight: number): void {
    const dpr = window.devicePixelRatio || 1;
    const maxDim = this.device.limits.maxTextureDimension2D;
    const w = Math.max(1, Math.min(Math.floor(containerWidth * dpr), maxDim));
    const h = Math.max(1, Math.min(Math.floor(containerHeight * dpr), maxDim));
    if (w === this.canvasWidth && h === this.canvasHeight) return;

    this.canvasWidth = w;
    this.canvasHeight = h;
    this.canvas.width = w;
    this.canvas.height = h;
    this.canvas.style.width = `${containerWidth}px`;
    this.canvas.style.height = `${containerHeight}px`;

    if (this.accBuffer) this.accBuffer.destroy();
    this.accBuffer = this.device.createBuffer({
      size: w * h * 3 * 4,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
    });
    this.createBindGroups();
    this.totalRays = 0;
    this.displayDirty = true;
  }

  start(): void {
    if (this.running) return;
    this.running = true;
    this.animFrameId = requestAnimationFrame(() => this.frame());
  }

  stop(): void {
    this.running = false;
    cancelAnimationFrame(this.animFrameId);
  }

  destroy(): void {
    this.stop();
    this.accBuffer?.destroy();
    this.device.destroy();
  }

  private createBindGroups(): void {
    if (!this.accBuffer) return;
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
        { binding: 1, resource: { buffer: this.accBuffer } },
        { binding: 2, resource: { buffer: this.displayParamsBuffer } },
      ],
    });
    this.displayBindGroup = this.device.createBindGroup({
      layout: this.displayPipeline.getBindGroupLayout(0),
      entries: [
        { binding: 0, resource: { buffer: this.accBuffer } },
        { binding: 1, resource: { buffer: this.displayParamsBuffer } },
      ],
    });
  }

  private frame(): void {
    if (
      !this.running ||
      !this.accBuffer ||
      !this.raytraceBindGroup ||
      !this.accumulateBindGroup ||
      !this.displayBindGroup
    ) {
      return;
    }

    if (this.resetRequested) {
      this.resetRequested = false;
      this.totalRays = 0;
      this.device.queue.writeBuffer(
        this.accBuffer,
        0,
        new Uint8Array(this.canvasWidth * this.canvasHeight * 3 * 4),
      );
    }

    const shouldTrace = this.totalRays < MAX_TOTAL_RAYS;

    if (shouldTrace) {
      this.rngSeed++;
      encodeSimParams(
        this.paramsBuf,
        this.simParams,
        this.canvasWidth,
        this.canvasHeight,
        this.rngSeed,
      );
      this.device.queue.writeBuffer(this.paramsBuffer, 0, this.paramsBuf);
      this.totalRays += ACTUAL_RAYS;
    }

    encodeDisplayParams(
      this.displayBuf,
      this.displayParams,
      this.totalRays,
      this.canvasWidth,
      this.canvasHeight,
    );
    this.device.queue.writeBuffer(this.displayParamsBuffer, 0, this.displayBuf);

    const encoder = this.device.createCommandEncoder();

    if (shouldTrace) {
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

      this.displayDirty = true;
    }

    if (this.displayDirty) {
      const dp = encoder.beginRenderPass({
        colorAttachments: [
          {
            view: this.context.getCurrentTexture().createView(),
            loadOp: "clear",
            storeOp: "store",
            clearValue: { r: 0, g: 0, b: 0, a: 1 },
          },
        ],
      });
      dp.setPipeline(this.displayPipeline);
      dp.setBindGroup(0, this.displayBindGroup);
      dp.draw(3);
      dp.end();
      this.displayDirty = false;
    }

    this.device.queue.submit([encoder.finish()]);
    this.animFrameId = requestAnimationFrame(() => this.frame());
  }
}
