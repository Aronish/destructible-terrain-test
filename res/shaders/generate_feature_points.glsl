#shader comp
#version 460 core

#include "qef_solver.glsl"

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

layout (std430, binding = 0) buffer HermiteData
{
	vec4 edge_data[]; // 3 per invocation except boundaries
};

struct Point
{
	float x, y, z;
};

layout (std430, binding = 1) buffer Vertices
{
    Point vertices[];
};

layout (binding = 2) buffer IndirectDraw
{
    uint vertex_count, prim_count, base_vertex, base_instance, point_count;
};

layout (binding = 4) buffer Points
{
	Point points[];
};

uint getEdgeIndex(uvec3 invoc)
{
	return uint(dot(uvec3(1, WORK_GROUP_SIZE, WORK_GROUP_SIZE * WORK_GROUP_SIZE), invoc)) * 3;
}

uint getCellIndex(uvec3 invoc)
{
	return uint(dot(uvec3(1, WORK_GROUP_SIZE - 1, (WORK_GROUP_SIZE - 1) * (WORK_GROUP_SIZE - 1)), invoc));
}

bool solutionInside(vec3 min_point)
{
	return
	min_point.x * WORK_GROUP_SIZE < gl_GlobalInvocationID.x || min_point.x * WORK_GROUP_SIZE > (gl_GlobalInvocationID.x + 1) ||
	min_point.y * WORK_GROUP_SIZE < gl_GlobalInvocationID.y || min_point.y * WORK_GROUP_SIZE > (gl_GlobalInvocationID.y + 1) ||
	min_point.z * WORK_GROUP_SIZE < gl_GlobalInvocationID.z || min_point.z * WORK_GROUP_SIZE > (gl_GlobalInvocationID.z + 1);
}

void main()
{
	const uint points_from_zero = WORK_GROUP_SIZE - 1;
	const uint x = gl_GlobalInvocationID.x, y = gl_GlobalInvocationID.y, z = gl_GlobalInvocationID.z;
	if (x > points_from_zero - 1 || y > points_from_zero - 1 || z > points_from_zero - 1) return;
	const vec4 cell_hermite_data[12] =
	{
		edge_data[getEdgeIndex(uvec3(x, y, z))],
		edge_data[getEdgeIndex(uvec3(x, y, z)) + 1],
		edge_data[getEdgeIndex(uvec3(x, y, z)) + 2],
		edge_data[getEdgeIndex(uvec3(x + 1, y, z)) + 1],
		edge_data[getEdgeIndex(uvec3(x + 1, y, z)) + 2],
		edge_data[getEdgeIndex(uvec3(x, y + 1, z))],
		edge_data[getEdgeIndex(uvec3(x, y + 1, z)) + 2],
		edge_data[getEdgeIndex(uvec3(x, y, z + 1))],
		edge_data[getEdgeIndex(uvec3(x, y, z + 1)) + 1],
		edge_data[getEdgeIndex(uvec3(x + 1, y + 1, z)) + 2],
		edge_data[getEdgeIndex(uvec3(x, y + 1, z + 1))],
		edge_data[getEdgeIndex(uvec3(x + 1, y, z + 1)) + 1]
	};
	const vec3 positions[12] =
	{
		mix(vec3(x, y, z), vec3(x + 1, y, z), cell_hermite_data[0].w),
		mix(vec3(x, y, z), vec3(x, y + 1, z), cell_hermite_data[1].w),
		mix(vec3(x, y, z), vec3(x, y, z + 1), cell_hermite_data[2].w),
		mix(vec3(x + 1, y, z), vec3(x + 1, y + 1, z), cell_hermite_data[3].w),
		mix(vec3(x + 1, y, z), vec3(x + 1, y, z + 1), cell_hermite_data[4].w),
		mix(vec3(x, y + 1, z), vec3(x + 1, y + 1, z), cell_hermite_data[5].w),
		mix(vec3(x, y + 1, z), vec3(x, y + 1, z + 1), cell_hermite_data[6].w),
		mix(vec3(x, y, z + 1), vec3(x + 1, y, z + 1), cell_hermite_data[7].w),
		mix(vec3(x, y, z + 1), vec3(x, y + 1, z + 1), cell_hermite_data[8].w),
		mix(vec3(x + 1, y + 1, z), vec3(x + 1, y + 1, z + 1), cell_hermite_data[9].w),
		mix(vec3(x, y + 1, z + 1), vec3(x + 1, y + 1, z + 1), cell_hermite_data[10].w),
		mix(vec3(x + 1, y, z + 1), vec3(x + 1, y + 1, z + 1), cell_hermite_data[11].w)
	};
	
	QEF3D qef_3d = QEF3D(mat3(0.0f), vec3(0.0f), vec4(0.0f));
    uint valid_normal_count = 0;
	for (uint i = 0; i < cell_hermite_data.length(); ++i)
	{
		vec3 normal = cell_hermite_data[i].xyz;
		if (length(normal) > 0.0f)
		{
			++valid_normal_count;
			qef_add(normal, positions[i] / WORK_GROUP_SIZE, qef_3d);
		}
	}
	if (valid_normal_count > 0)
    {
        vec3 mass_point = qef_3d.point_accum.xyz / qef_3d.point_accum.w;
        vec3 min_point = vec3(0.0f);
        qef_solve(qef_3d, min_point);
		if (!solutionInside(min_point))
		{
			vec2 min_point_xf;
			qef_solve(qef_fix_x(qef_3d, x), min_point_xf);
			if (!solutionInside(vec3(x, min_point_xf)))
			{
				min_point = mass_point;
			} else
			{
				min_point = vec3(x, min_point_xf);
			}
		}
#if 0
        Point point = Point(mass_point.x, mass_point.y, mass_point.z);
#else
		Point point = Point(min_point.x, min_point.y, min_point.z);
#endif
        vertices[getCellIndex(uvec3(x, y, z))] = point;
        points[atomicAdd(point_count, 1)] = point;
	}
}