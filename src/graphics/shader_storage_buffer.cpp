#include "graphics/shader_storage_buffer.hpp"

namespace eng
{
    ShaderStorageBuffer::ShaderStorageBuffer(size_t size, GLenum usage)
    {
        glCreateBuffers(1, &m_id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, usage);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    ShaderStorageBuffer::~ShaderStorageBuffer()
    {
        glDeleteBuffers(1, &m_id);
    }

    void ShaderStorageBuffer::bind(int binding_point) const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, m_id);
    }

    void ShaderStorageBuffer::bindBuffer() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
    }

    void ShaderStorageBuffer::unbindBuffer() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    GLuint ShaderStorageBuffer::getId() const
    {
        return m_id;
    }
}