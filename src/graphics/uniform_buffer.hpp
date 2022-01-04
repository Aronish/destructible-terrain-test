#include <glad/glad.h>

namespace eng
{
    class UniformBuffer
    {
    private:
        GLuint m_id;

    public:
        explicit UniformBuffer(size_t size);
        ~UniformBuffer();

        void bind(int binding_point) const;
        void setSubDataUnsafe(float * data, size_t size, int offset) const;
        void bindBuffer() const;
        void unbindBuffer() const;
    };
}