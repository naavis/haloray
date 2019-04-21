R""(
#version 460 core

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D out_image;

uint rng_state = 0;

uint rand_xorshift(void)
{
    // Xorshift algorithm from George Marsaglia's paper
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return rng_state;
}

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

void initialize_rng(uint seed) {
    rng_state = wang_hash(seed);
}

float rand(void) {
    return float(rand_xorshift()) / 4294967295.0f;
}

void main(void) {
    initialize_rng(gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x);
    // TODO: Generate crystal
    // TODO: Generate ray and rotate it
    // TODO: Go through crystal triangles and calculate solid angle
    // TODO: Select triangle to hit
    // TODO: Select point on triangle
    // TODO: Trace ray

    imageStore(out_image, ivec2(gl_GlobalInvocationID.xy), vec4(rand(), rand(), rand(), 1.0f));
}
)""
