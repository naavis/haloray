import { useEffect, useRef } from "react"
import { Box } from "@radix-ui/themes"

const shaderCode = `
struct Uniforms {
  time: f32,
}
@group(0) @binding(0) var<uniform> u: Uniforms;

struct VSOut {
  @builtin(position) pos: vec4f,
  @location(0) color: vec3f,
}

@vertex
fn vs(@builtin(vertex_index) i: u32) -> VSOut {
  var positions = array<vec2f, 3>(
    vec2f( 0.0,  0.6),
    vec2f(-0.6, -0.4),
    vec2f( 0.6, -0.4),
  );
  var colors = array<vec3f, 3>(
    vec3f(1.0, 0.3, 0.3),
    vec3f(0.3, 1.0, 0.4),
    vec3f(0.3, 0.5, 1.0),
  );
  let c = cos(u.time);
  let s = sin(u.time);
  let p = positions[i];
  let rotated = vec2f(c * p.x - s * p.y, s * p.x + c * p.y);
  var out: VSOut;
  out.pos = vec4f(rotated, 0.0, 1.0);
  out.color = colors[i];
  return out;
}

@fragment
fn fs(in: VSOut) -> @location(0) vec4f {
  return vec4f(in.color, 1.0);
}
`

function Canvas() {
  const containerRef = useRef<HTMLDivElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)

  useEffect(() => {
    const container = containerRef.current
    const canvas = canvasRef.current
    if (!container || !canvas) return

    const resize = () => {
      const { width, height } = container.getBoundingClientRect()
      const dpr = window.devicePixelRatio || 1
      canvas.width = Math.round(width * dpr)
      canvas.height = Math.round(height * dpr)
      canvas.style.width = `${width}px`
      canvas.style.height = `${height}px`
    }

    resize()
    const observer = new ResizeObserver(resize)
    observer.observe(container)
    return () => observer.disconnect()
  }, [])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return

    let cancelled = false
    let animFrameId = 0
    let device: GPUDevice | undefined

    (async () => {
      if (!navigator.gpu) {
        console.error("WebGPU is not supported in this browser")
        return
      }
      const adapter = await navigator.gpu.requestAdapter()
      if (!adapter) {
        console.error("No WebGPU adapter available")
        return
      }
      device = await adapter.requestDevice()
      if (cancelled) {
        device.destroy()
        return
      }

      const context = canvas.getContext("webgpu")
      if (!context) {
        console.error("Could not get a WebGPU context from the canvas")
        device.destroy()
        return
      }
      const format = navigator.gpu.getPreferredCanvasFormat()
      context.configure({ device, format, alphaMode: "premultiplied" })

      const module = device.createShaderModule({ code: shaderCode })
      const pipeline = device.createRenderPipeline({
        layout: "auto",
        vertex: { module, entryPoint: "vs" },
        fragment: { module, entryPoint: "fs", targets: [{ format }] },
        primitive: { topology: "triangle-list" },
      })

      const uniformBuffer = device.createBuffer({
        size: 16,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
      })
      const bindGroup = device.createBindGroup({
        layout: pipeline.getBindGroupLayout(0),
        entries: [{ binding: 0, resource: { buffer: uniformBuffer } }],
      })

      const start = performance.now()
      const frame = () => {
        if (cancelled || !device) return
        const t = (performance.now() - start) / 1000
        device.queue.writeBuffer(uniformBuffer, 0, new Float32Array([t]))

        const encoder = device.createCommandEncoder()
        const pass = encoder.beginRenderPass({
          colorAttachments: [
            {
              view: context.getCurrentTexture().createView(),
              loadOp: "clear",
              storeOp: "store",
              clearValue: { r: 0.05, g: 0.05, b: 0.08, a: 1 },
            },
          ],
        })
        pass.setPipeline(pipeline)
        pass.setBindGroup(0, bindGroup)
        pass.draw(3)
        pass.end()
        device.queue.submit([encoder.finish()])

        animFrameId = requestAnimationFrame(frame)
      }
      animFrameId = requestAnimationFrame(frame)
    })()

    return () => {
      cancelled = true
      cancelAnimationFrame(animFrameId)
      device?.destroy()
    }
  }, [])

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
  )
}

export default Canvas
