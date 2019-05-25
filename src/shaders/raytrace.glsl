R""(
#version 460 core

layout(local_size_x = 1) in;
layout(binding = 0, rgba32f) writeonly uniform image2D out_image;

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

void main(void) {
    // TODO: Generate crystal
    // TODO: Generate ray and rotate it
    // TODO: Go through crystal triangles and calculate solid angle
    // TODO: Select triangle to hit
    // TODO: Select point on triangle
    // TODO: Trace ray

    imageStore(out_image, ivec2(rand() * 1920, rand() * 1080), vec4(rand(), rand(), rand(), 1.0f));
}
)""
