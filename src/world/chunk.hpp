#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "first_person_camera.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_storage_buffer.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer.hpp"

namespace eng
{
    class Chunk
    {
    private:
        std::shared_ptr<VertexArray> m_vertex_array;
        std::shared_ptr<ShaderStorageBuffer> m_mesh;
        bool m_mesh_empty = true;
        glm::vec2 m_position;

    public:
        Chunk(glm::vec2 position, std::shared_ptr<VertexArray> && vertex_array, std::shared_ptr<ShaderStorageBuffer> && mesh, bool mesh_empty);

        void render(std::shared_ptr<Shader> const & shader, FirstPersonCamera const & camera) const;
    };
}