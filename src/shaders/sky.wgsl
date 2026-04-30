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

// Sun-altitude breakpoints (radians) for the Hosek/Preetham crossfade. Hosek
// is undefined below the horizon; Preetham keeps producing plausible colours
// down to a few degrees below, so we blend Hosek→Preetham in [0°, 1°] and
// fade Preetham→black in [-10°, 0°]. Below -10° the sky is just black.
const MIN_SUN_ELEVATION:    f32 = radians(-10.0);
const MIXING_MIN_ELEVATION: f32 = radians(0.0);
const MIXING_MAX_ELEVATION: f32 = radians(1.0);

struct SkyParams {
    resolution_x: u32,
    resolution_y: u32,
    projection:   u32,
    sun_altitude: f32,

    cam_pitch:    f32,
    cam_yaw:      f32,
    cam_fov:      f32,
    turbidity:    f32,

    radiances:    vec3f,
    _pad1:        f32,

    // configs[i] = (configX[i], configY[i], configZ[i], _) — 9 coefficients,
    // one component per CIE XYZ channel. Wrapped as vec4 to satisfy WGSL's
    // 16-byte uniform array stride.
    configs: array<vec4f, 9>,

    // Sun disk rendering — precomputed CIE XYZ radiance at top/bottom of disk
    // and per-channel limb-darkening scaler. See buildSunDiskState in
    // sun-spectrum.ts for how these are computed.
    sun_top_xyz:           vec3f,
    _pad2:                 f32,
    sun_bottom_xyz:        vec3f,
    _pad3:                 f32,
    limb_darkening_scaler: vec3f,
    _pad4:                 f32,
    solar_radius:          f32,
    elevation:             f32,
    _pad5:                 f32,
    _pad6:                 f32,
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

    if (params.projection == PROJ_RECTILINEAR && r > 0.5 * PI) {
        return RayInfo(vec3f(0.0), PI + 1.0);
    }
    if (params.projection == PROJ_ORTHOGRAPHIC && r > fov) {
        return RayInfo(vec3f(0.0), PI + 1.0);
    }
    if (params.projection == PROJ_EQUAL_AREA && r > 2.0 * fov) {
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

// Preetham analytic sky model — "A Practical Analytic Model for Daylight"
// (Preetham, Shirley, Smits, 1999). Used to extend the Hosek model below the
// horizon, where Hosek is undefined.

fn perez(cos_zenith: f32, sun_angle: f32, A: f32, B: f32, C: f32, D: f32, E: f32) -> f32 {
    let cos_sun = cos(sun_angle);
    return (1.0 + A * exp(B / cos_zenith)) *
           (1.0 + C * exp(D * sun_angle) + E * cos_sun * cos_sun);
}

fn preetham_luminance(cos_zenith: f32, sun_angle: f32, turbidity: f32) -> f32 {
    let sun_zenith = 0.5 * PI - params.sun_altitude;
    let a =  0.1787 * turbidity - 1.4630;
    let b = -0.3554 * turbidity + 0.4275;
    let c = -0.0227 * turbidity + 5.3251;
    let d =  0.1206 * turbidity - 2.5771;
    let e = -0.0670 * turbidity + 0.3703;
    let kappa = (4.0 / 9.0 - turbidity / 120.0) * (PI - 2.0 * sun_zenith);
    let Yz = (4.0453 * turbidity - 4.9710) * tan(kappa) - 0.2155 * turbidity + 2.4192;
    return Yz * perez(cos_zenith, sun_angle, a, b, c, d, e) /
                perez(1.0,        sun_zenith, a, b, c, d, e);
}

fn preetham_chroma_x(cos_zenith: f32, sun_angle: f32, turbidity: f32) -> f32 {
    let sz = 0.5 * PI - params.sun_altitude;
    let a = -0.0193 * turbidity - 0.2592;
    let b = -0.0665 * turbidity + 0.0008;
    let c = -0.0004 * turbidity + 0.2125;
    let d = -0.0641 * turbidity - 0.8989;
    let e = -0.0033 * turbidity + 0.0452;

    let z2 = sz * sz;
    let z3 = z2 * sz;
    let xz = turbidity * turbidity * ( 0.00166 * z3 - 0.00375 * z2 + 0.00209 * sz)
           + turbidity              * (-0.02903 * z3 + 0.06377 * z2 - 0.03202 * sz + 0.00394)
           +                          ( 0.11693 * z3 - 0.21196 * z2 + 0.06052 * sz + 0.25886);

    return xz * perez(cos_zenith, sun_angle, a, b, c, d, e) /
                perez(1.0,        sz,        a, b, c, d, e);
}

fn preetham_chroma_y(cos_zenith: f32, sun_angle: f32, turbidity: f32) -> f32 {
    let sz = 0.5 * PI - params.sun_altitude;
    let a = -0.0167 * turbidity - 0.2608;
    let b = -0.0950 * turbidity + 0.0092;
    let c = -0.0079 * turbidity + 0.2102;
    let d = -0.0441 * turbidity - 1.6537;
    let e = -0.0109 * turbidity + 0.0529;

    let z2 = sz * sz;
    let z3 = z2 * sz;
    let yz = turbidity * turbidity * ( 0.00275 * z3 - 0.00610 * z2 + 0.00317 * sz)
           + turbidity              * (-0.04214 * z3 + 0.08970 * z2 - 0.04153 * sz + 0.00516)
           +                          ( 0.15346 * z3 - 0.26756 * z2 + 0.06670 * sz + 0.26688);

    return yz * perez(cos_zenith, sun_angle, a, b, c, d, e) /
                perez(1.0,        sz,        a, b, c, d, e);
}

fn preetham_sky(dir: vec3f, sun_vec: vec3f, turbidity: f32) -> vec3f {
    let sun_angle = acos(clamp(dot(sun_vec, dir), -1.0, 1.0));
    let cos_zenith = max(dir.y, 1e-4);
    let Y = preetham_luminance(cos_zenith, sun_angle, turbidity);
    let x = preetham_chroma_x(cos_zenith, sun_angle, turbidity);
    let y = preetham_chroma_y(cos_zenith, sun_angle, turbidity);
    return vec3f(x * Y / y, Y, (1.0 - x - y) * Y / y);
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

fn hosek_sky(cos_theta: f32, gamma: f32) -> vec3f {
    return vec3f(
        hosek_channel(0u, cos_theta, gamma),
        hosek_channel(1u, cos_theta, gamma),
        hosek_channel(2u, cos_theta, gamma),
    );
}

fn hosek_preetham_mix(dir: vec3f, sun_vec: vec3f, cos_theta: f32, gamma: f32, turbidity: f32) -> vec3f {
    if (params.sun_altitude >= MIXING_MAX_ELEVATION) {
        return hosek_sky(cos_theta, gamma);
    }
    if (params.sun_altitude >= MIXING_MIN_ELEVATION) {
        let preetham = preetham_sky(dir, sun_vec, turbidity);
        let hosek    = hosek_sky(cos_theta, gamma);
        let t = (params.sun_altitude - MIXING_MIN_ELEVATION) /
                (MIXING_MAX_ELEVATION - MIXING_MIN_ELEVATION);
        return mix(preetham, hosek, t);
    }
    let t = clamp((params.sun_altitude - MIN_SUN_ELEVATION) / (-MIN_SUN_ELEVATION), 0.0, 1.0);
    return preetham_sky(dir, sun_vec, turbidity) * t;
}

// Renders the solar disk — port of renderSun() from desktop sky.glsl.
// Returns CIE XYZ radiance for the solar disk at direction `dir`, or zero
// if the direction misses the disk.
fn render_sun(dir: vec3f, sun_vec: vec3f) -> vec3f {
    let sun_angle = acos(clamp(dot(dir, sun_vec), -1.0, 1.0));
    if (sun_angle > params.solar_radius) { return vec3f(0.0); }

    let ray_elevation = asin(clamp(dir.y, -1.0, 1.0));
    // Interpolate top/bottom disk radiance by elevation within the disk.
    // The clamp+max handles the case where the sun is partly below the horizon.
    let factor = (ray_elevation - max(params.elevation - params.solar_radius, 0.0))
               / min(2.0 * params.solar_radius, params.elevation + params.solar_radius);
    let plain_radiance = mix(params.sun_bottom_xyz, params.sun_top_xyz, clamp(factor, 0.0, 1.0));

    // Limb darkening: sampleCosine = 1 at disk centre, 0 at edge.
    let sin_solar_radius = sin(params.solar_radius);
    let sin_gamma        = sin(sun_angle);
    let sc2              = max(0.0, 1.0 - (sin_gamma * sin_gamma) / (sin_solar_radius * sin_solar_radius));
    let sample_cosine    = sqrt(sc2);

    return mix(params.limb_darkening_scaler * plain_radiance, plain_radiance, sample_cosine);
}

@compute @workgroup_size(8, 8)
fn main(@builtin(global_invocation_id) gid: vec3u) {
    if (gid.x >= params.resolution_x || gid.y >= params.resolution_y) { return; }
    let idx = (gid.y * params.resolution_x + gid.x) * 3u;

    let ray = pixel_to_world_dir(gid.x, gid.y);
    if (ray.projected_angle > PI || ray.dir.y < 0.0 || params.sun_altitude < MIN_SUN_ELEVATION) {
        sky_buffer[idx]      = 0.0;
        sky_buffer[idx + 1u] = 0.0;
        sky_buffer[idx + 2u] = 0.0;
        return;
    }

    let sun_vec   = normalize(vec3f(0.0, sin(params.sun_altitude), cos(params.sun_altitude)));
    let cos_theta = max(ray.dir.y, 0.0);
    let gamma     = acos(clamp(dot(sun_vec, ray.dir), -1.0, 1.0));

    let sky_xyz = hosek_preetham_mix(ray.dir, sun_vec, cos_theta, gamma, params.turbidity);
    let sun_xyz = render_sun(ray.dir, sun_vec);
    let xyz     = sky_xyz + sun_xyz;

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
