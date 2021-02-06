#version 440 core

#define DISTRIBUTION_UNIFORM 0
#define DISTRIBUTION_GAUSSIAN 1

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) uniform coherent image2D outputImage;
layout(binding = 1, r32ui) uniform coherent uimage2D spinlock;

uniform uint rngSeed;
uniform float multipleScatter;

uniform struct sunProperties_t
{
    float altitude;
    float diameter;
    float spectrum[31];
} sun;

uniform struct crystalProperties_t
{
    float caRatioAverage;
    float caRatioStd;

    int tiltDistribution;
    float tiltAverage;
    float tiltStd;

    int rotationDistribution;
    float rotationAverage;
    float rotationStd;
} crystalProperties;

#define PROJECTION_STEREOGRAPHIC 0
#define PROJECTION_RECTILINEAR 1
#define PROJECTION_EQUIDISTANT 2
#define PROJECTION_EQUAL_AREA 3
#define PROJECTION_ORTHOGRAPHIC 4

uniform struct camera_t
{
    float pitch;
    float yaw;
    float fov;
    int projection;
    int hideSubHorizon;
} camera;

uniform int atmosphereEnabled;

const float PI = 3.1415926535;

struct intersection {
    bool didHit;
    uint triangleIndex;
    vec3 hitPoint;
};

vec3 vertices[] = vec3[](
    vec3(0.0, 1.0, 1.0),
    vec3(-0.8660254038, 1.0, 0.5),
    vec3(-0.8660254038, 1.0, -0.5),
    vec3(0.0, 1.0, -1.0),
    vec3(0.8660254038, 1.0, -0.5),
    vec3(0.8660254038, 1.0, 0.5),

    vec3(0.0, -1.0, 1.0),
    vec3(-0.8660254038, -1.0, 0.5),
    vec3(-0.8660254038, -1.0, -0.5),
    vec3(0.0, -1.0, -1.0),
    vec3(0.8660254038, -1.0, -0.5),
    vec3(0.8660254038, -1.0, 0.5)
);

ivec3 triangles[] = ivec3[](
    // Face 1 (basal)
    ivec3(0, 1, 3),
    ivec3(1, 2, 3),
    ivec3(0, 3, 4),
    ivec3(0, 4, 5),

    // Face 2 (basal)
    ivec3(6, 9, 7),
    ivec3(7, 9, 8),
    ivec3(6, 10, 9),
    ivec3(6, 11, 10),

    // Face 3 (prism)
    ivec3(0, 6, 1),
    ivec3(6, 7, 1),

    // Face 4 (prism)
    ivec3(1, 7, 2),
    ivec3(7, 8, 2),

    // Face 5 (prism)
    ivec3(2, 8, 3),
    ivec3(8, 9, 3),

    // Face 6 (prism)
    ivec3(3, 9, 4),
    ivec3(9, 10, 4),

    // Face 7 (prism)
    ivec3(4, 10, 5),
    ivec3(10, 11, 5),

    // Face 8 (prism)
    ivec3(5, 11, 0),
    ivec3(11, 6, 0)
);

uint wang_hash(uint a)
{
	a -= (a << 6);
	a ^= (a >> 17);
	a -= (a << 9);
	a ^= (a << 4);
	a -= (a << 3);
	a ^= (a << 10);
	a ^= (a >> 15);
	return a;
}

uint rngState = wang_hash(rngSeed + uint(gl_WorkGroupID.x));

uint rand_xorshift(void)
 {
    // Xorshift algorithm from George Marsaglia's paper
    rngState ^= (rngState << 13);
    rngState ^= (rngState >> 17);
    rngState ^= (rngState << 5);
    return rngState;
 }

float rand(void) { return float(rand_xorshift()) / 4294967295.0; }

vec2 randn(void)
{
    float u1 = sqrt(-2.0 * log(rand()));
    float u2 = 2.0 * PI * rand();
    return vec2(u1 * cos(u2), u1 * sin(u2));
}

float xFit_1931(float wave)
{
    float t1 = (wave - 442.0) * ((wave < 442.0) ? 0.0624 : 0.0374);
    float t2 = (wave - 599.8) * ((wave < 599.8) ? 0.0264 : 0.0323);
    float t3 = (wave - 501.1) * ((wave < 501.1) ? 0.0490 : 0.0382);
    return 0.362 * exp(-0.5 * t1 * t1) + 1.056 * exp(-0.5 * t2 * t2) - 0.065f * exp(-0.5 * t3 * t3);
}

float yFit_1931(float wave)
{
    float t1 = (wave - 568.8) * ((wave < 568.8) ? 0.0213 : 0.0247);
    float t2 = (wave - 530.9) * ((wave < 530.9) ? 0.0613 : 0.0322);
    return 0.821 * exp(-0.5 * t1 * t1) + 0.286 * exp(-0.5 * t2 * t2);
}

float zFit_1931(float wave)
{
    float t1 = (wave - 437.0) * ((wave < 437.0) ? 0.0845 : 0.0278);
    float t2 = (wave - 459.0) * ((wave < 459.0) ? 0.0385 : 0.0725);
    return 1.217 * exp(-0.5 * t1 * t1) + 0.681 * exp(-0.5 * t2 * t2);
}

float getIceIOR(float wavelength)
{
    // Eq. from Simulating rainbows and halos in color by Stanley Gedzelman
    return 1.3203 - 0.0000333 * wavelength;
}

uint selectFirstTriangle(vec3 rayDirection)
{
    // Calculate triangle normals and projected areas
    float triangleProjectedAreas[triangles.length()];
    float sumProjectedAreas = 0.0;
    for (int i = 0; i < triangles.length(); ++i)
    {
        ivec3 triangle = triangles[i];
        vec3 v0 = vertices[triangle.x];
        vec3 v1 = vertices[triangle.y];
        vec3 v2 = vertices[triangle.z];
        vec3 triangleCrossProduct = cross(v2 - v0, v1 - v0);
        float triangleArea = 0.5 * length(triangleCrossProduct);
        vec3 triangleNormal = normalize(triangleCrossProduct);

        triangleProjectedAreas[i] = max(0.0, triangleArea * dot(triangleNormal, -rayDirection));
        sumProjectedAreas += triangleProjectedAreas[i];
    }

    // Select triangle to hit
    float triangleSelector = rand() * sumProjectedAreas;
    for (int i = 0; i < triangleProjectedAreas.length(); ++i)
    {
        triangleSelector -= triangleProjectedAreas[i];
        if (triangleSelector < 0.0)
        {
            return i;
        }
    }

    return 0;
}

vec3 sampleTriangle(uint triangleIndex)
{
    ivec3 triangle = triangles[triangleIndex];
    vec3 v0 = vertices[triangle.x];
    vec3 v1 = vertices[triangle.y];
    vec3 v2 = vertices[triangle.z];
    float u = rand();
    float v = rand();
    if (u + v > 1.0) {
        u = 1.0 - u;
        v = 1.0 - v;
    }

    return v0 + u * (v1 - v0) + v * (v2 - v0);
}

vec3 getNormal(uint triangleIndex)
{
    ivec3 triangle = triangles[triangleIndex];
    vec3 v0 = vertices[triangle.x];
    vec3 v1 = vertices[triangle.y];
    vec3 v2 = vertices[triangle.z];
    return normalize(cross(v1 - v0, v2 - v0));
}

float getReflectionCoefficient(vec3 normal, vec3 rayDir, float n0, float n1)
{
    float incidentAngle = acos(dot(-rayDir, normal));
    if (n1 / n0 < sin(incidentAngle)) return 1.0;
    float transmittedAngle = asin(n0 * sin(incidentAngle) / n1);
    float incidentCos = cos(incidentAngle);
    float transmittedCos = cos(transmittedAngle);
    float rs = (n0 * incidentCos - n1 * transmittedCos) / (n0 * incidentCos + n1 * transmittedCos);
    rs = rs * rs;
    float rp = (n0 * transmittedCos - n1 * incidentCos) / (n0 * transmittedCos + n1 * incidentCos);
    rp = rp * rp;
    return 0.5 * (rs + rp);
}

intersection findIntersection(vec3 rayOrigin, vec3 rayDirection)
{
    for (int triangleIndex = 0; triangleIndex < triangles.length(); ++triangleIndex)
    {
        ivec3 triangle = triangles[triangleIndex];
        vec3 v0 = vertices[triangle.x];
        vec3 v1 = vertices[triangle.y];
        vec3 v2 = vertices[triangle.z];

        vec3 v0v1 = v1 - v0;
        vec3 v0v2 = v2 - v0;

        vec3 pVec = cross(rayDirection, v0v2);
        float determinant = dot(v0v1, pVec);
        if (determinant < 0.000001) continue;

        vec3 tVec = rayOrigin - v0;
        float u = dot(tVec, pVec);
        if (u < 0.0 || u > determinant) continue;

        vec3 qVec = cross(tVec, v0v1);
        float v = dot(rayDirection, qVec);
        if (v < 0.0 || u + v > determinant) continue;

        float t = dot(v0v2, qVec) / determinant;

        return intersection(true, triangleIndex, rayOrigin + t * rayDirection);
    }

    return intersection(false, 0, vec3(0.0));
}

vec3 traceRay(vec3 rayOrigin, vec3 rayDirection, float indexOfRefraction)
{
    vec3 ro = rayOrigin;
    vec3 rd = rayDirection;
    for (int i = 0; i < 10; ++i)
    {
        intersection hitResult = findIntersection(ro, rd);
        if (hitResult.didHit == false) break;
        vec3 normal = getNormal(hitResult.triangleIndex);
        float reflectionCoefficient = getReflectionCoefficient(normal, rd, indexOfRefraction, 1.0);
        if (rand() < reflectionCoefficient)
        {
            // Ray reflects back into crystal
            ro = hitResult.hitPoint;
            rd = reflect(rd, normal);
        } else {
            // Ray refracts out of crystal
            return refract(rd, normal, indexOfRefraction);
        }
    }
    return vec3(0.0);
}

vec3 getSunDirection(float altitude)
{
    // X and Z are horizontal, sun moves on the Y-Z plane
    return normalize(vec3(
        0.0,
        sin(altitude),
        cos(altitude)
    ));
}

vec3 sampleSun(float altitude)
{
    vec3 sunCenterDirection = getSunDirection(altitude);

    // X axis is always perpendicular to the Y-Z plane
    vec3 diskBasis0 = vec3(1.0, 0.0, 0.0);
    vec3 diskBasis1 = cross(sunCenterDirection, diskBasis0);
    // Sample uniform point on disk
    float sampleAngle = rand() * 2.0 * PI;
    float sampleDistance = sqrt(rand()) * 0.5 * radians(sun.diameter);
    vec3 offset = sampleDistance * (sin(sampleAngle) * diskBasis0 + cos(sampleAngle) * diskBasis1);
    vec3 sampleDirection = sunCenterDirection + offset;
    return normalize(sampleDirection);
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

mat3 rotateAroundZ(float angle)
{
    return mat3(
        cos(angle), sin(angle), 0.0,
        -sin(angle), cos(angle), 0.0,
        0.0, 0.0, 1.0
    );
}

mat3 getCameraOrientationMatrix()
{
    return rotateAroundX(radians(camera.pitch)) * rotateAroundY(radians(camera.yaw));
}

mat3 getUniformRandomRotationMatrix(void)
{
    // From Fast Random Rotation Matrices, by James Arvo
    float theta = 2.0 * PI * rand();
    float phi = 2.0 * PI * rand();
    float z = rand();
    mat3 zRotationMatrix = mat3(cos(theta), -sin(theta), 0.0, sin(theta), cos(theta), 0.0, 0.0, 0.0, 1.0);
    vec3 reflectionVector = vec3(cos(phi) * sqrt(z), sin(phi) * sqrt(z), sqrt(1.0 - z));
    return (2.0 * outerProduct(reflectionVector, reflectionVector) - mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0)) * zRotationMatrix;
}

vec2 cartesianToPolar(vec3 direction)
{
    float r = acos(direction.z);
    float angle = atan(direction.y, direction.x);
    return vec2(r, angle);
}

mat3 getRotationMatrix(void)
{
    if (crystalProperties.tiltDistribution == DISTRIBUTION_UNIFORM && crystalProperties.rotationDistribution == DISTRIBUTION_UNIFORM)
    {
        return getUniformRandomRotationMatrix();
    }

    // Tilt of the crystal C-axis
    mat3 tiltMat;

    // Rotation around crystal C-axis
    mat3 rotationMat;

    if (crystalProperties.tiltDistribution == DISTRIBUTION_UNIFORM) {
        tiltMat = rotateAroundZ(rand() * 2.0 * PI);
    } else {
        float angleAverage = crystalProperties.tiltAverage;
        float angleStd = crystalProperties.tiltStd;
        float tiltAngle = radians(angleAverage + angleStd * randn().x);
        tiltMat = rotateAroundZ(tiltAngle);
    }

    if (crystalProperties.rotationDistribution == DISTRIBUTION_UNIFORM)
    {
        rotationMat = rotateAroundY(rand() * 2.0 * PI);
    } else {
        float angleAverage = crystalProperties.rotationAverage;
        float angleStd = crystalProperties.rotationStd;
        float rotationAngle = radians(angleAverage + angleStd * randn().x);
        rotationMat = rotateAroundY(rotationAngle);
    }

    return rotateAroundY(rand() * 2.0 * PI) * tiltMat * rotationMat;
}

float daylightEstimate(float wavelength)
{
    return 1.0 - 0.0013333 * wavelength;
}

float sampleSunSpectrum(float wavelength)
{
    int index = clamp(int(floor((wavelength - 400.0) / 10.0)), 0, 29);
    float wavelengthFract = (wavelength - (400.0 + index * 10.0)) / 10.0;
    return mix(sun.spectrum[index], sun.spectrum[index + 1], wavelengthFract);
}

void storePixel(ivec2 pixelCoordinates, vec3 value)
{
    bool keepWaiting = true;
    while (keepWaiting)
    {
        if (imageAtomicCompSwap(spinlock, pixelCoordinates, 0, 1) == 0)
        {
            vec3 currentValue = imageLoad(outputImage, pixelCoordinates).xyz;
            vec3 newValue = currentValue + value;
            imageStore(outputImage, pixelCoordinates, vec4(newValue, 1.0));
            memoryBarrier();
            keepWaiting = false;
            imageAtomicExchange(spinlock, pixelCoordinates, 0);
        }
    }
}

vec3 castRayThroughCrystal(vec3 rayDirection, float wavelength)
{
    uint triangleIndex = selectFirstTriangle(rayDirection);
    vec3 startingPoint = sampleTriangle(triangleIndex);
    vec3 startingPointNormal = -getNormal(triangleIndex);
    float indexOfRefraction = getIceIOR(wavelength);
    float reflectionCoeff = getReflectionCoefficient(startingPointNormal, rayDirection, 1.0, indexOfRefraction);
    vec3 resultRay = vec3(0.0);
    if (rand() < reflectionCoeff)
    {
        // Ray reflects off crystal
        resultRay = reflect(rayDirection, startingPointNormal);
    } else {
        // Ray enters crystal
        vec3 refractedRayDirection = refract(rayDirection, startingPointNormal, 1.0 / indexOfRefraction);
        resultRay = traceRay(startingPoint, refractedRayDirection, indexOfRefraction);
    }

    return resultRay;
}

void main(void)
{
    float caMultiplier = crystalProperties.caRatioAverage + randn().x * crystalProperties.caRatioStd;
    for (int i = 0; i < vertices.length(); ++i)
    {
        vertices[i].y *= max(0.0, caMultiplier);
    }

    vec3 rayDirection = -sampleSun(radians(sun.altitude));
    float wavelength = 400.0 + rand() * 300.0;

    // Rotation matrix to orient ray/crystal
    mat3 rotationMatrix = getRotationMatrix();

    /* The inverse rotation matrix must be applied because we are
    rotating the incoming ray and not the crystal itself. */
    vec3 rotatedRayDirection = normalize(rayDirection * rotationMatrix);

    vec3 resultRay = castRayThroughCrystal(rotatedRayDirection, wavelength);

    if (length(resultRay) < 0.0001) return;

    resultRay = rotationMatrix * resultRay;

    if (multipleScatter != 0.0 && multipleScatter > rand())
    {
        // Rotation matrix to orient ray/crystal
        rotationMatrix = getRotationMatrix();

        /* The inverse rotation matrix must be applied because we are
        rotating the incoming ray and not the crystal itself. */
        rotatedRayDirection = normalize(resultRay * rotationMatrix);

        resultRay = castRayThroughCrystal(rotatedRayDirection, wavelength);

        if (length(resultRay) < 0.0001) return;

        resultRay = rotationMatrix * resultRay;
    }

    // Do not render rays coming from the solar disk when the sky model renders a separate sun
    if (acos(dot(-resultRay, getSunDirection(radians(sun.altitude)))) < 1.05 * radians(sun.diameter / 2.0) && resultRay.y < 0.0)
        return;

    // Hide subhorizon rays
    if (camera.hideSubHorizon == 1 && resultRay.y > 0.0) return;

    ivec2 resolution = imageSize(outputImage);
    float aspectRatio = float(resolution.y) / float(resolution.x);

    resultRay = normalize(-getCameraOrientationMatrix() * resultRay);
    vec2 polar = cartesianToPolar(resultRay);

    float fovRadians = radians(camera.fov);
    float projectionFunction;
    float focalLength;

    if (camera.projection == PROJECTION_STEREOGRAPHIC) {
        projectionFunction = 2.0 * tan(polar.x / 2.0);
        focalLength = 1.0 / (4.0 * tan(fovRadians / 4.0));
    } else if (camera.projection == PROJECTION_RECTILINEAR) {
        if (polar.x > 0.5 * PI) return;
        projectionFunction = tan(polar.x);
        focalLength = 1.0 / (2.0 * tan(fovRadians / 2.0));
    } else if (camera.projection == PROJECTION_EQUIDISTANT) {
        projectionFunction = polar.x;
        focalLength = 1.0 / fovRadians;
    } else if (camera.projection == PROJECTION_EQUAL_AREA) {
        projectionFunction = 2.0 * sin(polar.x / 2.0);
        focalLength = 1.0 / (4.0 * sin(fovRadians / 4.0));
    } else if (camera.projection == PROJECTION_ORTHOGRAPHIC) {
        if (polar.x > 0.5 * PI) return;
        projectionFunction = sin(polar.x);
        focalLength = 1.0 / (2.0 * sin(fovRadians / 2.0));
    }

    vec2 projected = focalLength * projectionFunction * vec2(aspectRatio * cos(polar.y), sin(polar.y));
    vec2 normalizedCoordinates = 0.5 + projected;

    if (any(lessThanEqual(normalizedCoordinates, vec2(0.0))) || any(greaterThanEqual(normalizedCoordinates, vec2(1.0))))
        return;

    float sunRadiance;
    if (atmosphereEnabled == 1)
    {
        sunRadiance = sampleSunSpectrum(wavelength);
    } else {
        sunRadiance = daylightEstimate(wavelength);
    }

    ivec2 pixelCoordinates = ivec2(resolution.x * normalizedCoordinates.x, resolution.y * normalizedCoordinates.y);
    vec3 cieXYZ = sunRadiance * vec3(xFit_1931(wavelength), yFit_1931(wavelength), zFit_1931(wavelength));
    mat3 xyzToSrgb = mat3(3.24096994, -0.96924364, 0.05563008, -1.53738318, 1.8759675, -0.20397696, -0.49861076, 0.04155506, 1.05697151);
    storePixel(pixelCoordinates, xyzToSrgb * cieXYZ);
}
