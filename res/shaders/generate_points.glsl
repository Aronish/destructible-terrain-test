#shader comp
#version 460 core

#include "simplex_noise.glsl"

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

uniform uint u_points_per_axis;
uniform vec3 u_position_offset;

layout(std140, binding = 0) uniform WorldGenerationConfig
{
    int u_octaves_2d, u_octaves_3d;
    float u_frequency_2d, u_lacunarity_2d, u_persistence_2d, u_amplitude_2d, u_exponent_2d;
    float u_frequency_3d, u_lacunarity_3d, u_persistence_3d, u_amplitude_3d, u_exponent_3d, u_weight_multiplier_3d, u_noise_weight_3d;
};

layout(std430, binding = 3) buffer DensityDistribution
{
    float values[];
};

float layeredNoise(vec2 position, int octaves, float frequency, float lacunarity, float persistence)
{
    float total_noise = 0.0f;
    float total_amplitude = 0.0f;
    float amplitude = 1.0f;
    for (int i = 0; i < octaves; ++i)
    {
        total_noise += amplitude * simplexNoise2d(position * frequency);
        total_amplitude += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    return total_noise / total_amplitude;
}

float layeredNoise(vec3 position, int octaves, float frequency, float lacunarity, float persistence)
{
    float total_noise = 0.0f;
    float total_amplitude = 0.0f;
    float amplitude = 1.0f;
    float weight = 1.0f;
    for (int i = 0; i < octaves; ++i)
    {
        float noise = 1.0f - abs(simplexNoise3d(position * frequency));
        noise = noise * noise * weight;
        weight = max(min(noise * u_weight_multiplier_3d, 1.0f), 0.0f);
        total_noise += amplitude * noise;
        total_amplitude += amplitude;
        frequency *= lacunarity;
        amplitude *= persistence;
    }
    return position.y - total_noise * u_noise_weight_3d;
}

void main()
{
    uint points_from_zero = u_points_per_axis - 1; // ppa is a count, can't be used as index
    uint resolution = uint(ceil(float(u_points_per_axis) / WORK_GROUP_SIZE));
    if (gl_GlobalInvocationID.x > points_from_zero || gl_GlobalInvocationID.y > points_from_zero || gl_GlobalInvocationID.z > points_from_zero) return;
    float x = (float(gl_GlobalInvocationID.x) + u_position_offset.x * float(points_from_zero)) / resolution,
          y = (float(gl_GlobalInvocationID.y) + u_position_offset.y * float(points_from_zero)) / resolution,
          z = (float(gl_GlobalInvocationID.z) + u_position_offset.z * float(points_from_zero)) / resolution;

    float final_density = layeredNoise(vec3(x, y, z), u_octaves_3d, u_frequency_3d, u_lacunarity_3d, u_persistence_3d);
    
    values[
        gl_GlobalInvocationID.z * u_points_per_axis * u_points_per_axis +
        gl_GlobalInvocationID.y * u_points_per_axis +
        gl_GlobalInvocationID.x
    ] = final_density;
}