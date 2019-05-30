R""(
#version 460 core

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) uniform coherent image2D out_image;
layout(binding = 1, r32ui) uniform coherent uimage2D spinlock;

struct crystalProperties_t {
    float caRatioAverage;
    float caRatioStd;

    bool aRotationDistribution;
    float aRotationAverage;
    float aRotationStd;

    bool bRotationDistribution;
    float bRotationAverage;
    float bRotationStd;

    bool cRotationDistribution;
    float cRotationAverage;
    float cRotationStd;
};

struct sunProperties_t {
    float altitude;
    float azimuth;
    float diameter;
};

uniform struct sunProperties_t sun;
uniform struct crystalProperties_t crystalProperties;

const float PI = 3.14159;

struct intersection {
    bool didHit;
    uint triangleIndex;
    vec3 hitPoint;
};

vec3 vertices[] = vec3[](
    vec3(0.0, 1.0, 1.0),
    vec3(0.8660254038, 1.0, 0.5),
    vec3(0.8660254038, 1.0, -0.5),
    vec3(0.0, 1.0, -1.0),
    vec3(-0.8660254038, 1.0, -0.5),
    vec3(-0.8660254038, 1.0, 0.5),

    vec3(0.0, -1.0, 1.0),
    vec3(0.8660254038, -1.0, 0.5),
    vec3(0.8660254038, -1.0, -0.5),
    vec3(0.0, -1.0, -1.0),
    vec3(-0.8660254038, -1.0, -0.5),
    vec3(-0.8660254038, -1.0, 0.5)
);

ivec3 triangles[] = ivec3[](
    // Face 1 (basal)
    ivec3(0, 1, 2),
    ivec3(0, 2, 3),
    ivec3(0, 3, 5),
    ivec3(3, 4, 5),

    // Face 2 (basal)
    ivec3(6, 8, 7),
    ivec3(6, 9, 8),
    ivec3(6, 11, 9),
    ivec3(9, 11, 10),

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
	a -= (a<<6);
	a ^= (a>>17);
	a -= (a<<9);
	a ^= (a<<4);
	a -= (a<<3);
	a ^= (a<<10);
	a ^= (a>>15);
	return a;
}

uint rngState = wang_hash(uint(gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x));

uint rand_lcg(void)
{
	rngState = 1664525u * rngState + 1013904223u;
	return rngState;
}

float rand(void) { return rand_lcg() / 4294967295.0f; }

vec2 randn(void)
{
    float u1 = sqrt(-2.0 * log(rand()));
    float u2 = 2.0 * PI * rand();
    return vec2(u1 * cos(u2), u1 * sin(u2));
}

uint intersectFirstTriangle(vec3 rayDirection)
{
    // Calculate triangle properties
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
    return (1.0 - u - v) * v0 + u * v1 + v * v2;
}

vec3 getNormal(uint triangleIndex)
{
    ivec3 triangle = triangles[triangleIndex];
    vec3 v0 = vertices[triangle.x];
    vec3 v1 = vertices[triangle.y];
    vec3 v2 = vertices[triangle.z];
    return normalize(cross(v2 - v0, v1 - v0));
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
        if (determinant < 0.0001) continue;
        float inverseDeterminant = 1.0 / determinant;
        vec3 tVec = rayOrigin - v0;
        float u = dot(tVec, pVec) * inverseDeterminant;
        if (u < 0.0 || u > 1.0) continue;

        vec3 qVec = cross(tVec, v0v1);
        float v = dot(rayDirection, qVec) * inverseDeterminant;
        if (v < 0.0 || u + v > 1.0) continue;

        float t = dot(v0v2, qVec) * inverseDeterminant;

        return intersection(true, triangleIndex, rayOrigin + t * rayDirection);
    }
    return intersection(false, 0, vec3(0.0));
}

vec3 traceRay(vec3 rayOrigin, vec3 rayDirection)
{
    vec3 ro = rayOrigin;
    vec3 rd = rayDirection;
    for (int i = 0; i < 10; ++i)
    {
        intersection hitResult = findIntersection(ro, rd);
        if (hitResult.didHit == false) break;
        vec3 normal = -getNormal(hitResult.triangleIndex);
        float reflectionCoefficient = getReflectionCoefficient(normal, rd, 1.31, 1.0);
        if (rand() < reflectionCoefficient)
        {
            // Ray reflects back into crystal
            ro = hitResult.hitPoint;
            rd = reflect(rd, normal);
        } else {
            // Ray refracts out of crystal
            return refract(rd, normal, 1.31);
        }
    }
    return vec3(0.0);
}

vec3 altAzToCartesian(vec2 altAz)
{
    return normalize(vec3(
        cos(altAz.x) * sin(altAz.y),
        sin(altAz.x),
        cos(altAz.x) * cos(altAz.y)
        ));
}

vec2 cartesianToAltAz(vec3 direction)
{
    return vec2(asin(direction.y), atan(direction.x / direction.z));
}

vec2 altAzToPolar(vec2 altAz)
{
    return vec2(length(altAz), atan(altAz.y, altAz.x));
}

vec2 sampleSun(vec2 altAz)
{
    float sampleAngle = rand() * 2.0 * PI;
    float sampleDistance = sqrt(rand()) * 0.5 * radians(sun.diameter);
    return altAz + vec2(sampleDistance * sin(sampleAngle), sampleDistance * cos(sampleAngle));
}

void main(void)
{
    float caMultiplier = crystalProperties.caRatioAverage + randn().x * crystalProperties.caRatioStd;
    for (int i = 0; i < vertices.length(); ++i)
    {
        vertices[i].y *= max(0.001, caMultiplier);
    }

    // Sun direction in alt-az
    vec2 sunDirection = radians(vec2(sun.altitude, sun.azimuth));

    // Hard coded arbitrary rotations for now
    vec2 gaussianRotation = randn();
    float aRotation = radians(gaussianRotation.x * 5.0);
    float bRotation = radians(gaussianRotation.y * 5.0);
    float cRotation = radians(rand() * 360.0);

    // Corresponds to rotation around z axis
    mat3 aRotationMat = mat3(
        cos(aRotation), sin(aRotation), 0.0,
        -sin(aRotation), cos(aRotation), 0.0,
        0.0, 0.0, 1.0
    );

    // Corresponds to rotation around x axis
    mat3 bRotationMat = mat3(
        1.0, 0.0, 0.0,
        0.0, cos(bRotation), sin(bRotation),
        0.0, -sin(bRotation), cos(bRotation)
    );

    // Corresponds to rotation around y axis
    mat3 cRotationMat = mat3(
        cos(cRotation), 0.0, -sin(cRotation),
        0.0, 1.0, 0.0,
        sin(cRotation), 0.0, cos(cRotation)
    );

    mat3 rotationMatrix = cRotationMat * bRotationMat * aRotationMat;
    mat3 inverseRotationMatrix = transpose(rotationMatrix);

    // Convert sun direction to incoming ray vector
    vec3 rayDirection = -altAzToCartesian(sampleSun(sunDirection));

    vec3 rotatedRayDirection = rotationMatrix * rayDirection;

    uint triangleIndex = intersectFirstTriangle(rotatedRayDirection);
    vec3 startingPoint = sampleTriangle(triangleIndex);
    vec3 startingPointNormal = getNormal(triangleIndex);
    float reflectionCoeff = getReflectionCoefficient(startingPointNormal, rotatedRayDirection, 1.0, 1.31);
    vec3 resultRay = vec3(0.0);
    if (rand() < reflectionCoeff)
    {
        // Ray reflects off crystal
        resultRay = reflect(rotatedRayDirection, startingPointNormal);
    } else {
        // Ray enters crystal
        vec3 refractedRayDirection = refract(rotatedRayDirection, startingPointNormal, 1.0 / 1.31);
        resultRay = traceRay(startingPoint, refractedRayDirection);
    }
    resultRay = -normalize(inverseRotationMatrix * resultRay);

    ivec2 resolution = imageSize(out_image);
    float aspectRatio = float(resolution.x) / float(resolution.y);

    // Convert cartesian direction vector to pixel coordinates
    vec2 altAz = cartesianToAltAz(resultRay);
    vec2 polar = altAzToPolar(altAz);

    // Rectilinear projection
    float projectionFactor = 2.0 * tan(polar.r / 2.0);
    vec2 rectilinear = vec2(aspectRatio * projectionFactor * cos(polar.y), projectionFactor * sin(polar.y));
    vec2 normalizedCoordinates = 0.5 + rectilinear / PI;

    ivec2 pixelCoordinates = ivec2(resolution.x * normalizedCoordinates.y, resolution.y * normalizedCoordinates.x);

    bool keepWaiting = true;
    while (keepWaiting)
    {
        if (imageAtomicCompSwap(spinlock, pixelCoordinates, 0, 1) == 0)
        {
            vec3 currentValue = imageLoad(out_image, pixelCoordinates).xyz;
            vec3 newValue = currentValue + vec3(0.01);
            imageStore(out_image, pixelCoordinates, vec4(newValue, 1.0));
            memoryBarrier();
            keepWaiting = false;
            imageAtomicExchange(spinlock, pixelCoordinates, 0);
        }
    }
}
)""
