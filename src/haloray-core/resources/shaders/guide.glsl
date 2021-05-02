#version 440 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 2, r32f) uniform image2D outputImage;

uniform struct sunProperties_t
{
    float altitude;
    float diameter;
} sun;

uniform struct camera_t
{
    float pitch;
    float yaw;
    float focalLength;
    int projection;
    int hideSubHorizon;
} camera;

#define PROJECTION_STEREOGRAPHIC 0
#define PROJECTION_RECTILINEAR 1
#define PROJECTION_EQUIDISTANT 2
#define PROJECTION_EQUAL_AREA 3
#define PROJECTION_ORTHOGRAPHIC 4

const float PI = 3.1415926535;
const float LINEWIDTHDEGREES = 0.25 / sqrt(camera.focalLength);
const float LINEWIDTH = LINEWIDTHDEGREES * PI / 180.0;

vec2 planarToPolar(vec2 point)
{
    float r = length(point);
    float angle = atan(point.y, point.x);
    return vec2(r, angle);
}

mat3 rotateAroundX(float angle)
{
    return mat3(
        1.0, 0.0, 0.0,
        0.0, cos(angle), sin(angle),
        0.0, -sin(angle), cos(angle)
    );
}

mat3 rotateAroundY(float angle)
{
    return mat3(
        cos(angle), 0.0, -sin(angle),
        0.0, 1.0, 0.0,
        sin(angle), 0.0, cos(angle)
    );
}

mat3 getCameraOrientationMatrix()
{
    return rotateAroundY(camera.yaw) * rotateAroundX(camera.pitch);
}

vec3 getSunVector()
{
    /* NOTE: The sun vector is now in the opposite Z direction
      than in the crystal raytracing shader. This should probably
      made the same in all shaders. */
    return normalize(vec3(0.0, sin(sun.altitude), -cos(sun.altitude)));
}

void main(void)
{
    ivec2 resolution = imageSize(outputImage);
    float aspectRatio = float(resolution.y) / float(resolution.x);

    ivec2 pixelCoordinates = ivec2(gl_GlobalInvocationID.xy);

    vec2 normCoord = pixelCoordinates / vec2(resolution) - 0.5;
    normCoord.x /= aspectRatio;
    vec2 polar = planarToPolar(normCoord);

    float projectedAngle;

    // The projection converts 2D coordinates to 3D vectors
    if (camera.projection == PROJECTION_STEREOGRAPHIC) {
        projectedAngle = 2.0 * atan(polar.x / 2.0 / camera.focalLength);
    } else if (camera.projection == PROJECTION_RECTILINEAR) {
        if (polar.x > 0.5 * PI) return;
        projectedAngle = atan(polar.x / camera.focalLength);
    } else if (camera.projection == PROJECTION_EQUIDISTANT) {
        projectedAngle = polar.x / camera.focalLength;
    } else if (camera.projection == PROJECTION_EQUAL_AREA) {
        projectedAngle = 2.0 * asin(polar.x / 2.0 / camera.focalLength);
    } else if (camera.projection == PROJECTION_ORTHOGRAPHIC) {
        if (polar.x > 0.5 * PI) return;
        projectedAngle = asin(polar.x / camera.focalLength);
    }

    if (projectedAngle > PI) return;

    float x = sin(projectedAngle) * cos(polar.y);
    float y = sin(projectedAngle) * sin(polar.y);
    float z = -cos(projectedAngle);

    vec3 dir = normalize(getCameraOrientationMatrix() * vec3(x, y, z));

    float horizonDistance = abs(dir.y);

    float zenithDistance = atan(length(dir.xz), dir.y);
    float nadirDistance = PI - zenithDistance;
    float minZenithNadirDistance = min(zenithDistance, nadirDistance);
    const float zenithNadirCrossRadius = 0.025;
    if (minZenithNadirDistance < zenithNadirCrossRadius) {
        float minAxisDistance = min(abs(dir.x), abs(dir.z));
        minZenithNadirDistance = minAxisDistance;
    }

    float sunDistance = acos(clamp(dot(getSunVector(), dir), -1.0, 1.0));
    float distance22r = abs(sunDistance - 22.0 / 180.0 * PI);
    float distance46r = abs(sunDistance - 46.0 / 180.0 * PI);
    float minRingDistance = min(distance22r, distance46r);

    float minDistance = min(min(minZenithNadirDistance, horizonDistance), minRingDistance);
    float value = 1.0 - smoothstep(LINEWIDTH * 0.3, LINEWIDTH * 0.5, minDistance);

    imageStore(outputImage, pixelCoordinates, vec4(value, 0.0, 0.0, 0.0));
}
