#include "logger.hpp"

#include "atomic_counter_buffer.hpp"

namespace eng
{
    AtomicCounterBuffer::AtomicCounterBuffer(size_t size)
    {
        glCreateBuffers(1, &m_id);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_id);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, size, nullptr, GL_DYNAMIC_READ);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }

    AtomicCounterBuffer::~AtomicCounterBuffer()
    {
        glDeleteBuffers(1, &m_id);
    }

    void AtomicCounterBuffer::bind(int unsigned binding_point) const
    {
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, binding_point, m_id);
    }

    void AtomicCounterBuffer::setBufferSubDataUnsafe(int unsigned * data, size_t size, int offset) const
    {
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, offset, size, data);
    }

    void AtomicCounterBuffer::bindBuffer() const
    {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_id);
    }

    void AtomicCounterBuffer::unbindBuffer() const
    {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }
}