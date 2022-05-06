#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

struct UnpaddedTriangle
{
    float  x_1,  y_1,  z_1;
    float nx_1, ny_1, nz_1;
    float  x_2,  y_2,  z_2;
    float nx_2, ny_2, nz_2;
    float  x_3,  y_3,  z_3;
    float nx_3, ny_3, nz_3;
};

layout(std430, binding = 0) buffer DensityDistribution
{
    float values[];
};

layout (std430, binding = 1) buffer RayHitData
{
    UnpaddedTriangle hit_triangle;
    float hit, chunk_x, chunk_z;
};

uniform int u_points_per_axis;
uniform float u_strength;
uniform float u_radius;
uniform float u_create_destroy_multiplier;

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

void main()
{
    int cube_volumes = u_points_per_axis - 1;
    if (gl_GlobalInvocationID.x > cube_volumes || gl_GlobalInvocationID.y > cube_volumes || gl_GlobalInvocationID.z > cube_volumes) return;
    vec3 terraform_point = vec3(round(hit_triangle.x_1 * u_points_per_axis), round(hit_triangle.y_1 * u_points_per_axis), round(hit_triangle.z_1 * u_points_per_axis));
    float distanceFromTerraformPoint = length(terraform_point - gl_GlobalInvocationID);

    if (distanceFromTerraformPoint < u_radius)
    {
        values[
            gl_GlobalInvocationID.z * u_points_per_axis * u_points_per_axis +
            gl_GlobalInvocationID.y * u_points_per_axis +
            gl_GlobalInvocationID.x
        ] -= u_create_destroy_multiplier * u_strength / ((distanceFromTerraformPoint * distanceFromTerraformPoint) + 0.00001f);
    }
}