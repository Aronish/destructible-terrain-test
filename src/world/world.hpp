#pragma once

#include <memory>
#include <unordered_map>

#include <glad/glad.h>

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
        int unsigned m_octaves = 5;
        float m_frequency = 0.08f, m_lacunarity = 1.7f, m_persistence = 1.0f / m_lacunarity;
    };

    class World
    {
        friend class Chunk;
        friend class Application; //For accessing these statics in GUI
    public:
        int unsigned static constexpr WORK_GROUP_SIZE = 8;
    private:
        int unsigned static inline s_resolution = 2, s_points_per_axis = s_resolution * WORK_GROUP_SIZE, s_max_triangle_amount = (s_points_per_axis - 1) * (s_points_per_axis - 1) * (s_points_per_axis - 1) * 4;
    public:
        int m_octaves = 5;
    private:
        int m_render_distance = 4;
        float m_surface_level = -0.1f;

        std::shared_ptr<UniformBuffer> m_uniform_buffer;
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

        void initDependentBuffers();
        
        void updateGenerationConfig(WorldGenerationConfig const config);
        
        void setResolution(int unsigned resolution);

        void setRenderDistance(int unsigned render_distance);

        void invalidateAllChunks();

        void generateChunks(glm::ivec2 const & origin);

        void render(FirstPersonCamera const & camera) const;

        int unsigned static getPointsPerAxis();
    };
}