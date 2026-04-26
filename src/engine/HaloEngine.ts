import { didViewChange } from "../state/params";
import type { SimParams, DisplayParams } from "../state/params";
import { DisplayPass } from "./DisplayPass";
import { GuidesPass } from "./GuidesPass";
import { HaloPass } from "./HaloPass";
import { SkyPass } from "./SkyPass";

/*
 * HaloEngine — rendering pipeline overview
 *
 *   setSimParams()          setDisplayParams()         resize()
 *       │                        │                        │
 *       │  resets halo acc,      │  marks display         │  rebuilds buffers,
 *       │  marks sky/guides      │  dirty                 │  resets everything
 *       │  dirty on view change  │                        │
 *       └────────┬───────────────┴────────────────────────┘
 *                │
 *                v
 *         ensureRunning()
 *                │
 *                v
 *   ┌─── requestAnimationFrame ◄───────────────────────────┐
 *   │                                                      │
 *   v                                                      │
 * frame()                                                  │
 *   │                                                      │
 *   │  ┌─────────────────── GPU command encoder ─────────────────────┐
 *   │  │                                                             │
 *   │  │  COMPUTE PASSES                                             │
 *   │  │                                                             │
 *   │  │  HaloPass.encode()  (every frame until maxRays)             │
 *   │  │    ┌───────────┐      ┌─────────────┐                       │
 *   │  │    │ raytrace  │─────►│ accumulate  │                       │
 *   │  │    │ (compute) │ ray  │  (compute)  │                       │
 *   │  │    └───────────┘ buf  └──────┬──────┘                       │
 *   │  │                              │ accBuffer (RGB f32)          │
 *   │  │                              v                              │
 *   │  │  SkyPass.encode()   (only when dirty)                       │
 *   │  │    ┌───────────┐                                            │
 *   │  │    │   sky     │──────────► skyBuffer (RGB f32)             │
 *   │  │    │ (compute) │                                            │
 *   │  │    └───────────┘                                            │
 *   │  │                                                             │
 *   │  │  GuidesPass.encode() (only when dirty)                      │
 *   │  │    ┌───────────┐                                            │
 *   │  │    │  guides   │──────────► guidesBuffer (RGBA f32)         │
 *   │  │    │ (compute) │                                            │
 *   │  │    └───────────┘                                            │
 *   │  │                                                             │
 *   │  │  DisplayPass.encode() (only when dirty — render pass)       │
 *   │  │                                                             │
 *   │  │    accBuffer ─────┐                                         │
 *   │  │    skyBuffer ─────┼──► display.wgsl ──► canvas              │
 *   │  │    guidesBuffer ──┘   (tone map + sRGB + blend)             │
 *   │  │                                                             │
 *   │  └─────────────────────────────────────────────────────────────┘
 *   │                                                      │
 *   │  queue.submit()                                      │
 *   │                                                      │
 *   └──── if !maxRaysReached ─────────────────────────────►┘
 *                else stop
 */
export class HaloEngine {
  private device: GPUDevice;
  private canvas: HTMLCanvasElement;

  private accBuffer: GPUBuffer | null = null;
  private skyBuffer: GPUBuffer | null = null;
  private guidesBuffer: GPUBuffer | null = null;

  private haloPass: HaloPass;
  private skyPass: SkyPass;
  private guidesPass: GuidesPass;
  private displayPass: DisplayPass;

  private canvasWidth = 0;
  private canvasHeight = 0;

  private simParams: SimParams;
  private displayParams: DisplayParams;

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
    this.canvas = canvas;
    this.simParams = simParams;
    this.displayParams = displayParams;

    this.haloPass = new HaloPass(device);
    this.skyPass = new SkyPass(device);
    this.guidesPass = new GuidesPass(device);
    this.displayPass = new DisplayPass(device, context, format);
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

    return new HaloEngine(device, context, format, canvas, simParams, displayParams);
  }

  setSimParams(params: SimParams): void {
    if (didViewChange(this.simParams, params)) {
      this.skyPass.markDirty();
      this.guidesPass.markDirty();
    }
    this.simParams = params;
    this.haloPass.resetAccumulation();
    this.displayPass.markDirty();
    this.ensureRunning();
  }

  setDisplayParams(params: DisplayParams): void {
    this.displayParams = params;
    this.displayPass.markDirty();
    this.ensureRunning();
  }

  resetAccumulation(): void {
    this.haloPass.resetAccumulation();
    this.skyPass.markDirty();
    this.guidesPass.markDirty();
    this.displayPass.markDirty();
    this.ensureRunning();
  }

  resize(containerWidth: number, containerHeight: number): void {
    const dpr = window.devicePixelRatio || 1;
    const maxDim = this.device.limits.maxTextureDimension2D;
    const w = Math.max(1, Math.min(Math.floor(containerWidth * dpr), maxDim));
    const h = Math.max(1, Math.min(Math.floor(containerHeight * dpr), maxDim));
    if (w === this.canvasWidth && h === this.canvasHeight) {
      return;
    }

    this.canvasWidth = w;
    this.canvasHeight = h;
    this.canvas.width = w;
    this.canvas.height = h;
    this.canvas.style.width = `${containerWidth}px`;
    this.canvas.style.height = `${containerHeight}px`;

    const bufferSizeBytes = w * h * 3 * 4;

    if (this.accBuffer) {
      this.accBuffer.destroy();
    }
    this.accBuffer = this.device.createBuffer({
      size: bufferSizeBytes,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
    });

    if (this.skyBuffer) {
      this.skyBuffer.destroy();
    }
    this.skyBuffer = this.device.createBuffer({
      size: bufferSizeBytes,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
    });

    if (this.guidesBuffer) {
      this.guidesBuffer.destroy();
    }
    this.guidesBuffer = this.device.createBuffer({
      size: w * h * 4 * 4,
      usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
    });

    this.haloPass.createBindGroups(this.accBuffer);
    this.haloPass.writeAccParams(w);
    this.skyPass.createBindGroups(this.skyBuffer);
    this.guidesPass.createBindGroups(this.guidesBuffer);
    this.displayPass.createBindGroups(this.accBuffer, this.skyBuffer, this.guidesBuffer);

    this.haloPass.resetAccumulation();
    this.skyPass.markDirty();
    this.guidesPass.markDirty();
    this.displayPass.markDirty();
    this.ensureRunning();
  }

  start(): void {
    if (this.running) {
      return;
    }
    this.running = true;
    this.animFrameId = requestAnimationFrame(() => this.frame());
  }

  private ensureRunning(): void {
    if (!this.running) {
      this.start();
    }
  }

  stop(): void {
    this.running = false;
    cancelAnimationFrame(this.animFrameId);
  }

  destroy(): void {
    this.stop();
    this.device.destroy();
  }

  private frame(): void {
    if (!this.running) {
      return;
    }

    if (!this.accBuffer || !this.skyBuffer || !this.guidesBuffer) {
      this.animFrameId = requestAnimationFrame(() => this.frame());
      return;
    }

    const encoder = this.device.createCommandEncoder();

    const traced = this.haloPass.encode(
      encoder,
      this.simParams,
      this.canvasWidth,
      this.canvasHeight,
    );

    const skyRendered = this.skyPass.encode(
      encoder,
      this.simParams,
      this.canvasWidth,
      this.canvasHeight,
    );

    const guidesRendered = this.guidesPass.encode(
      encoder,
      this.simParams,
      this.canvasWidth,
      this.canvasHeight,
    );

    if (traced || skyRendered || guidesRendered) {
      this.displayPass.markDirty();
    }

    this.displayPass.encode(
      encoder,
      this.displayParams,
      this.haloPass.totalRays,
      this.canvasWidth,
      this.canvasHeight,
      this.simParams.camFov,
    );

    this.device.queue.submit([encoder.finish()]);

    if (!this.haloPass.maxRaysReached) {
      this.animFrameId = requestAnimationFrame(() => this.frame());
    } else {
      this.running = false;
    }
  }
}
