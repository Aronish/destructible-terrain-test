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
    public:
        int unsigned constexpr static inline CHUNK_SIZE_IN_UNITS = 32;

    private:
        std::shared_ptr<VertexArray> m_vertex_array;
        std::shared_ptr<ShaderStorageBuffer> m_mesh;
        std::shared_ptr<ShaderStorageBuffer> m_indirect_draw_buffer;
        bool m_mesh_empty = false, m_active = false;
        union
        {
            glm::ivec2 m_position;
            Chunk * m_next_unused;
        };

    public:
        Chunk(GLuint max_index_buffer, int unsigned max_index_count);

        void activate(glm::ivec2 position);

        void render() const;

        void setNextUnused(Chunk * chunk);
        Chunk * getNextUnused() const;

        bool meshEmpty() const;

        void setIndexCount(int unsigned count);

        void setActive(bool active);
        bool isActive() const;

        std::shared_ptr<VertexArray> getVertexArray() const;
        std::shared_ptr<ShaderStorageBuffer> getMesh() const;
        std::shared_ptr<ShaderStorageBuffer> getIndirectDrawBuffer() const;

        glm::ivec2 const & getPosition() const;
    };

    class ChunkPool
    {
    private:
        std::vector<Chunk> m_chunks;
        Chunk * m_first_unused;
        GLuint m_max_index_buffer; //Owned by World
        int unsigned m_max_index_count;
    public:
        void initialize(int unsigned initial_size, GLuint max_index_buffer, int unsigned max_index_count);
        void setPoolSize(int unsigned size);
        bool activateChunk(Chunk ** out_chunk, glm::ivec2 position);
        void deactivateChunk(Chunk * chunk);

        std::vector<Chunk>::iterator begin() { return m_chunks.begin(); }
        std::vector<Chunk>::iterator end() { return m_chunks.end(); }
        std::vector<Chunk>::const_iterator begin() const { return m_chunks.begin(); }
        std::vector<Chunk>::const_iterator end() const { return m_chunks.end(); }
    };

}