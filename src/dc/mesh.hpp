#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glad/glad.h>

#include "game_system.hpp"

namespace eng
{
	struct MeshVertex
	{
		MeshVertex(glm::vec3 const & position, glm::vec3 const & normal);

		glm::vec3 position, normal;
	};

	class Mesh
	{
	public:
		using VertexBuffer = std::vector<MeshVertex>;

		GLuint va_, vb_, ib_;
		int index_count_{};
	public:
		explicit Mesh(GameSystem & game_system);
		void refreshMesh(VertexBuffer const & vertices, std::vector<int> const & indices);
	};
}
