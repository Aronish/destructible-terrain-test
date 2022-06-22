#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "graphics/shader.hpp"
#include "graphics/texture.hpp"

namespace eng
{
    class AssetManager
    {
    private:
        std::unordered_map<char const *, std::shared_ptr<Shader>> m_shaders;
        std::unordered_map<char const *, std::shared_ptr<Texture>> m_textures;

        std::vector<GLuint> m_buffers;
        std::vector<GLuint> m_vertex_arrays;

    public:
        ~AssetManager();

        std::shared_ptr<Shader> & getShader(char const * key);
        std::shared_ptr<Texture> & getTexture(char const * key);

        GLuint createBuffer();
        void deleteBuffer(GLuint buffer);

        GLuint createVertexArray();
    };
}