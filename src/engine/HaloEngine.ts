import { didSimulationChange, didViewChange } from "../state/params";
import type { SimParams, DisplayParams } from "../state/params";
import { DisplayPass } from "./DisplayPass";
import { GuidesPass } from "./GuidesPass";
import { HaloPass } from "./HaloPass";
import { SkyPass, TURBIDITY } from "./SkyPass";
import { buildSunSpectrum } from "./hosek-wilkie-sky/sun-spectrum";

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
  private userStarted = false;
  private readonly onAutoStop: () => void;

  private constructor(
    device: GPUDevice,
    context: GPUCanvasContext,
    format: GPUTextureFormat,
    canvas: HTMLCanvasElement,
    simParams: SimParams,
    displayParams: DisplayParams,
    onAutoStop: () => void,
  ) {
    this.device = device;
    this.canvas = canvas;
    this.simParams = simParams;
    this.displayParams = displayParams;
    this.onAutoStop = onAutoStop;

    this.haloPass = new HaloPass(device);
    this.skyPass = new SkyPass(device);
    this.guidesPass = new GuidesPass(device);
    this.displayPass = new DisplayPass(device, context, format);

    this.recomputeSunSpectrum();
  }

  // Hosek-Wilkie spectral solar radiance, recomputed when the sun moves or
  // when the user toggles the sky background. With sky off the raytracer
  // falls back to the simple linear daylight estimate inside the shader.
  private recomputeSunSpectrum(): void {
    if (this.displayParams.showSky) {
      const elevation = (this.simParams.sunAlt * Math.PI) / 180;
      this.haloPass.setSunSpectrum(buildSunSpectrum(TURBIDITY, elevation));
    } else {
      this.haloPass.setSunSpectrum(null);
    }
  }

  static async create(
    canvas: HTMLCanvasElement,
    simParams: SimParams,
    displayParams: DisplayParams,
    onAutoStop: () => void,
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

    return new HaloEngine(device, context, format, canvas, simParams, displayParams, onAutoStop);
  }

  setSimParams(params: SimParams): void {
    const prev = this.simParams;
    this.simParams = params;
    if (prev.sunAlt !== params.sunAlt) {
      this.recomputeSunSpectrum();
    }
    const viewChanged = didViewChange(prev, params);
    const simChanged = viewChanged || didSimulationChange(prev, params);
    if (viewChanged) {
      this.skyPass.markDirty();
      this.guidesPass.markDirty();
    }
    if (simChanged) {
      this.haloPass.resetAccumulation();
      this.displayPass.markDirty();
      this.ensureRunning();
    }
  }

  setDisplayParams(params: DisplayParams): void {
    const prev = this.displayParams;
    this.displayParams = params;
    if (prev.showSky !== params.showSky) {
      // Toggling the sky background swaps the halo spectrum (Hosek-Wilkie
      // vs. flat daylight estimate), so accumulated samples are no longer
      // physically consistent and must be discarded.
      this.recomputeSunSpectrum();
      this.haloPass.resetAccumulation();
    }
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
    this.userStarted = true;
    if (!this.running) {
      this.scheduleFrame();
    }
  }

  private scheduleFrame(): void {
    if (this.running) {
      return;
    }
    this.running = true;
    this.animFrameId = requestAnimationFrame(() => this.frame());
  }

  private ensureRunning(): void {
    if (this.userStarted && !this.running) {
      this.scheduleFrame();
    }
  }

  stop(): void {
    this.userStarted = false;
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
      this.simParams.populations,
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
      this.userStarted = false;
      this.onAutoStop();
    }
  }
}
