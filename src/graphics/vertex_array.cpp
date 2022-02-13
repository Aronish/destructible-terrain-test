#include "graphics/vertex_array.hpp"

namespace eng
{
    VertexArray::VertexArray(int * indices, size_t index_array_size) : m_index_count((GLsizei)index_array_size / sizeof(int))
    {
        glCreateVertexArrays(1, &m_vertex_array);
        glCreateBuffers(1, &m_index_buffer);

        glBindVertexArray(m_vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)index_array_size, indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &m_vertex_array);
        glDeleteBuffers(1, &m_index_buffer);
    }

    void VertexArray::setVertexData(std::shared_ptr<VertexBuffer> const & vertex_buffer)
    {
        glBindVertexArray(m_vertex_array);
        vertex_buffer->bind();
        int attrib_index = 0;
        for (auto const & element : vertex_buffer->getLayout())
        {
            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(attrib_index, element.m_size, element.m_type, GL_FALSE, vertex_buffer->getLayout().getStride(), reinterpret_cast<void const *>(element.m_offset));
            ++attrib_index;
        }
        glBindVertexArray(0);
    }

    void VertexArray::setVertexData(std::shared_ptr<ShaderStorageBuffer> const & vertex_buffer, VertexDataLayout && layout)
    {
        glBindVertexArray(m_vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->getId());
        int attrib_index = 0;
        for (auto const & element : layout)
        {
            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(attrib_index, element.m_size, element.m_type, GL_FALSE, layout.getStride(), reinterpret_cast<void const *>(element.m_offset));
            ++attrib_index;
        }
        glBindVertexArray(0);
    }

    void VertexArray::bind() const
    {
        glBindVertexArray(m_vertex_array);
    }

    void VertexArray::drawElements() const
    {
        glDrawElements(GL_TRIANGLES, m_index_count, GL_UNSIGNED_INT, nullptr);
    }
}