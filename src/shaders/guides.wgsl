// guides.wgsl — Compute shader that writes per-pixel guide overlay (linear RGBA)
// into the guides buffer: horizon line, zenith/nadir cross, 22° and 46° halo rings.

const PI: f32 = 3.1415926535;

const PROJ_STEREOGRAPHIC: u32 = 0u;
const PROJ_RECTILINEAR:   u32 = 1u;
const PROJ_EQUIDISTANT:   u32 = 2u;
const PROJ_EQUAL_AREA:    u32 = 3u;
const PROJ_ORTHOGRAPHIC:  u32 = 4u;

struct GuidesParams {
    resolution_x:   u32,
    resolution_y:   u32,
    cam_pitch:      f32,
    cam_yaw:        f32,
    cam_fov:        f32,
    cam_projection: u32,
    sun_altitude:   f32,
}

@group(0) @binding(0) var<uniform> params: GuidesParams;
@group(0) @binding(1) var<storage, read_write> guides_buffer: array<f32>;

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

    // Polar coordinates
    let r = length(vec2f(u, v));
    let polar_angle = atan2(v, u);

    let fov = params.cam_fov;

    if ((params.cam_projection == PROJ_RECTILINEAR ||
         params.cam_projection == PROJ_ORTHOGRAPHIC) && r > 0.5 * PI) {
        return RayInfo(vec3f(0.0), PI + 1.0);
    }

    var projected_angle: f32;
    if (params.cam_projection == PROJ_STEREOGRAPHIC) {
        projected_angle = 2.0 * atan(r / (2.0 * fov));
    } else if (params.cam_projection == PROJ_RECTILINEAR) {
        projected_angle = atan(r / fov);
    } else if (params.cam_projection == PROJ_EQUIDISTANT) {
        projected_angle = r / fov;
    } else if (params.cam_projection == PROJ_EQUAL_AREA) {
        projected_angle = 2.0 * asin(r / (2.0 * fov));
    } else {
        projected_angle = asin(r / fov);
    }

    let cam_dir = vec3f(sin(projected_angle) * cos(polar_angle), sin(projected_angle) * sin(polar_angle), cos(projected_angle));
    let orient  = rotate_y(-params.cam_yaw) * rotate_x(-params.cam_pitch);
    return RayInfo(normalize(orient * cam_dir), projected_angle);
}

@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) gid: vec3u) {
    if (gid.x >= params.resolution_x || gid.y >= params.resolution_y) { return; }
    let idx = (gid.y * params.resolution_x + gid.x) * 4u;

    let line_width = 0.25 / sqrt(params.cam_fov) * PI / 180.0;

    let ray = pixel_to_world_dir(gid.x, gid.y);
    if (ray.projected_angle > PI) {
        guides_buffer[idx]     = 0.0;
        guides_buffer[idx + 1] = 0.0;
        guides_buffer[idx + 2] = 0.0;
        guides_buffer[idx + 3] = 0.0;
        return;
    }
    let dir = ray.dir;

    let horizon_dist = abs(dir.y);

    let zenith_dist = atan2(length(dir.xz), dir.y);
    let nadir_dist  = PI - zenith_dist;
    var pole_dist   = min(zenith_dist, nadir_dist);
    if (pole_dist < 0.025) {
        pole_dist = min(abs(dir.x), abs(dir.z));
    }

    let sun_vec  = normalize(vec3f(0.0, sin(params.sun_altitude), cos(params.sun_altitude)));
    let sun_dist = acos(clamp(dot(sun_vec, dir), -1.0, 1.0));
    let ring_dist = min(
        abs(sun_dist - 22.0 / 180.0 * PI),
        abs(sun_dist - 46.0 / 180.0 * PI),
    );

    let min_dist = min(min(horizon_dist, pole_dist), ring_dist);
    let value    = 1.0 - smoothstep(line_width * 0.3, line_width * 0.5, min_dist);

    guides_buffer[idx]     = 1.0;
    guides_buffer[idx + 1] = 1.0;
    guides_buffer[idx + 2] = 1.0;
    guides_buffer[idx + 3] = value * 0.5;
}
