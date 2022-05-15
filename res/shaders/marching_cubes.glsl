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
uniform float u_threshold = 0.0f;
uniform int u_has_neighbors;

struct UnpaddedTriangle
{
    float  x_1,  y_1,  z_1;
    float nx_1, ny_1, nz_1;
    float  x_2,  y_2,  z_2;
    float nx_2, ny_2, nz_2;
    float  x_3,  y_3,  z_3;
    float nx_3, ny_3, nz_3;
};

layout (std430, binding = 0) readonly buffer TriangulationTable
{
    readonly int tri_table[256][16];
};

layout (std430, binding = 1) buffer Mesh
{
    UnpaddedTriangle triangles[];
};

layout (binding = 2) buffer IndirectDrawConfig
{
    uint index_count, prim_count, first_index, base_vertex, base_instance, triangle_count;
};

layout (std430, binding = 3) readonly buffer DensityDistribution
{
    readonly float values[];
} density_distributions[8];

uint indexFromCoord(uint x, uint y, uint z)
{
    return z * u_points_per_axis * u_points_per_axis + y * u_points_per_axis + x;
}

vec3 interpolateVertices(vec4 v1, vec4 v2)
{
    float t = (u_threshold - v1.w) / (v2.w - v1.w);
    return v1.xyz + t * (v2.xyz - v1.xyz);
}

float getDensityBasedOnNeighbors(uvec3 density_sample_point)
{
    if (bool(u_has_neighbors))
    {
        //if (bool((u_has_neighbors & 3) == 3) && density_sample_point.x + density_sample_point.z == 0) return density_distributions[3].values[indexFromCoord(u_points_per_axis - 1, density_sample_point.y, u_points_per_axis - 1)];
        if (density_sample_point.x == 0 && bool((u_has_neighbors & 1) == 1)) return density_distributions[1].values[indexFromCoord(u_points_per_axis - 1, density_sample_point.y, density_sample_point.z)];
        if (density_sample_point.z == 0 && bool((u_has_neighbors & 2) == 2)) return density_distributions[2].values[indexFromCoord(density_sample_point.x, density_sample_point.y, u_points_per_axis - 1)];
        if (density_sample_point.y == 0 && bool((u_has_neighbors & 4) == 4)) return density_distributions[4].values[indexFromCoord(density_sample_point.x, u_points_per_axis - 1, density_sample_point.z)];
    }
    return density_distributions[0].values[indexFromCoord(density_sample_point.x, density_sample_point.y, density_sample_point.z)];
}

layout (local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

void main()
{
    int points_from_zero = u_points_per_axis - 1; // ppa is a count, can't be used as index
    if (gl_GlobalInvocationID.x >= points_from_zero || gl_GlobalInvocationID.y >= points_from_zero || gl_GlobalInvocationID.z >= points_from_zero) return; // however there's one less cube volume per axis

    float step_size = 1.0f / float(points_from_zero);
    vec3 scaled_coordinate = vec3(gl_GlobalInvocationID) * step_size;

    const vec4 cube_corners[8] =
    {
        vec4(scaled_coordinate,                                             getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z))),
        vec4(scaled_coordinate + vec3(step_size,  0.0f,       0.0f),        getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z))),
        vec4(scaled_coordinate + vec3(step_size,  0.0f,       step_size),   getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z + 1))),
        vec4(scaled_coordinate + vec3(0.0f,       0.0f,       step_size),   getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y,        gl_GlobalInvocationID.z + 1))),
        vec4(scaled_coordinate + vec3(0.0f,       step_size,  0.0f),        getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z))),
        vec4(scaled_coordinate + vec3(step_size,  step_size,  0.0f),        getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z))),
        vec4(scaled_coordinate + vec3(step_size,  step_size,  step_size),   getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x + 1,  gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z + 1))),
        vec4(scaled_coordinate + vec3(0.0f,       step_size,  step_size),   getDensityBasedOnNeighbors(uvec3(gl_GlobalInvocationID.x,      gl_GlobalInvocationID.y + 1,    gl_GlobalInvocationID.z + 1)))
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
        atomicAdd(index_count, 3);
    }
}