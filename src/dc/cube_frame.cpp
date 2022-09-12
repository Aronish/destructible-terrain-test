#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer_layout.hpp"

#include "cube_frame.hpp"

namespace eng
{
	CubeFrame::CubeFrame(GameSystem & game_system, float size) :
        m_vb(game_system.getAssetManager().createBuffer()),
        m_ib(game_system.getAssetManager().createBuffer()),
        m_va(game_system.getAssetManager().createVertexArray())
	{
        float vertices[] =
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, size,
            0.0f, size, 0.0f,
            0.0f, size, size,
            size, 0.0f, 0.0f,
            size, 0.0f, size,
            size, size, 0.0f,
            size, size, size
        };

        int indices[] =
        {
            0, 4, 0, 2, 0, 1,
            7, 3, 7, 5, 7, 6,
            4, 6, 4, 5, 5, 1,
            2, 3, 3, 1, 2, 6
        };

        glNamedBufferStorage(m_vb, sizeof(vertices), vertices, 0);
        glNamedBufferStorage(m_ib, sizeof(indices), indices, 0);
        VertexArray::associateVertexBuffer(m_va, m_vb, VertexDataLayout{ {{ 3, GL_FLOAT }} });
        VertexArray::associateIndexBuffer(m_va, m_ib, indices, sizeof(indices));
	}

    void CubeFrame::render()
    {
        glBindVertexArray(m_va);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, nullptr);
    }
}