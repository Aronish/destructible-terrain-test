#pragma once

#include <glad/glad.h>

#include "game_system.hpp"

namespace eng
{
	class DCPlane
	{
	private:
		int unsigned constexpr static WORK_GROUP_SIZE = 10u;
		int unsigned constexpr static s_di_config[] = { 0, 1, 0, 0, 0 };
	public:
		GLuint m_density_distribution, m_dc_vertices, m_di, m_mesh, m_va;
		int m_vertex_count{};
		
		std::shared_ptr<Shader> m_dual_contouring;
		std::shared_ptr<Shader> m_density_generator;
		std::shared_ptr<Shader> m_dc_triangulator;

	public:
		DCPlane(GameSystem & game_system);

		void generate();

		int getIndexCount() const;

		bool meshEmpty() const;
	};
}