#pragma once

#include <glad/glad.h>

namespace eng
{
    class Texture
    {
    private:
        GLuint m_texture_handle;
        int m_width, m_height;

    public:
        explicit Texture(char const * path);
        ~Texture();

        void bind(unsigned int unit) const;

        int getWidth() const;
        int getHeight() const;
    };
}