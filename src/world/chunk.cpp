#include <chrono>

#include "logger.hpp"

#include "world/world.hpp"
#include "world/chunk.hpp"

namespace eng
{
    Chunk::Chunk(GLuint max_index_buffer, int unsigned max_triangle_count)
    {
        m_mesh = std::make_shared<ShaderStorageBuffer>(max_triangle_count * sizeof(float) * 18, nullptr, GL_DYNAMIC_STORAGE_BIT);
        m_vertex_array = std::make_shared<VertexArray>(max_index_buffer, max_triangle_count * 3);
        m_vertex_array->setVertexData(m_mesh, VertexDataLayout{{{3, GL_FLOAT}, {3, GL_FLOAT}}});

        int unsigned constexpr initial_indirect_config[] = { 0, 1, 0, 0, 0, 0 };
        m_indirect_draw_buffer = std::make_shared<ShaderStorageBuffer>(sizeof(initial_indirect_config), &initial_indirect_config, GL_DYNAMIC_STORAGE_BIT | GL_CLIENT_STORAGE_BIT);

        m_next_unused = nullptr;
    }

    void Chunk::activate(glm::ivec2 position)
    {
        m_position = position;
        m_active = true;
    }

    void Chunk::deactivate(Chunk * next_unused)
    {
        m_next_unused = next_unused;
        m_active = false;
    }

    void Chunk::render() const
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirect_draw_buffer->getId());
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr);
    }

    Chunk * Chunk::getNextUnused() const
    {
        return m_active ? nullptr : m_next_unused;
    }

    bool Chunk::meshEmpty() const
    {
        return m_mesh_empty;
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

    void ChunkPool::initialize(int unsigned initial_size, GLuint max_index_buffer, int unsigned max_triangle_count)
    {
        setMeshConfig(max_index_buffer, max_triangle_count);
        setPoolSize(initial_size);
    }

    void ChunkPool::setMeshConfig(GLuint max_index_buffer, int unsigned max_triangle_count)
    {
        m_max_index_buffer = max_index_buffer;
        m_max_triangle_count = max_triangle_count;
    }

    void ChunkPool::setPoolSize(int unsigned size)
    {
        m_chunks.clear();
        m_chunks.reserve(size);
        // Allocate all chunks and setup free list
        for (int unsigned i = 0; i < size; ++i)
        {
            Chunk & chunk = m_chunks.emplace_back(m_max_index_buffer, m_max_triangle_count * 3);
            if (i > 0) m_chunks[i - 1].deactivate(&chunk);
        }
        m_first_unused = &m_chunks[0];
        m_chunks[size - 1].deactivate(nullptr);
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
        chunk->deactivate(m_first_unused);
        m_first_unused = chunk;
    }
}