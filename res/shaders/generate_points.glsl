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
    int x = int(gl_GlobalInvocationID.x), y = int(gl_GlobalInvocationID.y), z = int(gl_GlobalInvocationID.z);
    int center = u_points_per_axis / 2;
    values[z * u_points_per_axis * u_points_per_axis + y * u_points_per_axis + x] = (x - center) * (x - center) + (y - center) * (y - center) + (z - center) * (z - center) - center * center;
}