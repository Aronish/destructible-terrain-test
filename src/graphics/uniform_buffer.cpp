#include "graphics/uniform_buffer.hpp"

namespace eng
{
    UniformBuffer::UniformBuffer(size_t size)
    {
        glCreateBuffers(1, &m_id);
        glBindBuffer(GL_UNIFORM_BUFFER, m_id);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    UniformBuffer::~UniformBuffer()
    {
        glDeleteBuffers(1, &m_id);
    }

    void UniformBuffer::bind(int binding_point) const
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_id);
    }

    void UniformBuffer::setSubDataUnsafe(float * data, size_t size, int offset) const
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_id);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void UniformBuffer::bindBuffer() const
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_id);
    }

    void UniformBuffer::unbindBuffer() const
    {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}