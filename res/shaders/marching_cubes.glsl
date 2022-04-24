#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

const int cornerIndexAFromEdge[12] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3
};

const int cornerIndexBFromEdge[12] =
{
    1, 2, 3, 0, 5, 6, 7, 4, 4, 5, 6, 7
};

uniform int u_points_per_axis;

uint indexFromCoord(uint x, uint y, uint z)
{
    return z * u_points_per_axis * u_points_per_axis + y * u_points_per_axis + x;
}

struct UnpaddedTriangle
{
    float  x_1,  y_1,  z_1;
    float nx_1, ny_1, nz_1;
    float  x_2,  y_2,  z_2;
    float nx_2, ny_2, nz_2;
    float  x_3,  y_3,  z_3;
    float nx_3, ny_3, nz_3;
};

layout (std430, binding = 0) readonly buffer IsoSurface
{
    readonly float iso_surface_values[];
};

layout (std430, binding = 2) readonly buffer TriangulationTable
{
    readonly int tri_table[256][16];
};

layout (std430, binding = 3) buffer Mesh
{
    UnpaddedTriangle triangles[];
};

layout (binding = 4) buffer IndirectDrawConfig
{
    uint index_count, prim_count, first_index, base_vertex, base_instance, triangle_count;
};

uniform float u_threshold = 0.0f;

vec3 interpolateVertices(vec4 v1, vec4 v2)
{
    float t = (u_threshold - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

void main()
{
    int cube_volumes = u_points_per_axis - 1;
    if (gl_GlobalInvocationID.x >= cube_volumes || gl_GlobalInvocationID.y >= cube_volumes || gl_GlobalInvocationID.z >= cube_volumes) return;

    float step_size = 1.0f / float(cube_volumes);
    vec3 scaled_coordinate = vec3(gl_GlobalInvocationID) * step_size;

    const vec4 cube_corners[8] =
    {
        vec4(scaled_coordinate,                                             iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z)]),
        vec4(scaled_coordinate + vec3(step_size,  0.0f,       0.0f),        iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z)]),
        vec4(scaled_coordinate + vec3(step_size,  0.0f,       step_size),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z + 1)]),
        vec4(scaled_coordinate + vec3(0.0f,       0.0f,       step_size),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z + 1)]),
        vec4(scaled_coordinate + vec3(0.0f,       step_size,  0.0f),        iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z)]),
        vec4(scaled_coordinate + vec3(step_size,  step_size,  0.0f),        iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z)]),
        vec4(scaled_coordinate + vec3(step_size,  step_size,  step_size),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z + 1)]),
        vec4(scaled_coordinate + vec3(0.0f,       step_size,  step_size),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z + 1)])
    };

    uint cube_index = 0;
    for (uint i = 0; i < cube_corners.length(); ++i)
    {
        if (cube_corners[i].w < u_threshold) cube_index |= 1 << i;
    }
    if (cube_index == 0 || cube_index == 255) return;

    const int index_configuration[16] = tri_table[cube_index];

    for (int i = 0; index_configuration[i] != -1; i += 3)
    {
        int a0 = cornerIndexAFromEdge[index_configuration[i]];
        int b0 = cornerIndexBFromEdge[index_configuration[i]];

        int a1 = cornerIndexAFromEdge[index_configuration[i + 1]];
        int b1 = cornerIndexBFromEdge[index_configuration[i + 1]];

        int a2 = cornerIndexAFromEdge[index_configuration[i + 2]];
        int b2 = cornerIndexBFromEdge[index_configuration[i + 2]];

        vec3 vertexA = interpolateVertices(cube_corners[a0], cube_corners[b0]);
        vec3 vertexB = interpolateVertices(cube_corners[a1], cube_corners[b1]);
        vec3 vertexC = interpolateVertices(cube_corners[a2], cube_corners[b2]);
        vec3 normal = normalize(cross(vertexB - vertexA, vertexC - vertexA));
        UnpaddedTriangle triangle = UnpaddedTriangle(
            vertexA.x, vertexA.y, vertexA.z, normal.x, normal.y, normal.z,
            vertexB.x, vertexB.y, vertexB.z, normal.x, normal.y, normal.z,
            vertexC.x, vertexC.y, vertexC.z, normal.x, normal.y, normal.z
        );
        triangles[atomicAdd(triangle_count, 1)] = triangle;
        //atomicAdd(index_count, 3);
        index_count = (u_points_per_axis - 1) * (u_points_per_axis - 1) * (u_points_per_axis - 1) * 5 * 3;
    }
}