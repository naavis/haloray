R""(
#version 460 core

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D out_image;

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
        vec3 vertA = vertices[triangles[i][0]];
        vec3 vertB = vertices[triangles[i][1]];
        vec3 vertC = vertices[triangles[i][2]];
        vec3 triangleCrossProduct = cross(vertC - vertA, vertB - vertA);
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

void main(void)
{
    // Sun direction in alt-az
    vec2 sunDirection = vec2(radians(0.0));

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

    // TODO: Select point on triangle
    // TODO: Trace ray

    imageStore(out_image, ivec2(rand() * 1920, rand() * 1080), vec4(rand(), rand(), rand(), 1.0f));
}
)""
