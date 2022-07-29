#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

struct UnpaddedTriangle
{
	float x_1, y_1, z_1;
	float x_2, y_2, z_2;
	float x_3, y_3, z_3;
};

layout (std430, binding = 0) buffer DensityDistribution
{
	readonly float values[];
};

layout (std430, binding = 1) buffer Vertices
{
	readonly vec4 vertices[];
};

layout (binding = 2) buffer IndirectDraw
{
    uint vertex_count, prim_count, base_vertex, base_instance, triangle_count;
};

layout (std430, binding = 3) buffer Mesh
{
	UnpaddedTriangle mesh[];
};

uint getPointIndex(uint x, uint y, uint z)
{
	return x + y * WORK_GROUP_SIZE + z * WORK_GROUP_SIZE * WORK_GROUP_SIZE;
}

uint getCellIndex(uint x, uint y, uint z)
{
	return x + y * (WORK_GROUP_SIZE - 1) + z * (WORK_GROUP_SIZE - 1) * (WORK_GROUP_SIZE - 1);
}

void triangulate(uint x, uint y, uint z, inout UnpaddedTriangle triangles[12], inout uint num_generated)
{
	const vec4 positions[7] =
	{
		vertices[getCellIndex(x,		y,		z)		],
		vertices[getCellIndex(x,		y,		z + 1)	],
		vertices[getCellIndex(x,		y + 1,	z)		],
		vertices[getCellIndex(x,		y + 1,	z + 1)	],
		vertices[getCellIndex(x + 1,	y,		z)		],
		vertices[getCellIndex(x + 1,	y,		z + 1)	],
		vertices[getCellIndex(x + 1,	y + 1,	z)		]
	};

	uint neighbor_mask = 0;
	for (uint i = 1; i <= 6; ++i) // Don't handle (x, y, z)(needed for triangulation) or (x + 1, y + 1, z + 1)(not needed at all)
	{
		neighbor_mask |= uint(positions[i].w != 0.0f) << i;
	}

	if ((neighbor_mask & 84) == 84) // Has face along xy
	{
		if (values[getPointIndex(x, y, z + 1)] - values[getPointIndex(x, y, z)] < 0.0f)
		{
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[0].x, positions[0].y, positions[0].z,
				positions[2].x, positions[2].y, positions[2].z,
				positions[4].x, positions[4].y, positions[4].z
			);
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[4].x, positions[4].y, positions[4].z,
				positions[2].x, positions[2].y, positions[2].z,
				positions[6].x, positions[6].y, positions[6].z
			);
		} else
		{
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[0].x, positions[0].y, positions[0].z,
				positions[4].x, positions[4].y, positions[4].z,
				positions[2].x, positions[2].y, positions[2].z
			);
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[2].x, positions[2].y, positions[2].z,
				positions[4].x, positions[4].y, positions[4].z,
				positions[6].x, positions[6].y, positions[6].z
			);
		}
	}
	if ((neighbor_mask & 14) == 14) // Has face along yz
	{
		if (values[getPointIndex(x + 1, y, z)] - values[getPointIndex(x, y, z)] < 0.0f)
		{
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[0].x, positions[0].y, positions[0].z,
				positions[1].x, positions[1].y, positions[1].z,
				positions[2].x, positions[2].y, positions[2].z
			);
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[2].x, positions[2].y, positions[2].z,
				positions[1].x, positions[1].y, positions[1].z,
				positions[3].x, positions[3].y, positions[3].z
			);
		} else
		{
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[0].x, positions[0].y, positions[0].z,
				positions[2].x, positions[2].y, positions[2].z,
				positions[1].x, positions[1].y, positions[1].z
			);
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[1].x, positions[1].y, positions[1].z,
				positions[2].x, positions[2].y, positions[2].z,
				positions[3].x, positions[3].y, positions[3].z
			);
		}
	}
	if ((neighbor_mask & 50) == 50) // Has face along xz
	{
		if (values[getPointIndex(x, y + 1, z)] - values[getPointIndex(x, y, z)] < 0.0f)
		{
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[0].x, positions[0].y, positions[0].z,
				positions[4].x, positions[4].y, positions[4].z,
				positions[1].x, positions[1].y, positions[1].z
			);
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[1].x, positions[1].y, positions[1].z,
				positions[4].x, positions[4].y, positions[4].z,
				positions[5].x, positions[5].y, positions[5].z
			);
		} else
		{
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[0].x, positions[0].y, positions[0].z,
				positions[1].x, positions[1].y, positions[1].z,
				positions[4].x, positions[4].y, positions[4].z
			);
			triangles[num_generated++] = UnpaddedTriangle
			(
				positions[5].x, positions[5].y, positions[5].z,
				positions[4].x, positions[4].y, positions[4].z,
				positions[1].x, positions[1].y, positions[1].z
			);
		}
	}
}

void main()
{
	const uint points_from_zero = WORK_GROUP_SIZE - 1;
	if (gl_GlobalInvocationID.x > points_from_zero - 1 || gl_GlobalInvocationID.y > points_from_zero - 1 || gl_GlobalInvocationID.z > points_from_zero - 1) return;
	const uint x = gl_GlobalInvocationID.x, y = gl_GlobalInvocationID.y, z = gl_GlobalInvocationID.z;
	if (vertices[getCellIndex(x, y, z)].w == 0.0f) return;

	uint num_generated = 0;
	UnpaddedTriangle triangles[12];
	if (x < points_from_zero && y < points_from_zero && z < points_from_zero) triangulate(x, y, z, triangles, num_generated); // Might be a problem later
	for (uint i = 0; i < num_generated; ++i)
	{
		mesh[atomicAdd(triangle_count, 1)] = triangles[i];
	}
	atomicAdd(vertex_count, num_generated * 3);
}