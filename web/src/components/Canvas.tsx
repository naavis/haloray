import { useEffect, useRef } from "react";
import { Box } from "@radix-ui/themes";
import raytraceCode from "../shaders/raytrace.wgsl?raw";
import accumulateCode from "../shaders/accumulate.wgsl?raw";
import displayCode from "../shaders/display.wgsl?raw";
import { useParams } from "../state/useParams";
import {
  DISPLAY_PARAMS_SIZE,
  PARAMS_SIZE,
  encodeDisplayParams,
  encodeSimParams,
} from "../state/encodeParams";

const WORKGROUP_SIZE = 64;
const RAYS_PER_STEP = 500_000;
const NUM_WORKGROUPS = Math.ceil(RAYS_PER_STEP / WORKGROUP_SIZE);
const ACTUAL_RAYS = NUM_WORKGROUPS * WORKGROUP_SIZE;
const RAY_RESULT_STRIDE = 20;

function Canvas() {
  const containerRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const { simParams, displayParams, simVersion } = useParams();

  const simRef = useRef(simParams);
  const displayRef = useRef(displayParams);
  useEffect(() => {
    simRef.current = simParams;
  }, [simParams]);
  useEffect(() => {
    displayRef.current = displayParams;
  }, [displayParams]);

  const resetRequestedRef = useRef(false);
  useEffect(() => {
    resetRequestedRef.current = true;
  }, [simVersion]);

  useEffect(() => {
    const container = containerRef.current;
    const canvas = canvasRef.current;
    if (!container || !canvas) return;

    let cancelled = false;
    let animFrameId = 0;
    let device: GPUDevice | undefined;
    const cleanupTasks: Array<() => void> = [];

    (async () => {
      if (!navigator.gpu) {
        console.error("WebGPU is not supported in this browser");
        return;
      }
      const adapter = await navigator.gpu.requestAdapter();
      if (!adapter) {
        console.error("No WebGPU adapter available");
        return;
      }
      device = await adapter.requestDevice();
      if (cancelled) {
        device.destroy();
        return;
      }

      const context = canvas.getContext("webgpu");
      if (!context) {
        console.error("Could not get a WebGPU context from the canvas");
        device.destroy();
        device = undefined;
        return;
      }
      const format = navigator.gpu.getPreferredCanvasFormat();
      context.configure({ device, format, alphaMode: "opaque" });

      const raytraceModule = device.createShaderModule({ code: raytraceCode });
      const accumulateModule = device.createShaderModule({ code: accumulateCode });
      const displayModule = device.createShaderModule({ code: displayCode });

      const raytracePipeline = device.createComputePipeline({
        layout: "auto",
        compute: { module: raytraceModule, entryPoint: "main" },
      });
      const accumulatePipeline = device.createComputePipeline({
        layout: "auto",
        compute: { module: accumulateModule, entryPoint: "main" },
      });
      const displayPipeline = device.createRenderPipeline({
        layout: "auto",
        vertex: { module: displayModule, entryPoint: "vs" },
        fragment: { module: displayModule, entryPoint: "fs", targets: [{ format }] },
      });

      const paramsBuffer = device.createBuffer({
        size: PARAMS_SIZE,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
      });
      const rayBuffer = device.createBuffer({
        size: ACTUAL_RAYS * RAY_RESULT_STRIDE,
        usage: GPUBufferUsage.STORAGE,
      });
      const displayParamsBuffer = device.createBuffer({
        size: DISPLAY_PARAMS_SIZE,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
      });

      const paramsBuf = new ArrayBuffer(PARAMS_SIZE);
      const displayBuf = new ArrayBuffer(DISPLAY_PARAMS_SIZE);

      let accBuffer: GPUBuffer | null = null;
      let canvasWidth = 0;
      let canvasHeight = 0;
      let raytraceBindGroup: GPUBindGroup | null = null;
      let accumulateBindGroup: GPUBindGroup | null = null;
      let displayBindGroup: GPUBindGroup | null = null;
      let totalRays = 0;
      let rngSeed = 0;

      const createBindGroups = () => {
        if (!device || !accBuffer) return;
        raytraceBindGroup = device.createBindGroup({
          layout: raytracePipeline.getBindGroupLayout(0),
          entries: [
            { binding: 0, resource: { buffer: paramsBuffer } },
            { binding: 1, resource: { buffer: rayBuffer } },
          ],
        });
        accumulateBindGroup = device.createBindGroup({
          layout: accumulatePipeline.getBindGroupLayout(0),
          entries: [
            { binding: 0, resource: { buffer: rayBuffer } },
            { binding: 1, resource: { buffer: accBuffer } },
            { binding: 2, resource: { buffer: displayParamsBuffer } },
          ],
        });
        displayBindGroup = device.createBindGroup({
          layout: displayPipeline.getBindGroupLayout(0),
          entries: [
            { binding: 0, resource: { buffer: accBuffer } },
            { binding: 1, resource: { buffer: displayParamsBuffer } },
          ],
        });
      };

      const handleResize = () => {
        if (!device) return;
        const rect = container.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        const maxDim = device.limits.maxTextureDimension2D;
        const w = Math.max(1, Math.min(Math.floor(rect.width * dpr), maxDim));
        const h = Math.max(1, Math.min(Math.floor(rect.height * dpr), maxDim));
        if (w === canvasWidth && h === canvasHeight) return;

        canvasWidth = w;
        canvasHeight = h;
        canvas.width = w;
        canvas.height = h;
        canvas.style.width = `${rect.width}px`;
        canvas.style.height = `${rect.height}px`;

        if (accBuffer) accBuffer.destroy();
        accBuffer = device.createBuffer({
          size: w * h * 3 * 4,
          usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
        });
        createBindGroups();
        totalRays = 0;
      };

      handleResize();
      const observer = new ResizeObserver(handleResize);
      observer.observe(container);
      cleanupTasks.push(() => {
        observer.disconnect();
        accBuffer?.destroy();
      });

      const frame = () => {
        if (
          cancelled ||
          !device ||
          !accBuffer ||
          !raytraceBindGroup ||
          !accumulateBindGroup ||
          !displayBindGroup
        ) {
          return;
        }

        if (resetRequestedRef.current) {
          resetRequestedRef.current = false;
          totalRays = 0;
          device.queue.writeBuffer(
            accBuffer,
            0,
            new Uint8Array(canvasWidth * canvasHeight * 3 * 4),
          );
        }

        rngSeed++;
        encodeSimParams(paramsBuf, simRef.current, canvasWidth, canvasHeight, rngSeed);
        device.queue.writeBuffer(paramsBuffer, 0, paramsBuf);

        totalRays += ACTUAL_RAYS;
        encodeDisplayParams(
          displayBuf,
          displayRef.current,
          totalRays,
          canvasWidth,
          canvasHeight,
        );
        device.queue.writeBuffer(displayParamsBuffer, 0, displayBuf);

        const encoder = device.createCommandEncoder();

        const rp = encoder.beginComputePass();
        rp.setPipeline(raytracePipeline);
        rp.setBindGroup(0, raytraceBindGroup);
        rp.dispatchWorkgroups(NUM_WORKGROUPS);
        rp.end();

        const ap = encoder.beginComputePass();
        ap.setPipeline(accumulatePipeline);
        ap.setBindGroup(0, accumulateBindGroup);
        ap.dispatchWorkgroups(NUM_WORKGROUPS);
        ap.end();

        const dp = encoder.beginRenderPass({
          colorAttachments: [
            {
              view: context.getCurrentTexture().createView(),
              loadOp: "clear",
              storeOp: "store",
              clearValue: { r: 0, g: 0, b: 0, a: 1 },
            },
          ],
        });
        dp.setPipeline(displayPipeline);
        dp.setBindGroup(0, displayBindGroup);
        dp.draw(3);
        dp.end();

        device.queue.submit([encoder.finish()]);
        animFrameId = requestAnimationFrame(frame);
      };

      animFrameId = requestAnimationFrame(frame);
    })();

    return () => {
      cancelled = true;
      cancelAnimationFrame(animFrameId);
      for (const fn of cleanupTasks) fn();
      device?.destroy();
    };
  }, []);

  return (
    <Box
      ref={containerRef}
      style={{
        flex: 1,
        height: "100vh",
        background: "var(--gray-2)",
        overflow: "hidden",
      }}
    >
      <canvas ref={canvasRef} style={{ display: "block" }} />
    </Box>
  );
}

export default Canvas;
