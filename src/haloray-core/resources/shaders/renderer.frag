#version 440 core

out vec4 color;
uniform float baseExposure;
uniform float adjustedExposure;
layout (binding = 0) uniform sampler2D haloTexture;
layout (binding = 1) uniform sampler2D backgroundTexture;
layout (binding = 2) uniform sampler2D guideTexture;

#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0

/* FXAA implementation adapted from https://github.com/mattdesl/glsl-fxaa
   The antialiasing procedure is only applied to the background sky image
   to smoothen the harsh horizon edge and the edges of the sun. */

vec4 fxaa(sampler2D tex, vec2 fragCoord)
{
    vec2 resolution = textureSize(tex, 0);
    vec2 inverseVP = 1.0 / resolution.xy;
    vec2 rgbNWCoords = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
    vec2 rgbNECoords = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
    vec2 rgbSWCoords = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
    vec2 rgbSECoords = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
    vec2 rgbMCoords = vec2(fragCoord * inverseVP);

    vec4 color;
    vec3 rgbNW = texture2D(tex, rgbNWCoords).xyz;
    vec3 rgbNE = texture2D(tex, rgbNECoords).xyz;
    vec3 rgbSW = texture2D(tex, rgbSWCoords).xyz;
    vec3 rgbSE = texture2D(tex, rgbSECoords).xyz;
    vec4 texColor = texture2D(tex, rgbMCoords);
    vec3 rgbM  = texColor.xyz;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * rcpDirMin)) * inverseVP;

    vec3 rgbA = 0.5 * (
        texture2D(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
        texture2D(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture2D(tex, fragCoord * inverseVP + dir * -0.5).xyz +
        texture2D(tex, fragCoord * inverseVP + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax))
        color = vec4(rgbA, texColor.a);
    else
        color = vec4(rgbB, texColor.a);
    return color;
}

void main(void) {
    vec4 antialiasedBackground = fxaa(backgroundTexture, gl_FragCoord.xy);
    vec3 backgroundLinearSrgb = max(vec3(0.0), baseExposure * antialiasedBackground.rgb);
    vec3 haloLinearSrgb = adjustedExposure * texelFetch(haloTexture, ivec2(gl_FragCoord.xy), 0).xyz;

    vec3 linearImage = 0.005 * backgroundLinearSrgb + 0.1 * haloLinearSrgb;
    vec3 gammaCorrected = 1.055 * pow(linearImage, vec3(0.417)) - 0.055;
    vec3 guide = vec3(texelFetch(guideTexture, ivec2(gl_FragCoord.xy), 0).r);

    vec3 finalColor = clamp(gammaCorrected + 0.5 * guide, 0.0, 1.0);
    color = vec4(finalColor, 1.0);
}
