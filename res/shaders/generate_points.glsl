#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 8;

uniform int u_points_per_axis;
uniform vec3 u_position_offset;

uniform int u_octaves = 1;
uniform float u_frequency;
uniform float u_amplitude;
uniform float u_lacunarity;
uniform float u_persistence;

layout(std430, binding = 0) buffer Points
{
    float values[];
};

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

// Simplex 2D noise
vec3 permute(vec3 x)
{
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

float simplexNoise2d(vec2 v)
{
    const vec4 C = vec4(0.211324865405187, 0.366025403784439, -0.577350269189626, 0.024390243902439);
    vec2 i = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);
    vec2 i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy ,x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m * m;
    m = m * m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0 * a0 + h * h);
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float layeredNoise(vec2 position)
{
    float numerator = 0.0f;
    float denominator = 0.0f;
    float frequency = u_frequency;
    float amplitude = u_amplitude;
    for (int i = 0; i < u_octaves; ++i)
    {
        numerator += (amplitude * simplexNoise2d(position * frequency));
        denominator += amplitude;
        frequency *= u_lacunarity;
        amplitude *= u_persistence;
    }
    return numerator / denominator;
}

void main()
{
    float resolution = u_points_per_axis / WORK_GROUP_SIZE;
    float x = (float(gl_GlobalInvocationID.x) + u_position_offset.x * u_points_per_axis) / resolution,
          y = (float(gl_GlobalInvocationID.y) + u_position_offset.y * u_points_per_axis) / resolution,
          z = (float(gl_GlobalInvocationID.z) + u_position_offset.z * u_points_per_axis) / resolution;
    values[gl_GlobalInvocationID.z * u_points_per_axis * u_points_per_axis + gl_GlobalInvocationID.y * u_points_per_axis + gl_GlobalInvocationID.x] = y - u_amplitude * layeredNoise(vec2(x, z));
}