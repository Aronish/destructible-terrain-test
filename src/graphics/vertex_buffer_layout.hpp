#pragma once

#include <vector>

#include <glad/glad.h>

namespace eng
{
    struct VertexDataElement
    {
    public:
        unsigned int m_size, m_type_size;
        unsigned long long int m_offset;
        GLenum m_type;
    public:
        VertexDataElement(unsigned int size, GLenum type);
    };

    class VertexDataLayout
    {
    private:
        std::vector<VertexDataElement> m_elements;
        unsigned int m_stride{};
    public:
        VertexDataLayout(std::vector<VertexDataElement> && elements);

        unsigned int getStride() const { return m_stride; }
        std::vector<VertexDataElement>::iterator begin() { return m_elements.begin(); }
        std::vector<VertexDataElement>::iterator end() { return m_elements.end(); }
        std::vector<VertexDataElement>::const_iterator begin() const { return m_elements.begin(); }
        std::vector<VertexDataElement>::const_iterator end() const { return m_elements.end(); }
    };
}