#pragma once

#include <glad/glad.h>

#include "vertex_buffer_layout.hpp"

namespace eng
{
    class VertexBuffer
    {
    private:
        VertexBufferLayout m_layout;
    public:
        GLuint m_id;
        VertexBuffer(float * vertex_data, size_t data_array_size, VertexBufferLayout && layout);
        ~VertexBuffer();

        void bind() const;

        VertexBufferLayout const & getLayout() const { return m_layout; }
    };
}