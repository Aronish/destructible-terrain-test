#include "graphics/asset.hpp"

namespace eng
{
    std::shared_ptr<Shader> & AssetManager::getShader(char const * key) const
    {
        return (*m_shaders.try_emplace(key, std::make_shared<Shader>(key)).first).second;
    }
}