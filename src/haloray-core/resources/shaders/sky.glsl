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

void main(void)
{
    ivec2 resolution = imageSize(outputImage);
    float aspectRatio = float(resolution.y) / float(resolution.x);
    ivec2 pixelCoordinates = ivec2(gl_GlobalInvocationID.xy);
    imageStore(outputImage, pixelCoordinates, vec4(0.5, 0.1, 0.0, 1.0));
}
