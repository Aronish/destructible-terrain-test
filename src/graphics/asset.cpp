#include "logger.hpp"

#include "graphics/asset.hpp"

namespace eng
{
    AssetManager::~AssetManager()
    {
        glDeleteBuffers(static_cast<GLsizei>(m_buffers.size()), m_buffers.data());
        glDeleteVertexArrays(static_cast<GLsizei>(m_vertex_arrays.size()), m_vertex_arrays.data());
    }

    std::shared_ptr<Shader> & AssetManager::getShader(char const * key)
    {
        return (*m_shaders.try_emplace(key, std::make_shared<Shader>(key)).first).second;
    }

    std::shared_ptr<Texture> & AssetManager::getTexture(char const * key)
    {
        return (*m_textures.try_emplace(key, std::make_shared<Texture>(key)).first).second;
    }

    GLuint AssetManager::createBuffer()
    {
        GLuint buffer;
        glCreateBuffers(1, &buffer);
        m_buffers.push_back(buffer);
        return buffer;
    }
    
    GLuint AssetManager::createVertexArray()
    {
        GLuint vertex_array;
        glCreateVertexArrays(1, &vertex_array);
        m_buffers.push_back(vertex_array);
        return vertex_array;
    }
}