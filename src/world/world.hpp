#pragma once

#include <memory>
#include <unordered_map>

#include <glad/glad.h>

#include "event/key_event.hpp"
#include "first_person_camera.hpp"
#include "graphics/asset.hpp"
#include "graphics/atomic_counter_buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/vertex_array.hpp"
#include "graphics/vertex_buffer.hpp"
#include "graphics/uniform_buffer.hpp"
#include "world/chunk.hpp"

namespace eng
{
    struct WorldGenerationConfig
    {
        int m_octaves_2d = 5, m_octaves_3d = 3;
        float m_frequency_2d = 0.05f, m_lacunarity_2d = 1.7f, m_persistence_2d = 1.0f / m_lacunarity_2d, m_amplitude_2d = 1.0f, m_exponent_2d = 4.0f;
        float m_frequency_3d = 0.05f, m_lacunarity_3d = 1.7f, m_persistence_3d = 1.0f / m_lacunarity_3d, m_amplitude_3d = 1.0f, m_exponent_3d = 4.0f;
        float m_water_level{};
    };

    class World
    {
        friend class Chunk;
        friend class Application; //For Debug UI
    private:
        int unsigned static constexpr WORK_GROUP_SIZE = 10;
    private:
        int m_points_per_axis = 8, m_resolution = static_cast<int>(std::ceil(static_cast<float>(m_points_per_axis) / WORK_GROUP_SIZE)), m_max_triangle_count = (m_points_per_axis - 1) * (m_points_per_axis - 1) * (m_points_per_axis - 1) * 4;
        glm::ivec2 m_last_chunk_coords{};
        int m_render_distance = 4;
        float m_threshold{};

        std::shared_ptr<UniformBuffer> m_generation_config;
        std::shared_ptr<Shader> m_density_generator;
        std::shared_ptr<Shader> m_marching_cubes;
        std::shared_ptr<Shader> m_chunk_renderer;
        std::shared_ptr<ShaderStorageBuffer> m_isosurface;
        std::shared_ptr<ShaderStorageBuffer> m_triangulation_table;

        GLuint m_max_chunk_index_buffer;

        ChunkPool m_chunk_pool;

    public:
        ~World();

        void onRendererInit(AssetManager const & asset_manager);

        void onMousePressed(MousePressedEvent const & event);

        void onKeyPressed(KeyPressedEvent const & event);

        void onPlayerMoved(FirstPersonCamera const & camera);

        void initDependentBuffers();
        
        void updateGenerationConfig(WorldGenerationConfig const config);
        
        void setPointsPerAxis(int unsigned point_number);

        void setRenderDistance(int unsigned render_distance);

        void invalidateAllChunks();

        void generateChunks();

        void render(FirstPersonCamera const & camera) const;

        int unsigned getPointsPerAxis();
    };
}