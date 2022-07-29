#shader comp
#version 460 core

#include "qef_solver.glsl"

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

const uint cornerIndexAFromEdge[12] =
{
    0, 1, 5, 4, 2, 3, 7, 6, 0, 1, 5, 4
};

const uint cornerIndexBFromEdge[12] =
{
    1, 5, 4, 0, 3, 7, 6, 2, 2, 3, 7, 6
};

layout (std430, binding = 0) buffer DensityDistribution
{
    readonly float values[];
};

layout (std430, binding = 1) buffer Vertices
{
    vec4 vertices[];
};

layout (binding = 2) buffer IndirectDraw
{
    uint vertex_count, prim_count, base_vertex, base_instance;
};

vec3 sphereGradTemp(vec3 pos)
{
    return 2.0f * (pos - 4.0f);
}

vec3 getNormalAtEdge(vec4 corner_a, vec4 corner_b, out vec3 interpolated)
{
    if (corner_a.w < 0.0f != corner_b.w < 0.0f)
    {
        float t = -corner_a.w / (corner_b.w - corner_a.w);
        interpolated = corner_a.xyz + t * (corner_b.xyz - corner_a.xyz);
        return normalize(sphereGradTemp(interpolated));
    }
    return vec3(0.0f);
}

uint getPointIndex(uint x, uint y, uint z)
{
	return x + y * WORK_GROUP_SIZE + z * WORK_GROUP_SIZE * WORK_GROUP_SIZE;
}

uint getCellIndex(uint x, uint y, uint z)
{
	return x + y * (WORK_GROUP_SIZE - 1) + z * (WORK_GROUP_SIZE - 1) * (WORK_GROUP_SIZE - 1);
}

void main()
{
    const uint points_from_zero = WORK_GROUP_SIZE - 1;
	if (gl_GlobalInvocationID.x > points_from_zero - 1 || gl_GlobalInvocationID.y > points_from_zero - 1 || gl_GlobalInvocationID.z > points_from_zero - 1) return;
    const uint x = gl_GlobalInvocationID.x, y = gl_GlobalInvocationID.y, z = gl_GlobalInvocationID.z;

    const vec4 cube_corners[8] =
    {
        vec4(x,     y,      z,      values[getPointIndex(x,     y,      z)      ]),
        vec4(x,     y,      z + 1,  values[getPointIndex(x,     y,      z + 1)  ]),
        vec4(x,     y + 1,  z,      values[getPointIndex(x,     y + 1,  z)      ]),
        vec4(x,     y + 1,  z + 1,  values[getPointIndex(x,     y + 1,  z + 1)  ]),
        vec4(x + 1, y,      z,      values[getPointIndex(x + 1, y,      z)      ]),
        vec4(x + 1, y,      z + 1,  values[getPointIndex(x + 1, y,      z + 1)  ]),
        vec4(x + 1, y + 1,  z,      values[getPointIndex(x + 1, y + 1,  z)      ]),
        vec4(x + 1, y + 1,  z + 1,  values[getPointIndex(x + 1, y + 1,  z + 1)  ])
    };

    vec3 normals[12];
    vec3 positions[12];
    // QEF Data
    vec4 pointaccum = vec4(0.0f);
    mat3 ATA = mat3(0.0f);
    vec3 ATb = vec3(0.0f);
    uint valid_normal_count = 0;
    for (uint i = 0; i < 12; ++i)
    {
        normals[i] = getNormalAtEdge(cube_corners[cornerIndexAFromEdge[i]], cube_corners[cornerIndexBFromEdge[i]], positions[i]);
        if (length(normals[i]) > 0.0f)
        {
            ++valid_normal_count;
            qef_add(normals[i], positions[i], ATA, ATb, pointaccum);
        }
    }
#if 0 // SUCKS
    // Bias
    const float bias_strength = 0.01f;
    vec3 mass_point = vec3(0.0f);
    for (uint i = 0; i < 12; ++i) mass_point += positions[i];
    mass_point /= 12.0f;
    qef_add(vec3(bias_strength, 0.0f, 0.0f), mass_point, ATA, ATb, pointaccum);
    qef_add(vec3(0.0f, bias_strength, 0.0f), mass_point, ATA, ATb, pointaccum);
    qef_add(vec3(0.0f, 0.0f, bias_strength), mass_point, ATA, ATb, pointaccum);
#endif
    // Solve QEF
    if (valid_normal_count > 0)
    {
        vec3 com = pointaccum.xyz / pointaccum.w;
        vec3 x_q = vec3(0.0f);
        qef_solve(ATA, ATb, pointaccum, x_q);
        vertices[getCellIndex(x, y, z)] = vec4(x_q, 1.0f);
    } else vertices[getCellIndex(x, y, z)] = vec4(0.0f);
}