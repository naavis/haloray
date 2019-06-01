R""(
#version 440 core

out vec4 color;
uniform float exposure;
uniform samplerCube s;
uniform float pitch;
uniform float yaw;

vec3 altAzToCartesian(vec2 altAz)
{
    return normalize(vec3(
        cos(altAz.x) * sin(altAz.y),
        sin(altAz.x),
        cos(altAz.x) * cos(altAz.y)
        ));
}

vec3 getLocalForwardVector(float pitch, float yaw)
{
    mat3 yRot = mat3(
        cos(yaw), 0.0, -sin(yaw),
        0.0, 1.0, 0.0,
        sin(yaw), 0.0, cos(yaw)
    );

    mat3 xRot = mat3(
        1.0, 0.0, 0.0,
        0.0, cos(-pitch), sin(-pitch),
        0.0, -sin(-pitch), cos(-pitch)
    );

    return yRot * xRot * vec3(0.0, 0.0, 1.0);
}

void main(void) {
    vec2 resolution = vec2(1920, 1080);
    vec2 screenCoord = gl_FragCoord.xy / resolution.xy;
    screenCoord = 2.0 * screenCoord - 1.0;
    screenCoord.y *= resolution.y / resolution.x;

    float halfFov = radians(60.0);
    float distanceFactor = 1.0 / tan(halfFov);

    vec3 globalUp = vec3(0.0, 1.0, 0.0);
    vec3 localForward = getLocalForwardVector(radians(pitch), radians(yaw));
    vec3 localRight = normalize(cross(localForward, globalUp));
    vec3 localUp = normalize(cross(localRight, localForward));

    vec3 rayDir = normalize(distanceFactor * localForward + screenCoord.x * localRight + screenCoord.y * localUp);

    vec3 xyz = texture(s, rayDir).xyz;
    mat3 xyzToSrgb = mat3(3.2406, -0.9689, 0.0557, -1.5372, 1.8758, -0.2040, -0.4986, 0.0415, 1.0570);
    vec3 linearSrgb = xyzToSrgb * xyz * exposure;
    vec3 gammaCorrected = pow(linearSrgb, vec3(0.42));
    color = vec4(gammaCorrected, 1.0);
};
)""
