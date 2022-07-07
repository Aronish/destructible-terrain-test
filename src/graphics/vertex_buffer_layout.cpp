#include "graphics/vertex_buffer_layout.hpp"

namespace eng
{
    VertexDataLayout const VertexDataLayout::POSITION_NORMAL_3F = {{{ 3, GL_FLOAT }, { 3, GL_FLOAT }}};
    VertexDataLayout const VertexDataLayout::POSIITON_UV_2F = {{{ 2, GL_FLOAT }, { 2, GL_FLOAT }}};

    VertexDataElement::VertexDataElement(int unsigned size, GLenum type) : m_size(size), m_type_size(GLTypeToSize(type)), m_type(type) {}

    VertexDataLayout::VertexDataLayout(std::vector<VertexDataElement> && elements) : m_elements(std::move(elements))
    {
        int unsigned offset = 0;
        for (auto & element : m_elements)
        {
            element.m_offset = offset;
            offset += element.m_size * element.m_type_size;
            m_stride += element.m_size * element.m_type_size;
        }
    }
}