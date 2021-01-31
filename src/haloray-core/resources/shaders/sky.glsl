#version 440 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0, rgba32f) uniform coherent image2D outputImage;

uniform struct sunProperties_t
{
    float altitude;
    float diameter;
} sun;

uniform struct camera_t
{
    float pitch;
    float yaw;
    float fov;
    int projection;
    int hideSubHorizon;
} camera;

#define PROJECTION_STEREOGRAPHIC 0
#define PROJECTION_RECTILINEAR 1
#define PROJECTION_EQUIDISTANT 2
#define PROJECTION_EQUAL_AREA 3
#define PROJECTION_ORTHOGRAPHIC 4

const float PI = 3.1415926535;

float perez(float zenithAngle, float sunAngle, float A, float B, float C, float D, float E)
{
    float firstTerm = 1.0f + A * exp(B / cos(zenithAngle));
    float secondTerm = 1.0f + C * exp(D * sunAngle) + E * cos(sunAngle) * cos(sunAngle);
    return firstTerm * secondTerm;
}

/*
  Sky shading model based on
  "A Practical Analytic Model for Daylight" (1999)
  by A. J. Preetham, Peter Shirley, Brian Smits
  University of Utah
*/

float luminance(float zenithAngle, float sunAngle, float turbidity)
{
    float sunZenithAngle = radians(90.0f - abs(sun.altitude));
    float ay = 0.1787 * turbidity - 1.4630;
    float by = -0.3554 * turbidity + 0.4275;
    float cy = -0.0227 * turbidity + 5.3251;
    float dy = 0.1206 * turbidity - 2.5771;
    float ey = -0.0670 * turbidity + 0.3703;
    float kappa = (4.0f / 9.0f - turbidity / 120.0f) * (PI - 2 * sunZenithAngle);
    float Yz = (4.0453 * turbidity - 4.9710) * tan(kappa) - 0.2155 * turbidity + 2.4192;
    float upperTerm = perez(zenithAngle, sunAngle, ay, by, cy, dy, ey);
    float lowerTerm = perez(0, sunZenithAngle, ay, by, cy, dy, ey);
    return Yz * upperTerm / lowerTerm;
}

float chromaX(float zenithAngle, float sunAngle, float turbidity)
{
    float sunZenithAngle = radians(90.0f - abs(sun.altitude));
    float ax = -0.0193 * turbidity - 0.2592;
    float bx = -0.0665 * turbidity + 0.0008;
    float cx = -0.0004 * turbidity + 0.2125;
    float dx = -0.0641 * turbidity - 0.8989;
    float ex = -0.0033 * turbidity + 0.0452;

    vec4 sunZenithVec = vec4(sunZenithAngle * sunZenithAngle * sunZenithAngle, sunZenithAngle * sunZenithAngle, sunZenithAngle, 1.0f);
    mat4 coefficientMatrix = mat4(
            0.00166, -0.02903, 0.11693, 0.0,
            -0.00375, 0.06377, -0.21196, 0.0,
            0.00209, -0.03202, 0.06052, 0.0,
            0.0, 0.00394, 0.25886, 0.0
    );

    vec4 turbidityVec = vec4(turbidity * turbidity, turbidity, 1.0, 0.0);

    float xz = dot(turbidityVec, coefficientMatrix * sunZenithVec);

    float upperTerm = perez(zenithAngle, sunAngle, ax, bx, cx, dx, ex);
    float lowerTerm = perez(0, sunZenithAngle, ax, bx, cx, dx, ex);
    return xz * upperTerm / lowerTerm;
}

float chromaY(float zenithAngle, float sunAngle, float turbidity)
{
    float sunZenithAngle = radians(90.0f - abs(sun.altitude));
    float ay = -0.0167 * turbidity - 0.2608;
    float by = -0.0950 * turbidity + 0.0092;
    float cy = -0.0079 * turbidity + 0.2102;
    float dy = -0.0441 * turbidity - 1.6537;
    float ey = -0.0109 * turbidity + 0.0529;

    vec4 sunZenithVec = vec4(sunZenithAngle * sunZenithAngle * sunZenithAngle, sunZenithAngle * sunZenithAngle, sunZenithAngle, 1.0f);
    mat4 coefficientMatrix = mat4(
            0.00275, -0.04214, 0.15346, 0.0,
            -0.00610, 0.08970, -0.26756, 0.0,
            0.00317, -0.04153, 0.06670, 0.0,
            0.0, 0.00516, 0.26688, 0.0
    );

    vec4 turbidityVec = vec4(turbidity * turbidity, turbidity, 1.0, 0.0);

    float yz = dot(turbidityVec, coefficientMatrix * sunZenithVec);

    float upperTerm = perez(zenithAngle, sunAngle, ay, by, cy, dy, ey);
    float lowerTerm = perez(0, sunZenithAngle, ay, by, cy, dy, ey);
    return yz * upperTerm / lowerTerm;
}

void main(void)
{
    ivec2 resolution = imageSize(outputImage);
    float aspectRatio = float(resolution.y) / float(resolution.x);
    ivec2 pixelCoordinates = ivec2(gl_GlobalInvocationID.xy);
    imageStore(outputImage, pixelCoordinates, vec4(0.5, 0.1, 0.0, 1.0));
}
