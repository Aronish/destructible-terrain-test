#include "graphics/vertex_array.hpp"

namespace eng::VertexArray
{
    void setVertexArrayFormat(GLuint vertex_array, VertexDataLayout const & layout)
    {
        for (int attrib_index = 0; auto const & element : layout)
        {
            glEnableVertexArrayAttrib(vertex_array, attrib_index);
            glVertexArrayAttribFormat(vertex_array, attrib_index, element.m_size, element.m_type, GL_FALSE, element.m_offset);
            glVertexArrayAttribBinding(vertex_array, attrib_index, 0);
            ++attrib_index;
        }
    }

    void bindVertexBuffer(GLuint vertex_array, GLuint vertex_buffer, VertexDataLayout const & layout)
    {
        glVertexArrayVertexBuffer(vertex_array, 0, vertex_buffer, 0, layout.getStride());
    }

    void associateVertexBuffer(GLuint vertex_array, GLuint vertex_buffer, VertexDataLayout const & layout)
    {
        setVertexArrayFormat(vertex_array, layout);
        bindVertexBuffer(vertex_array, vertex_buffer, layout);
    }

    void associateIndexBuffer(GLuint vertex_array, GLuint index_buffer, int * indices, size_t indices_size)
    {
        glBindVertexArray(vertex_array);
        glNamedBufferStorage(index_buffer, indices_size, indices, 0);
        glBindVertexArray(0);
    }
}