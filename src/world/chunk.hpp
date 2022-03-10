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
        int unsigned constexpr static inline CHUNK_SIZE_IN_UNITS = 1;

    private:
        std::shared_ptr<VertexArray> m_vertex_array;
        std::shared_ptr<ShaderStorageBuffer> m_mesh;
        bool m_mesh_empty = true, m_active = false;
        union
        {
            glm::ivec2 m_position;
            Chunk * m_next_unused;
        };

    public:
        Chunk(GLuint max_index_buffer, int unsigned max_index_count);

        void activate(glm::ivec2 position);

        void render(std::shared_ptr<Shader> const & shader, FirstPersonCamera const & camera) const;

        void setNextUnused(Chunk * chunk);
        Chunk * getNextUnused() const;

        void setMeshEmpty(bool mesh_empty);

        void setActive(bool active);
        bool isActive() const;

        std::shared_ptr<ShaderStorageBuffer> getMesh() const;

        glm::ivec2 const & getPosition() const;
    };

    class ChunkPool
    {
    private:
        std::vector<Chunk> m_chunks;
        Chunk * m_first_unused;
        GLuint m_max_index_buffer;
        int unsigned m_max_index_count;
    public:
        void initialize(int unsigned initial_size, GLuint max_index_buffer, int unsigned max_index_count);
        void setPoolSize(int unsigned size);
        bool activateChunk(Chunk ** out_chunk, glm::ivec2 position);
        void deactivateChunk(Chunk * chunk);

        std::vector<Chunk> const & getChunkList() const;

        std::vector<Chunk>::iterator begin() { return m_chunks.begin(); }
        std::vector<Chunk>::iterator end() { return m_chunks.end(); }
        std::vector<Chunk>::const_iterator begin() const { return m_chunks.begin(); }
        std::vector<Chunk>::const_iterator end() const { return m_chunks.end(); }
    };

}