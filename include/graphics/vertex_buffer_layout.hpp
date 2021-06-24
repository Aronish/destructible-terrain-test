#pragma once

#include <vector>

#include <glad/glad.h>

namespace eng
{
    struct VertexBufferElement
    {
    public:
        unsigned int m_size, m_type_size;
        unsigned long long int m_offset;
        GLenum m_type;
    public:
        VertexBufferElement(unsigned int size, GLenum type);
    };

    class VertexBufferLayout
    {
    private:
        std::vector<VertexBufferElement> m_elements;
        unsigned int m_stride{};
    public:
        VertexBufferLayout(std::vector<VertexBufferElement> && elements);

        unsigned int getStride() const { return m_stride; }
        std::vector<VertexBufferElement>::iterator begin() { return m_elements.begin(); }
        std::vector<VertexBufferElement>::iterator end() { return m_elements.end(); }
        std::vector<VertexBufferElement>::const_iterator begin() const { return m_elements.begin(); }
        std::vector<VertexBufferElement>::const_iterator end() const { return m_elements.end(); }
    };
}