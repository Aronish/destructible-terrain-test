#include "graphics/vertex_array.hpp"

namespace eng::VertexArray
{
    void associateVertexBuffer(GLuint vertex_array, GLuint vertex_buffer, VertexDataLayout const & layout)
    {
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        int attrib_index = 0;
        for (auto const & element : layout)
        {
            glEnableVertexAttribArray(attrib_index);
            glVertexAttribPointer(attrib_index, element.m_size, element.m_type, GL_FALSE, layout.getStride(), reinterpret_cast<void const *>(element.m_offset));
            ++attrib_index;
        }
        glBindVertexArray(0);
    }

    void associateIndexBuffer(GLuint vertex_array, GLuint index_buffer, int * indices, size_t indices_size)
    {
        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices, GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
}