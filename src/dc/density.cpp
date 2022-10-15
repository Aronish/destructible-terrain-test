#include <glm/gtc/noise.hpp>

#include "density.hpp"

namespace eng
{
	float sphereSDF(glm::vec3 const & sample_position, glm::vec3 const & origin, float radius)
	{
		return glm::length(sample_position - origin) - radius;
	}

	float cuboidSDF(glm::vec3 const & sample_position, glm::vec3 const & origin, glm::vec3 const & half_dimensions)
	{
		glm::vec3 const & local_pos = sample_position - origin;
		glm::vec3 const & pos = local_pos;

		glm::vec3 const & d = glm::abs(pos) - half_dimensions;
		float const m = std::max(d.x, std::max(d.y, d.z));
		return std::min(m, glm::length(glm::max(d, glm::vec3(0.0f))));
	}

	float fractalNoise(int octaves, float frequency, float lacunarity, float persistence, glm::vec2 const & position)
	{
		float const SCALE = 1.0f / 128.0f;
		glm::vec2 p = position * SCALE;
		float noise = 0.0f;
		float amplitude = 1.0f;
		p *= frequency;
		for (int i = 0; i < octaves; i++)
		{
			noise += glm::simplex(p) * amplitude;
			p *= lacunarity;
			amplitude *= persistence;
		}
		return 0.5f * noise + 0.5f;
	}

	float getDensity(glm::vec3 const & sample_position)
	{
		float const MAX_HEIGHT = 20.0f;
		float const noise = fractalNoise(4, 0.5343f, 2.2324f, 0.68324f, glm::vec2(sample_position.x, sample_position.z));
		float const terrain = sample_position.y - (MAX_HEIGHT * noise);

		float const cube = cuboidSDF(sample_position, glm::vec3(-4., 10.0f, -4.0f), glm::vec3(12.0f));
		float const sphere = sphereSDF(sample_position, glm::vec3(15.0f, 2.5f, 1.0f), 16.0f);

		return std::max(-cube, std::min(sphere, terrain));
	}
}