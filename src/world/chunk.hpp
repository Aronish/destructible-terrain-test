#pragma once

#include <memory>
#include <vector>

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
        std::vector<uint32_t> inline static s_indices{};

    public:
        static void generateIndices(size_t count);

    private:
        GLuint m_mesh_vb, m_density_distribution_ss, m_draw_indirect_buffer;
        int unsigned m_vertex_count{};
        physx::PxRigidStatic * m_static_rigid_body;
        bool m_active = false, m_has_valid_collider;

        GameSystem & r_game_system;
        union
        {
            glm::ivec3 m_position{};
            Chunk * m_next_unused;
        };

    public:
        Chunk(GameSystem & game_system, size_t max_triangle_count, size_t points_per_chunk_axis);

        void releasePhysics();
        void setMeshConfig(size_t max_triangle_count, size_t points_per_chunk_axis);
        void setMeshCollider(std::vector<float> const & mesh, physx::PxMaterial * material, float chunk_size);
        void removeCollider();
        void setMeshInfo(int unsigned vertex_count);

        void activate(glm::ivec3 position, float chunk_size);
        void deactivate(Chunk * chunk);

        glm::ivec3 const & getPosition() const;
        Chunk * getNextUnused() const;

        bool isActive() const;

        GLuint getMeshVB() const;
        GLuint getDensityDistributionBuffer() const;
        GLuint getDrawIndirectBuffer() const;

        physx::PxRigidStatic * getRigidBody() const;
    };

    class ChunkPool
    {
    private:
        std::vector<Chunk> m_chunks;
        Chunk * m_first_unused{};
        size_t m_max_triangle_count{}, m_points_per_chunk_axis{};

        GameSystem & r_game_system;
    public:
        ChunkPool(GameSystem & game_system);
        ~ChunkPool();
        void initialize(size_t initial_size, size_t max_triangle_count, size_t points_per_chunk_axis);
        void setMeshConfig(size_t max_triangle_count, size_t points_per_chunk_axis);
        void setPoolSize(size_t size);
        bool activateChunk(Chunk *& out_chunk, glm::ivec3 position, float chunk_size);
        void deactivateChunk(Chunk * chunk);
        bool getChunkAt(glm::ivec3 const & position, std::vector<Chunk>::iterator & out_chunk);
        bool hasChunkAt(glm::ivec3 const & position);

        std::vector<Chunk>::iterator begin() { return m_chunks.begin(); }
        std::vector<Chunk>::iterator end() { return m_chunks.end(); }
        std::vector<Chunk>::const_iterator begin() const { return m_chunks.begin(); }
        std::vector<Chunk>::const_iterator end() const { return m_chunks.end(); }
    };
}