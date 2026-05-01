// raytrace.wgsl — Ice crystal halo raytracer (ported from GLSL 440)
// Each invocation generates one crystal, traces one ray, writes result to ray_buffer.

const PI: f32 = 3.1415926535;
const MAX_HITS: u32 = 100u;
const NUM_TRIANGLES: u32 = 44u;
const MISS: u32 = 0xFFFFFFFFu;

const DIST_UNIFORM: u32 = 0u;
const DIST_GAUSSIAN: u32 = 1u;

const PROJ_STEREOGRAPHIC: u32 = 0u;
const PROJ_RECTILINEAR: u32 = 1u;
const PROJ_EQUIDISTANT: u32 = 2u;
const PROJ_EQUAL_AREA: u32 = 3u;
const PROJ_ORTHOGRAPHIC: u32 = 4u;

// Halo fade range: full intensity at 0°, gone at -10° (mirrors sky.wgsl).
const MIN_SUN_ELEVATION: f32 = radians(-10.0);

// ── Types ──────────────────────────────────────────────────────────────

struct Params {
    rng_seed: u32,
    sun_altitude: f32,
    sun_diameter: f32,
    ca_ratio_avg: f32,
    ca_ratio_std: f32,
    tilt_distribution: u32,
    tilt_avg: f32,
    tilt_std: f32,
    rotation_distribution: u32,
    rotation_avg: f32,
    rotation_std: f32,
    camera_pitch: f32,
    camera_yaw: f32,
    camera_focal_length: f32,
    camera_projection: u32,
    camera_hide_sub_horizon: u32,
    resolution_x: u32,
    resolution_y: u32,
    upper_apex_angle: f32,
    upper_apex_height_avg: f32,
    upper_apex_height_std: f32,
    lower_apex_angle: f32,
    lower_apex_height_avg: f32,
    lower_apex_height_std: f32,
    prism_distances: array<vec4f, 2>,  // [0].xyzw = d0-d3, [1].xy = d4-d5
    atmosphere_enabled: u32,
    _pad0: u32,
    _pad1: u32,
    _pad2: u32,
    // 31 spectral samples (400..700 nm @ 10 nm) packed into 8 vec4 lanes.
    // Index i lives at sun_spectrum[i / 4][i % 4]; lane 31 is unused padding.
    sun_spectrum: array<vec4f, 8>,
}

struct RayResult {
    pixel_x: u32,
    pixel_y: u32,
    r: f32,
    g: f32,
    b: f32,
}

struct HitResult {
    did_hit: bool,
    triangle_index: u32,
    hit_point: vec3f,
}

// ── Bindings ───────────────────────────────────────────────────────────

@group(0) @binding(0) var<uniform> params: Params;
@group(0) @binding(1) var<storage, read_write> ray_buffer: array<RayResult>;

// ── Per-invocation state ───────────────────────────────────────────────

var<private> rng_state: u32;
var<private> vertices: array<vec3f, 24>;
var<private> tri_normal_cache: array<vec3f, NUM_TRIANGLES>;

// ── Crystal mesh connectivity ──────────────────────────────────────────

const TRI = array<vec3u, 44>(
    // Face 1 (upper basal)
    vec3u(0, 1, 3), vec3u(1, 2, 3), vec3u(0, 3, 4), vec3u(0, 4, 5),
    // Upper pyramid faces
    vec3u(0, 6, 1), vec3u(6, 7, 1), vec3u(1, 7, 2), vec3u(7, 8, 2),
    vec3u(2, 8, 3), vec3u(8, 9, 3), vec3u(3, 9, 4), vec3u(9, 10, 4),
    vec3u(4, 10, 5), vec3u(10, 11, 5), vec3u(5, 11, 0), vec3u(11, 6, 0),
    // Face 2 (lower basal)
    vec3u(18, 21, 19), vec3u(19, 21, 20), vec3u(18, 22, 21), vec3u(18, 23, 22),
    // Lower pyramid faces
    vec3u(12, 18, 13), vec3u(18, 19, 13), vec3u(13, 19, 14), vec3u(19, 20, 14),
    vec3u(14, 20, 15), vec3u(20, 21, 15), vec3u(15, 21, 16), vec3u(21, 22, 16),
    vec3u(16, 22, 17), vec3u(22, 23, 17), vec3u(17, 23, 12), vec3u(23, 18, 12),
    // Prism faces 3-8
    vec3u(6, 12, 7), vec3u(12, 13, 7),
    vec3u(7, 13, 8), vec3u(13, 14, 8),
    vec3u(8, 14, 9), vec3u(14, 15, 9),
    vec3u(9, 15, 10), vec3u(15, 16, 10),
    vec3u(10, 16, 11), vec3u(16, 17, 11),
    vec3u(11, 17, 6), vec3u(17, 12, 6)
);

// ── RNG ────────────────────────────────────────────────────────────────

fn wang_hash(a_in: u32) -> u32 {
    var a = a_in;
    a -= (a << 6u);
    a ^= (a >> 17u);
    a -= (a << 9u);
    a ^= (a << 4u);
    a -= (a << 3u);
    a ^= (a << 10u);
    a ^= (a >> 15u);
    return a;
}

fn rand_xorshift() -> u32 {
    rng_state ^= (rng_state << 13u);
    rng_state ^= (rng_state >> 17u);
    rng_state ^= (rng_state << 5u);
    return rng_state;
}

fn rand() -> f32 {
    return f32(rand_xorshift()) / 4294967295.0;
}

fn randn() -> vec2f {
    let u1 = sqrt(-2.0 * log(rand()));
    let u2 = 2.0 * PI * rand();
    return vec2f(u1 * cos(u2), u1 * sin(u2));
}

// ── CIE 1931 color matching ───────────────────────────────────────────

fn x_fit_1931(wave: f32) -> f32 {
    let t1 = (wave - 442.0) * select(0.0374, 0.0624, wave < 442.0);
    let t2 = (wave - 599.8) * select(0.0323, 0.0264, wave < 599.8);
    let t3 = (wave - 501.1) * select(0.0382, 0.0490, wave < 501.1);
    return 0.362 * exp(-0.5 * t1 * t1) + 1.056 * exp(-0.5 * t2 * t2) - 0.065 * exp(-0.5 * t3 * t3);
}

fn y_fit_1931(wave: f32) -> f32 {
    let t1 = (wave - 568.8) * select(0.0247, 0.0213, wave < 568.8);
    let t2 = (wave - 530.9) * select(0.0322, 0.0613, wave < 530.9);
    return 0.821 * exp(-0.5 * t1 * t1) + 0.286 * exp(-0.5 * t2 * t2);
}

fn z_fit_1931(wave: f32) -> f32 {
    let t1 = (wave - 437.0) * select(0.0278, 0.0845, wave < 437.0);
    let t2 = (wave - 459.0) * select(0.0725, 0.0385, wave < 459.0);
    return 1.217 * exp(-0.5 * t1 * t1) + 0.681 * exp(-0.5 * t2 * t2);
}

// ── Optics ─────────────────────────────────────────────────────────────

fn get_ice_ior(wavelength: f32) -> f32 {
    return 9.35698756194051e-8 * wavelength * wavelength
         - 1.42326056729702e-4 * wavelength
         + 1.36093233643442;
}

fn get_reflection_coefficient(normal: vec3f, ray_dir: vec3f, n0: f32, n1: f32) -> f32 {
    let incident_cos = dot(-ray_dir, normal);
    let incident_angle = acos(clamp(incident_cos, -1.0, 1.0));
    let sin_i = sin(incident_angle);
    if (n1 / n0 < sin_i) { return 1.0; }
    let transmitted_angle = asin(n0 * sin_i / n1);
    let transmitted_cos = cos(transmitted_angle);
    let rs = (n0 * incident_cos - n1 * transmitted_cos) / (n0 * incident_cos + n1 * transmitted_cos);
    let rp = (n0 * transmitted_cos - n1 * incident_cos) / (n0 * transmitted_cos + n1 * incident_cos);
    return 0.5 * (rs * rs + rp * rp);
}

// ── Ray-triangle intersection (Moller-Trumbore) ───────────────────────

fn find_intersection(ray_origin: vec3f, ray_direction: vec3f) -> HitResult {
    for (var i = 0u; i < NUM_TRIANGLES; i++) {
        let tri = TRI[i];
        let v0 = vertices[tri.x];
        // v1/v2 swapped to flip normal inward for interior tracing
        let v1 = vertices[tri.z];
        let v2 = vertices[tri.y];

        let v0v1 = v1 - v0;
        let v0v2 = v2 - v0;

        let p_vec = cross(ray_direction, v0v2);
        let det = dot(v0v1, p_vec);
        if (det < 0.000001) { continue; }

        let t_vec = ray_origin - v0;
        let u = dot(t_vec, p_vec);
        if (u < 0.0 || u > det) { continue; }

        let q_vec = cross(t_vec, v0v1);
        let v = dot(ray_direction, q_vec);
        if (v < 0.0 || u + v > det) { continue; }

        let t = dot(v0v2, q_vec) / det;
        return HitResult(true, i, ray_origin + t * ray_direction);
    }
    return HitResult(false, 0u, vec3f(0.0));
}

// ── Triangle selection & sampling ──────────────────────────────────────

fn select_first_triangle(ray_direction: vec3f) -> u32 {
    var projected_areas: array<f32, 44>;
    var sum_areas = 0.0;

    for (var i = 0u; i < NUM_TRIANGLES; i++) {
        let tri = TRI[i];
        let v0 = vertices[tri.x];
        let v1 = vertices[tri.y];
        let v2 = vertices[tri.z];
        let cp = cross(v1 - v0, v2 - v0);
        let area = 0.5 * length(cp);
        let normal = normalize(cp);
        tri_normal_cache[i] = normal;

        projected_areas[i] = max(0.0, area * dot(normal, -ray_direction));
        sum_areas += projected_areas[i];
    }

    var selector = rand() * sum_areas;
    for (var i = 0u; i < NUM_TRIANGLES; i++) {
        selector -= projected_areas[i];
        if (selector < 0.0) { return i; }
    }
    return 0u;
}

fn sample_triangle(triangle_index: u32) -> vec3f {
    let tri = TRI[triangle_index];
    let v0 = vertices[tri.x];
    let v1 = vertices[tri.y];
    let v2 = vertices[tri.z];
    var u = rand();
    var v = rand();
    if (u + v > 1.0) {
        u = 1.0 - u;
        v = 1.0 - v;
    }
    return v0 + u * (v1 - v0) + v * (v2 - v0);
}

fn get_normal(triangle_index: u32) -> vec3f {
    return tri_normal_cache[triangle_index];
}

// ── Ray tracing through crystal ────────────────────────────────────────

fn trace_ray(ray_origin_in: vec3f, ray_direction_in: vec3f, ior: f32) -> vec3f {
    var ro = ray_origin_in;
    var rd = ray_direction_in;
    for (var i = 0u; i < MAX_HITS; i++) {
        let hit = find_intersection(ro, rd);
        if (!hit.did_hit) { break; }
        let normal = -get_normal(hit.triangle_index);
        let refl_coeff = get_reflection_coefficient(normal, rd, ior, 1.0);
        if (rand() < refl_coeff) {
            ro = hit.hit_point;
            rd = reflect(rd, normal);
        } else {
            return refract(rd, normal, ior);
        }
    }
    return vec3f(0.0);
}

fn cast_ray_through_crystal(ray_direction: vec3f, wavelength: f32) -> vec3f {
    let tri_index = select_first_triangle(ray_direction);
    let start_point = sample_triangle(tri_index);
    let normal = get_normal(tri_index);
    let ior = get_ice_ior(wavelength);
    let refl_coeff = get_reflection_coefficient(normal, ray_direction, 1.0, ior);

    if (rand() < refl_coeff) {
        return reflect(ray_direction, normal);
    }
    let refracted = refract(ray_direction, normal, 1.0 / ior);
    return trace_ray(start_point, refracted, ior);
}

// ── Rotation matrices ──────────────────────────────────────────────────

fn rotate_x(angle: f32) -> mat3x3f {
    let c = cos(angle); let s = sin(angle);
    return mat3x3f(
        1.0, 0.0, 0.0,
        0.0, c,   s,
        0.0, -s,  c
    );
}

fn rotate_y(angle: f32) -> mat3x3f {
    let c = cos(angle); let s = sin(angle);
    return mat3x3f(
        c,   0.0, -s,
        0.0, 1.0, 0.0,
        s,   0.0, c
    );
}

fn rotate_z(angle: f32) -> mat3x3f {
    let c = cos(angle); let s = sin(angle);
    return mat3x3f(
        c,  s,   0.0,
        -s, c,   0.0,
        0.0, 0.0, 1.0
    );
}

fn rotate_2d(angle: f32, point: vec2f) -> vec2f {
    let c = cos(angle); let s = sin(angle);
    return mat2x2f(c, s, -s, c) * point;
}

fn outer_product(a: vec3f, b: vec3f) -> mat3x3f {
    return mat3x3f(a * b.x, a * b.y, a * b.z);
}

// ── Crystal orientation ────────────────────────────────────────────────

fn get_uniform_random_rotation() -> mat3x3f {
    // Fast Random Rotation Matrices (James Arvo)
    let theta = 2.0 * PI * rand();
    let phi = 2.0 * PI * rand();
    let z = rand();
    let ct = cos(theta); let st = sin(theta);
    let z_rot = mat3x3f(ct, -st, 0.0, st, ct, 0.0, 0.0, 0.0, 1.0);
    let v = vec3f(cos(phi) * sqrt(z), sin(phi) * sqrt(z), sqrt(1.0 - z));
    let I = mat3x3f(1, 0, 0, 0, 1, 0, 0, 0, 1);
    return (2.0 * outer_product(v, v) - I) * z_rot;
}

fn get_rotation_matrix() -> mat3x3f {
    if (params.tilt_distribution == DIST_UNIFORM && params.rotation_distribution == DIST_UNIFORM) {
        return get_uniform_random_rotation();
    }

    var tilt_mat: mat3x3f;
    if (params.tilt_distribution == DIST_UNIFORM) {
        tilt_mat = rotate_z(rand() * 2.0 * PI);
    } else {
        let tilt_angle = params.tilt_avg + params.tilt_std * randn().x;
        tilt_mat = rotate_z(-tilt_angle);
    }

    var rot_mat: mat3x3f;
    if (params.rotation_distribution == DIST_UNIFORM) {
        rot_mat = rotate_y(rand() * 2.0 * PI);
    } else {
        let rot_angle = params.rotation_avg + params.rotation_std * randn().x;
        rot_mat = rotate_y(rot_angle);
    }

    return rotate_y(rand() * 2.0 * PI) * tilt_mat * rot_mat;
}

// ── Camera ─────────────────────────────────────────────────────────────

fn get_camera_orientation_matrix() -> mat3x3f {
    return rotate_y(-params.camera_yaw) * rotate_x(-params.camera_pitch);
}

fn cartesian_to_polar(direction: vec3f) -> vec2f {
    let r = atan2(length(direction.xy), direction.z);
    let angle = atan2(direction.y, direction.x);
    return vec2f(r, angle);
}

// ── Sun ────────────────────────────────────────────────────────────────

fn get_sun_direction(altitude: f32) -> vec3f {
    return normalize(vec3f(0.0, sin(altitude), cos(altitude)));
}

fn sample_sun(altitude: f32) -> vec3f {
    let sun_center = get_sun_direction(altitude);
    let basis0 = vec3f(1.0, 0.0, 0.0);
    let basis1 = cross(sun_center, basis0);
    let angle = rand() * 2.0 * PI;
    let dist = sqrt(rand()) * 0.5 * params.sun_diameter;
    let offset = dist * (sin(angle) * basis0 + cos(angle) * basis1);
    return normalize(sun_center + offset);
}

fn daylight_estimate(wavelength: f32) -> f32 {
    return 1.0 - 0.0013333 * wavelength;
}

fn sample_sun_spectrum(wavelength: f32) -> f32 {
    let i_f = clamp((wavelength - 400.0) / 10.0, 0.0, 30.0);
    let i = u32(floor(i_f));
    let frac = i_f - f32(i);
    let j = i + 1u;
    let a = params.sun_spectrum[i / 4u][i % 4u];
    let b = params.sun_spectrum[j / 4u][j % 4u];
    return mix(a, b, frac);
}

// ── Crystal geometry ───────────────────────────────────────────────────

fn get_next(i: u32) -> u32 { return (i + 1u) % 6u; }
fn get_prev(i: u32) -> u32 { return (i + 5u) % 6u; }

fn get_prism_distance(i: u32) -> f32 {
    return params.prism_distances[i / 4u][i % 4u];
}

fn generate_apex_normals(apex_angle: f32) -> array<vec3f, 6> {
    var normals: array<vec3f, 6>;
    for (var i = 0u; i < 6u; i++) {
        let rot_angle = f32(i) * PI / 3.0;
        let half_apex = apex_angle / 2.0;
        normals[i] = rotate_y(rot_angle) * rotate_x(-half_apex) * vec3f(0.0, 0.0, 1.0);
    }
    return normals;
}

fn get_maximum_apex_height(
    normals: array<vec3f, 6>,
    apex_angle: f32,
    vertex_offset: u32
) -> f32 {
    var max_h = 1e38;

    for (var face = 0u; face < 6u; face++) {
        let pf = get_prev(face);
        let nf = get_next(face);
        let n_prev = normals[pf];
        let n_curr = normals[face];
        let n_next = normals[nf];
        let prev_vert = vertices[pf + vertex_offset];
        let next_vert = vertices[face + vertex_offset];

        let numerator = dot(prev_vert, n_prev) * cross(n_curr, n_next)
                      + dot(prev_vert, n_curr) * cross(n_next, n_prev)
                      + dot(next_vert, n_next) * cross(n_prev, n_curr);
        let det_mat = mat3x3f(n_prev, n_curr, n_next);
        let denom = determinant(det_mat);
        let face_h = abs((numerator / denom).y);
        max_h = min(face_h, max_h);
    }

    for (var i = 0u; i < 3u; i++) {
        let d = get_prism_distance(i);
        let d_opp = get_prism_distance(i + 3u);
        let h = (d + d_opp) / (2.0 * tan(apex_angle / 2.0));
        max_h = min(h, max_h);
    }

    return max_h;
}

fn initialize_crystal() {
    var hex: array<vec2f, 6>;

    for (var i = 0u; i < 6u; i++) {
        let d1 = get_prism_distance(i);
        let d2 = get_prism_distance(get_next(i));
        let angle = -f32(i) * PI / 3.0;
        let x_s = 2.0 * d2 / sqrt(3.0) - d1 / sqrt(3.0);
        hex[i] = rotate_2d(angle, vec2f(x_s, d1));
    }

    // Fix degenerate edges
    for (var face = 0u; face < 6u; face++) {
        let pf = get_prev(face);
        let nf = get_next(face);
        let angle = -f32(face) * PI / 3.0;
        let d1 = get_prism_distance(face);
        let d2 = get_prism_distance(nf);
        let d3 = get_prism_distance(pf);
        if (d1 > d2 + d3) {
            let x_s = d2 / sqrt(3.0) - d3 / sqrt(3.0);
            let pt = rotate_2d(angle, vec2f(x_s, d2 + d3));
            hex[face] = pt;
            hex[pf] = pt;
        }
    }

    // Scale so A axis length = 2
    let scaler = length(hex[1] - hex[4]);
    for (var i = 0u; i < 6u; i++) {
        hex[i] = hex[i] * (2.0 / scaler);
    }

    // Assign initial vertex positions (4 layers share x/z)
    for (var face = 0u; face < 6u; face++) {
        let v = vec3f(hex[face].x, 0.0, hex[face].y);
        vertices[face]       = v;
        vertices[face + 6u]  = v;
        vertices[face + 12u] = v;
        vertices[face + 18u] = v;
    }

    let rn = randn();
    let upper_h = clamp(params.upper_apex_height_avg + params.upper_apex_height_std * rn.x, 0.0, 1.0);
    let lower_h = clamp(params.lower_apex_height_avg + params.lower_apex_height_std * rn.y, 0.0, 1.0);

    // Upper pyramid cap
    if (upper_h > 0.0 && params.upper_apex_angle < PI && params.upper_apex_angle > 0.0) {
        let un = generate_apex_normals(params.upper_apex_angle);
        let mh = get_maximum_apex_height(un, params.upper_apex_angle, 0u);
        for (var i = 0u; i < 6u; i++) {
            let edge = cross(un[i], un[get_next(i)]);
            vertices[i] = vertices[i] + upper_h * mh * edge / edge.y;
        }
    }

    // Lower pyramid cap
    if (lower_h > 0.0 && params.lower_apex_angle < PI && params.lower_apex_angle > 0.0) {
        var ln = generate_apex_normals(params.lower_apex_angle);
        for (var i = 0u; i < 6u; i++) {
            ln[i] = ln[i] * vec3f(1.0, -1.0, 1.0);
        }
        let mh = get_maximum_apex_height(ln, params.lower_apex_angle, 18u);
        for (var i = 0u; i < 6u; i++) {
            let edge = cross(ln[i], ln[get_next(i)]);
            vertices[i + 18u] = vertices[i + 18u] - lower_h * mh * edge / edge.y;
        }
    }

    // Scale vertically for C/A ratio
    let ca = max(0.0, params.ca_ratio_avg + randn().x * params.ca_ratio_std);
    for (var i = 0u; i < 12u; i++) {
        vertices[i].y       += ca;
        vertices[i + 12u].y -= ca;
    }

    // Rotate for face-numbering convention
    let conv = rotate_y(-PI / 2.0);
    for (var i = 0u; i < 24u; i++) {
        vertices[i] = conv * vertices[i];
    }
}

// ── Entry point ────────────────────────────────────────────────────────

@compute @workgroup_size(64)
fn main(@builtin(global_invocation_id) global_id: vec3u) {
    let gid = global_id.x;
    rng_state = wang_hash(wang_hash(params.rng_seed) ^ gid);

    let halo_fade = clamp(
        (params.sun_altitude - MIN_SUN_ELEVATION) / (-MIN_SUN_ELEVATION),
        0.0, 1.0
    );
    if (halo_fade <= 0.0) {
        ray_buffer[gid] = RayResult(MISS, 0u, 0.0, 0.0, 0.0);
        return;
    }

    initialize_crystal();

    // Pick random wavelength 400-700 nm
    let wavelength = 400.0 + rand() * 300.0;

    // Incoming ray from sun (towards origin)
    let incident_sun = -sample_sun(params.sun_altitude);

    // Transform into crystal-oriented space
    let rot = get_rotation_matrix();
    let incident_crystal = normalize(transpose(rot) * incident_sun);

    // Trace through crystal
    let exitant_crystal = cast_ray_through_crystal(incident_crystal, wavelength);
    if (length(exitant_crystal) < 0.0001) {
        ray_buffer[gid] = RayResult(MISS, 0u, 0.0, 0.0, 0.0);
        return;
    }

    // Back to world space
    let exitant_ray = rot * exitant_crystal;

    // Sub-horizon filter
    if (params.camera_hide_sub_horizon == 1u && exitant_ray.y > 0.0) {
        ray_buffer[gid] = RayResult(MISS, 0u, 0.0, 0.0, 0.0);
        return;
    }

    // Camera projection
    let res = vec2f(f32(params.resolution_x), f32(params.resolution_y));
    let aspect = res.y / res.x;
    let exitant_cam = normalize(transpose(get_camera_orientation_matrix()) * exitant_ray);
    let light_dir = -exitant_cam;
    let polar = cartesian_to_polar(light_dir);
    let pr = polar.x;
    let pa = polar.y;

    var proj_fn: f32;
    if (params.camera_projection == PROJ_STEREOGRAPHIC) {
        proj_fn = 2.0 * tan(pr / 2.0);
    } else if (params.camera_projection == PROJ_RECTILINEAR) {
        if (pr > 0.5 * PI) { ray_buffer[gid] = RayResult(MISS, 0u, 0.0, 0.0, 0.0); return; }
        proj_fn = tan(pr);
    } else if (params.camera_projection == PROJ_EQUIDISTANT) {
        proj_fn = pr;
    } else if (params.camera_projection == PROJ_EQUAL_AREA) {
        proj_fn = 2.0 * sin(pr / 2.0);
    } else {
        // Orthographic
        if (pr > 0.5 * PI) { ray_buffer[gid] = RayResult(MISS, 0u, 0.0, 0.0, 0.0); return; }
        proj_fn = sin(pr);
    }

    let projected = params.camera_focal_length * proj_fn * vec2f(aspect * cos(pa), sin(pa));
    let nc = vec2f(0.5) + projected;

    if (nc.x <= 0.0 || nc.y <= 0.0 || nc.x >= 1.0 || nc.y >= 1.0) {
        ray_buffer[gid] = RayResult(MISS, 0u, 0.0, 0.0, 0.0);
        return;
    }

    // Spectral → sRGB linear
    var sun_rad: f32;
    if (params.atmosphere_enabled == 1u) {
        sun_rad = sample_sun_spectrum(wavelength);
    } else {
        sun_rad = daylight_estimate(wavelength);
    }
    let cie_xyz = sun_rad * vec3f(x_fit_1931(wavelength), y_fit_1931(wavelength), z_fit_1931(wavelength));
    let xyz_to_srgb = mat3x3f(
         3.24096994, -0.96924364,  0.05563008,
        -1.53738318,  1.87596750, -0.20397696,
        -0.49861076,  0.04155506,  1.05697151
    );
    let rgb = xyz_to_srgb * cie_xyz;

    let px = u32(res.x * nc.x);
    let py = u32(res.y * (1.0 - nc.y));
    ray_buffer[gid] = RayResult(px, py, rgb.x * halo_fade, rgb.y * halo_fade, rgb.z * halo_fade);
}
