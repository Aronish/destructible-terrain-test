#shader comp
#version 460 core

layout (std430, binding = 0) buffer DispatchCommand
{
    uint num_groups_x;
    uint num_groups_y;
    uint num_groups_z;
};

layout (std430, binding = 1) buffer IndirectDrawConfig
{
    uint index_count, prim_count, first_index, base_vertex, base_instance, triangle_count;
};

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main()
{
    num_groups_x = uint(ceil(float(triangle_count) / 1536.0f));
    num_groups_y = 1u;
    num_groups_z = 1u;
}