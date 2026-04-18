// sky.wgsl — Compute shader that writes per-pixel sky radiance (linear RGB)
// into the sky buffer.  Currently a placeholder that writes black.

struct SkyParams {
    resolution_x: u32,
    resolution_y: u32,
}

@group(0) @binding(0) var<uniform> params: SkyParams;
@group(0) @binding(1) var<storage, read_write> sky_buffer: array<f32>;

@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) gid: vec3u) {
    if (gid.x >= params.resolution_x || gid.y >= params.resolution_y) { return; }
    let idx = (gid.y * params.resolution_x + gid.x) * 3u;
    sky_buffer[idx]     = 0.0;
    sky_buffer[idx + 1] = 0.0;
    sky_buffer[idx + 2] = 0.0;
}
