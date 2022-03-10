#include <chrono>

#include "logger.hpp"

#include "world/world.hpp"
#include "world/chunk.hpp"

namespace eng
{
    Chunk::Chunk(GLuint max_index_buffer, int unsigned max_index_count)
    {
        m_mesh = std::make_shared<ShaderStorageBuffer>(World::s_max_triangle_amount * sizeof(float) * 18, GL_DYNAMIC_COPY);
        m_vertex_array = std::make_shared<VertexArray>(max_index_buffer, max_index_count);
        m_vertex_array->setVertexData(m_mesh, VertexDataLayout{{{3, GL_FLOAT}, {3, GL_FLOAT}}});
        m_next_unused = nullptr;
    }

    void Chunk::activate(glm::ivec2 position)
    {
        m_position = position;
        m_active = true;
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

    void Chunk::setNextUnused(Chunk * chunk)
    {
        m_next_unused = chunk;
        m_active = false;
    }

    Chunk * Chunk::getNextUnused() const
    {
        return m_next_unused;
    }

    void Chunk::setMeshEmpty(bool mesh_empty)
    {
        m_mesh_empty = mesh_empty;
    }

    void Chunk::setActive(bool active)
    {
        m_active = active;
    }

    bool Chunk::isActive() const
    {
        return m_active;
    }

    std::shared_ptr<ShaderStorageBuffer> Chunk::getMesh() const
    {
        return m_mesh;
    }

    glm::ivec2 const & Chunk::getPosition() const
    {
        return m_position;
    }
    
    //ChunkPool

    void ChunkPool::initialize(int unsigned initial_size, GLuint max_index_buffer, int unsigned max_index_count)
    {
        m_max_index_buffer = max_index_buffer;
        m_max_index_count = max_index_count;
        setPoolSize(initial_size);
        m_first_unused = &m_chunks[0];
        for (int i = 0; i < m_chunks.size() - 1; ++i)
        {
            m_chunks[i].setNextUnused(&m_chunks[i + 1]);
        }
        m_chunks[m_chunks.size() - 1].setNextUnused(nullptr);
    }

    void ChunkPool::setPoolSize(int unsigned size)
    {
        m_chunks.clear();
        m_chunks.reserve(size);
        for (int unsigned i = 0; i < size; ++i)
        {
            m_chunks.emplace_back(m_max_index_buffer, m_max_index_count);
        }
    }

    bool ChunkPool::activateChunk(Chunk ** out_chunk, glm::ivec2 position)
    {
        if (m_first_unused == nullptr) return false;
        *out_chunk = m_first_unused;
        m_first_unused = (*out_chunk)->getNextUnused();
        (*out_chunk)->activate(position);
        return true;
    }

    void ChunkPool::deactivateChunk(Chunk * chunk)
    {
        chunk->setNextUnused(m_first_unused);
        m_first_unused = chunk;
    }

    std::vector<Chunk> const & ChunkPool::getChunkList() const
    {
        return m_chunks;
    }
}