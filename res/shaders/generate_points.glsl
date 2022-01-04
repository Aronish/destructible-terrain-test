#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 8;

uniform int u_points_per_axis;

layout(std430, binding = 0) buffer Points
{
    float values[];
};

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

void main()
{
    //int center = u_points_per_axis / 2;
    //values[gl_GlobalInvocationID.z * u_points_per_axis * u_points_per_axis + gl_GlobalInvocationID.y * u_points_per_axis + gl_GlobalInvocationID.x] =
    //(gl_GlobalInvocationID.x - center) * (gl_GlobalInvocationID.x - center) +
    //(gl_GlobalInvocationID.y - center) * (gl_GlobalInvocationID.y - center) +
    //(gl_GlobalInvocationID.z - center) * (gl_GlobalInvocationID.z - center) - 4.0f;
    values[gl_GlobalInvocationID.z * u_points_per_axis * u_points_per_axis + gl_GlobalInvocationID.y * u_points_per_axis + gl_GlobalInvocationID.x] =
    sin(gl_GlobalInvocationID.x) * sin(gl_GlobalInvocationID.y) + 0.5f;
}