#pragma once

#include <memory>
#include <unordered_map>

#include "graphics/atomic_counter_buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_storage_buffer.hpp"
#include "graphics/texture.hpp"

namespace eng
{
    class AssetManager
    {
    private:
        mutable std::unordered_map<char const *, std::shared_ptr<Shader>> m_shaders;
        mutable std::unordered_map<char const *, std::shared_ptr<Texture>> m_textures;

    public:
        std::shared_ptr<Shader> & getShader(char const * key) const;
        std::shared_ptr<Texture> & getTexture(char const * key) const;
    };
}