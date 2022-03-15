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

        int unsigned constexpr initial_indirect_config[] = { 0, 1, 0, 0, 0, 0 };
        m_indirect_draw_buffer = std::make_shared<ShaderStorageBuffer>(sizeof(initial_indirect_config), &initial_indirect_config, GL_DYNAMIC_STORAGE_BIT);

        m_next_unused = nullptr;
    }

    void Chunk::activate(glm::ivec2 position)
    {
        m_position = position;
        m_active = true;
    }

    void Chunk::render() const
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_draw_buffer->getId());
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
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

    bool Chunk::meshEmpty() const
    {
        return m_mesh_empty;
    }

    void Chunk::setIndexCount(int unsigned count)
    {
        m_vertex_array->setIndexCount(count);
        m_mesh_empty = count == 0;
    }

    void Chunk::setActive(bool active)
    {
        m_active = active;
    }

    bool Chunk::isActive() const
    {
        return m_active;
    }


    std::shared_ptr<VertexArray> Chunk::getVertexArray() const
    {
        return m_vertex_array;
    }

    std::shared_ptr<ShaderStorageBuffer> Chunk::getMesh() const
    {
        return m_mesh;
    }

    std::shared_ptr<ShaderStorageBuffer> Chunk::getIndirectDrawBuffer() const
    {
        return m_indirect_draw_buffer;
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
}