#version 440 core

layout(local_size_x = 64) in;
layout(binding = 0, rgba32f) uniform coherent image2D outputImage;

/* MAX_HITS defines how many times
   a ray of light is allowed to bounce inside
   the ice crystal before it is abandoned */
#define MAX_HITS 100

/* RAY_REUSE defines how many times a given
   scattering angle is reused before raytracing
   through a new crystal to speed up computations.
   Value of 20 was recommended in "An improved
   algorithm for simulations of divergent-light
   halos" by Gislen et al. */
#define SCATTERING_ANGLE_REUSE 20

uniform uint rngSeed;
uniform float multipleScatter;

uniform struct sunProperties_t
{
    float altitude;
    float diameter;
    float spectrum[31];
} sun;

#define DISTRIBUTION_UNIFORM 0
#define DISTRIBUTION_GAUSSIAN 1

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

    float upperApexAngle;
    float upperApexHeightAverage;
    float upperApexHeightStd;

    float lowerApexAngle;
    float lowerApexHeightAverage;
    float lowerApexHeightStd;

    float prismFaceDistances[6];
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
    float focalLength;
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

vec3 vertices[24];

ivec3 triangles[] = ivec3[](
    // Face 1 (basal)
    ivec3(0, 1, 3),
    ivec3(1, 2, 3),
    ivec3(0, 3, 4),
    ivec3(0, 4, 5),

    // Upper pyramid faces
    ivec3(0, 6, 1),
    ivec3(6, 7, 1),
    ivec3(1, 7, 2),
    ivec3(7, 8, 2),
    ivec3(2, 8, 3),
    ivec3(8, 9, 3),
    ivec3(3, 9, 4),
    ivec3(9, 10, 4),
    ivec3(4, 10, 5),
    ivec3(10, 11, 5),
    ivec3(5, 11, 0),
    ivec3(11, 6, 0),

    // Face 2 (basal)
    ivec3(18, 21, 19),
    ivec3(19, 21, 20),
    ivec3(18, 22, 21),
    ivec3(18, 23, 22),

    // Lower pyramid faces
    ivec3(12, 18, 13),
    ivec3(18, 19, 13),
    ivec3(13, 19, 14),
    ivec3(19, 20, 14),
    ivec3(14, 20, 15),
    ivec3(20, 21, 15),
    ivec3(15, 21, 16),
    ivec3(21, 22, 16),
    ivec3(16, 22, 17),
    ivec3(22, 23, 17),
    ivec3(17, 23, 12),
    ivec3(23, 18, 12),

    // Face 3 (prism)
    ivec3(6, 12, 7),
    ivec3(12, 13, 7),

    // Face 4 (prism)
    ivec3(7, 13, 8),
    ivec3(13, 14, 8),

    // Face 5 (prism)
    ivec3(8, 14, 9),
    ivec3(14, 15, 9),

    // Face 6 (prism)
    ivec3(9, 15, 10),
    ivec3(15, 16, 10),

    // Face 7 (prism)
    ivec3(10, 16, 11),
    ivec3(16, 17, 11),

    // Face 8 (prism)
    ivec3(11, 17, 6),
    ivec3(17, 12, 6)
);

vec3 triangleNormalCache[triangles.length()];

// ***********************************
// Random number generator functions *
// ***********************************

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

uint rngState = wang_hash(rngSeed + uint(gl_GlobalInvocationID.x));

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

// **************************
// Color matching functions *
// **************************

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

// ***********************
// Ray tracing functions *
// ***********************

vec3 randomRay(void)
{
    vec3 ray;
    ray.x = randn().x;
    ray.y = randn().x;
    ray.z = randn().x;
    return normalize(ray);
}

float getIceIOR(float wavelength)
{
    // The index of refraction is based on a second degree polynomial fitted to data from
    // "Optical constants of ice from the ultraviolet to the microwave" by Warren and Brandt
    // The raw data is available here: https://atmos.uw.edu/ice_optical_constants/
    return 9.35698756194051e-8 * wavelength * wavelength - 1.42326056729702e-4 * wavelength + 1.36093233643442;
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
        vec3 triangleCrossProduct = cross(v1 - v0, v2 - v0);
        float triangleArea = 0.5 * length(triangleCrossProduct);
        vec3 triangleNormal = normalize(triangleCrossProduct);
        triangleNormalCache[i] = triangleNormal;

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
    return triangleNormalCache[triangleIndex];
}

float getReflectionCoefficient(vec3 normal, vec3 rayDir, float n0, float n1)
{
    float incidentCos = dot(-rayDir, normal);
    float incidentAngle = acos(incidentCos);
    if (n1 / n0 < sin(incidentAngle)) return 1.0;
    float transmittedAngle = asin(n0 * sin(incidentAngle) / n1);
    float transmittedCos = cos(transmittedAngle);
    float rs = (n0 * incidentCos - n1 * transmittedCos) / (n0 * incidentCos + n1 * transmittedCos);
    rs = rs * rs;
    float rp = (n0 * transmittedCos - n1 * incidentCos) / (n0 * transmittedCos + n1 * incidentCos);
    rp = rp * rp;
    return 0.5 * (rs + rp);
}

// Find triangle which ray intersects with using
// the Trumbore-Möller algorithm.
intersection findIntersection(vec3 rayOrigin, vec3 rayDirection)
{
    for (int triangleIndex = 0; triangleIndex < triangles.length(); ++triangleIndex)
    {
        // Note the ordering of the vertices!
        // v1 and v2 are switched, because with
        // the conventional ordering the triangle
        // normal ends up pointing outwards, and it
        // must point inwards when tracing rays
        // inside the ice crystal.
        ivec3 triangle = triangles[triangleIndex];
        vec3 v0 = vertices[triangle.x];
        vec3 v1 = vertices[triangle.z];
        vec3 v2 = vertices[triangle.y];

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
    for (int i = 0; i < MAX_HITS; ++i)
    {
        intersection hitResult = findIntersection(ro, rd);
        if (hitResult.didHit == false) break;
        vec3 normal = -getNormal(hitResult.triangleIndex);
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

vec3 castRayThroughCrystal(vec3 rayDirection, float wavelength)
{
    uint triangleIndex = selectFirstTriangle(rayDirection);
    vec3 startingPoint = sampleTriangle(triangleIndex);
    vec3 startingPointNormal = getNormal(triangleIndex);
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

// ***********************************************
// Light source direction and spectrum functions *
// ***********************************************

// The light source is on the YZ plane, in the negative Z direction
vec3 getLightSourceVector(float elevation, float distance)
{
    return distance * vec3(0.0, sin(elevation), -cos(elevation));
}

float daylightEstimate(float wavelength)
{
    return 1.0 - 0.0013333 * wavelength;
}

// ********************
// Rotation functions *
// ********************

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

vec2 rotate2D(float angle, vec2 point)
{
    return mat2(cos(angle), sin(angle), -sin(angle), cos(angle)) * point;
}

// ******************************************
// Camera and crystal orientation functions *
// ******************************************

mat3 getCameraOrientationMatrix(void)
{
    return rotateAroundY(-camera.yaw) * rotateAroundX(-camera.pitch);
}

mat3 getRotationMatrixToMatchVectorPairs(vec3 target1, vec3 target2, vec3 vector1, vec3 vector2)
{
    // Based on TRIAD: https://en.wikipedia.org/wiki/Triad_method
    vec3 t1 = normalize(target1);
    vec3 t2 = normalize(target2);
    vec3 v1 = normalize(vector1);
    vec3 v2 = normalize(vector2);

    vec3 m1 = normalize(cross(t1, t2));
    vec3 m2 = normalize(cross(v1, v2));

    mat3 rot1 = mat3(t1, m1, cross(t1, m1));
    mat3 rot2 = mat3(v1, m2, cross(v1, m2));

    return rot1 * transpose(rot2);
}

// ***************************************
// Crystal geometry generation functions *
// ***************************************

int getNext(int i)
{
    return int(mod(i + 1, 6));
}

int getPrev(int i)
{
    return int(mod(mod(i - 1, 6) + 6, 6));
}

vec3[6] generateApexNormals(float apexAngle)
{
    vec3 apexNormals[6];
    for (int i = 0; i < 6; ++i)
    {
        float rotAngle = i * PI / 3.0;
        float halfApex = apexAngle / 2.0;
        apexNormals[i] = rotateAroundY(rotAngle) * rotateAroundX(-halfApex) * vec3(0.0, 0.0, 1.0);
    }
    return apexNormals;
}

float getMaximumApexHeight(vec3 normals[6],
                           vec3 vertices[24],
                           float prismFaceDistances[6],
                           float apexAngle,
                           int vertexOffset)
{
    float maxApexHeight = 1e38;
    // Find non-convex pyramid face heights
    for (int face = 0; face < 6; ++face)
    {
        int prevFace = getPrev(face);
        int nextFace = getNext(face);

        vec3 nPrev = normals[prevFace];
        vec3 nCurr = normals[face];
        vec3 nNext = normals[nextFace];

        vec3 prevVert = vertices[prevFace + vertexOffset];
        vec3 nextVert = vertices[face + vertexOffset];

        vec3 numerator = dot(prevVert, nPrev) * cross(nCurr, nNext) +
                         dot(prevVert, nCurr) * cross(nNext, nPrev) +
                         dot(nextVert, nNext) * cross(nPrev, nCurr);
        mat3 detMatrix = mat3(nPrev, nCurr, nNext);
        float denominator = determinant(detMatrix);
        float faceHeight = abs((numerator / denominator).y);
        maxApexHeight = min(faceHeight, maxApexHeight);
    }

    // Find maximum apex heights based on prism face distances and apex angle
    for (int i = 0; i < 3; ++i)
    {
        float dist = prismFaceDistances[i];
        float distOpposite = prismFaceDistances[i + 3];
        float h = (dist + distOpposite) / (2.0 * tan(apexAngle / 2.0));
        maxApexHeight = min(h, maxApexHeight);
    }

    return maxApexHeight;
}

void initializeCrystal()
{
    vec2 hexagonCorners[6];

    // Calculate initial hexagon shape
    for (int i = 0; i < 6; ++i)
    {
        float d1 = crystalProperties.prismFaceDistances[i];
        float d2 = crystalProperties.prismFaceDistances[getNext(i)];

        float angle = -i * PI / 3.0;
        float x_stat = 2.0 * d2 / sqrt(3.0) - d1 / sqrt(3.0);
        float y_stat = d1;
        hexagonCorners[i] = rotate2D(angle, vec2(x_stat, y_stat));
    }

    // Fix degenerate edges on hexagon
    for (int face = 0; face < 6; ++face)
    {
        int prevFace = getPrev(face);
        int nextFace = getNext(face);
        float angle = -face * PI / 3.0;

        float d1 = crystalProperties.prismFaceDistances[face];
        float d2 = crystalProperties.prismFaceDistances[nextFace];
        float d3 = crystalProperties.prismFaceDistances[prevFace];
        if (d1 > d2 + d3)
        {
            float x_stat = d2 / sqrt(3.0) - d3 / sqrt(3.0);
            float y_stat = d2 + d3;
            vec2 rotatedPoint = rotate2D(angle, vec2(x_stat, y_stat));
            hexagonCorners[face] = rotatedPoint;
            hexagonCorners[prevFace] = rotatedPoint;
        }
    }

    /* Scaling value makes sure eventual A axis length is 2.0, so C/A ratio
     * can be easily corrected. */
    float hexagonScaler = length(hexagonCorners[1] - hexagonCorners[4]);
    for (int i = 0; i < 6; ++i)
    {
        hexagonCorners[i] *= 2.0 / hexagonScaler;
    }

    for (int face = 0; face < 6; ++face)
    {
        // Corresponding vertices of each crystal layer have the same X and Z coordinates
        // First six vertices are the top apex cap
        // Next six vertices are the top of the base hexagonal crystal
        // Next six vertices are the bottom of the base hexagonal crystal
        // Last six vertices are the bottom apex cap
        vec3 v = vec3(hexagonCorners[face].x, 0.0, hexagonCorners[face].y);
        vertices[face] = v;
        vertices[face + 6] = v;
        vertices[face + 12] = v;
        vertices[face + 18] = v;
    }

    vec2 random = randn();
    float upperApexHeight = clamp(crystalProperties.upperApexHeightAverage + crystalProperties.upperApexHeightStd * random.x, 0.0, 1.0);
    float lowerApexHeight = clamp(crystalProperties.lowerApexHeightAverage + crystalProperties.lowerApexHeightStd * random.y, 0.0, 1.0);

    if (upperApexHeight > 0.0 && crystalProperties.upperApexAngle < PI && crystalProperties.upperApexAngle > 0.0)
    {
        // Generate normals for upper pyramid cap
        vec3 upperApexNormals[6] = generateApexNormals(crystalProperties.upperApexAngle);

        // Set upper pyramid cap vertex positions
        float maxUpperApexHeight = getMaximumApexHeight(upperApexNormals, vertices, crystalProperties.prismFaceDistances, crystalProperties.upperApexAngle, 0);
        for (int i = 0; i < 6; ++i)
        {
            int next = getNext(i);
            vec3 pyramidEdge = cross(upperApexNormals[i], upperApexNormals[next]);
            vertices[i] += upperApexHeight * maxUpperApexHeight * pyramidEdge / pyramidEdge.y;
        }
    }

    if (lowerApexHeight > 0.0 && crystalProperties.lowerApexAngle < PI && crystalProperties.lowerApexAngle > 0.0)
    {
        // Generate normals for lower pyramid cap
        vec3 lowerApexNormals[6] = generateApexNormals(crystalProperties.lowerApexAngle);
        for (int i = 0; i < 6; ++i)
        {
            lowerApexNormals[i].y *= -1.0;
        }

        // Set lower pyramid cap vertex positions
        float maxLowerApexHeight = getMaximumApexHeight(lowerApexNormals, vertices, crystalProperties.prismFaceDistances, crystalProperties.lowerApexAngle, 18);
        for (int i = 0; i < 6; ++i)
        {
            int next = getNext(i);
            vec3 pyramidEdge = cross(lowerApexNormals[i], lowerApexNormals[next]);
            vertices[i + 18] -= lowerApexHeight * maxLowerApexHeight * pyramidEdge / pyramidEdge.y;
        }
    }

    // Scale crystal vertically to have correct C/A ratio
    float caRatio = max(0.0, crystalProperties.caRatioAverage + randn().x * crystalProperties.caRatioStd);
    for (int i = 0; i < 12; ++i)
    {
        vertices[i].y += caRatio;
        vertices[i + 12].y -= caRatio;
    }

    // Rotate crystal around C-axis so that face numbering follows conventions
    // Prism face 0 (Face 3 in the UI) should be up in a column Parry position
    mat3 conventionMatrix = rotateAroundY(-PI / 2.0);
    for (int i = 0; i < 24; ++i)
    {
        vertices[i] = conventionMatrix * vertices[i];
    }
}

// ******************************
// Ray probability calculations *
// ******************************

float normalDistribution(float mean, float sigma, float x)
{
    float z = (x - mean) / sigma;
    return exp(-0.5 * z * z) / (sigma * sqrt(2.0 * PI));
}

float getOrientationWeight(mat3 standardToWorldMatrix)
{
    float tiltWeight = 0.0;
    float cAxisRotationWeight = 0.0;

    if (crystalProperties.tiltDistribution == DISTRIBUTION_GAUSSIAN) {
        // If the tilt standard deviation is 0.0, it is statistically impossible that
        // a random crystal distribution would equal the given tilt average value, which
        // will result in a black image. This is a shortcut to the same result.
        if (crystalProperties.tiltStd == 0.0) return 0.0;
        float tilt = acos(abs(standardToWorldMatrix[1][1]));
        tiltWeight = normalDistribution(crystalProperties.tiltAverage, crystalProperties.tiltStd, tilt);
    } else {
        tiltWeight = 1.0;
    }

    if (crystalProperties.rotationDistribution == DISTRIBUTION_GAUSSIAN) {
        // If the rotation standard deviation is 0.0, it is statistically impossible that
        // a random crystal distribution would equal the given rotation average value, which
        // will result in a black image. This is a shortcut to the same result.
        if (crystalProperties.rotationStd == 0.0) return 0.0;
        float rotation = mod(atan(standardToWorldMatrix[0][0], standardToWorldMatrix[2][0]), radians(60.0));
        cAxisRotationWeight = normalDistribution(crystalProperties.rotationAverage + radians(30.0), crystalProperties.rotationStd, rotation);
    } else {
        cAxisRotationWeight = 1.0;
    }

    return tiltWeight * cAxisRotationWeight;
}

float getMinnaertCigarWeight(float scatteringAngle)
{
    float cigarWeight = 0.0;

    float limitAngle = radians(178.0);
    if (scatteringAngle < limitAngle) {
        cigarWeight = scatteringAngle / sin(scatteringAngle);
    } else {
        cigarWeight = limitAngle / sin(limitAngle);
    }

    return cigarWeight;
}

// ***********
// Utilities *
// ***********

// Convert 3D unit vector to polar coordinates in the XY plane
vec2 cartesianToPolar(vec3 direction)
{
    float r = atan(length(direction.xy), direction.z);
    float angle = atan(direction.y, direction.x);
    return vec2(r, angle);
}

void storePixel(ivec2 pixelCoordinates, vec3 value)
{
    memoryBarrierImage();
    vec3 currentValue = imageLoad(outputImage, pixelCoordinates).xyz;
    imageStore(outputImage, pixelCoordinates, vec4(min(currentValue + value, 3.402823466e+38), 1.0));
}

// ************
// Main logic *
// ************

void main(void)
{
    initializeCrystal();

    // Pick ray wavelength in nanometers
    float wavelength = 400.0 + rand() * 300.0;

    float elevation = sun.altitude;
    float distance = 5;

    // Generate vector from light source to observer
    vec3 lightSourceToObserver = -getLightSourceVector(elevation, distance);

    mat3 lightSourceMatrix = mat3(
                vec3(1.0, 0.0, 0.0),
                vec3(0.0, cos(elevation), sin(elevation)),
                vec3(0.0, -sin(elevation), cos(elevation)));

    vec3 incidentStandardRay = randomRay();

    vec3 exitantStandardRay = castRayThroughCrystal(incidentStandardRay, wavelength);
    if (length(exitantStandardRay) < 0.0001) return;

    float scatteringAngle = acos(min(1.0, dot(incidentStandardRay, exitantStandardRay)));

    for (int rayReuseCounter = 0; rayReuseCounter < SCATTERING_ANGLE_REUSE; ++rayReuseCounter) {
        vec3 exitantRay = vec3(0.0, 1.0, 0.0);
        float totalWeight = 0.0;

        if (scatteringAngle < 0.0001) {
            totalWeight = 1.0;
            exitantRay = -normalize(lightSourceToObserver);
        } else {
            float polarAngleTheta = rand() * scatteringAngle;
            float polarAnglePhi = rand() * 2.0 * PI;
            float psi = scatteringAngle - polarAngleTheta;

            vec3 incidentResultRay = lightSourceMatrix * (
                        distance * sin(psi) / sin(scatteringAngle) * vec3(
                            sin(polarAngleTheta) * cos(polarAnglePhi),
                            sin(polarAngleTheta) * sin(polarAnglePhi),
                            cos(polarAngleTheta)));
            vec3 exitantResultRay = lightSourceToObserver - incidentResultRay;

            mat3 standardToWorldMatrix = getRotationMatrixToMatchVectorPairs(incidentResultRay,
                                                                             exitantResultRay,
                                                                             incidentStandardRay,
                                                                             exitantStandardRay);

            float orientationWeight = getOrientationWeight(standardToWorldMatrix);
            float cigarWeight = getMinnaertCigarWeight(scatteringAngle);

            totalWeight = cigarWeight * orientationWeight;
            exitantRay = normalize(exitantResultRay);
        }

        // Rotate ray 180 degrees around vertical axis to match same coordinate system as with other shaders
        exitantRay = vec3(-exitantRay.x, exitantRay.y, -exitantRay.z);

        // Hide subhorizon rays
        if (camera.hideSubHorizon == 1 && exitantRay.y > 0.0) return;

        ivec2 resolution = imageSize(outputImage);
        float aspectRatio = float(resolution.y) / float(resolution.x);

        // Camera is looking down the positive Z axis.
        // Ray is now transformed into camera space.
        // Camera matrix must be inverted (transposed) because ray
        // is being transformed, not the camera.
        vec3 exitantRayCameraSpace = normalize(transpose(getCameraOrientationMatrix()) * exitantRay);
        vec3 lightDirectionCameraSpace = -exitantRayCameraSpace;
        vec2 polar = cartesianToPolar(lightDirectionCameraSpace);
        float polarRadius = polar.x;
        float polarAngle = polar.y;

        float projectionFunction;
        if (camera.projection == PROJECTION_STEREOGRAPHIC) {
            projectionFunction = 2.0 * tan(polarRadius / 2.0);
        } else if (camera.projection == PROJECTION_RECTILINEAR) {
            if (polarRadius > 0.5 * PI) return;
            projectionFunction = tan(polarRadius);
        } else if (camera.projection == PROJECTION_EQUIDISTANT) {
            projectionFunction = polarRadius;
        } else if (camera.projection == PROJECTION_EQUAL_AREA) {
            projectionFunction = 2.0 * sin(polarRadius / 2.0);
        } else if (camera.projection == PROJECTION_ORTHOGRAPHIC) {
            if (polarRadius > 0.5 * PI) return;
            projectionFunction = sin(polarRadius);
        }

        vec2 projected = camera.focalLength * projectionFunction * vec2(aspectRatio * cos(polarAngle), sin(polarAngle));
        vec2 normalizedCoordinates = 0.5 + projected;

        if (any(lessThanEqual(normalizedCoordinates, vec2(0.0))) || any(greaterThanEqual(normalizedCoordinates, vec2(1.0))))
            return;

        float sunRadiance = daylightEstimate(wavelength);

        ivec2 pixelCoordinates = ivec2(resolution.x * normalizedCoordinates.x, resolution.y * normalizedCoordinates.y);
        vec3 cieXYZ = 10.0 * totalWeight * sunRadiance * vec3(xFit_1931(wavelength), yFit_1931(wavelength), zFit_1931(wavelength)) / sqrt(SCATTERING_ANGLE_REUSE);
        mat3 xyzToSrgb = mat3(3.24096994, -0.96924364, 0.05563008, -1.53738318, 1.8759675, -0.20397696, -0.49861076, 0.04155506, 1.05697151);
        storePixel(pixelCoordinates, xyzToSrgb * cieXYZ);
    }
}
