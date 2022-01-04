#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 8;

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

struct Triangle
{
    vec4 vertexA;
    vec4 vertexB;
    vec4 vertexC;
    vec4 normal;
};

layout (std430, binding = 0) readonly buffer IsoSurface
{
    readonly float iso_surface_values[];
};

layout (std430, binding = 1) buffer Mesh
{
    Triangle triangles[];
};

layout (std430, binding = 2) readonly buffer TriangulationTable
{
    readonly int tri_table[256][16];
};

layout (binding = 3) uniform atomic_uint triangle_count;

uniform float u_surface_level = 0.0f;

vec3 interpolateVertices(vec4 v1, vec4 v2)
{
    float t = (u_surface_level - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

void main()
{
    if (gl_GlobalInvocationID.x >= u_points_per_axis - 1 || gl_GlobalInvocationID.y >= u_points_per_axis - 1 || gl_GlobalInvocationID.z >= u_points_per_axis - 1) return;

    float step_size = 1.0f / u_points_per_axis;

    const vec4 cube_corners[8] =
    {
        vec4(gl_GlobalInvocationID,                                                     iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(1.0f,    0.0f,       0.0f),        iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(1.0f,    0.0f,       1.0f),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z + 1)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(0.0f,         0.0f,       1.0f),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z + 1)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(0.0f,         1.0f,  0.0f),        iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(1.0f,    1.0f,  0.0f),        iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(1.0f,    1.0f,  1.0f),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z + 1)]),
        vec4(vec3(gl_GlobalInvocationID) + vec3(0.0f,         1.0f,  1.0f),   iso_surface_values[indexFromCoord(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z + 1)])
    };

    uint cube_index = 0;
    for (uint i = 0; i < cube_corners.length(); ++i)
    {
        if (cube_corners[i].w < u_surface_level) cube_index |= 1 << i;
    }
    if (cube_index == 0 || cube_index == 255) return;

    const int index_configuration[] = tri_table[cube_index];

    for (int i = 0; index_configuration[i] != -1; i += 3)
    {
        int a0 = cornerIndexAFromEdge[index_configuration[i]];
        int b0 = cornerIndexBFromEdge[index_configuration[i]];

        int a1 = cornerIndexAFromEdge[index_configuration[i + 1]];
        int b1 = cornerIndexBFromEdge[index_configuration[i + 1]];

        int a2 = cornerIndexAFromEdge[index_configuration[i + 2]];
        int b2 = cornerIndexBFromEdge[index_configuration[i + 2]];

        Triangle tri;
        tri.vertexA = vec4(interpolateVertices(cube_corners[a0], cube_corners[b0]), 0.0f);
        tri.vertexB = vec4(interpolateVertices(cube_corners[a1], cube_corners[b1]), 0.0f);
        tri.vertexC = vec4(interpolateVertices(cube_corners[a2], cube_corners[b2]), 0.0f);
        tri.normal = vec4(normalize(cross(vec3(tri.vertexB) - vec3(tri.vertexA), vec3(tri.vertexC) - vec3(tri.vertexA))), 0.0);
        triangles[atomicCounterIncrement(triangle_count)] = tri;
    }
}