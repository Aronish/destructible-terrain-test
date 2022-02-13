#pragma once
#include <glad/glad.h>

namespace eng
{
    class AtomicCounterBuffer
    {
    private:
        GLuint m_id;

    public:
        AtomicCounterBuffer(size_t size);
        ~AtomicCounterBuffer();

        void bind(int unsigned binding_point) const;
        void setBufferSubDataUnsafe(int unsigned * data, size_t size, int offset = 0) const;
        void bindBuffer() const;
        void unbindBuffer() const;
    };
}