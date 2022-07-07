#pragma once

#include <vector>

#include <glad/glad.h>

#include "logger.hpp"

namespace eng
{
    constexpr static GLuint GLTypeToSize(GLenum type)
    {
        switch (type)
        {
        case GL_INT:
        case GL_FLOAT: return 4;
        default:
            ENG_LOG_F("Unsupported GL type: %d!", type);
            return 0;
        }
    }

    struct VertexDataElement
    {
    public:
        GLuint m_size, m_type_size;
        GLuint m_offset{};
        GLenum m_type;
    public:
        VertexDataElement(int unsigned size, GLenum type);
    };

    class VertexDataLayout
    {
    public:
        VertexDataLayout const static POSITION_NORMAL_3F;
        VertexDataLayout const static POSIITON_UV_2F;
    private:
        std::vector<VertexDataElement> m_elements;
        int unsigned m_stride{};
    public:
        VertexDataLayout(std::vector<VertexDataElement> && elements);

        int unsigned getStride() const { return m_stride; }
        std::vector<VertexDataElement>::iterator begin() { return m_elements.begin(); }
        std::vector<VertexDataElement>::iterator end() { return m_elements.end(); }
        std::vector<VertexDataElement>::const_iterator begin() const { return m_elements.begin(); }
        std::vector<VertexDataElement>::const_iterator end() const { return m_elements.end(); }
    };
}