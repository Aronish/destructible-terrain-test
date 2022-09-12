#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "cube_frame.hpp"

namespace eng
{
	class Octree
	{
	private:
		static constexpr int unsigned MAX_DEPTH = 4, INITIAL_SIZE = 16;
	private:
		glm::vec3 m_position;
		int unsigned m_depth, m_size;

		std::vector<glm::vec3> m_contents;
		Octree * m_child_nodes{};

	public:
		Octree(glm::vec3 const & position = {}, int unsigned size = INITIAL_SIZE, int unsigned depth = 0);
		~Octree();

		void addContent(glm::vec3 const & position);
		void render(std::shared_ptr<Shader> const & shader, CubeFrame const & frame);
	};
}