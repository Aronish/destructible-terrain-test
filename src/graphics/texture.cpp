#include <stb_image/stb_image.hpp>

#include "logger.hpp"

#include "graphics/texture.hpp"

namespace eng
{
    Texture::Texture(char const * path)
    {
        stbi_set_flip_vertically_on_load(true);
        int channels;
        unsigned char * data = stbi_load(path, &m_width, &m_height, &channels, 0);
#ifdef ENG_DEBUG
        if (!data)
        {
            ENG_LOG_F("Failed to load texture at %s! %s", path, stbi_failure_reason());
        }
#endif
        int internal_format, format;
        switch (channels)
        {
            case 3:
                internal_format = GL_RGB8;
                format = GL_RGB;
                break;
            case 4:
                internal_format = GL_RGBA8;
                format = GL_RGBA;
                break;
            default:
                internal_format = -1;
                format = -1;
                ENG_LOG_F("Unsupported channel format in image at %s! Channels found: %d", path, channels);
                break;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_handle);
        glBindTexture(GL_TEXTURE_2D, m_texture_handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
        ENG_LOG_F("Loaded texture %s | Width: %d, Height %d, Channels: %d", path, m_width, m_height, channels);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &m_texture_handle);
    }

    void Texture::bind(unsigned int unit) const
    {
        glBindTextureUnit(unit, m_texture_handle);
    }

    int Texture::getWidth() const
    {
        return m_width;
    }

    int Texture::getHeight() const
    {
        return m_height;
    }
}