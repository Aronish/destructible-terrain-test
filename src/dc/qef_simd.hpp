#pragma once
//
// Quadric Error Function / Singluar Value Decomposition SSE2 implementation
// Public domain
//
// Input is a set of positions / vertices of a surface and the surface normals
// at these positions (i.e. Hermite data). A "best fit" position is calculated
// from these positions along with an error value.
//
// Exmaple use cases:
//   - placing a vertex inside a voxel/node for algorithms like Dual Contouring
//     (the positions and normals corresponding to edge/isosurface intersections)
//   - placing the vertex resulting from collapsing an edge when simplifying a mesh
//     (the positions and normals of the collapsed edge should be supplied)
//
// Usage:
// 
// SSE registers:
//
//	__m128 position[2] = { ... };
//	__m128 normal[2] = { ... };
//	__m128 solvedPos;
//	float error = qef_solve_from_points(position, normal, 2, &solvedPos);
//
// 4D vectors:
//
//	__declspec(align(16)) float positions[2 * 4] = { ... };
//	__declspec(align(16)) float normals[2 * 4] = { ... };
//	__declspec(align(16)) float solved[4];
//	float error = qef_solve_from_points_4d(positions, normals, 2, solvedPos);
//
// 3D vectors (or struct with 3 float members, e.g. glm::vec3):
//
//	glm::vec3 positions[2] = { ... };
//	glm::vec3 normals[2] = { ... };
//	glm::vec3 solvedPos;
//	float error = qef_solve_from_points_3d(&positions[0].x, &normals[0].x, 2, &solvedPos.x);
//

#include	<xmmintrin.h>
#include	<immintrin.h>

const int QEF_MAX_INPUT_COUNT = 12;

union Mat4x4
{
	float	m[4][4];
	__m128	row[4];
};

void qef_simd_add(
	const __m128 & p, const __m128 & n,
	Mat4x4 & ATA,
	__m128 & ATb,
	__m128 & pointaccum);

float qef_simd_calc_error(const Mat4x4 & A, const __m128 & x, const __m128 & b);


float qef_simd_solve(
	const Mat4x4 & ATA,
	const __m128 & ATb,
	const __m128 & pointaccum,
	__m128 & x);
// Ideally the data would already be in SSE registers & returned in a SEE register
float qef_solve_from_points(
	const __m128 * positions,
	const __m128 * normals,
	const int count,
	__m128 * solved_position);

// Expects 4d vectors contiguous in memory for the positions/normals
// Addresses pointed to by positions, normals and solved_position MUST be 16 byte aligned
// Writes result to 4d vector.
float qef_solve_from_points_4d(
	const float * positions,
	const float * normals,
	const int count,
	float * solved_position);

float qef_solve_from_points_4d_interleaved(
	const float * data,
	const size_t stride,
	const int count,
	float * solved_position);

// Expects 3d vectors contiguous in memory for the positions/normals
// No alignment requirements.
// Writes result to 3d vector.
float qef_solve_from_points_3d(
	const float * positions,
	const float * normals,
	const int count,
	float * solved_position);
