#include <algorithm>
#include <chrono>
#include <cmath>

#include "graphics/vertex_data_layout.hpp"
#include "logger.hpp"
#include "world/world.hpp"

#include "world/chunk.hpp"

namespace eng
{
    void Chunk::generateIndices(size_t count)
    {
        s_indices.clear();
        s_indices.resize(count);
        std::generate(s_indices.begin(), s_indices.end(), [n = 0]() mutable { return n++; });
        s_indices.shrink_to_fit();
    }

    Chunk::Chunk(GameSystem & game_system, int unsigned base_lod_point_width) : r_game_system(game_system), m_next_unused(nullptr)
    {
        m_mesh_vb = r_game_system.getAssetManager().createBuffer();
        m_density_distribution_ss = r_game_system.getAssetManager().createBuffer();
        setMeshConfig(base_lod_point_width);

        m_draw_indirect_buffer = r_game_system.getAssetManager().createBuffer();
        glNamedBufferStorage(m_draw_indirect_buffer, sizeof(World::INITIAL_INDIRECT_DRAW_CONFIG), &World::INITIAL_INDIRECT_DRAW_CONFIG, GL_DYNAMIC_STORAGE_BIT | GL_CLIENT_STORAGE_BIT);

        m_static_rigid_body = r_game_system.getPhysx()->createRigidStatic(physx::PxTransform(physx::PxIdentity));
    }

    void Chunk::releasePhysics()
    {
        m_static_rigid_body->release();
    }
    
    void Chunk::setMeshConfig(int unsigned point_width)
    {
        glNamedBufferData(m_mesh_vb, maxChunkTriangles(point_width) * sizeof(float) * 18, nullptr, GL_DYNAMIC_COPY);
        glNamedBufferData(m_density_distribution_ss, point_width * point_width * point_width * sizeof(float), nullptr, GL_DYNAMIC_COPY);
    }

#define COOK_REALTIME 0

    void Chunk::setMeshCollider(std::vector<float> const & mesh, physx::PxMaterial * material, float chunk_size)
    {
        if (m_has_valid_collider) return;

        removeCollider();

        if (m_vertex_count == 0) return;
#if COOK_REALTIME
        physx::PxTolerancesScale tolerances_scale;
        physx::PxCookingParams params(tolerances_scale);
        params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
        params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
        params.midphaseDesc.mBVH33Desc.meshCookingHint = physx::PxMeshCookingHint::eCOOKING_PERFORMANCE;

        r_game_system.getPhysxCooking()->setParams(params);
#endif
        // Triangle mesh (Might need the realtime version of cooking for terraforming)
        physx::PxTriangleMeshDesc mesh_desc;
        mesh_desc.points.count = m_vertex_count; 
        mesh_desc.points.stride = 2 * sizeof(physx::PxVec3);
        mesh_desc.points.data = mesh.data();
        mesh_desc.triangles.count = m_vertex_count / 3;
        mesh_desc.triangles.stride = 3 * sizeof(physx::PxU32);
        auto sub_indices = std::vector<uint32_t>(s_indices.begin(), s_indices.begin() + m_vertex_count);
        mesh_desc.triangles.data = sub_indices.data();

#if COOK_REALTIME
        physx::PxTriangleMesh * triangle_mesh = r_game_system.getPhysxCooking()->createTriangleMesh(mesh_desc, r_game_system.getPhysx()->getPhysicsInsertionCallback());
#else
        physx::PxDefaultMemoryOutputStream write_buffer;
        physx::PxTriangleMeshCookingResult::Enum result;
        bool status = r_game_system.getPhysxCooking()->cookTriangleMesh(mesh_desc, write_buffer, &result);
        if (!status) ENG_LOG_F("Failed to cook triangle mesh of chunk at (%d, %d, %d)", m_position.x, m_position.y, m_position.z);
        physx::PxDefaultMemoryInputData read_buffer(write_buffer.getData(), write_buffer.getSize());
        physx::PxTriangleMesh * triangle_mesh = r_game_system.getPhysx()->createTriangleMesh(read_buffer);
#endif
        physx::PxMeshScale scale({ chunk_size });
        physx::PxTriangleMeshGeometry geometry(triangle_mesh, scale);

        physx::PxRigidActorExt::createExclusiveShape(*m_static_rigid_body, geometry, *material);
        triangle_mesh->release();
        m_has_valid_collider = true;
    }

    void Chunk::removeCollider()
    {
        m_has_valid_collider = false;
        if (int shape_count = m_static_rigid_body->getNbShapes() > 0)
        {
            std::vector<physx::PxShape *> shapes(shape_count);
            m_static_rigid_body->getShapes(shapes.data(), static_cast<physx::PxU32>(shapes.size() * sizeof(physx::PxShape *)));
            for (auto shape_ptr : shapes) m_static_rigid_body->detachShape(*shape_ptr);
        }
    }

    void Chunk::setMeshInfo(int unsigned vertex_count)
    {
        m_vertex_count = vertex_count;
    }

    void Chunk::activate(glm::ivec3 position, float chunk_size)
    {
        m_position = position;
        m_active = true;
        m_static_rigid_body->setGlobalPose(physx::PxTransform(physx::PxVec3{ static_cast<float>(position.x), static_cast<float>(position.y), static_cast<float>(position.z) } * chunk_size));
    }

    void Chunk::deactivate(Chunk * next_unused)
    {
        m_next_unused = next_unused;
        m_active = false;
        removeCollider();
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

    glm::ivec3 const & Chunk::getPosition() const
    {
        return m_position;
    }
    
    physx::PxRigidStatic * Chunk::getRigidBody() const
    {
        return m_static_rigid_body;
    }
    
    //ChunkPool

    ChunkPool::ChunkPool(GameSystem & game_system) : r_game_system(game_system)
    {
    }

    ChunkPool::~ChunkPool()
    {
        for (auto & chunk : m_chunks)
        {
            chunk.releasePhysics();
        }
    }

    void ChunkPool::initialize(size_t initial_size, int unsigned base_lod_point_width)
    {
        setPoolSize(initial_size);
        m_base_lod_point_width = base_lod_point_width;
        Chunk::generateIndices(3 * maxChunkTriangles(base_lod_point_width));
    }

    void ChunkPool::setPoolSize(size_t size)
    {
        m_chunks.clear();
        m_chunks.reserve(size);
        // Allocate all chunks and setup free list
        for (size_t i = 0; i < size; ++i)
        {
            Chunk & chunk = m_chunks.emplace_back(r_game_system, m_base_lod_point_width);
            if (i > 0) m_chunks[i - 1].deactivate(&chunk);
        }
        m_first_unused = &m_chunks[0];
        m_chunks[size - 1].deactivate(nullptr);
    }

    bool ChunkPool::activateChunk(Chunk *& out_chunk, glm::ivec3 position, float chunk_size)
    {
        if (m_first_unused == nullptr) return false;
        out_chunk = m_first_unused;
        m_first_unused = out_chunk->getNextUnused();
        out_chunk->activate(position, chunk_size);
        return true;
    }

    void ChunkPool::deactivateChunk(Chunk * chunk)
    {
        chunk->deactivate(m_first_unused);
        m_first_unused = chunk;
    }

    bool ChunkPool::getChunkAt(glm::ivec3 const & position, std::vector<Chunk>::iterator & out_chunk)
    {
        auto result = std::find_if(begin(), end(), [&](Chunk chunk)
        {
            return chunk.isActive() && chunk.getPosition() == position;
        });
        if (result != end())
        {
            out_chunk = result;
            return true;
        }
        return false;
    }

    bool ChunkPool::hasChunkAt(glm::ivec3 const & position)
    {
        return std::find_if(begin(), end(), [&](Chunk chunk)
        {
            return chunk.isActive() && chunk.getPosition() == position;
        }) != end();
    }

    int unsigned ChunkPool::getBaseLodPointWidth() const
    {
        return m_base_lod_point_width;
    }
}