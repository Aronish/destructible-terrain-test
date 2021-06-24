#pragma once

#include <glad/glad.h>

#include "vertex_buffer_layout.hpp"

namespace eng
{
    class VertexBuffer
    {
    private:
        GLuint m_vertex_buffer;
        VertexBufferLayout m_layout;
    public:
        VertexBuffer(float * vertex_data, size_t data_array_size, VertexBufferLayout && layout);
        ~VertexBuffer();

        void bind() const;

        VertexBufferLayout const & getLayout() const { return m_layout; }
    };
}