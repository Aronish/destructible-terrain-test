#pragma once

#include <memory>
#include <unordered_map>

#include "graphics/atomic_counter_buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_storage_buffer.hpp"

namespace eng
{
    class AssetManager
    {
    private:
        mutable std::unordered_map<char const *, std::shared_ptr<Shader>> m_shaders;

    public:
        std::shared_ptr<Shader> & getShader(char const * key) const;
    };
}