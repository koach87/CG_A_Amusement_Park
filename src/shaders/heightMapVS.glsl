#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coordinate;

const float PI = 3.14159;
const float speed = 1.0f;

vec2 dropPoint = vec2(-1.0f, -1.0f);
float interactiveAmplitude = 0.15f;
float interactiveWavelength = 0.5f;
float interactiveSpeed = 8.0f;

uniform mat4 u_model;
uniform sampler2D u_texture;

uniform float amplitude;
uniform float wavelength;
uniform float time;

uniform float interactiveRadius;

uniform float dropTime;

layout(std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 clipSpace;
} v_out;

void main()
{
    vec3 heightMap = position;
    float tempHeight = (texture(u_texture, texture_coordinate / 200).r - 0.5f) * amplitude;
    float tempInteractive = 0.0f;
    if (dropPoint.x > 0.0f)
    {
        float d = distance(texture_coordinate, dropPoint) / interactiveWavelength * 100.0f;
        float t = (time - dropTime) * (interactiveRadius * PI) * interactiveSpeed;
        tempInteractive = interactiveAmplitude * sin((d - t) * clamp(0.0125f * t, 0.0f, 1.0f)) / (exp(0.1f * abs(d - t) + (0.05f * t))) * 1.5f;
    }

    if ((tempHeight < 0 && tempInteractive > 0) || (tempHeight > 0 && tempInteractive < 0))
        heightMap.y += (tempHeight + tempInteractive);
    else
        heightMap.y += (abs(tempHeight) > abs(tempInteractive)) ? tempHeight : tempInteractive;

    vec4 worldPosition = u_model * vec4(position, 1.0f);
    v_out.clipSpace = u_projection * u_view * worldPosition;
    gl_Position = u_projection * u_view * u_model * vec4(heightMap, 1.0f);

    v_out.position = vec3(u_model * vec4(heightMap, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * normal;
    v_out.texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y);
}