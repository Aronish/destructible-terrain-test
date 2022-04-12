#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "first_person_camera.hpp"
#include "graphics/shader.hpp"
#include "graphics/shader_storage_buffer.hpp"
#include "graphics/vertex_array.hpp"

namespace eng
{
    class Chunk
    {
    private:
        GLuint m_vertex_array;
        GLuint m_mesh_vb, m_draw_indirect_buffer;
        bool m_mesh_empty = false, m_active = false;
        union
        {
            glm::ivec2 m_position;
            Chunk * m_next_unused;
        };

    public:
        Chunk(AssetManager & asset_manager, int unsigned max_triangle_count);

        void activate(glm::ivec2 position);
        void deactivate(Chunk * chunk);

        glm::ivec2 const & getPosition() const;
        Chunk * getNextUnused() const;

        bool meshEmpty() const;
        bool isActive() const;

        GLuint getVertexArray() const;
        GLuint getMeshVB() const;
        GLuint getDrawIndirectBuffer() const;
    };

    class ChunkPool
    {
    private:
        std::vector<Chunk> m_chunks;
        Chunk * m_first_unused;
        int unsigned m_max_triangle_count;

        AssetManager & m_asset_manager;
    public:
        ChunkPool(AssetManager & asset_manager);
        void initialize(AssetManager & asset_manager, int unsigned initial_size, int unsigned max_triangle_count);
        void setMeshConfig(int unsigned max_triangle_count);
        void setPoolSize(int unsigned size);
        bool activateChunk(Chunk ** out_chunk, glm::ivec2 position);
        void deactivateChunk(Chunk * chunk);

        std::vector<Chunk>::iterator begin() { return m_chunks.begin(); }
        std::vector<Chunk>::iterator end() { return m_chunks.end(); }
        std::vector<Chunk>::const_iterator begin() const { return m_chunks.begin(); }
        std::vector<Chunk>::const_iterator end() const { return m_chunks.end(); }
    };
}