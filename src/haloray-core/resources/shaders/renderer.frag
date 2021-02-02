#version 440 core

out vec4 color;
uniform float baseExposure;
uniform float adjustedExposure;
layout (binding = 0) uniform sampler2D haloTexture;
layout (binding = 1) uniform sampler2D backgroundTexture;

void main(void) {
    vec3 haloLinearSrgb = adjustedExposure * texelFetch(haloTexture, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 background = max(vec3(0.0), baseExposure * texelFetch(backgroundTexture, ivec2(gl_FragCoord.xy), 0).xyz);
    vec3 gammaCorrected = pow(background + haloLinearSrgb, vec3(0.42));
    color = vec4(gammaCorrected, 1.0);
}
