#pragma once

#include <memory>
#include <unordered_map>

#include <glad/glad.h>
#include <PxPhysicsAPI.h>

#include "event/key_event.hpp"
#include "first_person_camera.hpp"
#include "game_system.hpp"
#include "graphics/asset.hpp"
#include "graphics/gpu_synchronizer.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"
#include "player.hpp"
#include "world/chunk.hpp"

namespace eng
{
    struct WorldGenerationConfig
    {
        int m_octaves_2d = 5, m_octaves_3d = 1;
        float m_frequency_2d = 0.05f, m_lacunarity_2d = 1.7f, m_persistence_2d = 1.0f / m_lacunarity_2d, m_amplitude_2d = 10.0f, m_exponent_2d = 4.2f;
        float m_frequency_3d = 0.05f, m_lacunarity_3d = 1.7f, m_persistence_3d = 1.0f / m_lacunarity_3d, m_amplitude_3d = 1.0f, m_exponent_3d = 4.2f;
        float m_water_level{};
    };

    class World
    {
        friend class Chunk;
        friend class Application; //For Debug UI
    private:
        int unsigned static constexpr WORK_GROUP_SIZE = 10, RAY_HIT_DATA_SIZE = 22;
    private:
        int m_points_per_axis = 16, m_resolution = static_cast<int>(std::ceil(static_cast<float>(m_points_per_axis) / WORK_GROUP_SIZE)), m_max_triangle_count = (m_points_per_axis - 1) * (m_points_per_axis - 1) * (m_points_per_axis - 1) * 5;
        glm::ivec3 m_last_chunk_coords{};
        int m_render_distance = 2;
        float m_threshold = 0.1f, m_chunk_size_in_units = 8.0f, m_terraform_strength = 0.24f, m_terraform_radius = 2.4f, m_create_destroy_multiplier = 1.0f;

        GameSystem & r_game_system;

        std::shared_ptr<Shader> m_density_generator;
        std::shared_ptr<Shader> m_marching_cubes;
        std::shared_ptr<Shader> m_chunk_renderer;
        std::shared_ptr<Shader> m_mesh_ray_intersect;
        std::shared_ptr<Shader> m_ray_mesh_command;
        std::shared_ptr<Shader> m_terraform;
        GLuint m_generation_config_u;
        GLuint m_triangulation_table_ss;
        GLuint m_ray_hit_data_ss;
        GLuint m_chunk_va;
        GLuint m_dispatch_indirect_buffer;

        float * m_hit_info_ptr;

        physx::PxMaterial * m_chunk_collider_material;
        ChunkPool m_chunk_pool;

        Player m_player;

        physx::PxScene * m_scene;

    public:
        World(GameSystem & game_system);
        ~World();

        void initDynamicBuffers();

        void castRay(FirstPersonCamera const & camera);
        void chunkRayIntersection(glm::ivec3 const & chunk_coordinate, glm::vec3 const & origin, glm::vec3 const & direction);

        void onKeyPressed(KeyPressedEvent const & event);
        void onPlayerMoved(FirstPersonCamera const & camera);

        void invalidateAllChunks();
        void bindNeighborChunks(int unsigned starting_index, uint8_t neighbor_mask, glm::ivec3 const & chunk_coordinate);
        void generateChunks();
        void terraform(glm::ivec3 const & chunk);

        void update(float delta_time, Window const & window, FirstPersonCamera & camera);
        void render(FirstPersonCamera const & camera);
        
        void updateGenerationConfig(WorldGenerationConfig const & config);
        
        void setRenderDistance(int unsigned render_distance);

        void setPointsPerAxis(int unsigned point_number);
        int unsigned getPointsPerAxis();
    };
}