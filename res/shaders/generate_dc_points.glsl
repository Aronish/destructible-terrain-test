#shader comp
#version 460 core

const uint WORK_GROUP_SIZE = 10;

layout(local_size_x = WORK_GROUP_SIZE, local_size_y = WORK_GROUP_SIZE, local_size_z = WORK_GROUP_SIZE) in;

layout (std430, binding = 0) buffer DensityDistribution
{
	float values[];
};

void main()
{
	const uint points_from_zero = WORK_GROUP_SIZE - 1;
    const float radius = 4.0f;
	if (gl_GlobalInvocationID.x > points_from_zero || gl_GlobalInvocationID.y > points_from_zero || gl_GlobalInvocationID.z > points_from_zero) return;
	float x = float(gl_GlobalInvocationID.x),
          y = float(gl_GlobalInvocationID.y),
          z = float(gl_GlobalInvocationID.z);
		      
    values[
        gl_GlobalInvocationID.z * WORK_GROUP_SIZE * WORK_GROUP_SIZE +
        gl_GlobalInvocationID.y * WORK_GROUP_SIZE +
        gl_GlobalInvocationID.x
    ] = (x - radius) * (x - radius) + (y - radius) * (y - radius) + (z - radius) * (z - radius) - radius * radius;
}