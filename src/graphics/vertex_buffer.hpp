#pragma once

#include <glad/glad.h>

#include "vertex_buffer_layout.hpp"

namespace eng
{
    class VertexBuffer
    {
    private:
        VertexDataLayout m_layout;
    public:
        GLuint m_id;
        VertexBuffer(float * vertex_data, size_t data_array_size, VertexDataLayout && layout);
        ~VertexBuffer();

        void bind() const;

        VertexDataLayout const & getLayout() const { return m_layout; }
    };
}