#version 440 core

out vec4 color;
uniform float exposure;
uniform sampler2D s;

void main(void) {
    vec3 linearSrgb = exposure * texelFetch(s, ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 gammaCorrected = pow(linearSrgb, vec3(0.42));
    color = vec4(gammaCorrected, 1.0);
}
