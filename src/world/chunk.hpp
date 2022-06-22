#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <PxPhysicsAPI.h>

#include "first_person_camera.hpp"
#include "game_system.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"

namespace eng
{
    class Chunk
    {
    private:
        GLuint m_mesh_vb, m_density_distribution_ss, m_draw_indirect_buffer;
        physx::PxRigidStatic * m_static_rigid_body;
        bool m_active = false;
        GameSystem & r_game_system;
        union
        {
            glm::ivec3 m_position;
            Chunk * m_next_unused;
        };

    public:
        Chunk(GameSystem & game_system, int unsigned max_triangle_count, int unsigned points_per_chunk_axis);
        void releasePhysics();
        void setMeshConfig(int unsigned max_triangle_count, int unsigned points_per_chunk_axis);

        void activate(glm::ivec3 position);
        void deactivate(Chunk * chunk);

        glm::ivec3 const & getPosition() const;
        Chunk * getNextUnused() const;

        bool isActive() const;

        GLuint getMeshVB() const;
        GLuint getDensityDistributionBuffer() const;
        GLuint getDrawIndirectBuffer() const;

        physx::PxRigidStatic * getStaticRigidBody() const;
    };

    class ChunkPool
    {
    private:
        std::vector<Chunk> m_chunks;
        Chunk * m_first_unused;
        int unsigned m_max_triangle_count, m_points_per_chunk_axis;

        GameSystem & r_game_system;
    public:
        ChunkPool(GameSystem & game_system);
        ~ChunkPool();
        void initialize(int unsigned initial_size, int unsigned max_triangle_count, int unsigned points_per_chunk_axis);
        void setMeshConfig(int unsigned max_triangle_count, int unsigned points_per_chunk_axis);
        void setPoolSize(int unsigned size);
        bool activateChunk(Chunk ** out_chunk, glm::ivec3 position);
        void deactivateChunk(Chunk * chunk);
        bool getChunkAt(glm::ivec3 const & position, std::vector<Chunk>::iterator & out_chunk);
        bool hasChunkAt(glm::ivec3 const & position);

        std::vector<Chunk>::iterator begin() { return m_chunks.begin(); }
        std::vector<Chunk>::iterator end() { return m_chunks.end(); }
        std::vector<Chunk>::const_iterator begin() const { return m_chunks.begin(); }
        std::vector<Chunk>::const_iterator end() const { return m_chunks.end(); }
    };
}