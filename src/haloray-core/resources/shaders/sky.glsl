#version 440 core

layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 2, rgba32f) uniform coherent image2D outputImage;

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

uniform struct hosekSkyModelState_t
{
    float configs[3][9];
    float radiances[3];
    float turbidity;
    float solar_radius;
    float albedo;
    float elevation;
    float sunTopCIEXYZ[3];
    float sunBottomCIEXYZ[3];
    float limbDarkeningScaler[3];
} skyModelState;

#define PROJECTION_STEREOGRAPHIC 0
#define PROJECTION_RECTILINEAR 1
#define PROJECTION_EQUIDISTANT 2
#define PROJECTION_EQUAL_AREA 3
#define PROJECTION_ORTHOGRAPHIC 4

const float PI = 3.1415926535;
const float minSunElevation = -10.0;
const float mixingMaxElevation = 1.0;
const float mixingMinElevation = 0.0;



vec3 getSunVector()
{
    float sunAltitudeRadians = radians(sun.altitude);
    /* NOTE: The sun vector is now in the opposite Z direction
      than in the crystal raytracing shader. This should probably
      made the same in all shaders. */
    return normalize(vec3(0.0, sin(sunAltitudeRadians), -cos(sunAltitudeRadians)));
}

/*
  Sky shading model below based on
  "A Practical Analytic Model for Daylight" (1999)
  by A. J. Preetham, Peter Shirley, Brian Smits
  University of Utah
*/

float perez(float cosZenithAngle, float sunAngle, float A, float B, float C, float D, float E)
{
    float firstTerm = 1.0f + A * exp(B / cosZenithAngle);
    float secondTerm = 1.0f + C * exp(D * sunAngle) + E * cos(sunAngle) * cos(sunAngle);
    return firstTerm * secondTerm;
}

float luminance(float cosZenithAngle, float sunAngle, float turbidity)
{
    float sunZenithAngle = radians(90.0 - sun.altitude);
    float ay = 0.1787 * turbidity - 1.4630;
    float by = -0.3554 * turbidity + 0.4275;
    float cy = -0.0227 * turbidity + 5.3251;
    float dy = 0.1206 * turbidity - 2.5771;
    float ey = -0.0670 * turbidity + 0.3703;
    float kappa = (4.0 / 9.0 - turbidity / 120.0) * (PI - 2 * sunZenithAngle);
    float Yz = (4.0453 * turbidity - 4.9710) * tan(kappa) - 0.2155 * turbidity + 2.4192;
    float upperTerm = perez(cosZenithAngle, sunAngle, ay, by, cy, dy, ey);
    float lowerTerm = perez(1.0, sunZenithAngle, ay, by, cy, dy, ey);
    return Yz * upperTerm / lowerTerm;
}

float chromaX(float cosZenithAngle, float sunAngle, float turbidity)
{
    float sunZenithAngle = radians(90.0 - sun.altitude);
    float ax = -0.0193 * turbidity - 0.2592;
    float bx = -0.0665 * turbidity + 0.0008;
    float cx = -0.0004 * turbidity + 0.2125;
    float dx = -0.0641 * turbidity - 0.8989;
    float ex = -0.0033 * turbidity + 0.0452;

    vec4 sunZenithVec = vec4(sunZenithAngle * sunZenithAngle * sunZenithAngle, sunZenithAngle * sunZenithAngle, sunZenithAngle, 1.0);
    mat4 coefficientMatrix = mat4(
            0.00166, -0.02903, 0.11693, 0.0,
            -0.00375, 0.06377, -0.21196, 0.0,
            0.00209, -0.03202, 0.06052, 0.0,
            0.0, 0.00394, 0.25886, 0.0
    );

    vec4 turbidityVec = vec4(turbidity * turbidity, turbidity, 1.0, 0.0);

    float xz = dot(turbidityVec, coefficientMatrix * sunZenithVec);

    float upperTerm = perez(cosZenithAngle, sunAngle, ax, bx, cx, dx, ex);
    float lowerTerm = perez(1.0, sunZenithAngle, ax, bx, cx, dx, ex);
    return xz * upperTerm / lowerTerm;
}

float chromaY(float cosZenithAngle, float sunAngle, float turbidity)
{
    float sunZenithAngle = radians(90.0 - sun.altitude);
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

    float upperTerm = perez(cosZenithAngle, sunAngle, ay, by, cy, dy, ey);
    float lowerTerm = perez(1.0, sunZenithAngle, ay, by, cy, dy, ey);
    return yz * upperTerm / lowerTerm;
}

vec3 preethamSky(vec3 direction, float turbidity)
{
    vec3 sunVec = getSunVector();
    float sunAngle = acos(dot(sunVec, direction));
    /*
      Direction vector's zenith angle or theta equals
      acos(direction.y)
      */
    float cosZenithAngle = direction.y;
    float Y = luminance(cosZenithAngle, sunAngle, turbidity);
    float x = chromaX(cosZenithAngle, sunAngle, turbidity);
    float y = chromaY(cosZenithAngle, sunAngle, turbidity);
    vec3 XYZ = vec3(x * Y/y, Y, (1.0 - x - y) * Y/y);
    return XYZ;
}

/*
  Sky shading model below based on
  "An Analytics Model for Full Spectral Sky-Dome Radiance" (2012)
  by Lukas Hosek, Alexander Wilkie,
  Charles University in Prague
*/

vec3 hosekSky(vec3 direction, float turbidity)
{
    vec3 sunVec = getSunVector();
    float sunAngle = acos(dot(sunVec, direction));
    /*
      Direction vector's zenith angle or theta equals
      acos(direction.y)
      */
    float cosTheta = direction.y;
    float gamma = sunAngle;

    vec3 skyCIEXYZ;
    for (int channel = 0; channel < 3; ++channel)
    {
        float[9] configuration = skyModelState.configs[channel];
        float expM = exp(configuration[4] * gamma);
        float rayM = cos(gamma)*cos(gamma);
        float mieM = (1.0 + cos(gamma)*cos(gamma)) / pow((1.0 + configuration[8]*configuration[8] - 2.0*configuration[8]*cos(gamma)), 1.5);
        float zenith = sqrt(cosTheta);

        float temp = (1.0 + configuration[0] * exp(configuration[1] / (cosTheta + 0.01))) *
                (configuration[2] + configuration[3] * expM + configuration[5] * rayM + configuration[6] * mieM + configuration[7] * zenith);
        skyCIEXYZ[channel] = skyModelState.radiances[channel] * temp;
    }
    return skyCIEXYZ;
}

vec3 hosekPreethamMix(vec3 direction, float turbidity)
{
    vec3 skyCIEXYZ;
    if (sun.altitude >= mixingMaxElevation)
    {
        skyCIEXYZ = hosekSky(direction, turbidity);
    }
    else if (sun.altitude < mixingMaxElevation && sun.altitude >= mixingMinElevation)
    {
        vec3 preethamCIEXYZ = preethamSky(direction, turbidity);
        vec3 hosekCIEXYZ = hosekSky(direction, turbidity);
        float mixingFactor = (sun.altitude - mixingMinElevation) / (mixingMaxElevation - mixingMinElevation);
        skyCIEXYZ = mix(preethamCIEXYZ, hosekCIEXYZ, mixingFactor);
    }
    else
    {
        float mixingFactor = clamp((sun.altitude - minSunElevation) / (-minSunElevation), 0.0, 1.0);
        skyCIEXYZ = mix(vec3(0.0), preethamSky(direction, turbidity), mixingFactor);
    }

    return skyCIEXYZ;
}

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
    return rotateAroundY(radians(camera.yaw)) * rotateAroundX(radians(camera.pitch));
}

vec3 renderSun(vec3 direction)
{
    float sunAngle = acos(dot(direction, getSunVector()));
    if (sunAngle > skyModelState.solar_radius)
    {
        return vec3(0.0);
    }

    float rayElevation = asin(direction.y);
    float factor = (rayElevation - (skyModelState.elevation - skyModelState.solar_radius)) / (2.0 * skyModelState.solar_radius);
    float x = mix(skyModelState.sunBottomCIEXYZ[0], skyModelState.sunTopCIEXYZ[0], factor);
    float y = mix(skyModelState.sunBottomCIEXYZ[1], skyModelState.sunTopCIEXYZ[1], factor);
    float z = mix(skyModelState.sunBottomCIEXYZ[2], skyModelState.sunTopCIEXYZ[2], factor);

    float sol_rad_sin = sin(skyModelState.solar_radius);
    float ar2 = 1.0 / (sol_rad_sin * sol_rad_sin);
    float singamma = sin(sunAngle);
    float sc2 = 1.0 - ar2 * singamma * singamma;
    if (sc2 < 0.0 ) sc2 = 0.0;
    float sampleCosine = sqrt(sc2);

    vec3 plainRadiance = vec3(x, y, z);
    vec3 scaler = vec3(skyModelState.limbDarkeningScaler[0], skyModelState.limbDarkeningScaler[1], skyModelState.limbDarkeningScaler[2]);
    vec3 withLimbDarkening = mix(scaler * plainRadiance, plainRadiance, sampleCosine);

    return withLimbDarkening;
}

void main(void)
{
    if (sun.altitude < minSunElevation) return;

    ivec2 resolution = imageSize(outputImage);
    float aspectRatio = float(resolution.y) / float(resolution.x);
    ivec2 pixelCoordinates = ivec2(gl_GlobalInvocationID.xy);

    vec2 normCoord = vec2(pixelCoordinates) / vec2(resolution) - 0.5;
    normCoord.x /= aspectRatio;
    vec2 polar = planarToPolar(normCoord);

    float fovRadians = radians(camera.fov);
    float projectedAngle;

    if (camera.projection == PROJECTION_STEREOGRAPHIC) {
        float focalLength = 1.0 / (4.0 * tan(fovRadians / 4.0));
        projectedAngle = 2.0 * atan(polar.x / 2.0 / focalLength);
    } else if (camera.projection == PROJECTION_RECTILINEAR) {
        if (polar.x > 0.5 * PI) return;
        float focalLength = 1.0 / (2.0 * tan(fovRadians / 2.0));
        projectedAngle = atan(polar.x / focalLength);
    } else if (camera.projection == PROJECTION_EQUIDISTANT) {
        float focalLength = 1.0 / fovRadians;
        projectedAngle = polar.x / focalLength;
    } else if (camera.projection == PROJECTION_EQUAL_AREA) {
        float focalLength = 1.0 / (4.0 * sin(fovRadians / 4.0));
        projectedAngle = 2.0 * asin(polar.x / 2.0 / focalLength);
    } else if (camera.projection == PROJECTION_ORTHOGRAPHIC) {
        if (polar.x > 0.5 * PI) return;
        float focalLength = 1.0 / (2.0 * sin(fovRadians / 2.0));
        projectedAngle = asin(polar.x / focalLength);
    }

    float x = sin(projectedAngle) * cos(polar.y);
    float y = sin(projectedAngle) * sin(polar.y);
    float z = -cos(projectedAngle);

    vec3 dir = normalize(getCameraOrientationMatrix() * vec3(x, y, z));
    if (dir.y < 0.0) return;

    vec3 skyCIEXYZ = hosekPreethamMix(dir, skyModelState.turbidity);
    vec3 sunCIEXYZ = renderSun(dir);
    vec3 resultCIEXYZ = skyCIEXYZ + sunCIEXYZ;

    mat3 xyzToSrgb = mat3(3.24096994, -0.96924364, 0.05563008, -1.53738318, 1.8759675, -0.20397696, -0.49861076, 0.04155506, 1.05697151);
    imageStore(outputImage, pixelCoordinates, vec4(xyzToSrgb * resultCIEXYZ, 1.0));
}
