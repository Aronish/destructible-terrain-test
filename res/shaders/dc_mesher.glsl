#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

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

struct UnpaddedTriangle
{
	float x_1, y_1, z_1;
	float nx_1, ny_1, nz_1;
	float x_2, y_2, z_2;
	float nx_2, ny_2, nz_2;
	float x_3, y_3, z_3;
	float nx_3, ny_3, nz_3;
};

layout (binding = 3) buffer Mesh
{
	UnpaddedTriangle triangles[];
};

layout (binding = 7) buffer Materials
{
	float materials[];
};

uint getArrayIndex(uvec3 invoc)
{
	return uint(dot(uvec3(1, WORK_GROUP_SIZE, WORK_GROUP_SIZE * WORK_GROUP_SIZE), invoc));
}

uint getCellIndex(uvec3 invoc)
{
	return uint(dot(uvec3(1, WORK_GROUP_SIZE - 1, (WORK_GROUP_SIZE - 1) * (WORK_GROUP_SIZE - 1)), invoc));
}

vec3 subtractPoints(Point p1, Point p2)
{
	return vec3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
}

void addFace(Point vertices[4], bool cw) // Optimization opportunity: Have this guy output triangles and do one single atomicAdd at end of shader
{
	const uint index = atomicAdd(vertex_count, 6) / 3;
	const uint index_11 = 2 + uint(cw), index_12 = 3 - uint(cw);
	const uint index_21 = 2 * (1 - uint(cw)) + 1, index_22 = 2 * uint(cw) + 1;
	vec3 normal1 = normalize(cross(subtractPoints(vertices[index_11], vertices[0]), subtractPoints(vertices[index_12], vertices[0])));
	vec3 normal2 = normalize(cross(subtractPoints(vertices[index_21], vertices[0]), subtractPoints(vertices[index_22], vertices[0])));
	triangles[index] = UnpaddedTriangle
	(
		vertices[0].x, vertices[0].y, vertices[0].z,
		normal1.x, normal1.y, normal1.z,
		vertices[index_11].x, vertices[index_11].y, vertices[index_11].z,
		normal1.x, normal1.y, normal1.z,
		vertices[index_12].x, vertices[index_12].y, vertices[index_12].z,
		normal1.x, normal1.y, normal1.z
	);
	triangles[index + 1] = UnpaddedTriangle
	(
		vertices[0].x, vertices[0].y, vertices[0].z,
		normal2.x, normal2.y, normal2.z,
		vertices[index_21].x, vertices[index_21].y, vertices[index_21].z,
		normal2.x, normal2.y, normal2.z,
		vertices[index_22].x, vertices[index_22].y, vertices[index_22].z,
		normal2.x, normal2.y, normal2.z
	);
}

void main()
{
	const uint points_from_zero = WORK_GROUP_SIZE - 1;
	const uint x = gl_GlobalInvocationID.x, y = gl_GlobalInvocationID.y, z = gl_GlobalInvocationID.z;
	if (x > points_from_zero - 1 || y > points_from_zero - 1 || z > points_from_zero - 1) return;

	float material_here = materials[getArrayIndex(uvec3(x, y, z))], material_other;
	Point face_vertices[4];
	if (y > 0 && z > 0)
	{
		float material_other = materials[getArrayIndex(uvec3(x + 1, y, z))];
		if (material_here != material_other)
		{
			face_vertices = Point[4]
			(
				vertices[getCellIndex(uvec3(x, y - 1, z - 1))],
				vertices[getCellIndex(uvec3(x, y, z - 1))],
				vertices[getCellIndex(uvec3(x, y - 1, z))],
				vertices[getCellIndex(uvec3(x, y, z))]
			);
			addFace(face_vertices, material_here > material_other);
		}
	}
	if (x > 0 && z > 0)
	{
		material_other = materials[getArrayIndex(uvec3(x, y + 1, z))];
		if (material_here != material_other)
		{
			face_vertices = Point[4]
			(
				vertices[getCellIndex(uvec3(x - 1, y, z - 1))],
				vertices[getCellIndex(uvec3(x - 1, y, z))],
				vertices[getCellIndex(uvec3(x, y, z - 1))],
				vertices[getCellIndex(uvec3(x, y, z))]
			);
			addFace(face_vertices, material_here > material_other);
		}
	}
	if (x > 0 && y > 0)
	{
		material_other = materials[getArrayIndex(uvec3(x, y, z + 1))];
		if (material_here != material_other)
		{
			face_vertices = Point[4]
			(
				vertices[getCellIndex(uvec3(x - 1, y - 1, z))],
				vertices[getCellIndex(uvec3(x, y - 1, z))],
				vertices[getCellIndex(uvec3(x - 1, y, z))],
				vertices[getCellIndex(uvec3(x, y, z))]
			);
			addFace(face_vertices, material_here > material_other);
		}
	}
}