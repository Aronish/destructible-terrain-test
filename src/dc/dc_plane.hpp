#pragma once

#include <glad/glad.h>

#include "game_system.hpp"

namespace eng
{
	class DCPlane
	{
	private:
		GLuint m_quad_va, m_quad_data, m_quad_indices;
		int m_vertex_count{};
		float m_size_in_units{ 4.0f };

		std::shared_ptr<Shader> m_dual_contouring;

	public:
		DCPlane(GameSystem & game_system);

		GLuint getVertexArray() const;
		int getIndexCount() const;

		bool meshEmpty() const;
	};
}