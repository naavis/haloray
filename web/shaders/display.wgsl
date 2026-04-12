// display.wgsl — Fullscreen triangle that reads the accumulation buffer and tone-maps.

struct DisplayParams {
    total_rays: f32,
    resolution_x: f32,
    resolution_y: f32,
    _pad: f32,
}

@group(0) @binding(0) var<storage, read> accumulation: array<u32>;
@group(0) @binding(1) var<uniform> dp: DisplayParams;

struct VsOut {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
}

@vertex fn vs(@builtin(vertex_index) vi: u32) -> VsOut {
    // Single triangle covering the full screen
    let x = f32(vi & 1u) * 4.0 - 1.0;
    let y = f32(vi >> 1u) * 4.0 - 1.0;
    var out: VsOut;
    out.position = vec4f(x, y, 0.0, 1.0);
    out.uv = vec2f((x + 1.0) * 0.5, (1.0 - y) * 0.5);
    return out;
}

const SCALE: f32 = 65536.0;

fn srgb_gamma(c: f32) -> f32 {
    return select(1.055 * pow(c, 1.0 / 2.4) - 0.055, 12.92 * c, c <= 0.0031308);
}

@fragment fn fs(in: VsOut) -> @location(0) vec4f {
    let rx = u32(dp.resolution_x);
    let ry = u32(dp.resolution_y);
    let px = min(u32(in.uv.x * dp.resolution_x), rx - 1u);
    let py = min(u32(in.uv.y * dp.resolution_y), ry - 1u);

    let idx = (py * rx + px) * 3u;
    let r_raw = f32(accumulation[idx])      / SCALE;
    let g_raw = f32(accumulation[idx + 1u]) / SCALE;
    let b_raw = f32(accumulation[idx + 2u]) / SCALE;

    let total = max(dp.total_rays, 1.0);
    let exposure = 120000.0;
    var color = vec3f(r_raw, g_raw, b_raw) * exposure / total;

    // Reinhard tone mapping per channel
    color = color / (vec3f(1.0) + color);

    // sRGB gamma
    color = vec3f(srgb_gamma(color.x), srgb_gamma(color.y), srgb_gamma(color.z));

    return vec4f(color, 1.0);
}
