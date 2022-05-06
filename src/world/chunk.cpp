#include "graphics/vertex_buffer_layout.hpp"
#include "logger.hpp"
#include "world/world.hpp"

#include "world/chunk.hpp"

namespace eng
{
    Chunk::Chunk(AssetManager & asset_manager, int unsigned max_triangle_count, int unsigned points_per_chunk_axis)
    {
        m_mesh_vb = asset_manager.createBuffer();
        m_density_distribution_ss = asset_manager.createBuffer();
        setMeshConfig(max_triangle_count, points_per_chunk_axis);

        int unsigned constexpr initial_indirect_config[] = { 0, 1, 0, 0, 0, 0 };
        m_draw_indirect_buffer = asset_manager.createBuffer();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_draw_indirect_buffer);
        glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(initial_indirect_config), &initial_indirect_config, GL_DYNAMIC_STORAGE_BIT | GL_CLIENT_STORAGE_BIT);

        m_next_unused = nullptr;
    }
    
    void Chunk::setMeshConfig(int unsigned max_triangle_count, int unsigned points_per_chunk_axis)
    {
        //glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_mesh_vb);
        //glBufferData(GL_SHADER_STORAGE_BUFFER, max_triangle_count * sizeof(float) * 18, nullptr, GL_DYNAMIC_COPY);
        glNamedBufferData(m_mesh_vb, max_triangle_count * sizeof(float) * 18, nullptr, GL_DYNAMIC_COPY);
        
        //glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_density_distribution_ss);
        //glBufferData(GL_SHADER_STORAGE_BUFFER, points_per_chunk_axis * points_per_chunk_axis * points_per_chunk_axis * sizeof(float), nullptr, GL_DYNAMIC_COPY);
        glNamedBufferData(m_density_distribution_ss, points_per_chunk_axis * points_per_chunk_axis * points_per_chunk_axis * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    }

    void Chunk::setShit(bool shit)
    {
        m_shit = shit;
    }

    bool Chunk::isShit() const
    {
        return m_shit;
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

    Chunk * Chunk::getNextUnused() const
    {
        return m_active ? nullptr : m_next_unused;
    }
    
    bool Chunk::isActive() const
    {
        return m_active;
    }

    GLuint Chunk::getMeshVB() const
    {
        return m_mesh_vb;
    }

    GLuint Chunk::getDensityDistributionBuffer() const
    {
        return m_density_distribution_ss;
    }

    GLuint Chunk::getDrawIndirectBuffer() const
    {
        return m_draw_indirect_buffer;
    }

    glm::ivec2 const & Chunk::getPosition() const
    {
        return m_position;
    }
    
    //ChunkPool

    ChunkPool::ChunkPool(AssetManager & asset_manager) : m_asset_manager(asset_manager)
    {
    }

    void ChunkPool::initialize(AssetManager & asset_manager, int unsigned initial_size, int unsigned max_triangle_count, int unsigned points_per_chunk_axis)
    {
        m_asset_manager = asset_manager;
        setMeshConfig(max_triangle_count, points_per_chunk_axis);
        setPoolSize(initial_size);
    }

    void ChunkPool::setMeshConfig(int unsigned max_triangle_count, int unsigned points_per_chunk_axis)
    {
        m_max_triangle_count = max_triangle_count;
        m_points_per_chunk_axis = points_per_chunk_axis;

        for (auto & chunk : m_chunks)
        {
            chunk.setMeshConfig(max_triangle_count, points_per_chunk_axis);
        }
    }

    void ChunkPool::setPoolSize(int unsigned size)
    {
        m_chunks.clear();
        m_chunks.reserve(size);
        // Allocate all chunks and setup free list
        for (int unsigned i = 0; i < size; ++i)
        {
            Chunk & chunk = m_chunks.emplace_back(m_asset_manager, m_max_triangle_count, m_points_per_chunk_axis);
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