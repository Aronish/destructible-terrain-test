#pragma once

#include <memory>

#include <glad/glad.h>

#include "vertex_buffer.hpp"

namespace eng
{
    class VertexArray
    {
    private:
        GLuint m_vertex_array, m_index_buffer;
        std::shared_ptr<VertexBuffer> m_vertex_buffer;
        GLsizei m_index_count;
    public:
        VertexArray(int * indices, size_t index_array_size);
        ~VertexArray();

        void bind() const;
        void drawElements() const;
        void setVertexBuffer(std::shared_ptr<VertexBuffer> const & vertex_buffer);
    };
}