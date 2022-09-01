//position.y = (sin(2.0 * position.x + a_time / 10.0) * cos(2.0* position.y + a_time / 10.0) * 0.2);
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texture_coordinate;

const float PI = 3.14159;
float steepness = 0.5f;
vec4 direction = vec4(1.0f, 1.0f, 0.0f, 0.0f);
const float tiling = 6.0f;

vec3 tangent = vec3(1.0f, 0.0f, 0.0f);
vec3 binormal = vec3(0.0f, 0.0f, 1.0f);

uniform mat4 u_model;
uniform float amplitude;
uniform float wavelength;
uniform float time;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;

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
   vec3 toCameraVector;
   vec3 fromLightVector;
} v_out;

vec3 GerstnerWave(vec4 wave, vec3 p)
{
    float steepness = wave.z;
    float wavelength = wave.w;
    float k = 2 * PI / wavelength;
    float c = sqrt(9.8 / k);
    vec2 d = normalize(wave.xy);
    float f = k * (dot(d, p.xz) - c * time);
    float a = steepness / k;

    tangent += vec3(-d.x * d.x * (steepness * sin(f)), d.x * (steepness * cos(f)), -d.x * d.y * (steepness * sin(f)));
    binormal += vec3(-d.x * d.y * (steepness * sin(f)), d.y * (steepness * cos(f)), -d.y * d.y * (steepness * sin(f)));

    return vec3(d.x * (a * cos(f)), a * sin(f), d.y * (a * cos(f)));
}

void main()
{
    //vec4 wave = vec4(direction.x, direction.y, amplitude, wavelength);
    //vec3 gridPoint = position;
    //vec3 p = gridPoint;

    vec3 p;
    vec3 pGrandient;
        //p += GerstnerWave(wave, gridPoint);

    p = vec3(position.x, (sin(2.0 * position.x + time / 10.0) * sin(2.0 * position.y + time / 10.0) * 0.2), position.z);
    pGrandient = vec3(position.x, (cos(2.0 * position.x + time / 10.0) * cos(2.0 * position.y + time / 10.0) * 0.2), position.z);
    //vec3 pNeighbor = vec3()
    vec3 n_normal = normalize(cross(binormal, tangent));

    vec4 worldPosition = u_model * vec4(p, 1.0f);
    v_out.clipSpace = u_projection * u_view * worldPosition;
    gl_Position = v_out.clipSpace;

    v_out.position = p;
    v_out.normal = mat3(transpose(inverse(u_model))) * n_normal;
    v_out.texture_coordinate = vec2(texture_coordinate.x, 1.0f - texture_coordinate.y) * tiling;

    
}