#pragma once

#include <type_traits>

#include <glad/glad.h>

namespace eng
{
    class ShaderStorageBuffer
    {
    private:
        GLuint m_id;

    public:
        ShaderStorageBuffer(size_t size, GLenum usage);
        ShaderStorageBuffer(size_t size, auto * data, GLenum usage) //Immutable buffer, mutable data
        {
            glCreateBuffers(1, &m_id);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
            glBufferStorage(GL_SHADER_STORAGE_BUFFER, size, data, usage);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
        ShaderStorageBuffer(size_t size, std::nullptr_t, GLenum usage);

        ~ShaderStorageBuffer();

        void bind(int binding_point) const;

        void setSubDataUnsafe(auto * data, size_t size, int offset) const
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
        
        void bindBuffer() const;
        void unbindBuffer() const;

        GLuint getId() const;
    };
}