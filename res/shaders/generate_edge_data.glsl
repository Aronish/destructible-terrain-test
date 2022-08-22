#shader comp
#version 460 core

#include "simplex_noise_grad.glsl"

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

// TODO: Store material indices for reorienting faces correctly. QEF solving does not guarantee the right facing

layout (std430, binding = 0) buffer HermiteData
{
	vec4 edge_data[]; // 3 per invocation except boundaries
};

struct PosNorm
{
	float x, y, z, nx, ny, nz;
};

layout (std430, binding = 5) buffer Signs
{
	PosNorm pos_norms[];
};

layout (binding = 6) buffer IndirectDraw
{
    uint vertex_count, prim_count, base_vertex, base_instance, point_count;
};

layout (binding = 7) buffer Materials
{
	float materials[];
};

const float epsilon = 1.e-20f;

const float r = 25.0f;

uniform float u_threshold = 0.0f;
uniform float u_frequency = 0.15f;

float noiseFunc(vec3 pos)
{
    //return pos.y - (simplexNoise3d(pos * 0.05f) * 0.5f + 0.5f) * 9.0f;
	pos -= 5.0f;
    return dot(pos, pos) - u_threshold * u_threshold;
	//vec3 gradient = vec3(0.0f);
	//return pos.y - simplexNoise3dGrad(vec3(pos.x, 0.0f, pos.z) * u_frequency, gradient) - u_threshold;
    //return pos.y - (simplexNoise3dGrad(vec3(pos.x, 0.0f, pos.z) * freq, gradient) * 0.5f + 0.5f);
	//pos *= u_frequency;
	//return pos.y - u_frequency * pos.x - u_threshold;
	//return pos.y - abs(pos.x - u_threshold) - abs(pos.z - u_threshold);
	return pos.x * pos.x + pos.y * pos.y - pos.z * pos.z - u_threshold;
}

vec3 functionGradient(vec3 pos)
{
	//pos -= 5.0f;
	//return vec3(2.0f * pos.x, 2.0f * pos.y, -2.0f * pos.z);
	//return normalize
	//(
	//	vec3(
	//		(noiseFunc(vec3(pos.x + epsilon, pos.yz)) - noiseFunc(vec3(pos.x - epsilon, pos.yz))) / (2.0f * epsilon),
	//		(noiseFunc(vec3(pos.x, pos.y + epsilon, pos.z)) - noiseFunc(vec3(pos.x, pos.y - epsilon, pos.z))) / (2.0f * epsilon),
	//		(noiseFunc(vec3(pos.xy, pos.z + epsilon)) - noiseFunc(vec3(pos.xy, pos.z - epsilon))) / (2.0f * epsilon)
	//	)
	//);
	//vec3 gradient = vec3(0.0f);
	//simplexNoise3dGrad(vec3(pos.x, 0.0f, pos.z) * u_frequency, gradient);
	//return normalize
	//(
	//	gradient
	//);
	pos -= 5.0f;
	return 2.0f * pos;
	//pos *= u_frequency;
	//return normalize(vec3(-u_frequency, 1.0f, 0.0f));
}  

uint getArrayIndex(uvec3 invoc)
{
	return uint(dot(uvec3(1, WORK_GROUP_SIZE, WORK_GROUP_SIZE * WORK_GROUP_SIZE), invoc));
}

void generateEdge(vec3 direction, out vec4 edge)
{
	vec3 here = vec3(gl_GlobalInvocationID);
	float density_here = noiseFunc(here), density_other = noiseFunc(here + direction);
	materials[getArrayIndex(uvec3(here))] = density_here < 0.0f ? 1.0f : 0.0f;
	if (density_here > 0.0f != density_other > 0.0f) // Edge intersects isosurface
	{
		float t = -density_here / (density_other - density_here);
		vec3 intersection = mix(here, here + direction, t);
		edge = vec4(functionGradient(intersection), t);
		pos_norms[atomicAdd(vertex_count, 1)] = PosNorm(intersection.x / WORK_GROUP_SIZE, intersection.y / WORK_GROUP_SIZE, intersection.z / WORK_GROUP_SIZE, edge.x, edge.y, edge.z);
		return;
	}
	edge = vec4(0.0f);
}

void main()
{
	const uint points_from_zero = WORK_GROUP_SIZE - 1;
	const uint x = gl_GlobalInvocationID.x,
         y = gl_GlobalInvocationID.y,
         z = gl_GlobalInvocationID.z;
		 
	//signs[atomicAdd(vertex_count, 1)] = vec4(here / WORK_GROUP_SIZE, sign(noiseFunc(here)));
	if (x < points_from_zero) generateEdge(vec3(1.0f, 0.0f, 0.0f), edge_data[getArrayIndex(uvec3(x, y ,z)) * 3]);
	if (y < points_from_zero) generateEdge(vec3(0.0f, 1.0f, 0.0f), edge_data[getArrayIndex(uvec3(x, y, z)) * 3 + 1]);
	if (z < points_from_zero) generateEdge(vec3(0.0f, 0.0f, 1.0f), edge_data[getArrayIndex(uvec3(x, y, z)) * 3 + 2]);
}