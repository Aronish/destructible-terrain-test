#include "graphics/vertex_buffer.hpp"

namespace eng
{
    VertexBuffer::VertexBuffer(float * vertex_data, size_t data_array_size, VertexBufferLayout && layout) : m_layout(std::move(layout))
    {
        glCreateBuffers(1, &m_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
        glBufferData(GL_ARRAY_BUFFER, data_array_size, vertex_data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &m_id);
    }

    void VertexBuffer::bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
    }
}