R""(
#version 460 core

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D out_image;

const float PI = 3.14159;

struct intersection {
    uint triangleIndex;
    vec2 uv;
    vec3 hitPoint;
    float t;
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

uint intersectTriangleFromOutside(vec3 rayDirection)
{
    // Calculate triangle properties
    float triangleProjectedAreas[triangles.length()];
    float sumProjectedAreas = 0.0;
    for (int i = 0; i < triangles.length(); ++i)
    {
        vec3 v0 = vertices[triangles[i][0]];
        vec3 v1 = vertices[triangles[i][1]];
        vec3 v2 = vertices[triangles[i][2]];
        vec3 triangleCrossProduct = cross(v2 - v0, v1 - v0);
        float triangleArea = 0.5 * length(triangleCrossProduct);
        vec3 triangleNormal = normalize(triangleCrossProduct);

        triangleProjectedAreas[i] = max(0.0, triangleArea * dot(triangleNormal, -rayDirection));
        sumProjectedAreas += triangleProjectedAreas[i];
    }

    // Select triangle to hit
    float triangleSelector = rand() * sumProjectedAreas;
    float areaAccumulator = 0.0;
    int selectedTriangleIndex = -1;
    for (int i = 0; i < triangleProjectedAreas.length(); ++i)
    {
        areaAccumulator += triangleProjectedAreas[i];
        if (areaAccumulator > triangleSelector)
        {
            selectedTriangleIndex = i;
            break;
        }
    }

    return uint(selectedTriangleIndex);
}

vec3 sampleTriangle(uint triangleIndex)
{
    vec3 v0 = vertices[triangles[triangleIndex][0]];
    vec3 v1 = vertices[triangles[triangleIndex][1]];
    vec3 v2 = vertices[triangles[triangleIndex][2]];
    float u = rand();
    float v = rand();
    return (1.0 - u - v) * v0 + u * v1 + v * v2;
}

vec3 getNormal(uint triangleIndex)
{
    vec3 v0 = vertices[triangles[triangleIndex][0]];
    vec3 v1 = vertices[triangles[triangleIndex][1]];
    vec3 v2 = vertices[triangles[triangleIndex][2]];
    return normalize(cross(v2 - v0, v1 - v0));
}

float getReflectionCoefficient(vec3 normal, vec3 rayDir, float n0, float n1)
{
    // TODO: Calculate refracted angle smarter
    float incomingAngle = dot(normal, -rayDir);
    if (n1 / n0 < sin(incomingAngle)) return 1.0;

    vec3 refractedDir = refract(-rayDir, normal, n0 / n1);
    float refractedAngle = dot(-normal, refractedDir);
    float rs = abs((n0 * incomingAngle - n1 * refractedAngle) / (n0 * incomingAngle + n1 * refractedAngle));
    rs = rs * rs;
    float rp = abs((n0 * refractedAngle - n1 * incomingAngle) / (n0 * refractedAngle + n1 * incomingAngle));
    rp = rp * rp;
    return 0.5 * (rs + rp);
}

intersection findIntersection(vec3 rayOrigin, vec3 rayDirection)
{
    float maxT;
    int index = 0;
    vec2 barycentricHitpoint;
    for (int i = 0; i < triangles.length(); ++i)
    {
        vec3 v0 = vertices[triangles[i][0]];
        vec3 v1 = vertices[triangles[i][1]];
        vec3 v2 = vertices[triangles[i][2]];

        vec3 normal = cross(v1 - v0, v2 - v0);
        if (abs(dot(normal, rayDirection)) < 0.001) continue;
        float area = 0.5 * length(normal);

        // Check if ray is parallel to triangle
        float d = dot(normal, v0);
        float t = - (dot(normal, rayOrigin) + d) / dot(normal, rayDirection);
        if (t < 0.0) continue;

        vec3 c;
        vec2 uv;
        vec3 p = rayOrigin + t * rayDirection;

        // Edge 0
        vec3 edge0 = v1 - v0;
        vec3 vp0 = p - v0;
        c = cross(edge0, vp0);
        if (dot(normal, c) < 0) continue;

        // Edge 1
        vec3 edge1 = v2 - v1;
        vec3 vp1 = p - v1;
        c = cross(edge1, vp1);
        uv.x = 0.5 * length(c) / area;
        if (dot(normal, c) < 0) continue;

        // Edge 2
        vec3 edge2 = v0 - v2;
        vec3 vp2 = p - v2;
        c = cross(edge2, vp2);
        uv.y = 0.5 * length(c) / area;
        if (dot(normal, c) < 0) continue;

        // Checking for farthest intersection
        // There should be only 1-2 intersections, with the first one always
        // being an erroneous match with the triangle at the ray origin
        if (t > maxT)
        {
            maxT = t;
            index = i;
            barycentricHitpoint = uv;
        }
    }

    vec3 hitPoint = rayOrigin + maxT * rayDirection;

    return intersection(index, barycentricHitpoint, hitPoint, maxT);
}

vec3 traceRay(vec3 rayOrigin, vec3 rayDirection)
{
    vec3 ro = rayOrigin;
    vec3 rd = rayDirection;
    while (true)
    {
        intersection hitResult = findIntersection(ro, rd);
        vec3 normal = -getNormal(hitResult.triangleIndex);
        float reflectionCoefficient = getReflectionCoefficient(normal, rd, 1.31, 1.00);
        if (rand() < reflectionCoefficient)
        {
            // Ray refracts out of crystal
            return refract(-rd, normal, 1.31);
        } else {
            // Ray reflects back into crystal
            ro = hitResult.hitPoint;
            rd = reflect(-rd, normal);
        }
    }
}

void main(void)
{
    // Sun direction in alt-az
    vec2 sunDirection = radians(vec2(0.0, 0.0));

    // Hard coded arbitrary rotations for now
    float cRotation = radians(rand() * 360.0);
    float aRotation = radians(rand() * 360.0);
    float bRotation = radians(rand() * 360.0);

    // Corresponds to rotation around y axis
    mat3 cRotationMat = mat3(
        cos(cRotation), 0.0, -sin(cRotation),
        0.0, 1.0, 0.0,
        sin(cRotation), 0.0, cos(cRotation)
    );

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

    mat3 rotationMatrix = bRotationMat * aRotationMat * cRotationMat;
    mat3 inverseRotationMatrix = transpose(rotationMatrix);

    // Convert sun direction to incoming ray vector
    vec3 rayDirection = -vec3(
        sin(sunDirection.y) * cos(sunDirection.x),
        cos(sunDirection.y) * cos(sunDirection.x),
        sin(sunDirection.x)
        );

    rayDirection = rotationMatrix * rayDirection;

    uint triangleIndex = intersectTriangleFromOutside(rayDirection);
    vec3 startingPoint = sampleTriangle(triangleIndex);
    vec3 startingPointNormal = getNormal(triangleIndex);
    float reflectionCoeff = getReflectionCoefficient(startingPointNormal, rayDirection, 1.0, 1.31);
    vec3 resultingRay;
    if (rand() < reflectionCoeff)
    {
        // Ray reflects off crystal
        resultingRay = reflect(-rayDirection, startingPointNormal);
    } else {
        // Ray enters crystal
        vec3 refractedRayDirection = refract(-rayDirection, startingPointNormal, 1.0 / 1.31);
        resultingRay = traceRay(startingPoint, refractedRayDirection);
    }

    resultingRay = normalize(inverseRotationMatrix * resultingRay);

    // Convert cartesian direction vector to pixel coordinates
    vec2 resultAltAz = 0.5 + vec2(asin(resultingRay.z), atan(resultingRay.y / resultingRay.x)) / PI;
    ivec2 resolution = imageSize(out_image);
    ivec2 resultPixel = ivec2(resolution.x * resultAltAz.y, resolution.y * resultAltAz.x);

    imageStore(out_image, resultPixel, vec4(1.0));
}
)""
