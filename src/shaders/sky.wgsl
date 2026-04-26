// sky.wgsl — Compute shader that writes per-pixel sky radiance (linear sRGB)
// into the sky buffer. Evaluates the Hosek-Wilkie analytic sky model using
// per-channel configs + radiance scales precomputed on the CPU
// (`buildSkyState` in `hosek-wilkie-sky/calculate.ts`).

const PI: f32 = 3.1415926535;

const PROJ_STEREOGRAPHIC: u32 = 0u;
const PROJ_RECTILINEAR:   u32 = 1u;
const PROJ_EQUIDISTANT:   u32 = 2u;
const PROJ_EQUAL_AREA:    u32 = 3u;
const PROJ_ORTHOGRAPHIC:  u32 = 4u;

struct SkyParams {
    resolution_x: u32,
    resolution_y: u32,
    projection:   u32,
    sun_altitude: f32,

    cam_pitch:    f32,
    cam_yaw:      f32,
    cam_fov:      f32,
    _pad0:        f32,

    radiances:    vec3f,
    _pad1:        f32,

    // configs[i] = (configX[i], configY[i], configZ[i], _) — 9 coefficients,
    // one component per CIE XYZ channel. Wrapped as vec4 to satisfy WGSL's
    // 16-byte uniform array stride.
    configs: array<vec4f, 9>,
}

@group(0) @binding(0) var<uniform> params: SkyParams;
@group(0) @binding(1) var<storage, read_write> sky_buffer: array<f32>;

fn rotate_x(angle: f32) -> mat3x3f {
    let c = cos(angle); let s = sin(angle);
    return mat3x3f(
        1.0, 0.0, 0.0,
        0.0, c,   s,
        0.0, -s,  c,
    );
}

fn rotate_y(angle: f32) -> mat3x3f {
    let c = cos(angle); let s = sin(angle);
    return mat3x3f(
        c,   0.0, -s,
        0.0, 1.0, 0.0,
        s,   0.0, c,
    );
}

struct RayInfo {
    dir: vec3f,
    projected_angle: f32,
}

fn pixel_to_world_dir(px: u32, py: u32) -> RayInfo {
    let resolution = vec2f(f32(params.resolution_x), f32(params.resolution_y));
    let aspect = resolution.y / resolution.x;
    let normalized_coordinates = vec2f(f32(px), f32(py)) / resolution - vec2f(0.5);
    let u = normalized_coordinates.x / aspect;
    let v = -normalized_coordinates.y;

    let r = length(vec2f(u, v));
    let polar_angle = atan2(v, u);

    let fov = params.cam_fov;

    if ((params.projection == PROJ_RECTILINEAR ||
         params.projection == PROJ_ORTHOGRAPHIC) && r > 0.5 * PI) {
        return RayInfo(vec3f(0.0), PI + 1.0);
    }

    var projected_angle: f32;
    if (params.projection == PROJ_STEREOGRAPHIC) {
        projected_angle = 2.0 * atan(r / (2.0 * fov));
    } else if (params.projection == PROJ_RECTILINEAR) {
        projected_angle = atan(r / fov);
    } else if (params.projection == PROJ_EQUIDISTANT) {
        projected_angle = r / fov;
    } else if (params.projection == PROJ_EQUAL_AREA) {
        projected_angle = 2.0 * asin(r / (2.0 * fov));
    } else {
        projected_angle = asin(r / fov);
    }

    let cam_dir = vec3f(sin(projected_angle) * cos(polar_angle), sin(projected_angle) * sin(polar_angle), cos(projected_angle));
    let orient  = rotate_y(-params.cam_yaw) * rotate_x(-params.cam_pitch);
    return RayInfo(normalize(orient * cam_dir), projected_angle);
}

fn hosek_channel(channel: u32, cos_theta: f32, gamma: f32) -> f32 {
    let c0 = params.configs[0][channel];
    let c1 = params.configs[1][channel];
    let c2 = params.configs[2][channel];
    let c3 = params.configs[3][channel];
    let c4 = params.configs[4][channel];
    let c5 = params.configs[5][channel];
    let c6 = params.configs[6][channel];
    let c7 = params.configs[7][channel];
    let c8 = params.configs[8][channel];

    let cos_gamma = cos(gamma);
    let exp_m  = exp(c4 * gamma);
    let ray_m  = cos_gamma * cos_gamma;
    let mie_m  = (1.0 + ray_m) / pow(1.0 + c8 * c8 - 2.0 * c8 * cos_gamma, 1.5);
    let zenith = sqrt(cos_theta);

    let f = (1.0 + c0 * exp(c1 / (cos_theta + 0.01))) *
            (c2 + c3 * exp_m + c5 * ray_m + c6 * mie_m + c7 * zenith);
    return params.radiances[channel] * f;
}

@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) gid: vec3u) {
    if (gid.x >= params.resolution_x || gid.y >= params.resolution_y) { return; }
    let idx = (gid.y * params.resolution_x + gid.x) * 3u;

    let ray = pixel_to_world_dir(gid.x, gid.y);
    if (ray.projected_angle > PI || ray.dir.y < 0.0 || params.sun_altitude < 0.0) {
        sky_buffer[idx]      = 0.0;
        sky_buffer[idx + 1u] = 0.0;
        sky_buffer[idx + 2u] = 0.0;
        return;
    }

    let sun_vec   = normalize(vec3f(0.0, sin(params.sun_altitude), cos(params.sun_altitude)));
    let cos_theta = max(ray.dir.y, 0.0);
    let gamma     = acos(clamp(dot(sun_vec, ray.dir), -1.0, 1.0));

    let xyz = vec3f(
        hosek_channel(0u, cos_theta, gamma),
        hosek_channel(1u, cos_theta, gamma),
        hosek_channel(2u, cos_theta, gamma),
    );

    let xyz_to_srgb = mat3x3f(
         3.24096994, -0.96924364,  0.05563008,
        -1.53738318,  1.8759675,  -0.20397696,
        -0.49861076,  0.04155506,  1.05697151,
    );
    let rgb = max(xyz_to_srgb * xyz, vec3f(0.0));

    sky_buffer[idx]      = rgb.x;
    sky_buffer[idx + 1u] = rgb.y;
    sky_buffer[idx + 2u] = rgb.z;
}
