#pragma once

#include <memory>

#include <glad/glad.h>

#include "vertex_buffer.hpp"
#include "shader_storage_buffer.hpp"

namespace eng
{
    class VertexArray
    {
    private:
        GLuint m_vertex_array, m_index_buffer;
        GLsizei m_index_count;
    public:
        VertexArray(int * indices, size_t index_array_size);
        VertexArray(GLuint shared_index_buffer, int unsigned index_count);
        ~VertexArray();

        void setIndexCount(int unsigned count);
        GLsizei getIndexCount() const;

        GLuint getVertexArrayId() const;

        void bind() const;
        void drawElements() const;
        void setVertexData(std::shared_ptr<VertexBuffer> const & vertex_buffer);
        void setVertexData(std::shared_ptr<ShaderStorageBuffer> const & vertex_buffer, VertexDataLayout && layout);
    };
}