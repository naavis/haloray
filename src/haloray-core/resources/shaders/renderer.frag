#version 440 core

out vec4 color;
uniform float baseExposure;
uniform float adjustedExposure;
layout (binding = 0) uniform sampler2D haloTexture;
layout (binding = 1) uniform sampler2D backgroundTexture;

void main(void) {
    vec3 haloLinearSrgb = adjustedExposure * texelFetch(haloTexture, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 background = max(vec3(0.0), baseExposure * texelFetch(backgroundTexture, ivec2(gl_FragCoord.xy), 0).xyz);
    vec3 linearImage = background + haloLinearSrgb;
    vec3 gammaCorrected = 1.055 * pow(linearImage, vec3(0.417)) - 0.055;
    color = vec4(clamp(gammaCorrected, 0.0, 1.0), 1.0);
}
