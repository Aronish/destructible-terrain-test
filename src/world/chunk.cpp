#include <chrono>

#include "logger.hpp"

#include "world/world.hpp"
#include "world/chunk.hpp"

namespace eng
{
    Chunk::Chunk(glm::vec2 position, std::shared_ptr<VertexArray> && vertex_array, std::shared_ptr<ShaderStorageBuffer> && mesh, bool mesh_empty)
        : m_position(position), m_vertex_array(std::move(vertex_array)), m_mesh(std::move(mesh)), m_mesh_empty(mesh_empty)
    {
    }

    void Chunk::render(std::shared_ptr<Shader> const & shader, FirstPersonCamera const & camera) const
    {
        if (m_mesh_empty) return;
        shader->bind();
        shader->setUniformMatrix4f("u_model", glm::scale(glm::mat4(1.0f), glm::vec3(CHUNK_SIZE_IN_UNITS)) * glm::translate(glm::mat4(1.0f), glm::vec3(m_position.x, 0.0f, m_position.y)));
        shader->setUniformMatrix4f("u_view", camera.getViewMatrix());
        shader->setUniformMatrix4f("u_projection", camera.getProjectionMatrix());
        shader->setUniformVector3f("u_camera_position_W", camera.getPosition());
        m_vertex_array->bind();
        m_vertex_array->drawElements();
    }
}