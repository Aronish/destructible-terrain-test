#pragma once

#include <glad/glad.h>

#include "game_system.hpp"

namespace eng
{
	class CubeFrame
	{
	public:
		GLuint const m_va, m_vb, m_ib;

	public:
		CubeFrame(GameSystem & game_system, float size);

		void render();
	};
}