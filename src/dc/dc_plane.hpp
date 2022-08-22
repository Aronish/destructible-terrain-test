#pragma once

#include <glad/glad.h>

#include "game_system.hpp"

namespace eng
{
	class DCPlane
	{
	private:
		int unsigned constexpr static WORK_GROUP_SIZE = 10;
		int unsigned constexpr static s_di_config[] = { 0, 1, 0, 0, 0, 0 };
	public:
		GLuint m_edge_data, m_dc_vertices, m_points, m_materials;
		GLuint m_va_mesh, m_mesh, m_di_mesh;
		GLuint m_va_normals, m_normals, m_di_normals;
		int unsigned chunk_size{ WORK_GROUP_SIZE - 1 };
		
		std::shared_ptr<Shader> m_feature_point_generator;
		std::shared_ptr<Shader> m_edge_data_generator;
		std::shared_ptr<Shader> m_mesher;

		float m_threshold{}, m_frequency{ 0.15f };
	public:
		DCPlane(GameSystem & game_system);

		void generate(bool recompile = false);
	};
}