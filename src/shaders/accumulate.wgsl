// accumulate.wgsl — Reads ray results and atomically accumulates into image buffer.

const MISS: u32 = 0xFFFFFFFFu;
const SCALE: f32 = 65536.0;

struct RayResult {
    pixel_x: u32,
    pixel_y: u32,
    r: f32,
    g: f32,
    b: f32,
}

// This struct mirrors DisplayParams in display.wgsl because both shaders bind
// the same GPU uniform buffer (displayParamsBuffer). Only resolution_x is used
// by the accumulate pass; the other fields exist to keep the byte layout
// identical so offsets stay in sync.  See encodeDisplayParams() for the write
// side and HaloEngine.createBindGroups() for the shared binding.
struct AccParams {
    total_rays: f32,     // used only by display pass (normalization)
    resolution_x: f32,
    resolution_y: f32,   // used only by display pass (UV → pixel)
    _pad0: f32,          // brightness in DisplayParams; unused here
    _pad1: u32,          // show_guides in DisplayParams; unused here
}

@group(0) @binding(0) var<storage, read> ray_buffer: array<RayResult>;
@group(0) @binding(1) var<storage, read_write> accumulation: array<atomic<u32>>;
@group(0) @binding(2) var<uniform> acc_params: AccParams;

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) global_id: vec3u) {
    let ray = ray_buffer[global_id.x];
    if (ray.pixel_x == MISS) { return; }

    let res_x = u32(acc_params.resolution_x);
    let idx = (ray.pixel_y * res_x + ray.pixel_x) * 3u;

    atomicAdd(&accumulation[idx],      u32(max(ray.r, 0.0) * SCALE));
    atomicAdd(&accumulation[idx + 1u], u32(max(ray.g, 0.0) * SCALE));
    atomicAdd(&accumulation[idx + 2u], u32(max(ray.b, 0.0) * SCALE));
}
