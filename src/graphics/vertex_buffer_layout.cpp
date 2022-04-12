#include "logger.hpp"

#include "graphics/vertex_buffer_layout.hpp"

namespace eng
{
    static constexpr unsigned int GLTypeToSize(GLenum type)
    {
        switch (type)
        {
            case GL_FLOAT: return 4;
            default:
                ENG_LOG_F("Unsupported GL type: %d!", type);
                return 0;
        }
    }

    VertexDataLayout const VertexDataLayout::POSITION_NORMAL_3F = {{{ 3, GL_FLOAT }, { 3, GL_FLOAT }}};
    VertexDataLayout const VertexDataLayout::POSIITON_UV_2F = {{{ 2, GL_FLOAT }, { 2, GL_FLOAT }}};

    VertexDataElement::VertexDataElement(unsigned int size, GLenum type) : m_size(size), m_type_size(GLTypeToSize(type)), m_type(type) {}

    VertexDataLayout::VertexDataLayout(std::vector<VertexDataElement> && elements) : m_elements(std::move(elements))
    {
        unsigned int offset = 0;
        for (auto & element : m_elements)
        {
            element.m_offset = offset;
            offset += element.m_size * element.m_type_size;
            m_stride += element.m_size * element.m_type_size;
        }
    }
}